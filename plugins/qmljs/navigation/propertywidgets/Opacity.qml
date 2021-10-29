/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2 as QtControls

PropertyWidget {
    id: root
    width: 120
    height: 120
    value: (slider.value / 100).toFixed(2)
    onInitialValueChanged: slider.value = parseFloat(root.initialValue) * 100.0

    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Opacity of an item")
        color: "white"
        opacity: 0.8
    }
    QtControls.Slider {
        id: slider
        anchors.top: parent.top
        anchors.left: parent.left
        maximumValue: 100
        width: 100
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
