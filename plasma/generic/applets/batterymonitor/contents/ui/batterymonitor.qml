/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *   Copyright 2013 Kai Uwe Broulik <kde@privat.broulik.de>
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

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import "plasmapackage:/code/logic.js" as Logic

Item {
    id: batterymonitor
    property int minimumWidth: theme.iconSizes.dialog * 9
    property int minimumHeight: dialogItem.implicitHeight
    property int maximumHeight: dialogItem.implicitHeight

    property bool show_remaining_time: false

    LayoutMirroring.enabled: Qt.application.layoutDirection == Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    Component.onCompleted: {
        //plasmoid.aspectRatioMode = PlasmaCore.Types.IgnoreAspectRatio
        updateLogic(true);
        plasmoid.addEventListener('ConfigChanged', configChanged);
        plasmoid.popupEvent.connect(popupEventSlot);
    }

    function configChanged() {
        show_remaining_time = plasmoid.readConfig("showRemainingTime");
    }

    function updateLogic(updateBrightness) {
        Logic.updateCumulative();
        plasmoid.status = Logic.plasmoidStatus();
        Logic.updateTooltip();
        if (updateBrightness) {
            Logic.updateBrightness(pmSource);
        }
    }

    function popupEventSlot(popped) {
        dialogItem.popupShown = popped;
        if (popped) {
            dialogItem.forceActiveFocus();
        }
    }

    property Component compactRepresentation: CompactRepresentation { }

    property QtObject pmSource: PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: sources
        onDataChanged: {
            updateLogic(true);
        }
        onSourceAdded: {
            if (source == "Battery0") {
                disconnectSource(source);
                connectSource(source);
            }
        }
        onSourceRemoved: {
            if (source == "Battery0") {
                disconnectSource(source);
            }
        }
    }

    property QtObject batteries: PlasmaCore.SortFilterModel {
        id: batteries
        filterRole: "Is Power Supply"
        sortOrder: Qt.DescendingOrder
        sourceModel: PlasmaCore.SortFilterModel {
            sortRole: "Pretty Name"
            sortOrder: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            sourceModel: PlasmaCore.DataModel {
                dataSource: pmSource
                sourceFilter: "Battery[0-9]+"

                onDataChanged: updateLogic()
            }
        }

        property int cumulativePercent
        property bool cumulativePluggedin
        // true  --> all batteries charged
        // false --> one of the batteries charging/discharging
        property bool allCharged
        property string tooltipText
        property string tooltipImage
    }

    PopupDialog {
        id: dialogItem
        model: batteries
        anchors.fill: parent
        focus: true

        property bool disableBrightnessUpdate: false

        isBrightnessAvailable: pmSource.data["PowerDevil"]["Screen Brightness Available"] ? true : false
        isKeyboardBrightnessAvailable: pmSource.data["PowerDevil"]["Keyboard Brightness Available"] ? true : false

        showRemainingTime: show_remaining_time
        remainingTime: Number(pmSource.data["Battery"]["Remaining msec"])

        pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]

        onBrightnessChanged: {
            if (disableBrightnessUpdate) {
                return;
            }
            var service = pmSource.serviceForSource("PowerDevil");
            var operation = service.operationDescription("setBrightness");
            operation.brightness = screenBrightness;
            service.startOperationCall(operation);
        }
        onKeyboardBrightnessChanged: {
            if (disableBrightnessUpdate) {
                return;
            }
            var service = pmSource.serviceForSource("PowerDevil");
            var operation = service.operationDescription("setKeyboardBrightness");
            operation.brightness = keyboardBrightness;
            service.startOperationCall(operation);
        }
        property int cookie1: -1
        property int cookie2: -1
        onPowermanagementChanged: {
            var service = pmSource.serviceForSource("PowerDevil");
            if (checked) {
                var op1 = service.operationDescription("stopSuppressingSleep");
                op1.cookie = cookie1;
                var op2 = service.operationDescription("stopSuppressingScreenPowerManagement");
                op2.cookie = cookie2;

                var job1 = service.startOperationCall(op1);
                job1.finished.connect(function(job) {
                    cookie1 = -1;
                });

                var job2 = service.startOperationCall(op2);
                job2.finished.connect(function(job) {
                    cookie2 = -1;
                });
            } else {
                var reason = i18n("The battery applet has enabled system-wide inhibition");
                var op1 = service.operationDescription("beginSuppressingSleep");
                op1.reason = reason;
                var op2 = service.operationDescription("beginSuppressingScreenPowerManagement");
                op2.reason = reason;

                var job1 = service.startOperationCall(op1);
                job1.finished.connect(function(job) {
                    cookie1 = job.result;
                });

                var job2 = service.startOperationCall(op2);
                job2.finished.connect(function(job) {
                    cookie2 = job.result;
                });
            }
            Logic.powermanagementDisabled = !checked;
            updateLogic();
        }
    }
}
