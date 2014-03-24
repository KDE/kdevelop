/*
   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>

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

// Rectangle for the background
Rectangle {
    id: root

    color: config.background

    width: row.width + 16
    height: row.height + 7

    Connections {
        id: configConnections
        target: config
        // Allow cancelling the popup animation from outside
        onShouldCancelAnimation: {
            popupAnimation.running = false;
            root.opacity = 1;
            root.y = 0;
        }
    }

    // start values for the popup animation; change y to 0 to disable the slight move animation
    opacity: 0.2
    y: -2
    ParallelAnimation {
        id: popupAnimation
        running: true
        NumberAnimation { target: root; properties: "opacity"; to: 1; duration: 250 }
        NumberAnimation { target: root; properties: "y"; to: 0; duration: 100 }
    }

    // Keys which are assigned to the action buttons, in this order.
    // The hide button is always Key_0, which is handled below.
    property variant keysForIndex: [Qt.Key_1, Qt.Key_2, Qt.Key_3, Qt.Key_4, Qt.Key_5,
                                    Qt.Key_6, Qt.Key_7, Qt.Key_8, Qt.Key_9]

    // Layout for the buttons and the title
    Row {
        id: row
        anchors.centerIn: parent
        spacing: 8
        function doTriggerButton(triggerIndex) {
            config.model[triggerIndex].trigger();
            // highlight the triggered button
            for ( var i = 0; i < config.model.length; i++ ) {
                if ( i == triggerIndex ) {
                    items.itemAt(i).highlightTrigger();
                }
                else {
                    items.itemAt(i).opacity = 0.25;
                }
            }
        }
        Text {
            // Title of the assistant
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 1
            color: config.foreground
            text: {
                if ( typeof config.title !== 'undefined' ) {
                    return "<style type=\"text/css\">" +
                           "code { font-weight: bold; }\n" +
                           "</style><html>" + config.title + "</html>";
                }
                return ""
            }
        }
        Repeater {
            // Buttons
            focus: true
            y: 5
            id: items
            objectName: "items"
            // config.model contains a list of buttons to be displayed, set from C++
            model: config.model
            onModelChanged: {
                root.opacity = 0.2
                root.y = -2
                popupAnimation.start();
            }
            AssistantButton {
                Connections {
                    target: config
                    onShouldShowHighlight: {
                        highlightKey(show);
                    }
                }
                text: modelData.name
                // what is displayed in the hotkey field of the button
                button: index == items.model.length - 1 ? 0 : index + 1
                buttonIndex: index
                onTriggered: row.doTriggerButton(buttonIndex)
                foreground: config.foreground
                background: config.background
                highlight: config.highlight
            }
            Keys.onPressed: {
                console.log("key pressed", event.key);
                var triggerIndex = -1;
                if ( event.key == Qt.Key_0 ) {
                    triggerIndex = model.length-1;
                }
                else {
                    var buttonIndex = root.keysForIndex.indexOf(event.key);
                    if ( buttonIndex < model.length ) {
                        triggerIndex = buttonIndex;
                    }
                }
                if ( triggerIndex == -1 ) {
                    return;
                }
                event.accepted = true;
                row.doTriggerButton(triggerIndex);
            }
        }
    }
}
