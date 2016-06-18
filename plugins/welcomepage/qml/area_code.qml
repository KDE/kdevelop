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

import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2

StandardBackground {
    id: root
    
    state: "develop"

    tools: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: parent.width/4
            Layout.maximumHeight: parent.width/4

            Image {
                id: icon
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter

                sourceSize {
                    width: icon.height
                    height: icon.height
                }

                source: "image://icon/kdevelop"
                smooth: true
                fillMode: Image.PreserveAspectFit
            }
            Label {
                Layout.fillWidth: true
                Layout.fillHeight: true
                verticalAlignment: Text.AlignVCenter
                text: "KDevelop"
                fontSizeMode: Text.Fit
                font {
                    pointSize: icon.height/3
                    weight: Font.ExtraLight
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        GroupBox {
            Layout.fillWidth: true
            flat: true

            ColumnLayout {
                Heading {
                    text: i18n("Need Help?")
                }

                Link {
                    text: i18n("KDevelop.org")
                    iconName: "applications-webbrowsers"
                    onClicked: {
                        Qt.openUrlExternally("https://kdevelop.org")
                    }
                }
                Link {
                    text: i18n("Learn about KDevelop")
                    iconName: "applications-webbrowsers"
                    onClicked: Qt.openUrlExternally("https://userbase.kde.org/KDevelop")
                }
                Link {
                    text: i18n("Join KDevelop's team!")
                    iconName: "applications-webbrowsers"
                    onClicked: Qt.openUrlExternally("https://techbase.kde.org/KDevelop5")
                }
                Link {
                    text: i18n("Handbook")
                    iconName: "applications-webbrowsers"
                    onClicked: kdev.retrieveMenuAction("help/help_contents").trigger()
                }
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
