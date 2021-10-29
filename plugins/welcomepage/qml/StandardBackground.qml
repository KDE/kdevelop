/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

GroupBox
{
    id: bg
    property alias tools: toolsLoader.sourceComponent
    property string pageIcon
    readonly property real marginLeft: toolsLoader.x + toolsLoader.width
    property real margins: 5

    Loader {
        id: toolsLoader

        width: bg.width/4

        anchors {
            top: parent.top
            topMargin: 15
            left: parent.left
            leftMargin: 15
            bottom: parent.bottom
            bottomMargin: 15
        }
    }

    Image {
        id: theIcon
        anchors {
            bottom: parent.bottom
            left: parent.left
            margins: bg.margins
        }
        source: bg.pageIcon !== "" ? "image://icon/" + bg.pageIcon : ""
        width: 64
        height: width
    }
}
