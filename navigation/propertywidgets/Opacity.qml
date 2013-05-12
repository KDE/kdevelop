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

Rectangle {
    width: 120
    height: 120
    property string value: "1"
    signal valueChanged(string newValue)
    color: "#00FFFFFF"
    function sliderToValue(slider) {
        return Math.round(slider/96*20)/20.0;
    }
    function valueToSlider(value) {
        return value*96;
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Opacity of an item"
        color: "white"
        opacity: 0.8
    }
    Slider {
        z: 20
        id: "slider"
        initialValue: valueToSlider(parent.value)
        anchors.verticalCenter: parent.horizontalCenter
        width: 100
        height: 12
        color: "#929292"
    }
    Rectangle {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -8
        anchors.horizontalCenterOffset: -8
        color: "#1f61a8"
        width: 45
        height: 45
        opacity: parent.value
        z: 1
    }
    Rectangle {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 8
        anchors.horizontalCenterOffset: 8
        color: "#a8002f"
        width: 45
        height: 45
        opacity: 1
        z: 0
    }
}