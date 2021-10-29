/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2 as QtControls

// TODO support other size types than PointSize

PropertyWidget {
    id: root
    width: 220
    height: 120
    value: slider.value.toFixed(1)
    onInitialValueChanged: slider.value = root.initialValue

    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Font size")
        color: "white"
        opacity: 0.8
    }
    QtControls.Slider {
        id: slider
        anchors.top: parent.top
        anchors.left: parent.left
        minimumValue: 4
        maximumValue: 50
        width: 100
    }
    Text {
        color: "white"
        anchors.centerIn: parent
        font.pointSize: slider.value
        font.family: "sans" // TODO
        text: i18nc("example text to test a font", "The lazy dog jumps over<br>the quick brown fox.")
        horizontalAlignment: TextInput.AlignHCenter
    }
}
