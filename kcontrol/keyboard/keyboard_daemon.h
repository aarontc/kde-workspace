/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KEYBOARD_DAEMON_H_
#define KEYBOARD_DAEMON_H_

#include <kdedmodule.h>
#include <QtCore/QStringList>

#include "layout_memory.h"
#include "keyboard_dbus.h"


class KActionCollection;
class XEventNotifier;

class KDE_EXPORT KeyboardDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Keyboard")

    KActionCollection *actionCollection;
    XEventNotifier* xEventNotifier;
    LayoutMemory layoutMemory;

    void registerListeners();
    void registerShortcut();
    void unregisterListeners();
    void unregisterShortcut();

private Q_SLOTS:
	void switchToNextLayout();
	void globalSettingsChanged(int category);
    void configureKeyboard();

public Q_SLOTS:
	Q_SCRIPTABLE bool setLayout(const QString& layout);
	Q_SCRIPTABLE QString getCurrentLayout();
	Q_SCRIPTABLE QStringList getLayoutsList();

public:
    KeyboardDaemon(QObject *parent, const QList<QVariant>&);
    virtual ~KeyboardDaemon();
};

#endif /* KEYBOARD_DAEMON_H_ */