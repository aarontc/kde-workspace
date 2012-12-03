/*
  This file is part of the KDE project.

  Copyright (c) 2011 Lionel Chauvin <megabigbug@yahoo.fr>
  Copyright (c) 2011,2012 Cédric Bellegarde <gnumdk@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "menuwidget.h"

#include <QMenu>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>

#include <KWindowSystem>
#include <KDebug>
#include <KApplication>

MenuWidget::MenuWidget(QGraphicsView *view, QMenu *menu) :
    QGraphicsWidget(),
    m_mouseTimer(new QTimer(this)),
    m_view(view),
    m_layout(new QGraphicsLinearLayout(this)),
    m_currentButton(0),
    m_contentBottomMargin(0),
    m_visibleMenu(0),
    m_menu(menu)
{
    connect(m_mouseTimer, SIGNAL(timeout()), SLOT(slotCheckActiveItem()));
    m_menu->installEventFilter(this);
}

MenuWidget::~MenuWidget()
{
    delete m_mouseTimer;
    while (!m_buttons.isEmpty()) {
        delete m_buttons.front();
        m_buttons.pop_front();
    }
}

void MenuWidget::initLayout()
{
    MenuButton* button = 0;
    foreach (QAction* action, m_menu->actions())
    {
        action->setShortcut(QKeySequence());
        if( action->isSeparator() || !action->menu())
            continue;

        //Create a new button, we do not set menu here as it may have changed on showMenu()
        button = new MenuButton(this);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
        button->setText(action->text());
        connect(button, SIGNAL(clicked()), SLOT(slotButtonClicked()));
        m_layout->addItem(button);
        m_buttons << button;
    }

    //Assume all buttons have same margins
    if (button) {
        m_contentBottomMargin = button->bottomMargin();
    }
}


// Update menubar layout
void MenuWidget::updateLayout(QMenu *menu)
{
    // Revalidate menu
    int i = 0;
    bool menu_ok = m_buttons.length() == menu->actions().length();
    foreach (MenuButton *button, m_buttons) {
        if (button->text() != menu->actions()[i]->text()) {
            menu_ok = false;
        }
        ++i;
    }

    m_menu = menu;

    if (!menu_ok) { // Rebuild complete layout
        bool mouseTimer = false;

        if (m_mouseTimer->isActive()) {
        m_mouseTimer->stop();
        mouseTimer = true;
        }

        m_menu->removeEventFilter(this);
        m_menu = 0;
        m_currentButton = 0;

        //Clear layout
        foreach (MenuButton *button, m_buttons) {
            disconnect(button, SIGNAL(clicked()), this, SLOT(slotButtonClicked()));
            m_layout->removeItem(button);
            button->hide();
            m_buttons.removeOne(button);
            button->deleteLater();
        }

        m_menu = menu;
        m_menu->installEventFilter(this);
        initLayout();
        m_layout->invalidate();
        if (mouseTimer) {
            m_mouseTimer->start();
        }
    }
}

bool MenuWidget::eventFilter(QObject* object, QEvent* event)
{
    QMenu *menu = static_cast<QMenu*>(object);

    if (event->type() == QEvent::KeyPress) {
        menu->removeEventFilter(this);
        QApplication::sendEvent(menu, event);
        menu->installEventFilter(this);
        if (!event->isAccepted()) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
            case Qt::Key_Left:
                showLeftRightMenu(false);
                break;
            case Qt::Key_Right:
                showLeftRightMenu(true);
                break;
            case Qt::Key_Escape:
                menu->hide();
                break;
            default:
                break;
            }
        }
        return true;
    }
    return false;
}

void MenuWidget::slotCheckActiveItem()
{
    MenuButton* buttonBelow = 0;
    QPoint pos =  m_view->mapFromGlobal(QCursor::pos());
    QGraphicsItem* item = m_view->itemAt(pos);

    if (item)
        buttonBelow = qobject_cast<MenuButton*>(item->toGraphicsObject());

    if (!buttonBelow)
        return;

    if (buttonBelow != m_currentButton) {
        if (m_currentButton) {
            m_currentButton->nativeWidget()->setDown(false);
            m_currentButton->setHovered(false);
            m_currentButton = buttonBelow;
            m_currentButton->nativeWidget()->setDown(true);
            m_visibleMenu = showMenu();
        }
    }
}

void MenuWidget::slotMenuAboutToHide()
{
    if (m_currentButton) {
        m_currentButton->setDown(false);
    }
    if (m_mouseTimer->isActive())
        m_mouseTimer->stop();
    m_visibleMenu = 0;
    emit aboutToHide();
}

void MenuWidget::slotButtonClicked()
{
    m_currentButton = qobject_cast<MenuButton*>(sender());

    m_currentButton->nativeWidget()->setDown(true);
    m_visibleMenu = showMenu();
    // Start auto navigation after click
    if (!m_mouseTimer->isActive())
        m_mouseTimer->start(100);
}

void MenuWidget::setActiveAction(QAction *action)
{
    m_currentButton = m_buttons.first();

    if (action) {
        QMenu *menu;
        int i = 0;
        foreach (MenuButton *button, m_buttons) {
            menu = m_menu->actions()[i]->menu();
            if (menu && menu == action->menu()) {
                m_currentButton = button;
                break;
            }
        }
    }
    m_currentButton->nativeWidget()->animateClick();
}

void MenuWidget::hide()
{
    if (m_mouseTimer->isActive())
        m_mouseTimer->stop();
    QGraphicsWidget::hide();
}

QMenu* MenuWidget::showMenu()
{
    if (m_visibleMenu) {
        disconnect(m_visibleMenu, SIGNAL(aboutToHide()), this, SLOT(slotMenuAboutToHide()));
        m_visibleMenu->hide();
    }

    QMenu *menu = 0;
    // Search for submenu
    foreach (QAction *action, m_menu->actions()) {
        if (action->text() == m_currentButton->text()) {
            menu = action->menu();
            break;
        }
    }

    if (menu) {
        QPoint globalPos = m_view->mapToGlobal(QPoint(0,0));
        QPointF parentPos =  m_currentButton->mapFromParent(QPoint(0,0));
        QRect screen = KApplication::desktop()->screenGeometry();
        int x = globalPos.x() - parentPos.x();
        int y = globalPos.y() + m_currentButton->size().height() - parentPos.y();

        menu->popup(QPoint(x, y));

        // Fix offscreen menu
        if (menu->size().height() + y > screen.height() + screen.y()) {
            y = globalPos.y() - parentPos.y() - menu->size().height();
            if (menu->size().width() + x > screen.width() + screen.x())
                x = screen.width() + screen.x() - menu->size().width();
            else if (menu->size().width() + x < screen.x())
                x = screen.x();
            menu->move(x, y);
        }

        connect(menu, SIGNAL(aboutToHide()), this, SLOT(slotMenuAboutToHide()));

        installEventFilterForAll(menu, this);
    }
    return menu;
}

void MenuWidget::showLeftRightMenu(bool next)
{
    int index = m_buttons.indexOf(m_currentButton);
    if (index == -1) {
        kWarning() << "Couldn't find button!";
        return;
    }
    if (next) {
        index = (index + 1) % m_buttons.count();
    } else {
        index = (index == 0 ? m_buttons.count() : index) - 1;
    }

    m_currentButton->setDown(false);
    m_currentButton = m_buttons.at(index);
    m_currentButton->nativeWidget()->setDown(true);
    m_visibleMenu = showMenu();
}

void MenuWidget::installEventFilterForAll(QMenu *menu, QObject *object)
{
    menu->installEventFilter(this);

    foreach (QAction *action, menu->actions()) {
        if (action->menu())
            installEventFilterForAll(action->menu(), object);
    }
}

#include "menuwidget.moc"