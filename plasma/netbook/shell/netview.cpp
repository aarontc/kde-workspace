/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "netview.h"
#include "netcorona.h"
#include "netpanelcontroller.h"

#include <QAction>
#include <QCoreApplication>

#include <KWindowSystem>

#include <Plasma/Applet>
#include <Plasma/PopupApplet>
#include <Plasma/Corona>
#include <Plasma/Containment>

NetView::NetView(Plasma::Containment *containment, int uid, QWidget *parent)
    : Plasma::View(containment, uid, parent),
      m_panelController(0)
{
    setFocusPolicy(Qt::NoFocus);
    connectContainment(containment);

    const int w = 25;
    QPixmap tile(w * 2, w * 2);
    tile.fill(palette().base().color());
    QPainter pt(&tile);
    QColor color = palette().mid().color();
    color.setAlphaF(.6);
    pt.fillRect(0, 0, w, w, color);
    pt.fillRect(w, w, w, w, color);
    pt.end();
    QBrush b(tile);
    setBackgroundBrush(tile);
    connect(this, SIGNAL(lostContainment()), SLOT(grabContainment()));
}

NetView::~NetView()
{
}

void NetView::connectContainment(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    connect(containment, SIGNAL(activate()), this, SIGNAL(containmentActivated()));
    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updateGeometry()));
    connect(containment, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
}

void NetView::setContainment(Plasma::Containment *c)
{
    if (containment()) {
        disconnect(containment(), 0, this, 0);
    }

    Plasma::View::setContainment(c);
    connectContainment(c);
    updateGeometry();
}

bool NetView::autoHide() const
{
    return config().readEntry("panelAutoHide", true);
}

void NetView::setAutoHide(bool hide)
{
    if (hide != autoHide()) {
        emit autoHideChanged(hide);
    }
    config().writeEntry("panelAutoHide", hide);
}


// This function is reimplemented from QGraphicsView to work around the problem
// that QPainter::fillRect(QRectF/QRect, QBrush), which QGraphicsView uses, is
// potentially slow when the anti-aliasing hint is set and as implemented won't
// hit accelerated code at all when it isn't set.  This implementation avoids
// the problem by using integer coordinates and by using drawTiledPixmap() in
// the case of a texture brush, and fillRect(QRect, QColor) in the case of a
// solid pattern.  As an additional optimization it draws the background with
// CompositionMode_Source.
void NetView::drawBackground(QPainter *painter, const QRectF &rect)
{
    const QPainter::CompositionMode savedMode = painter->compositionMode();
    const QBrush brush = backgroundBrush();

    if (containment() && (containment()->containmentType() == Plasma::Containment::PanelContainment ||
        containment()->containmentType() == Plasma::Containment::CustomPanelContainment)) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect.toAlignedRect(), Qt::transparent);
        painter->setCompositionMode(savedMode);
        return;
    }

    switch (brush.style())
    {
    case Qt::TexturePattern:
    {
        // Note: this assumes that the brush origin is (0, 0), and that
        //       the brush has an identity transformation matrix.
        const QPixmap texture = brush.texture();
        QRect r = rect.toAlignedRect();
        r.setLeft(r.left() - (r.left() % texture.width()));
        r.setTop(r.top() - (r.top() % texture.height()));
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->drawTiledPixmap(r, texture);
        painter->setCompositionMode(savedMode);
        return;
    }

    case Qt::SolidPattern:
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect.toAlignedRect(), brush.color());
        painter->setCompositionMode(savedMode);
        return;

    default:
        QGraphicsView::drawBackground(painter, rect);
    }
}


void NetView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateGeometry();
    emit geometryChanged();
}

void NetView::screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment)
{
    kDebug() << "was, is, containment:" << wasScreen << isScreen << (QObject*)containment;
    if (containment->containmentType() == Plasma::Containment::PanelContainment) {
        // we don't care about panel containments changing screens on us
        return;
    }

    if (wasScreen == screen() && this->containment() == containment) {
        setContainment(0);
    }

    if (isScreen == screen() && this->containment( )!= containment) {
        setContainment(containment);
    }
}

Plasma::Location NetView::location() const
{
    return containment()->location();
}

Plasma::FormFactor NetView::formFactor() const
{
    return containment()->formFactor();
}

void NetView::updateGeometry()
{
    Plasma::Containment *c = containment();
    if (!c) {
        return;
    }

    kDebug() << "New containment geometry is" << c->geometry();

    switch (c->location()) {
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
        setMinimumWidth(0);
        setMaximumWidth(QWIDGETSIZE_MAX);
        setFixedHeight(c->size().height());
        emit locationChanged(this);
        break;
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        setMinimumHeight(0);
        setMaximumHeight(QWIDGETSIZE_MAX);
        setFixedWidth(c->size().width());
        emit locationChanged(this);
        break;
    //ignore changes in the main view
    default:
        break;
    }

    if (c->size().toSize() != size()) {
        c->setMaximumSize(size());
        c->setMinimumSize(size());
        c->resize(size());
    }
}

void NetView::grabContainment()
{

    NetCorona *corona = qobject_cast<NetCorona*>(scene());
    if (!corona) {
        kDebug() << "no corona :(";
        return;
    }

    Plasma::Containment *cont = corona->findFreeContainment();
    if (cont) {
        setContainment(cont);
    }
}

void NetView::updateConfigurationMode(bool config)
{
    Plasma::Containment *cont = containment();
    if (config && cont && cont->immutability() == Plasma::Mutable &&
        (cont->formFactor() == Plasma::Horizontal || cont->formFactor() == Plasma::Vertical)) {
        m_panelController = new NetPanelController(0, this, cont);
    } else {
        delete m_panelController;
        m_panelController = 0;
    }
}

#include "netview.moc"

