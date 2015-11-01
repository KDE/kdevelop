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

import QtQuick 2.2

Rectangle {
    id: root

    readonly property int vSpacing: 4
    readonly property int hSpacing: 4
    readonly property real itemsWidth: {
        var totalWidth = title.width;
        for (var i = 0; i < items.count; ++i) {
            totalWidth += items.itemAt(i).width;
        }
        return totalWidth + (items.count + 2) * hSpacing;
    }
    readonly property bool useVerticalLayout: config.viewSize.width * 0.90 < itemsWidth

    // QQuickWidget crashes if either of these is zero
    // Use ceil to ensure the widget always fits the non-integral content size
    width: Math.ceil(Math.max(hSpacing, mainFlow.width + hSpacing * 2))
    height: Math.ceil(Math.max(vSpacing, mainFlow.height + vSpacing * 2))

    border.width: 1
    border.color: Qt.lighter(config.foreground)
    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.lighter(config.background) }
        GradientStop { position: 1.0; color: config.background }
    }

    Flow {
        id: mainFlow

        anchors {
            centerIn: parent
        }

        flow: root.useVerticalLayout ? Flow.TopToBottom : Flow.LeftToRight
        spacing: root.useVerticalLayout ? root.vSpacing : root.hSpacing

        Text {
            id: title

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
