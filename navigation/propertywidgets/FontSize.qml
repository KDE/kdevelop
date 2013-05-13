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

PropertyWidget {
    width: 220
    height: 120
    value: "10"
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider/7*20)/20.0;
    }
    function valueToSlider(value) {
        return value*7;
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Font size"
        color: "white"
        opacity: 0.8
    }
    Slider {
        initialValue: valueToSlider(parent.value)
        anchors.top: parent.top
        anchors.left: parent.left
        width: 100
        height: 12
    }
    Text {
        color: "white"
        anchors.centerIn: parent
        font.pointSize: parent.value
        font.family: "sans" // TODO
        text: "The lazy dog jumps over<br>the quick brown fox."
        horizontalAlignment: TextInput.AlignHCenter
    }
}