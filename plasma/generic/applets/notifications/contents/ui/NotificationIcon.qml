/***************************************************************************
 *   Copyright 2011 Davide Bettio <davide.bettio@kdemail.net>              *
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras


Item {
    PlasmaCore.SvgItem {
        id: notificationSvgItem
        svg: notificationSvg
        elementId: "notification-disabled"
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width

        state: notificationsApplet.state

        PlasmaCore.Svg {
            id: notificationSvg
            imagePath: "icons/notification"
        }

        Item {
            id: jobProgressItem
            width: notificationSvgItem.width * globalProgress
            clip: true
            visible: jobs.count > 0
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            PlasmaCore.SvgItem {
                svg: notificationSvg
                elementId: "notification-progress-active"
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                width: notificationSvgItem.width
            }
        }
        PlasmaComponents.BusyIndicator {
            anchors.fill: parent
            visible: jobs ? jobs.count > 0 : false
            running: visible
        }

        Column {
            id: countColumn
            visible: false
            anchors.centerIn: parent
            PlasmaCore.SvgItem {
                svg: notificationSvg
                elementId: {
                    switch (plasmoid.location) {
                    case PlasmaCore.Types.TopEdge:
                        return "expander-top"
                    case PlasmaCore.Types.LeftEdge:
                        return "expander-left"
                    case PlasmaCore.Types.RightEdge:
                        return "expander-right"
                    default:
                        return "expander-bottom"
                    }
                }
                width: naturalSize.width
                height: naturalSize.height
                anchors.horizontalCenter: parent.horizontalCenter
            }
            PlasmaComponents.Label {
                property int totalCount: notificationsApplet.totalCount
                text: totalCount

                property int oldTotalCount: 0
                font.pointSize: theme.smallestFont.pointSize
                height: paintedHeight - 3
                onTotalCountChanged: {
                    if (totalCount > oldTotalCount) {
                        notificationAnimation.running = true
                    }
                    oldTotalCount = totalCount
                }
            }
        }

        PlasmaCore.SvgItem {
            id: notificationAnimatedItem
            anchors.fill: parent
            svg: notificationSvg
            elementId: "notification-active"
            opacity: 0
            scale: 2

            SequentialAnimation {
                id: notificationAnimation
                NumberAnimation {
                    target: notificationAnimatedItem
                    duration: 250
                    properties: "opacity, scale"
                    to: 1
                    easing.type: Easing.InOutQuad
                }
                PauseAnimation { duration: 500 }
                ParallelAnimation {
                    NumberAnimation {
                        target: notificationAnimatedItem
                        duration: 250
                        properties: "opacity"
                        to: 0
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: notificationAnimatedItem
                        duration: 250
                        properties: "scale"
                        to: 2
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (notificationsApplet.totalCount > 0) {
                    plasmoid.expanded = !plasmoid.expanded;
                } else {
                    plasmoid.expanded = false;
                }
            }
        }
        states: [
            State {
                name: "default"
                PropertyChanges {
                    target: notificationSvgItem
                    elementId: "notification-disabled"
                }
                PropertyChanges {
                    target: countColumn
                    visible: false
                }
                PropertyChanges {
                    target: plasmoid
                    status: PlasmaCore.Types.PassiveStatus
                }
            },
            State {
                name: "new-notifications"
                PropertyChanges {
                    target: notificationSvgItem
                    elementId: jobs.count > 0 ? "notification-progress-inactive" : "notification-empty"
                }
                PropertyChanges {
                    target: countColumn
                    visible: true
                }
                PropertyChanges {
                    target: plasmoid
                    status: ActiveStatus
                }
            }
        ]
    }

    Component.onCompleted: {
        print(" NOTificationsIcon.qml loaded");
    }
}

