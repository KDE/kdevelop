/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2 as QtControls

PropertyWidget {
    id: root
    width: 220
    height: 120
    property string value: '"' + combo.currentText + '"'
    onInitialValueChanged: combo.currentText = root.initialValue.slice(1, 3)

    QtControls.ComboBox {
        id: combo
        anchors.top: parent.top
        anchors.left: parent.left
        editable: true
        model: Qt.fontFamilies()
    }
    Text {
        color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.pointSize: 9
        font.family: combo.currentText
        text: i18nc("example text to test a font", "The lazy dog jumps over<br>the quick brown fox.")
        horizontalAlignment: TextInput.AlignHCenter
    }
    Text {
        z: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Font family")
        color: "white"
        opacity: 0.8
    }
}
