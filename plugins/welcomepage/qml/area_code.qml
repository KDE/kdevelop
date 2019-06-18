/* KDevelop
 *
 * Copyright 2011 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.6

StandardBackground {
    id: root
    
    state: "develop"

    tools: ColumnLayout {
        spacing: 10

        Row {
            Layout.fillWidth: true
            spacing: 5

            Image {
                id: icon

                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter

                source: "image://icon/kdevelop"
                smooth: true
                fillMode: Image.PreserveAspectFit
            }
            Label {
                verticalAlignment: Text.AlignVCenter
                height: icon.height
                text: "KDevelop"
                font {
                    pointSize: 20
                    weight: Font.ExtraLight
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Heading {
            id: newsHeading

            Layout.fillWidth: true
            text: i18n("News")
        }

        NewsFeed {
            id: newsFeed

            readonly property int maxEntries: 3

            Layout.fillWidth: true
            Layout.minimumHeight: !loading ? (Math.min(count, maxEntries) * 40) : 40

            Behavior on Layout.minimumHeight { PropertyAnimation {} }
        }

        // add some spacing
        Item {
            Layout.fillWidth: true
            height: 10
        }

        Heading {
            text: i18n("Need Help?")
        }

        Column {
            spacing: 10

            Link {
                x: 10
                text: i18n("KDevelop.org")
                onClicked: {
                    Qt.openUrlExternally("https://kdevelop.org")
                }
            }
            Link {
                x: 10
                text: i18n("Learn about KDevelop")
                onClicked: Qt.openUrlExternally("https://userbase.kde.org/KDevelop")
            }
            Link {
                x: 10
                text: i18n("Join KDevelop's team!")
                onClicked: Qt.openUrlExternally("https://kdevelop.org/contribute-kdevelop")
            }
            Link {
                x: 10
                text: i18n("Handbook")
                onClicked: kdev.retrieveMenuAction("help/help_contents").trigger()
            }
        }
    }

    Develop {
        anchors {
            fill: parent
            leftMargin: root.marginLeft+root.margins
        }
    }
}
