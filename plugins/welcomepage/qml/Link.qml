/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0

Label {
    id: root

    signal clicked

    elide: Text.ElideRight
    opacity: mouseArea.containsMouse ? 0.8 : 1

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: root.clicked()
    }

}
