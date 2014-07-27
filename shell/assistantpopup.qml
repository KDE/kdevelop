/*
   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// This file provides the whole assistant, including title and buttons.

import QtQuick 1.0

Rectangle {
    id: root

    width: flow.width + 16
    height: flow.height + 7

    border.width: 1
    border.color: Qt.lighter(config.foreground)
    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.lighter(config.background) }
        GradientStop { position: 1.0; color: config.background }
    }

    // Layout for the buttons and the title
    Flow {
        id: flow
        anchors.centerIn: parent
        flow: items.count >= 3 ? Flow.TopToBottom : Flow.LeftToRight
        spacing: 8

        Text {
            id: title
            anchors.verticalCenter: parent.flow == Flow.LeftToRight ? parent.verticalCenter : undefined
            anchors.verticalCenterOffset: 1
            color: config.foreground
            font.bold: true
            text: config.title
        }

        Repeater {
            id: items
            objectName: "items"

            y: 5
            model: config.model

            AssistantButton {
                text: modelData.text
                highlighted: config.active
                // what is displayed in the hotkey field of the button
                button: index == items.model.length - 1 ? 0 : index + 1
                foreground: config.foreground
                background: config.background
                highlight: config.highlight

                onTriggered: { modelData.trigger() }
            }
        }
    }
}
