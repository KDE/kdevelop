/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Rectangle {
    id: root

    SystemPalette {
        id: myPalette
    }

    color: myPalette.window

    Loader {
        id: loader

        anchors.fill: parent

        // non-code areas are broken ATM, so just go blank for them
        // old: source: "qrc:///qml/area_"+area+".qml"
        source: area === "code" ? "qrc:///qml/area_code.qml" : ""
        asynchronous: true
        opacity: status === Loader.Ready

        Behavior on opacity {
            PropertyAnimation {}
        }
    }

}
