/********************************************************************
Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 1.1
import org.kde.qtextracomponents 0.1 as QtExtra

DecorationButton {
    buttonType: "M"
    QtExtra.QIconItem {
        icon: decoration.icon
        anchors.fill: parent
    }
    Timer {
        id: timer
        interval: decoration.doubleClickInterval
        repeat: false
        onTriggered: decoration.menuClicked()
    }
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            parent.pressed = true;
            // we need a timer to figure out whether there is a double click in progress or not
            // if we have a "normal" click we want to open the context menu. This would eat our
            // second click of the double click. To properly get the double click we have to wait
            // the double click delay to ensure that it was only a single click.
            if (timer.running) {
                timer.stop();
            } else {
                timer.start();
            }
        }
        onReleased: {
            parent.pressed = false;
        }
        onExited: {
            if (!parent.pressed) {
                return;
            }
            if (timer.running) {
                timer.stop();
            }
            parent.pressed = false;
        }
        onClicked: {
            // for right clicks we show the menu instantly
            if (mouse.button == Qt.RightButton) {
                decoration.menuClicked();
                timer.stop();
            }
        }
        onDoubleClicked: decoration.closeWindow()
    }
}