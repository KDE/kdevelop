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

import QtQuick 1.0

// Component which provides a single button for the assistant widget.

Rectangle {
    y: -1
    clip: true
    id: root
    property color foreground
    property color background
    property color highlight
    color: {
        var v = rgb(background)
        // make the color a bit lighter
        return Qt.rgba(Math.min(1, v[0]+0.15), Math.min(1, v[1]+0.15), Math.min(1, v[2]+0.15), 1.0)
    }
    // text on the button
    property string text
    // index of the button
    property int buttonIndex
    // text in the shortcut field
    property int button
    // emitted when the button is clicked with the mouse
    signal triggered()
    width: text.width + 4
    height: number.height + 4
    border {
        color: {
            var v = rgb(root.foreground)
            return Qt.rgba(v[0], v[1], v[2], 0.4)
        }
        width: 1
    }
    // TODO remove Qt5; in Qt4 you can't get the rgb components
    function rgb(color) {
        var res = Array();
        color = String(color)
        for ( var i = 0; i < 3; i++ ) {
            res[i] = (parseInt(color[i*2+1], 16) * 16 + parseInt(color[i*2+2], 16)) / 255.0;
        }
        return res;
    }
    function highlightKey(doHighlight) {
        highlight.opacity = doHighlight ? 0.5 : 0;
    }
    function highlightTrigger() {
        highlight.opacity = 1.0
    }
    Behavior on opacity {
        NumberAnimation { duration: 150; }
    }
    MouseArea {
        hoverEnabled: true
        property bool wasClicked: false
        id: mouseArea
        anchors.fill: parent
        onClicked: { root.triggered(); mouseArea.wasClicked = true }
        onEntered: highlight.opacity = 0.5
        onExited: if ( ! mouseArea.wasClicked ) highlight.opacity = 0.0
        Row {
            // row containing the separators, shortcut text, and button text
            z: 3
            id: text
            anchors.centerIn: parent
            spacing: 0
            Rectangle { width: 2; height: 1; color: Qt.rgba(0, 0, 0, 0) } // padding
            Text {
                // shortcut key
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 1
                id: number
                color: root.foreground
                text: button
                z: 2
            }
            Rectangle { width: 3; height: 1; color: Qt.rgba(0, 0, 0, 0) } // padding
            Rectangle { y: 1; width: 1; color: root.foreground; height: root.height - 1; opacity: 0.3 } // line
            Rectangle { width: 4; height: 1; color: Qt.rgba(0, 0, 0, 0) } // padding
            Text {
                // actual button text
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 1
                color: root.foreground
                text: root.text
            }
            Rectangle { width: 2; height: 1; color: Qt.rgba(0, 0, 0, 0) } // padding
        }
        Rectangle {
            // the background color for the shortcut key box, invisible by default.
            id: highlight
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
            opacity: 0
            x: 1
            y: 1
            z: 1
            height: text.height
            width: number.width + 6
            color: root.highlight
        }
    }
}
