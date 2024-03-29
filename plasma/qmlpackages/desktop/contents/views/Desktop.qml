/*
 *  Copyright 2012 Marco Martin <mart@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import "../activitymanager"
import "../explorer"


Rectangle {
    id: root
    color: Qt.rgba(0, 0, 0, 0.2)
    width: 1024
    height: 768

    property Item containment

    function toggleWidgetExplorer(containment) {
        console.log("Widget Explorer toggled");

        sidePanelStack.pop(blankPage);

        if (sidePanelStack.state == "widgetExplorer") {
            sidePanelStack.state = "closed";
        } else {
            var page = sidePanelStack.push(Qt.resolvedUrl("../explorer/WidgetExplorer.qml"));
            page.closed.connect(function(){sidePanelStack.state = "closed";});
            page.containment = containment;
            sidePanelStack.state = "widgetExplorer";
        }
    }

    function toggleActivityManager() {
        console.log("Activity manger toggled");

        sidePanelStack.pop(blankPage);

        if (sidePanelStack.state == "activityManager") {
            sidePanelStack.state = "closed";
        } else {
            var page = sidePanelStack.push(Qt.resolvedUrl("../activitymanager/ActivityManager.qml"));
            page.closed.connect(function(){sidePanelStack.state = "closed";});
            sidePanelStack.state = "activityManager";
        }
    }

    PlasmaCore.Dialog {
        id: sidePanel
        location: PlasmaCore.Types.LeftEdge
        onVisibleChanged: {
            if (!visible) {
                sidePanelStack.pop(blankPage);
            }
        }
        type: PlasmaCore.Dialog.Dock
        windowFlags: Qt.Window|Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint

        mainItem: PlasmaComponents.PageStack {
            id: sidePanelStack
            state: "closed"
            width: 250
            height: 500
            initialPage: Item {
                id: blankPage
            }

            states: [
                State {
                    name: "closed"
                    PropertyChanges {
                        target: sidePanel
                        visible: false
                        height: containment.availableScreenRegion(containment.screen)[0].height;
                    }
                },
                State {
                    name: "widgetExplorer"
                    PropertyChanges {
                        target: sidePanel
                        visible: true
                        height: containment.availableScreenRegion(containment.screen)[0].height;
                    }
                },
                State {
                    name: "activityManager"
                    PropertyChanges {
                        target: sidePanel
                        visible: true
                        height: containment.availableScreenRegion(containment.screen)[0].height;
                    }
                }
            ]
        }
    }

    onContainmentChanged: {
        print("New Containment: " + containment);
        print("Old Containment: " + internal.oldContainment);
        //containment.parent = root;
        containment.visible = true;

        internal.newContainment = containment;
        if (internal.oldContainment && internal.oldContainment != containment) {
            switchAnim.running = true;
        } else {
            internal.oldContainment = containment;
        }
    }

    //some properties that shouldn't be accessible from elsewhere
    QtObject {
        id: internal;

        property Item oldContainment;
        property Item newContainment;
    }

    SequentialAnimation {
        id: switchAnim
        ScriptAction {
            script: {
                containment.anchors.left = undefined;
                containment.anchors.top = undefined;
                containment.anchors.right = undefined;
                containment.anchors.bottom = undefined;

                internal.oldContainment.anchors.left = undefined;
                internal.oldContainment.anchors.top = undefined;
                internal.oldContainment.anchors.right = undefined;
                internal.oldContainment.anchors.bottom = undefined;

                internal.oldContainment.z = 0;
                internal.oldContainment.x = 0;
                containment.z = 1;
                containment.x = root.width;
            }
        }
        ParallelAnimation {
            NumberAnimation {
                target: internal.oldContainment
                properties: "x"
                to: -root.width
                duration: 400
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: internal.newContainment
                properties: "x"
                to: 0
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                containment.anchors.left = root.left;
                containment.anchors.top = root.top;
                containment.anchors.right = root.right;
                containment.anchors.bottom = root.bottom;

                internal.oldContainment.visible = false;
                internal.oldContainment = containment;
            }
        }
    }


    Component.onCompleted: {
        //configure the view behavior
        desktop.stayBehind = true;
        desktop.fillScreen = true;
        print("View org.kde.desktop QML loaded")
    }
}
