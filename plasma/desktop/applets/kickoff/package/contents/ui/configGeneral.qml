/*
 *  Copyright 2013 David Edmundson <davidedmundson@kde.org>
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
import QtQuick.Controls 1.0 as QtControls
// import org.kde.locale 2.0

//FIXME this causes a crash in Oxygen style
//QtControls.GroupBox {
Item {

//FIXME i18n is broken currently
//when fixed delete this
function i18n(arg) {
    return arg;
}
    width: childrenRect.width
    height: childrenRect.height

//FIXME enable when we're back to being a group box
//     flat: true
//     title: i18n("Appearance")

    property alias cfg_switchTabsOnHover: switchTabsOnHoverCheckbox.checked
    property alias cfg_showAppsByName: showApplicationsByNameCheckbox.checked
    property alias cfg_showRecentlyInstalled: showRecentlyInstalledCheckbox.checked

    Column {
        QtControls.CheckBox {
            id: switchTabsOnHoverCheckbox
            text: i18n("Switch tabs on hover:")
        }
        QtControls.CheckBox {
            id: showApplicationsByNameCheckbox
            text: i18n("Show applications by name:")
        }
        QtControls.CheckBox {
            id: showRecentlyInstalledCheckbox
            text: i18n("Show 'Recently Installed':")
        }
    }
}
