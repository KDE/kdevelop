/*************************************************************************************
 *  Copyright (C) 2013 by Sven Brauch <svenbrauch@gmail.com>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

import QtQuick 1.1

PropertyWidget {
    width: 150
    height: 110
    value: "3"
    function sliderToValue(slider) {
        return Math.round(slider/8.0)
    }
    function valueToSlider(value) {
        return value*8;
    }
    Slider {
        id: "slider"
        initialValue: valueToSlider(parent.value)
        anchors.top: parent.top
        anchors.left: parent.left
        width: 100
        height: 12
        color: "#929292"
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Spacing between items"
        color: "white"
        opacity: 0.8
    }
    Grid {
        columns: 3
        spacing: parent.value
        anchors.centerIn: parent
        Repeater {
            model: 9
            delegate: Rectangle {
                color: "#40a838"
                height: 8
                width: 8
            }
        }
    }
}