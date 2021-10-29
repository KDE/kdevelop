/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

StandardBackground
{
    id: root
    tools: Link {
            id: goCode

            iconName: "go-previous"
            text: i18n("Back to code")
            onClicked: kdev.setArea("code")
        }
    pageIcon: "tools-report-bug"

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
            
            RowLayout {
                Link { iconName: "configure"; text: i18n("Configure a new Launcher"); onClicked: kdev.retrieveMenuAction("run/configure_launches").trigger() }
                Link { iconName: "audio-input-line"; text: i18n("Attach to Process"); onClicked: kdev.retrieveMenuAction("run/debug_attach").trigger() }
                Link { iconName: "debug-run"; text: i18n("Debug your program"); onClicked: kdev.retrieveMenuAction("run/run_debug").trigger() }
            }
            Heading { text: i18n("Debug Area") }
            Label {
                width: parent.width
                text: i18n("On the <em>Debug</em> area you will be able to see and analyze how your program works on execution. On the <em>Run</em> menu you will find all the possible options.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
            }
            Label {
                width: parent.width
                text: i18n("As you can see, here you can just execute your application or debug it if you need further runtime information. You can select what is going to be run by configuring the launches and selecting the one you want to use in the <em>Current Launch Configuration</em> sub-menu.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
            }
        }
    }
}
