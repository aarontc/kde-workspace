/*
 *   Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>
 *   Copyright 2012 Marco Martin <mart@kde.org>
 *   Copyright 2013 David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: secondHand

    property alias rotation: rotation.angle
    property double svgScale

    width: naturalSize.width * svgScale
    height: naturalSize.height * svgScale
    anchors.top: clock.verticalCenter
    anchors.horizontalCenter: clock.horizontalCenter
    svg: clockSvg
    smooth: true
    transform: Rotation {
        id: rotation
        angle: 0
        origin {
            x: width/2
            y: 0
        }
        Behavior on angle {
            SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
        }
    }
}
