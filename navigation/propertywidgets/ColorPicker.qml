/*
 * Copyright 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 2.010-1301, USA.
 */
import QtQuick 2.2

PropertyWidget {
    id: picker
    width: 214
    height: 140

    property real hue
    property real saturation
    property real lightness
    property bool __reactOnChange: updateColor(value)

    function currentColor() {
        return Qt.hsla(picker.hue, picker.saturation, picker.lightness, 1.0)
    }

    function updateColor(clr) {
        // QML does not expose any way of getting the components of a color
        // parsed by Qt, thus we have to to the parsing ourselves (this breaks
        // named colors)
        // TODO: QtQuick2 exposes the r, g and b attributes of color.
        if (clr[0] == '"') {
            clr = clr.slice(1, 8);
        }
        if (clr[0] == '#') {
            clr = clr.slice(1);
        }

        var r = parseInt(clr.slice(0, 2), 16) / 255;
        var g = parseInt(clr.slice(2, 4), 16) / 255;
        var b = parseInt(clr.slice(4, 6), 16) / 255;

        // Formulae taken from ColorPicker.qml, Plasma Workspace,
        // Copyright 2013 Marco Martin <mart@kde.org>
        var min = Math.min(r, Math.min(g, b))
        var max = Math.max(r, Math.max(g, b))
        var c = max - min
        var h

        if (c == 0) {
            h = 0
        } else if (max == r) {
            h = ((g - b) / c) % 6
        } else if (max == g) {
            h = ((b - r) / c) + 2
        } else if (max == b) {
            h = ((r - g) / c) + 4
        }

        picker.hue = (1/6) * h
        picker.saturation = c / (1 - Math.abs(2 * ((max+min)/2) - 1))
        picker.lightness = (max + min)/2

        return true;
    }

    // Rectangle that displays the hue and the saturation of the color
    MouseArea {
        id: rectangle
        width: 140
        height: 140

        onPositionChanged: {
            picker.hue = mouse.x/width
            picker.saturation = 1 - mouse.y/height

            updateTimer.restart()
        }
        // Display the colors
        Rectangle {
            anchors.fill: parent
            z: 0
            rotation: 270

            gradient: Gradient {
                GradientStop { position: 0.0/6.0; color: Qt.hsla(0.0/6.0, 1, picker.lightness, 1) }
                GradientStop { position: 1.0/6.0; color: Qt.hsla(1.0/6.0, 1, picker.lightness, 1) }
                GradientStop { position: 2.0/6.0; color: Qt.hsla(2.0/6.0, 1, picker.lightness, 1) }
                GradientStop { position: 3.0/6.0; color: Qt.hsla(3.0/6.0, 1, picker.lightness, 1) }
                GradientStop { position: 4.0/6.0; color: Qt.hsla(4.0/6.0, 1, picker.lightness, 1) }
                GradientStop { position: 5.0/6.0; color: Qt.hsla(5.0/6.0, 1, picker.lightness, 1) }
                GradientStop { position: 6.0/6.0; color: Qt.hsla(6.0/6.0, 1, picker.lightness, 1) }
            }
        }

        // Display the saturation
        Rectangle {
            anchors.fill: parent
            z: 1

            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.hsla(0, 0, picker.lightness, 0) }
                GradientStop { position: 1.0; color: Qt.hsla(0, 0, picker.lightness, 1) }
            }
        }

        // Marker
        Rectangle {
            id: hsMarker
            width: 5
            height: 5
            x: picker.hue * rectangle.width - 2
            y: rectangle.height * (1.0 - picker.saturation)

            color: "black"
            border {
                color: "white"
                width: 1
            }
        }
    }

    // Vertical bar that displays the lightness of the color
    MouseArea {
        id: bar
        width: 20
        height: 140
        anchors.left: rectangle.right
        anchors.leftMargin: 7

        onPositionChanged: {
            picker.lightness = 1 - mouse.y/height

            updateTimer.restart()
        }

        Rectangle {
            anchors.fill: parent

            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.hsla(picker.hue, picker.saturation, 1, 1) }
                GradientStop { position: 0.5; color: Qt.hsla(picker.hue, picker.saturation, 0.5, 1) }
                GradientStop { position: 1.0; color: Qt.hsla(picker.hue, picker.saturation, 0, 1) }
            }
        }
        Rectangle {
            id: vMarker
            width: 19
            height: 5
            y: bar.height * (1 - picker.lightness)

            color: "black"
            border {
                color: "white"
                width: 1
            }
        }
    }

    // Preview of the color
    Rectangle {
        id: preview
        anchors.left: bar.right
        anchors.verticalCenter: bar.verticalCenter
        anchors.leftMargin: 7
        width: 40
        height: 30
        color: picker.currentColor()
    }

    // Timer to update the value only 4 times per second
    Timer {
        id: updateTimer
        interval: 250
        repeat: false

        onTriggered: {
            picker.valueChanged('"' + currentColor().toString() + '"');
        }
    }
}
