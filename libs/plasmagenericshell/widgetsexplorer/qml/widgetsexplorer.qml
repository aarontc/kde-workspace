/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: main
    property int minimumWidth: 200
    property int minimumHeight: 150
    signal addAppletRequested(string pluginName)
    signal closeRequested()
    property variant extraActions
    property variant getWidgetsActions

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaComponents.ContextMenu {
        id: categoriesDialog
        visualParent: categoryButton
    }
    Repeater {
        model: filterModel
        delegate: PlasmaComponents.MenuItem {
            text: display
            separator: model["separator"]
            onClicked: {
                var item = filterModel.get(index)

            appletsModel.filterType = item.filterType
            appletsModel.filterQuery = item.filterData
            }
            Component.onCompleted: {
                parent = categoriesDialog
            }
        }
    }

    PlasmaComponents.ContextMenu {
        id: getWidgetsDialog
        visualParent: getWidgetsButton
    }
    Repeater {
        model: getWidgetsActions
        delegate: PlasmaComponents.MenuItem {
            icon: modelData.icon
            text: modelData.text
            separator: modelData.separator
            onClicked: modelData.trigger()
            Component.onCompleted: {
                parent = getWidgetsDialog
            }
        }
    }

    PlasmaCore.Dialog {
        id: tooltipDialog
        property Item appletDelegate

        Component.onCompleted: {
            tooltipDialog.setAttribute(Qt.WA_X11NetWmWindowTypeDock, true)
            tooltipDialog.windowFlags |= Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint
        }

        onAppletDelegateChanged: {
            if (!appletDelegate) {
                toolTipHideTimer.restart()
                toolTipShowTimer.running = false
            } else {
                toolTipShowTimer.restart()
                toolTipHideTimer.running = false
            }
        }
        mainItem: Tooltip { id: tooltipWidget }
        Behavior on x {
            NumberAnimation { duration: 250 }
        }
    }
    Timer {
        id: toolTipShowTimer
        interval: 500
        repeat: false
        onTriggered: {
            var point = tooltipDialog.popupPosition(tooltipDialog.appletDelegate)
            tooltipDialog.x = point.x
            tooltipDialog.y = point.y
            tooltipDialog.visible = true
        }
    }
    Timer {
        id: toolTipHideTimer
        interval: 1000
        repeat: false
        onTriggered: tooltipDialog.visible = false
    }

    Item {
        id: topBar
        anchors.top: parent.top
        anchors.left:parent.left
        anchors.right: parent.right
        height: categoryButton.height
        Row {
            spacing: 4
            PlasmaComponents.TextField {
                width: list.width / Math.floor(list.width / 180)
                clearButtonShown: true
                onTextChanged: appletsModel.searchTerm = text
                Component.onCompleted: forceActiveFocus()
            }
            PlasmaComponents.Button {
                id: categoryButton
                text: "Categories"
                onClicked: categoriesDialog.open()
            }
        }
        Row {
            anchors.right: parent.right
            spacing: 4
            PlasmaComponents.Button {
                id: getWidgetsButton
                iconSource: "get-hot-new-stuff"
                text: i18n("Get new widgets")
                onClicked: getWidgetsDialog.open()
            }

            Repeater {
                model: extraActions.length
                PlasmaComponents.Button {
                    iconSource: extraActions[modelData].icon
                    text: extraActions[modelData].text
                    onClicked: {
                        extraActions[modelData].trigger()
                    }
                }
            }
            PlasmaComponents.ToolButton {
                iconSource: "window-close"
                onClicked: main.closeRequested()
            }
        }
    }
    ListView {
        id: list
        anchors.topMargin: 4
        anchors.top: topBar.bottom
        anchors.left:parent.left
        anchors.right: parent.right
        anchors.bottom: scrollBar.top
        clip: true
        orientation: ListView.Horizontal
        snapMode: ListView.SnapToItem
        model: appletsModel

        onContentXChanged: {
            if (!scrollBar.moving) {
                scrollBar.value = contentX/10
            }
        }

        onContentWidthChanged: {
            if (!scrollBar.moving) {
                scrollBar.minimum = 0
                scrollBar.maximum = (contentWidth - width)/10
            }
        }

        delegate: AppletDelegate {}
    }
    PlasmaComponents.ScrollBar {
        id: scrollBar
        orientation: Qt.Horizontal
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        flickableItem: list
    }
}
