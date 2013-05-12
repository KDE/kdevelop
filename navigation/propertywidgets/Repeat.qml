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

// TODO support other size types than PointSize

Rectangle {
    id: root
    width: 140
    height: 140
    property string value: "10"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider)+2;
    }
    function valueToSlider(value) {
        return value;
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Item count"
        color: "white"
        opacity: 0.8
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
    Rectangle {
        width: (4+2)*10
        height: (4+2)*10
        anchors.centerIn: parent
        color: "#00000000"
        Grid {
            spacing: 2
            columns: 10
            Repeater {
                model: root.value < 100 ? root.value : 100
                delegate: Rectangle {
                    opacity: root.value <= 100 ? 1 : 0.2
                    color: "#0095ff"
                    width: 4
                    height: 4
                }
            }
        }
    }
}