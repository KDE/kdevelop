/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

StandardBackground
{
    id: root
    tools: Link {
            id: goCode

            iconName: "go-previous"
            text: i18n("Back to code")
            onClicked: kdev.setArea("code")
        }
    pageIcon: "applications-engineering"

    StandardPage {
        id: startingPage
        anchors {
            fill: parent
            leftMargin: root.marginLeft+root.margins
            margins: root.margins
        }
        
        Column {
            anchors.margins: 30
            anchors.fill: parent
            spacing: 30
            
            Item {
                tools: Flow {
                    Link {
                        iconName: "kompare"
                        text: i18n("Review a Patch")
                        onClicked: {
                            kdev.raiseToolView("EditPatch")
                        }
                    }
                }
            }
            
            Heading { text: i18n("Review Area") }
            
            Label {
                width: parent.width
                text: i18n("On the <em>Review</em> area you will be able to find the tools you need to review changes in your projects, either the ones you made or some external patch.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
            }
            Label {
                width: parent.width
                text: i18n("Also it will help you send the changes to the community you're contributing to, either by committing the changes, sending them by e-mail or putting them on a ReviewBoard service.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
            }
        }
    }
}
