/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.private.systemtray 2.0 as SystemTray

Item {
    id: root
    objectName: "SystemTrayRootItem"
//     width: 256
//     height: 32
//     anchors {
//         left: parent.left
//         right: parent.right
//         verticalCenter: parent.verticalCenter
//     }

    property int _h: 32

    SystemTray.Host {
        id: host
        rootItem: gridView

    }

    function loadNotificationsPlasmoid() {
        var plugin = "org.kde.systrayplasmoidtest";
        plugin = "org.kde.notifications";
        print("Loading notifications plasmoid: " + plugin);
        host.rootItem = gridView;
        var notificationsPlasmoid = host.notificationsPlasmoid(plugin);
        if (notificationsPlasmoid == null) {
            print("Bah. Failed to load " + plugin);
            return;
        }
        notificationsPlasmoid.parent = notificationsContainer;
        notificationsPlasmoid.anchors.fill = notificationsContainer;
    }

    Item {
        id: notificationsContainer

        anchors {
            top: parent.top
            left: parent.left
        }
        height: _h
        width: _h

        Rectangle {
            anchors.fill: parent;
            border.width: 2;
            border.color: "black";
            color: "transparent";
            opacity: 0;
        }
        Timer {
            interval: 1000
            running: true
            onTriggered: loadNotificationsPlasmoid()
        }
    }

    ListView {
        id: gridView
        objectName: "gridView"

        anchors {
            left: notificationsContainer.right
            leftMargin: spacing
            right: parent.right
            //verticalCenter: parent.verticalCenter
        }

        orientation: Qt.Horizontal
        interactive: false
        spacing: 4
        //Rectangle { anchors.fill: parent; color: "blue"; opacity: 0.2; }

        model: host.tasks

        delegate: TaskDelegate {}

        //delegate: StatusNotifierItem {}
    }
}
