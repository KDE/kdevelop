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
    height: 220
    value: "10"
    function sliderToValue(slider) {
        return Math.round(slider*slider/30);
    }
    function valueToSlider(value) {
        return Math.sqrt(value*30);
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Height of an item in pixels"
        color: "white"
        opacity: 0.8
    }
    Slider {
        z: 20
        initialValue: valueToSlider(parent.value)
        width: 100
        height: 12
    }
    Rectangle {
        anchors.centerIn: parent
        color: "#65a800"
        width: 30
        height: parent.value < parent.height - 40 ? parent.value : parent.height - 40
        opacity: parent.value < parent.height - 40 ? 1 : 0.5
    }
}