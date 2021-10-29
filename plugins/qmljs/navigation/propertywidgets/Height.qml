/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2 as QtControls

PropertyWidget {
    id: root
    width: 150
    height: 220
    value: sliderToValue(slider.value)
    onInitialValueChanged: slider.value = Math.sqrt(root.initialValue * 30.0)

    function sliderToValue(slider) {
        return Math.round(slider*slider/30);
    }

    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Height of an item in pixels")
        color: "white"
        opacity: 0.8
    }
    QtControls.Slider {
        id: slider
        anchors.top: parent.top
        anchors.left: parent.left
        maximumValue: 100.0
        width: 100
    }
    Rectangle {
        anchors.centerIn: parent
        color: "#65a800"
        width: 30
        height: parent.value < parent.height - 40 ? parent.value : parent.height - 40
        opacity: parent.value < parent.height - 40 ? 1 : 0.5
    }
}
