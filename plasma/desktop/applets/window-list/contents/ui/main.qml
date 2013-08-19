/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http: //www.gnu.org/licenses/>.
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: main
    property int minimumWidth: Math.max(200, windowListMenu.implicitWidth) 
    property int minimumHeight:  Math.max(400, windowListMenu.implicitHeight)
    property alias data: tasksSource.data;
    property variant desktopList: []
    property int iconSize: theme.smallMediumIconSize 
    property bool showDesktop: true
    property bool highlight: false

    PlasmaComponents.Highlight {
        id: highlightItem

        property Item trackingItem
        onTrackingItemChanged: {
            y = trackingItem.mapToItem(main, 0, 0).y - highlightItem.marginHints.top
        }

        hover: true
        width: windowListMenu.width
        x: 0
        y: 0
        height: 30
        visible: true
        opacity: trackingItem && (mouse.containsMouse || mouseArea.containsMouse || highlight) ? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing: Easing.InOutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: 250
                easing: Easing.InOutQuad
            }
        }
    }
    Connections {
        target: windowListMenu.flickableItem
        onContentYChanged: {
            highlightItem.trackingItem = null
        }
    }

    
    function performOperation(op) {
        var service =tasksSource.serviceForSource("");
        var operation = service.operationDescription(op);
        service.startOperationCall(operation);
    }
    
    function executeJob(operationName, source) {
        var service = tasksSource.serviceForSource(source);
        var operation = service.operationDescription(operationName);
        service.startOperationCall(operation);
    }
    
    function setOnDesktop(source, desktop) {
        var service = tasksSource.serviceForSource(source);
        var operation = service.operationDescription("toDesktop");
        operation.desktop = desktop;
        service.startOperationCall(operation);
    }
    
    Component.onCompleted: {
        var toolTipData = new Object;
        toolTipData["image"] = "preferences-system-window"; 
        toolTipData["mainText"] = i18n("Window List"); 
        toolTipData["subText"] = i18n("Show list of opened windows");
        plasmoid.popupIconToolTip = toolTipData;
        plasmoid.popupIcon = QIcon("preferences-system-windows"); 
        plasmoid.aspectRatioMode = ConstrainedSquare;
    }
    
    PlasmaCore.DataSource {
        id: tasksSource
        engine: "tasks"
        onSourceAdded: {
            connectSource(source);
        }
        Component.onCompleted: {
            connectedSources = sources;
            connectSource("virtualDesktops");
            main.desktopList = tasksSource.data["virtualDesktops"]["names"];
        }
    }
    
    PlasmaCore.SortFilterModel {
        id: tasksModelSortedByDesktop
        sortRole: "desktop"
        sourceModel: tasksModel
    }
    
    PlasmaCore.SortFilterModel { 
        id: tasksModel
        sourceModel: PlasmaCore.DataModel {
            dataSource: tasksSource
        }
    }

    Column {
        id: col
        spacing: 5
        PlasmaComponents.Highlight {
            hover: menu.focus
            width: windowListMenu.width
            height: 30
            PlasmaComponents.Label {
                id: actions
                text: "Actions"
                anchors {
                    centerIn: parent
                }
            }
        }
        Rectangle {
            id: action1
            width: windowListMenu.width
            height: 30
            color: "transparent"
            PlasmaComponents.Label {
                id: unclutter
                text: "Unclutter Windows"
                anchors {
                    leftMargin: 10
                    left: action1.left
                }
            }
            MouseArea {
                id: mouse
                hoverEnabled: true
                anchors.fill: parent
                onClicked: {
                    performOperation("unclutter");
                }
                onEntered: {
                    highlightItem.trackingItem = action1
                }
                onExited: {
                    highlight=false
                }
            }
        }
        Rectangle {
            id: rect_1
            width: windowListMenu.width
            height: 30
            color: "transparent"
            PlasmaComponents.Label {
                id: cascade
                text: "Cascade Windows"
                anchors {
                    left: parent.left
                    leftMargin: 10
                    verticalCenter: highlightItem.verticalCenter
                }
            }
            MouseArea {
                id: mouseArea
                hoverEnabled: true
                anchors.fill: parent
                onClicked: { 
                    performOperation("cascade");
                }
                onEntered: {
                    highlightItem.trackingItem = rect_1
                }
                onExited: {
                    highlight=false
                }
            }
        }
    }

    Menu {
        id: windowListMenu
        anchors {
            top: col.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        model: tasksModelSortedByDesktop
        section.property: "desktop"
        section.criteria: ViewSection.FullString
        iconSize: main.iconSize
        showDesktop: main.showDesktop
        onItemSelected: main.executeJob("activate", source);
        onExecuteJob: main.executeJob(jobName, source);
        onSetOnDesktop: main.setOnDesktop(source, desktop);
    }
}
