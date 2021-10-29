/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import QtQuick.Controls 2.0

Label {
    property real scale: 1.4

    height: 2 * implicitHeight

    Component.onCompleted: {
        font.pointSize = font.pointSize * scale;
    }
}
