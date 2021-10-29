/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2 as QtControls

PropertyWidget {
    id: root
    width: 150
    height: 110
    value: sliderToValue(slider.value)
    onInitialValueChanged: slider.value = root.initialValue * 4.0

    function sliderToValue(slider) {
        return Math.round(slider/4.0)
    }

    QtControls.Slider {
        id: slider
        anchors.top: parent.top
        anchors.left: parent.left
        maximumValue: 100.0
        width: 100
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Spacing between items")
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
