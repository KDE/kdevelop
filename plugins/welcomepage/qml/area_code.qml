/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

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

 /** FIXME: News section is currently unmaintained, also unconditional ping-back to KDE violating privacy
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
*/

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
