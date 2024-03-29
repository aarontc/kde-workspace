/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "triggers/triggers.h"
#include "action_data/action_data.h"
#include "windows_handler.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KGlobalAccel>

#include <QAction>

#include "shortcuts_handler.h"

namespace KHotKeys {

ShortcutTriggerVisitor::~ShortcutTriggerVisitor()
    {}


ShortcutTrigger::ShortcutTrigger(
        ActionData* data_P,
        const KShortcut& shortcut,
        const QUuid &uuid )
    :   Trigger( data_P ),
        _uuid(uuid),
        _active(false),
        _shortcut(shortcut)
    {
    }


ShortcutTrigger::~ShortcutTrigger()
    {
    keyboard_handler->removeAction( _uuid.toString() );
    }


void ShortcutTrigger::accept(TriggerVisitor& visitor)
    {
    if (ShortcutTriggerVisitor *v = dynamic_cast<ShortcutTriggerVisitor*>(&visitor))
        {
        v->visit(*this);
        }
    else
        {
        kDebug() << "Visitor error";
        }
    }


void ShortcutTrigger::aboutToBeErased()
    {
    disable();
    }


void ShortcutTrigger::activate( bool newState )
    {
#ifdef KHOTKEYS_TRACE
    kDebug() << "new:" << newState << "old:" << _active;
#endif
    // If there is no change in state just return.
    if (newState == _active)
        return;

    _active = newState;

    if (_active)
        {
        QString name = data
            ? data->name()
            : "TODO";

        // FIXME: The following workaround tries to prevent having two actions with
        // the same uuid. That happens wile exporting/importing actions. The uuid
        // is exported too.
        QAction *act = keyboard_handler->addAction( _uuid.toString(), name, _shortcut );
        // addAction can change the uuid. That's why we store the uuid from the
        // action
        _uuid = act->objectName();

        connect(
            act, SIGNAL(triggered(bool)),
            this, SLOT(trigger()) );

        connect(
            act, SIGNAL(globalShortcutChanged(QKeySequence)),
            this, SIGNAL(globalShortcutChanged(QKeySequence)));
        }
    else
        {
        // Disable the trigger. Delete the action.
        QAction *action = keyboard_handler->getAction( _uuid.toString() );
        if(action)
            {
            // In case the shortcut was changed from the kcm.
            auto shortcuts = KGlobalAccel::self()->shortcut(action);
            if (!shortcuts.isEmpty()) {
                _shortcut = KShortcut(shortcuts.first());
            } else {
                _shortcut = KShortcut();
            }
            keyboard_handler->removeAction(_uuid.toString());
            }
        }
    }

QString ShortcutTrigger::shortcuts() const
{
    const auto shortcuts = shortcut();
    QString shortcutString;
    for (auto it = shortcuts.constBegin(); it != shortcuts.constEnd(); ++it ) {
        if (it != shortcuts.constBegin()) {
            shortcutString.append(QStringLiteral(";"));
        }
        shortcutString.append((*it).toString());
    }
    return shortcutString;
}

void ShortcutTrigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Key", shortcuts());
    cfg_P.writeEntry( "Type", "SHORTCUT" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "Uuid", _uuid.toString() );
    }


ShortcutTrigger* ShortcutTrigger::copy( ActionData* data_P ) const
    {
    return new ShortcutTrigger( data_P ? data_P : data, shortcut(), QUuid::createUuid());
    }


const QString ShortcutTrigger::description() const
    {
    return i18n( "Shortcut trigger: " ) + shortcuts();
    }


void ShortcutTrigger::disable()
    {
    activate(false);

    // Unregister the shortcut with kglobalaccel
    QAction *action = keyboard_handler->getAction( _uuid.toString() );
    if(action)
        {
        // In case the shortcut was changed from the kcm.
        auto shortcuts = KGlobalAccel::self()->shortcut(action);
        if (!shortcuts.isEmpty()) {
            _shortcut = KShortcut(shortcuts.first());
        } else {
            _shortcut = KShortcut();
        }


        // Unregister the global shortcut.
        KGlobalAccel::self()->removeAllShortcuts(action);
        keyboard_handler->removeAction(_uuid.toString());
        }
    }


void ShortcutTrigger::enable()
    {
    // To enable the shortcut we have to just register it once with
    // kglobalaccel and deactivate it immediately
    activate(true);
    activate(false);
    }


void ShortcutTrigger::set_key_sequence( const QKeySequence &seq )
    {
    // Get the action from the keyboard handler
    QAction *action = keyboard_handler->getAction( _uuid.toString() );
    if (!action)
        {
        _shortcut.setPrimary(seq);
        }
    else
        {
        KGlobalAccel::self()->setShortcut(action,
                                          QList<QKeySequence>() << seq,
                                          KGlobalAccel::KGlobalAccel::NoAutoloading);
        }
    }


QList<QKeySequence> ShortcutTrigger::shortcut() const
    {
    // Get the action from the keyboard handler
    QAction *action = keyboard_handler->getAction( _uuid.toString() );
    if (!action)
        {
        // Not active!
        return _shortcut;
        }

    return KGlobalAccel::self()->shortcut(action);
    }

QString ShortcutTrigger::primaryShortcut() const
    {
    const auto shortcuts = shortcut();
    if (!shortcuts.isEmpty()) {
        return shortcuts.first().toString();
    } else {
        return QString();
    }
    }

void ShortcutTrigger::trigger()
    {
    if (khotkeys_active())
        {
        windows_handler->set_action_window( 0 ); // use active window
        data->execute();
        }
    }


} // namespace KHotKeys

