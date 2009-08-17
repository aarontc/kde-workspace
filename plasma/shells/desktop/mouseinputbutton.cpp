/*
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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

#include "mouseinputbutton.h"

#include <KDebug>

#include <QKeyEvent>

#include <plasma/contextaction.h>

MouseInputButton::MouseInputButton(QWidget *parent)
    :QPushButton(parent)
{
    setCheckable(true);
    connect(this, SIGNAL(clicked()), SLOT(getTrigger()));

    //translations
    //FIXME 'left' may be wrong if mousebuttons are swapped?
    m_prettyStrings.insert("LeftButton", i18n("Left-Button"));
    m_prettyStrings.insert("RightButton", i18n("Right-Button"));
    m_prettyStrings.insert("MidButton", i18n("Middle-Button"));
    //FIXME XButton1/2?
    m_prettyStrings.insert("wheel:Vertical", i18n("Vertical-Scroll"));
    m_prettyStrings.insert("wheel:Horizontal", i18n("Horizontal-Scroll"));

    //FIXME I bet these are wrong for Macs
    m_prettyStrings.insert("ShiftModifier", i18n("Shift"));
    m_prettyStrings.insert("ControlModifier", i18n("Ctrl"));
    m_prettyStrings.insert("AltModifier", i18n("Alt"));
    m_prettyStrings.insert("MetaModifier", i18n("Meta"));
    //FIXME keypad/groupswitch?
}

QString MouseInputButton::trigger()
{
    return m_trigger;
}

void MouseInputButton::getTrigger()
{
    setText(i18n("Input here..."));
}

bool MouseInputButton::event(QEvent *event)
{
    if (isChecked()) {
        //got a trigger or cancel
        //I'm intentionally not using break here
        switch (event->type()) {
            case QEvent::Wheel:
                //kDebug() << "wheel@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
            case QEvent::MouseButtonRelease:
                changeTrigger(Plasma::ContextAction::eventToString(event));
            case QEvent::MouseButtonPress:
                event->accept();
                return true;
            case QEvent::KeyPress:
                if ((dynamic_cast<QKeyEvent*>(event))->key() == Qt::Key_Escape) {
                    //cancel
                    setTrigger(m_trigger);
                    event->accept();
                    return true;
                }
            default:
                break;
        }
    }
    return QPushButton::event(event);
}

void MouseInputButton::changeTrigger(const QString &newTrigger)
{
    QString oldTrigger = m_trigger;
    setTrigger(newTrigger);

    emit triggerChanged(oldTrigger, newTrigger);
}

void MouseInputButton::setTrigger(const QString &trigger)
{
    m_trigger=trigger;
    setChecked(false);

    if (trigger.isEmpty()) {
        setText(i18n("No Button"));
    } else {
        //make it prettier and translatable
        QString button = trigger.section(';', 0, 0);
        QStringList modifiers = trigger.section(';', 1, 1).split('|');

        QString pretty;
        foreach (const QString &mod, modifiers) {
            if (mod == "NoModifier") {
                break;
            }
            pretty += m_prettyStrings.value(mod) + "+";
        }
        pretty += m_prettyStrings.value(button);

        setText(pretty);
    }
}


// vim: sw=4 sts=4 et tw=100
