/***************************************************************************
 *   Copyright 2013 Sven Brauch <svenbrauch@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

import QtQuick 1.1

// The background image for the splash is still a bitmap, provided by QSplashScreen.
// This widget has a transprent background and appears above that pixmap.
// Thus, if for some reason this doesn't work, the user still has a splash screen
// with a kdev logo and name.

Rectangle {
    id: root
    // this property is updated from C++
    property int progress: 0
    // color for the non-colored rectangles
    property string defaultColor: "#3E3E3E"
    // amount of rectangles in each column
    property variant counts: [22, 20, 21, 19, 17, 19, 20, 17, 18, 15, 16, 15, 14, 16, 13, 11, 12, 10, 12,
                              11, 8, 10, 6, 8, 9, 5, 7, 6, 4, 5, 6, 3, 5, 4, 2, 3, 2, 1, 2, 1]
    // "active" colors for the rectangles
    // only half as many colors, use each color twice.
    // TODO could try if it looks nicer with one color each
    property variant activeColors: ["#B70F0F", "#FF1616", "#FF2727", "#FF3300", "#FF7B00", "#FFCC00", "#CCFF00", "#99FF00", "#1DB300", "#076813",
                                    "#08BA5B", "#00DA99", "#00B5E7", "#085BBB", "#2A5CFF", "#7044FF", "#9625FF", "#F013FF", "#FF2CC0", "#FF1A1D"]
    // this size is a fallback, it's scaled in the c++ code actually,
    // but this is the real splash size, so it's useful for testing in qmlviewer
    width: 475
    height: 301
    anchors.fill: parent
    color: "#00FFFFFF"
    // scanlines always look fancy
    ListView {
        anchors.fill: parent
        model: Math.floor(parent.height / 2)
        spacing: 2
        delegate: Rectangle {
            width: root.width
            height: 1
            color: "#555555"
            opacity: 0.10
        }
    }
    // draw the rectangles
    ListView {
        x: -40
        y: -52
        height: root.height
        width: root.width
        // this is needed to make the columns fill the widget from the bottom upwards
        rotation: 180
        spacing: 4
        // draw one column per entry in the root.counts list
        model: root.counts
        orientation: ListView.Horizontal
        // this delegate draws one column of rectangles
        delegate: Column {
            opacity: 0.75
            property string color
            property int count
            color: (1-root.progress/100.0) * root.counts.length <= index ? activeColors[Math.floor(index/2)] : root.defaultColor
            count: root.counts[index]
            x: 6
            y: 12
            spacing: 4
            Repeater {
                model: parent.count
                delegate: Rectangle {
                    color: parent.color
                    width: 6
                    height: 6
                }
            }
        }
    }
    // this is, quite obvioulsy, the text in the upper right corner
    Text {
        anchors.margins: 6
        anchors.left: parent.left
        anchors.top: parent.top
        color: "white"
        opacity: 0.65
        text: "KDevelop Integrated Development Environment - http://kdevelop.org"
        font.pointSize: 7
    }
}