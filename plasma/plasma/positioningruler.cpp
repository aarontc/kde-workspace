 /*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "positioningruler.h"

#include <QPainter>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QToolTip>

#include <KIcon>
#include <KColorUtils>
#include <KWindowSystem>

#include <plasma/theme.h>
#include <plasma/panelsvg.h>
#include <plasma/containment.h>


class PositioningRuler::Private
{
public:
    Private()
       : location(Plasma::BottomEdge),
         alignment(Qt::AlignLeft),
         dragging(NoElement),
         startDragPos(0,0),
         offset(0),
         minLength(0),
         maxLength(0),
         availableLength(0),
         leftMaxSliderRect(QRect(0,0,0,0)),
         rightMaxSliderRect(QRect(0,0,0,0)),
         leftMinSliderRect(QRect(0,0,0,0)),
         rightMinSliderRect(QRect(0,0,0,0)),
         offsetSliderRect(QRect(0,0,0,0)),
         sliderGraphics(0),
         elementPrefix(QString())
    {
    }

    ~Private()
    {
    }

    bool moveSlider(QRect &sliderRect, QRect &symmetricSliderRect, const QPoint &newPos)
    {
        if (location == Plasma::LeftEdge || location == Plasma::RightEdge) {
            if (newPos.y() < 0 || newPos.y() > availableLength) {
                return false;
            }

            if (alignment == Qt::AlignCenter) {
                const int newTop = offsetSliderRect.center().y() + (offsetSliderRect.center().y() - newPos.y());
                if (newTop < 0 || newTop > availableLength) {
                    return false;
                }
                symmetricSliderRect.moveCenter(QPoint(symmetricSliderRect.center().x(), newTop));
            }
            sliderRect.moveCenter(QPoint(sliderRect.center().x(), newPos.y()));
        } else {
            if (newPos.x() < 0 || newPos.x() > availableLength) {
                return false;
            }

            if (alignment == Qt::AlignCenter) {
                const int newLeft = offsetSliderRect.center().x() + (offsetSliderRect.center().x() - newPos.x());
                if (newLeft < 0 || newLeft > availableLength) {
                    return false;
                }
                symmetricSliderRect.moveCenter(QPoint(newLeft, symmetricSliderRect.center().y()));
            }
            sliderRect.moveCenter(QPoint(newPos.x(), sliderRect.center().y()));
        }

        return true;
    }

    int sliderRectToLength(const QRect &sliderRect)
    {
        int sliderPos;
        int offsetPos;

        if (location == Plasma::LeftEdge || location == Plasma::RightEdge) {
            sliderPos = sliderRect.center().y();
            offsetPos = offsetSliderRect.center().y();
        } else {
            sliderPos = sliderRect.center().x();
            offsetPos = offsetSliderRect.center().x();
        }

        if (alignment == Qt::AlignCenter) {
            return 2 * qAbs(sliderPos - offsetPos);
        } else {
            return qAbs(sliderPos - offsetPos);
        }
    }

    void loadSlidersGraphics()
    {
        QString elementPrefix;

        switch (location) {
        case Plasma::LeftEdge:
            elementPrefix = "west-";
            break;
        case Plasma::RightEdge:
            elementPrefix = "east-";
            break;
        case Plasma::TopEdge:
            elementPrefix = "north-";
            break;
        case Plasma::BottomEdge:
        default:
            elementPrefix = "south-";
            break;
        }

        leftMaxSliderRect.setSize(sliderGraphics->elementSize(elementPrefix + "maxslider"));
        rightMaxSliderRect.setSize(leftMaxSliderRect.size());
        leftMinSliderRect.setSize(sliderGraphics->elementSize(elementPrefix + "minslider"));
        rightMinSliderRect.setSize(leftMinSliderRect.size());
        offsetSliderRect.setSize(sliderGraphics->elementSize(elementPrefix + "offsetslider"));
    }

    void setupSliders(const QSize &totalSize)
    {
        //FIXME: the following is waay too clumsy...

        int rightMaxPos;
        int leftMaxPos;
        int rightMinPos;
        int leftMinPos;
        int offsetPos;

        int totalLength;
        if (location == Plasma::LeftEdge || location == Plasma::RightEdge) {
            totalLength = totalSize.height();
        } else {
            totalLength = totalSize.width();
        }

        switch (alignment) {
        case Qt::AlignLeft:
            //Here substracting one to everything because QRect.moveCenter(pos) moves the rect with
            //the width/2 th pixel at pos.x (and so for y) resulting in the painted image moved
            //one pixel to the right
            rightMaxPos = offset + maxLength - 1;
            leftMaxPos = 0;
            rightMinPos = offset + minLength - 1;
            leftMinPos = 0;
            offsetPos = offset - 1;
            break;
        case Qt::AlignRight:
            leftMaxPos = totalLength - offset - maxLength - 1;
            rightMaxPos = 0;
            leftMinPos = totalLength - offset - minLength - 1;
            rightMinPos = 0;
            offsetPos = totalLength - offset - 1;
            break;
        case Qt::AlignCenter:
        default:
            leftMaxPos = totalLength/2 + offset - maxLength/2 - 1;
            rightMaxPos = totalLength/2 + offset + maxLength/2 - 1;

            leftMinPos = totalLength/2 + offset - minLength/2 - 1;
            rightMinPos = totalLength/2 + offset + minLength/2 - 1;

            offsetPos = totalLength/2 + offset - 1;
            break;
        }
    
        switch (location) {
        case Plasma::LeftEdge:
            leftMaxSliderRect.moveCenter(QPoint(3*(totalSize.width()/4), leftMaxPos));
            rightMaxSliderRect.moveCenter(QPoint(3*(totalSize.width()/4), rightMaxPos));
        
            leftMinSliderRect.moveCenter(QPoint(totalSize.width()/4, leftMinPos));
            rightMinSliderRect.moveCenter(QPoint(totalSize.width()/4, rightMinPos));
        
            offsetSliderRect.moveCenter(QPoint(3*(totalSize.width()/4), offsetPos));
            break;
        case Plasma::RightEdge:
            leftMaxSliderRect.moveCenter(QPoint(totalSize.width()/4, leftMaxPos));
            rightMaxSliderRect.moveCenter(QPoint(totalSize.width()/4, rightMaxPos));
        
            leftMinSliderRect.moveCenter(QPoint(3*(totalSize.width()/4), leftMinPos));
            rightMinSliderRect.moveCenter(QPoint(3*(totalSize.width()/4), rightMinPos));
        
            offsetSliderRect.moveCenter(QPoint(totalSize.width()/4, offsetPos));
            break;
        case Plasma::TopEdge:
            leftMaxSliderRect.moveCenter(QPoint(leftMaxPos, 3*(totalSize.height()/4)));
            rightMaxSliderRect.moveCenter(QPoint(rightMaxPos, 3*(totalSize.height()/4)));
        
            leftMinSliderRect.moveCenter(QPoint(leftMinPos, totalSize.height()/4));
            rightMinSliderRect.moveCenter(QPoint(rightMinPos, totalSize.height()/4));
        
            offsetSliderRect.moveCenter(QPoint(offsetPos, 3*(totalSize.height()/4)));
            break;
        case Plasma::BottomEdge:
        default:
            leftMaxSliderRect.moveCenter(QPoint(leftMaxPos, totalSize.height()/4));
            rightMaxSliderRect.moveCenter(QPoint(rightMaxPos, totalSize.height()/4));
        
            leftMinSliderRect.moveCenter(QPoint(leftMinPos, 3*(totalSize.height()/4)));
            rightMinSliderRect.moveCenter(QPoint(rightMinPos, 3*(totalSize.height()/4)));
        
            offsetSliderRect.moveCenter(QPoint(offsetPos, totalSize.height()/4));
            break;
        }
    }

    enum SubElement { NoElement = 0,
                       LeftMaxSlider,
                       RightMaxSlider,
                       LeftMinSlider,
                       RightMinSlider,
                       OffsetSlider
                     };

    Plasma::Location location;
    Qt::Alignment alignment;
    SubElement dragging;
    QPoint startDragPos;
    int offset;
    int minLength;
    int maxLength;
    int availableLength;
    QRect leftMaxSliderRect;
    QRect rightMaxSliderRect;
    QRect leftMinSliderRect;
    QRect rightMinSliderRect;
    QRect offsetSliderRect;
    Plasma::PanelSvg *sliderGraphics;
    QString elementPrefix;
};

PositioningRuler::PositioningRuler(QWidget* parent)
   : QWidget(parent),
     d(new Private())
{
   d->sliderGraphics = new Plasma::PanelSvg(this);
   d->sliderGraphics->setImagePath("widgets/containment-controls");

   d->loadSlidersGraphics();
}

PositioningRuler::~PositioningRuler()
{
   delete d;
}

QSize PositioningRuler::sizeHint() const
{
    //FIXME:why must add 6 pixels????
    
    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        return QSize(d->leftMaxSliderRect.width() + d->leftMinSliderRect.width() + 6, d->availableLength);
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        return QSize(d->availableLength, d->leftMaxSliderRect.height() + d->leftMinSliderRect.height() + 6);
        break;
    }
    
}

void PositioningRuler::setLocation(const Plasma::Location &loc)
{
    if (d->location == loc) {
        return;
    }

    d->location = loc;

    d->setupSliders(size());
    d->loadSlidersGraphics();

    update();
}

Plasma::Location PositioningRuler::location() const
{
    return d->location;
}

void PositioningRuler::setAlignment(const Qt::Alignment &align)
{
    if (d->alignment == align) {
        return;
    }

    d->alignment = align;

    d->setupSliders(size());

    update();
}

Qt::Alignment PositioningRuler::alignment() const
{
    return d->alignment;
}

void PositioningRuler::setOffset(int newOffset)
{
    d->offset = newOffset;

    d->setupSliders(size());
}

int PositioningRuler::offset() const
{
    return d->offset;
}

void PositioningRuler::setMaxLength(int newMax)
{
    int deltaX;
    int deltaY;

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        deltaX = 0;
        deltaY = newMax - d->maxLength;
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        deltaX = newMax - d->maxLength;
        deltaY = 0;
        break;
    }

    if (d->alignment == Qt::AlignLeft) {
        d->rightMaxSliderRect.moveCenter(QPoint(d->rightMaxSliderRect.center().x() + deltaX,
                                         d->rightMaxSliderRect.center().y() + deltaY));
    } else if (d->alignment == Qt::AlignRight) {
        d->leftMaxSliderRect.moveCenter(QPoint(d->leftMaxSliderRect.center().x() - deltaX,
                                        d->leftMaxSliderRect.center().y() - deltaY));
    //center
    } else {
        d->rightMaxSliderRect.moveCenter(QPoint(d->rightMaxSliderRect.center().x() + deltaX/2,
                                         d->rightMaxSliderRect.center().y() + deltaY/2));
        d->leftMaxSliderRect.moveCenter(QPoint(d->leftMaxSliderRect.center().x() - deltaX/2,
                                        d->leftMaxSliderRect.center().y() - deltaY/2));
    }

    d->maxLength = newMax;
    if (d->minLength > d->maxLength) {
        setMinLength(newMax);
    }
}

int PositioningRuler::maxLength() const
{
    return d->maxLength;
}

void PositioningRuler::setMinLength(int newMin)
{
    int deltaX;
    int deltaY;

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        deltaX = 0;
        deltaY = newMin - d->minLength;
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        deltaX = newMin - d->minLength;
        deltaY = 0;
        break;
    }


    if (d->alignment == Qt::AlignLeft) {
        d->rightMinSliderRect.moveCenter(QPoint(d->rightMinSliderRect.center().x() + deltaX,
                                         d->rightMinSliderRect.center().y() + deltaY));
    } else if (d->alignment == Qt::AlignRight) {
        d->leftMinSliderRect.moveCenter(QPoint(d->leftMinSliderRect.center().x() - deltaX,
                                        d->leftMinSliderRect.center().y() - deltaY));
    //center
    } else {
        d->rightMinSliderRect.moveCenter(QPoint(d->rightMinSliderRect.center().x() + deltaX/2,
                                         d->rightMinSliderRect.center().y() + deltaY/2));
        d->leftMinSliderRect.moveCenter(QPoint(d->leftMinSliderRect.center().x() - deltaX/2,
                                        d->leftMinSliderRect.center().y() - deltaY/2));
    }

    d->minLength = newMin;
    if (d->minLength > d->minLength) {
        setMaxLength(newMin);
    }
}

int PositioningRuler::minLength() const
{
    return d->minLength;
}

void PositioningRuler::setAvailableLength(int newAvail)
{
    d->availableLength = newAvail;
    
    if (d->maxLength > newAvail) {
        setMaxLength(newAvail);
    }

    if (d->minLength > newAvail) {
        setMinLength(newAvail);
    }
}

int PositioningRuler::availableLength() const
{
    return d->availableLength;
}

bool PositioningRuler::event(QEvent *ev)
{
    //Show a different tooltip on each slider
    if (ev->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(ev);
        if (d->offsetSliderRect.contains(helpEvent->pos())) {
            QToolTip::showText(helpEvent->globalPos(), i18n("Move this slider to set the panel position"), this);
        } else if ((d->alignment != Qt::AlignLeft && d->leftMaxSliderRect.contains(helpEvent->pos())) ||
                   (d->alignment != Qt::AlignRight && d->rightMaxSliderRect.contains(helpEvent->pos()))) {
            QToolTip::showText(helpEvent->globalPos(), i18n("Move this slider to set the maximum panel size"), this);
        } else if ((d->alignment != Qt::AlignLeft && d->leftMinSliderRect.contains(helpEvent->pos())) ||
                   (d->alignment != Qt::AlignRight && d->rightMinSliderRect.contains(helpEvent->pos()))) {
            QToolTip::showText(helpEvent->globalPos(), i18n("Move this slider to set the minimum panel size"), this);
        }

        return true;
    } else {
        return QWidget::event(ev);
    }
}

void PositioningRuler::paintEvent(QPaintEvent *event)
{
    //Draw background
    d->sliderGraphics->setElementPrefix(d->location);

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    d->sliderGraphics->resizePanel(event->rect().size());
    d->sliderGraphics->paintPanel(&painter, event->rect());
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    //Draw center indicators
    if (d->alignment == Qt::AlignCenter && (d->location == Plasma::LeftEdge || d->location == Plasma::RightEdge)) {
        d->sliderGraphics->paint(&painter, QPoint(event->rect().left(), event->rect().center().y()), "vertical-centerindicator");
    } else if (d->alignment == Qt::AlignCenter) {
        d->sliderGraphics->paint(&painter, QPoint(event->rect().center().x(), event->rect().top()), "horizontal-centerindicator");
    }

    //Draw handles
    QString elementPrefix;

    switch (d->location) {
    case Plasma::LeftEdge:
        elementPrefix = "west-";
        break;
    case Plasma::RightEdge:
        elementPrefix = "east-";
        break;
    case Plasma::TopEdge:
        elementPrefix = "north-";
        break;
    case Plasma::BottomEdge:
    default:
        elementPrefix = "south-";
        break;
    }

    if (d->alignment != Qt::AlignLeft) {
        d->sliderGraphics->paint(&painter, d->leftMaxSliderRect, elementPrefix + "maxslider");
        d->sliderGraphics->paint(&painter, d->leftMinSliderRect, elementPrefix + "minslider");
    }

    if (d->alignment != Qt::AlignRight) {
        d->sliderGraphics->paint(&painter, d->rightMaxSliderRect, elementPrefix + "maxslider");
        d->sliderGraphics->paint(&painter, d->rightMinSliderRect, elementPrefix + "minslider");
    }

    d->sliderGraphics->paint(&painter, d->offsetSliderRect, elementPrefix + "offsetslider");
}

void PositioningRuler::wheelEvent(QWheelEvent *event)
{
    QPoint movement;
    int hMargins = 0;
    int vMargins = 0;

    if (d->location == Plasma::LeftEdge || d->location == Plasma::RightEdge) {
        vMargins = 200;
        if (event->delta() < 0) {
            movement = QPoint(0, 20);
        } else {
            movement = QPoint(0, -20);
        }
    } else {
        hMargins = 100;
        if (event->delta() < 0) {
            movement = QPoint(20, 0);
        } else {
            movement = QPoint(-20, 0);
        }
    }

    if (d->alignment != Qt::AlignLeft && d->leftMaxSliderRect.adjusted(-hMargins, -vMargins, hMargins, vMargins).contains(event->pos())) {
        d->dragging = Private::LeftMaxSlider;
        movement += d->leftMaxSliderRect.center();
    } else if (d->alignment != Qt::AlignRight && d->rightMaxSliderRect.adjusted(-hMargins, -vMargins, hMargins, vMargins).contains(event->pos())) {
        d->dragging = Private::RightMaxSlider;
        movement += d->rightMaxSliderRect.center();
    } else if (d->alignment != Qt::AlignLeft && d->leftMinSliderRect.adjusted(-hMargins, -vMargins, hMargins, vMargins).contains(event->pos())) {
        d->dragging = Private::LeftMinSlider;
        movement += d->leftMinSliderRect.center();
    } else if (d->alignment != Qt::AlignRight && d->rightMinSliderRect.adjusted(-hMargins, -vMargins, hMargins, vMargins).contains(event->pos())) {
        d->dragging = Private::RightMinSlider;
        movement += d->rightMinSliderRect.center();
    } else if (d->offsetSliderRect.adjusted(-hMargins, -vMargins, hMargins, vMargins).contains(event->pos())) {
        d->dragging = Private::OffsetSlider;
        movement += d->offsetSliderRect.center();
    } else {
        d->dragging = Private::NoElement;
    }

    //do a fake mouse move event that drags the slider around
    if (d->dragging != Private::NoElement) {
        d->startDragPos = QPoint(0,0);
        QMouseEvent mouseEvent(QEvent::MouseMove, movement, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mouseMoveEvent(&mouseEvent);
        d->dragging = Private::NoElement;
    }
}

void PositioningRuler::mousePressEvent(QMouseEvent *event)
{
    if (d->alignment != Qt::AlignLeft && d->leftMaxSliderRect.contains(event->pos())) {
        d->dragging = Private::LeftMaxSlider;
        d->startDragPos = QPoint(event->pos().x() - d->leftMaxSliderRect.center().x(), event->pos().y() - d->leftMaxSliderRect.center().y());
    } else if (d->alignment != Qt::AlignRight && d->rightMaxSliderRect.contains(event->pos())) {
        d->dragging = Private::RightMaxSlider;
        d->startDragPos = QPoint(event->pos().x() - d->rightMaxSliderRect.center().x(), event->pos().y() - d->rightMaxSliderRect.center().y());
    } else if (d->alignment != Qt::AlignLeft && d->leftMinSliderRect.contains(event->pos())) {
        d->dragging = Private::LeftMinSlider;
        d->startDragPos = QPoint(event->pos().x() - d->leftMinSliderRect.center().x(), event->pos().y() - d->leftMinSliderRect.center().y());
    } else if (d->alignment != Qt::AlignRight && d->rightMinSliderRect.contains(event->pos())) {
        d->dragging = Private::RightMinSlider;
        d->startDragPos = QPoint(event->pos().x() - d->rightMinSliderRect.center().x(), event->pos().y() - d->rightMinSliderRect.center().y());
    } else if (d->offsetSliderRect.contains(event->pos())) {
        d->dragging = Private::OffsetSlider;
        d->startDragPos = QPoint(event->pos().x() - d->offsetSliderRect.center().x(), event->pos().y() - d->offsetSliderRect.center().y());
    } else {
        d->dragging = Private::NoElement;
    }

    QWidget::mousePressEvent(event);
}

void PositioningRuler::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    d->startDragPos = QPoint(0, 0);
    d->dragging = Private::NoElement;
}

void PositioningRuler::mouseMoveEvent(QMouseEvent *event)
{
    if (d->dragging == Private::NoElement) {
        return;
    }

    //bound to width, height
    QPoint newPos = QPoint(qMin(event->pos().x() - d->startDragPos.x(), width()),
                           qMin(event->pos().y() - d->startDragPos.y(), height()));
    //bound to 0,0
    newPos = QPoint(qMax(newPos.x(), 0), qMax(newPos.y(), 0));


    const bool horizontal = (d->location == Plasma::TopEdge || d->location == Plasma::BottomEdge);
    const int widthBound = 20;
    const int snapSize = 5;

    //Snap
    if (horizontal) {
        if (newPos.x() <= snapSize) {
            newPos.setX(0);
        } else if (qAbs(newPos.x() - d->availableLength/2) <= snapSize) {
            newPos.setX(d->availableLength/2);
        } else if (d->availableLength - newPos.x() <= snapSize) {
            newPos.setX(d->availableLength);
        }
    } else {
        if (newPos.y() <= snapSize) {
            newPos.setY(0);
        } else if (qAbs(newPos.y() - d->availableLength/2) <= snapSize) {
            newPos.setY(d->availableLength/2);
        } else if (d->availableLength - newPos.y() <= snapSize) {
            newPos.setY(d->availableLength);
        }
    }
    
    switch (d->dragging) {
    case Private::LeftMaxSlider:
        //don't let the slider "cross" with the offset slider
        if (horizontal && newPos.x() > d->offsetSliderRect.left() - widthBound) {
            return;
        } else if (!horizontal && newPos.y() > d->offsetSliderRect.top() - widthBound) {
            return;
        }

        if (!d->moveSlider(d->leftMaxSliderRect, d->rightMaxSliderRect, newPos)) {
            return;
        }

        d->maxLength = d->sliderRectToLength(d->leftMaxSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->leftMinSliderRect, d->rightMinSliderRect, newPos);
            d->minLength = d->maxLength;
        }
        break;
    case Private::RightMaxSlider:
        if (horizontal && newPos.x() < d->offsetSliderRect.left() + widthBound) {
            return;
        } else if (!horizontal && newPos.y() < d->offsetSliderRect.top() + widthBound) {
            return;
        }

        if (!d->moveSlider(d->rightMaxSliderRect, d->leftMaxSliderRect, newPos)) {
            return;
        }

        d->maxLength = d->sliderRectToLength(d->rightMaxSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->rightMinSliderRect, d->leftMinSliderRect, newPos);
            d->minLength = d->maxLength;
        }
        break;
    case Private::LeftMinSlider:
        if (horizontal && newPos.x() > d->offsetSliderRect.left() - widthBound) {
            return;
        } else if (!horizontal && newPos.y() > d->offsetSliderRect.top() - widthBound) {
            return;
        }

        if (!d->moveSlider(d->leftMinSliderRect, d->rightMinSliderRect, newPos)) {
            return;
        }

        d->minLength = d->sliderRectToLength(d->leftMinSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->leftMaxSliderRect, d->rightMaxSliderRect, newPos);
            d->maxLength = d->minLength;
        }
        break;
    case Private::RightMinSlider:
        if (horizontal && newPos.x() < d->offsetSliderRect.left() + widthBound) {
            return;
        } else if (!horizontal && newPos.y() < d->offsetSliderRect.top() + widthBound) {
            return;
        }

        if (!d->moveSlider(d->rightMinSliderRect, d->leftMinSliderRect, newPos)) {
            return;
        }

        d->minLength = d->sliderRectToLength(d->rightMinSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->rightMaxSliderRect, d->leftMaxSliderRect, newPos);
            d->maxLength = d->minLength;
        }
        break;
    case Private::OffsetSlider:
    {
        if (d->location == Plasma::LeftEdge || d->location == Plasma::RightEdge) {
            d->offsetSliderRect.moveCenter(QPoint(d->offsetSliderRect.center().x(), newPos.y()));
            d->offset = d->offsetSliderRect.center().y();
        } else {
            d->offsetSliderRect.moveCenter(QPoint(newPos.x(), d->offsetSliderRect.center().y()));
            d->offset = d->offsetSliderRect.center().x();
        }

        if (d->alignment == Qt::AlignCenter) {
            d->offset -= d->availableLength/2;
        } else if (d->alignment == Qt::AlignRight) {
            d->offset = d->availableLength - d->offset;
        }

        int centerFactor = 1;
        if (d->alignment == Qt::AlignCenter) {
            centerFactor = 2;
        }

        d->maxLength = centerFactor*qMin(d->maxLength/centerFactor, d->availableLength/centerFactor - qAbs(d->offset));
        d->minLength = centerFactor*qMin(d->minLength/centerFactor, d->availableLength/centerFactor - qAbs(d->offset));

        if (d->location == Plasma::LeftEdge || d->location == Plasma::RightEdge) {
             d->leftMaxSliderRect.moveTop(d->offsetSliderRect.top() - d->maxLength/centerFactor);
             d->leftMinSliderRect.moveTop(d->offsetSliderRect.top() - d->minLength/centerFactor);
             d->rightMaxSliderRect.moveTop(d->offsetSliderRect.top() + d->maxLength/centerFactor);
             d->rightMinSliderRect.moveTop(d->offsetSliderRect.top() + d->minLength/centerFactor);
        } else {
             d->leftMaxSliderRect.moveLeft(d->offsetSliderRect.left() - d->maxLength/centerFactor);
             d->leftMinSliderRect.moveLeft(d->offsetSliderRect.left() - d->minLength/centerFactor);
             d->rightMaxSliderRect.moveLeft(d->offsetSliderRect.left() + d->maxLength/centerFactor);
             d->rightMinSliderRect.moveLeft(d->offsetSliderRect.left() + d->minLength/centerFactor);
        }

        break;
    }
    default:
        break;
    }

    emit rulersMoved(d->offset, d->minLength, d->maxLength);
    update();
}

void PositioningRuler::resizeEvent(QResizeEvent *event)
{
    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        setAvailableLength(event->size().height());
        break;
    default:
        setAvailableLength(event->size().width());
        break;
    }

    d->setupSliders(event->size());

    event->accept();
}

#include "positioningruler.moc"
