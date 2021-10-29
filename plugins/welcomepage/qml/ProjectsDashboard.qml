/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.2

import "plugins"

StandardPage {
    ScrollView {
        anchors {
            fill: parent
            margins: 30
        }
        GridView {
            id: grid
            width: parent.width
            cellWidth: grid.width/2
            cellHeight: grid.height/3

            model: [ "qrc:/qml/plugins/Branches.qml", "qrc:/qml/plugins/Projects.qml" ]
//          TODO: make this the plugin
            delegate: Loader {
                x: 5
                y: 5
                width: grid.cellWidth-5
                height: grid.cellHeight-5
                source: modelData
            }
        }
    }
}
