/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2 as QtControls

PropertyWidget {
    id: root
    width: 220
    height: 110
    value: sliderToValue(slider.value)
    onInitialValueChanged: slider.value = Math.sqrt(root.initialValue * 30.0)

    function sliderToValue(slider) {
        return Math.round(slider*slider/30.0);
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Width of an item in pixels")
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
        height: 30
        width: parent.value < parent.width - 20 ? parent.value : parent.width - 20
        opacity: parent.value < parent.width - 20 ? 1 : 0.5
    }
}
