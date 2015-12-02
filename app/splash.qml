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

import QtQuick 2.1
import QtQuick.Layouts 1.1

// The background image for the splash is still a bitmap, provided by QSplashScreen.
// This widget has a transprent background and appears above that pixmap.
// Thus, if for some reason this doesn't work, the user still has a splash screen
// with a kdev logo and name.

Rectangle {
    id: root

    property int progress: 50
    property url appIcon
    property string appVersionMajor: "01"
    property string appVersionMinor: "23"
    property string appVersionPatch: "45"

    // color for the non-colored rectangles
    readonly property string defaultColor: "#3E3E3E"
    // amount of rectangles in each column
    readonly property var counts: [22, 20, 21, 19, 17, 19, 20, 17, 18, 15, 16, 15, 14, 16, 13, 11, 12, 10, 12,
                                   11, 8, 10, 6, 8, 9, 5, 7, 6, 4, 5, 6, 3, 5, 4, 2, 3, 2, 1, 2, 1]
    // "active" colors for the rectangles
    // only half as many colors, use each color twice.
    // TODO could try if it looks nicer with one color each
    readonly property var activeColors: ["#960A0A", "#B40F0F", "#FF4000", "#FF8400", "#FFDD00", "#CDEC00", "#99FF00", "#4CB700", "#1DB300", "#076813",
                                         "#08BA5B", "#00DA99", "#00B5E7", "#085BBB", "#2A5CFF", "#7044FF", "#9625FF", "#F013FF", "#FF2CC0", "#FF1A1D"]

    gradient: Gradient {
         GradientStop { position: 0.0; color: "#0A0A0A" }
         GradientStop { position: 0.5; color: "#1F1F1F" }
         GradientStop { position: 1.0; color: "#0A0A0A" }
    }

    width: centerWidget.width + mainLayout.anchors.margins * 2
    height: centerWidget.height + mainLayout.anchors.margins * 2

    // scanlines always look fancy
    ListView {
        id: background

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

    ColumnLayout {
        id: centerWidget

        ColumnLayout {
            id: mainLayout

            anchors.fill: parent
            anchors.margins: 15

            RowLayout {
                Text {
                    Layout.fillWidth: true

                    color: "white"
                    opacity: 0.65
                    text: "www.kdevelop.org"
                }

                Text {
                    color: "white"
                    opacity: 0.65
                    text: root.progress+"%"
                }
            }

            Item { height: 10 } // spacer

            // draw the rectangles
            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 4
                rotation: 180

                Repeater {
                    // draw one column per entry in the root.counts list
                    model: root.counts
                    // this delegate draws one column of rectangles
                    delegate: Column {
                        readonly property string color: (1-root.progress/100.0) * root.counts.length <= index ? activeColors[Math.floor(index/2)] : root.defaultColor
                        readonly property int count: root.counts[index]

                        opacity: 0.75
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
            }

            Item { height: 5 } // spacer

            RowLayout {
                Image {
                    width: 48
                    height: width
                    source: appIcon
                }

                Text {
                    Layout.fillWidth: true
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    font {
                        pointSize: 26
                        bold: true
                        family: "sans"
                    }
                    style: Text.Raised;
                    styleColor: "black"
                    text: "KDevelop " + appVersionMajor + "." + appVersionMinor +
                          "<font size=\"1\">." + appVersionPatch + "</span>"
                }
            }
        }
    }
}
