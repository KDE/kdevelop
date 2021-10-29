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
    value: spin.value
    onInitialValueChanged: spin.value = root.initialValue

    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Duration in milliseconds")
        color: "white"
        opacity: 0.8
    }
    QtControls.SpinBox {
        id: spin
        anchors.top: parent.top
        anchors.left: parent.left
        maximumValue: 5000
        minimumValue: 200
        stepSize: 100
        decimals: 0
        width: 100
    }
    Rectangle {
        id: sample
        anchors.centerIn: parent
        color: "#1f61a8"
        width: 50
        height: 50
        SequentialAnimation {
            ColorAnimation { target: sample; property: "color"; to: "white"; duration: 0 }
            ParallelAnimation {
                ColorAnimation { target: sample; property: "color"; to: "#8aca00"; duration: spin.value }
                NumberAnimation { target: sample; property: "anchors.horizontalCenterOffset"; to: 20; duration: spin.value }
            }
            PauseAnimation { duration: 500 }
            ColorAnimation { target: sample; property: "color"; to: "white"; duration: 0 }
            ParallelAnimation {
                ColorAnimation { target: sample; property: "color"; to: "#8aca00"; duration: spin.value }
                NumberAnimation { target: sample; property: "anchors.horizontalCenterOffset"; to: -20; duration: spin.value }
            }
            PauseAnimation { duration: 500 }
            running: true
            loops: Animation.Infinite
        }
    }
}
