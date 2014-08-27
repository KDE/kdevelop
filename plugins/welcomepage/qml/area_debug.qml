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
import org.kde.plasma.components 2.0

StandardBackground
{
    id: root
    gradient: Gradient {
         GradientStop { position: 0.0; color: "#21257C" }
         GradientStop { position: 1.0; color: "#62B6F1" }
    }
    tools: Link {
            id: goCode

            iconSource: "go-previous"
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
            
            ToolBar {
                tools: Flow {
                    Link { iconSource: "configure"; text: i18n("Configure a new Launcher"); onClicked: kdev.retrieveMenuAction("run/configure_launches").trigger() }
                    Link { iconSource: "audio-input-line"; text: i18n("Attach to Process"); onClicked: kdev.retrieveMenuAction("run/debug_attach").trigger() }
                    Link { iconSource: "debug-run"; text: i18n("Debug your program"); onClicked: kdev.retrieveMenuAction("run/run_debug").trigger() }
                }
            }
            Heading { text: i18n("Debug Area") }
            Label {
                width: parent.width
                text: i18n("On the <em>Debug</em> area you will be able to see and analyze how your program works on execution. "+
                      "On the <em>Run</em> menu you will find all the possible options.<br/><br/>"+
                      "As you can see, here you can just execute your application or debug it if you need "+
                      "further runtime information. You can select what is going to be run by configuring "+
                      "the launches and selecting the one you want to use in the <em>Current Launch Configuration</em> sub-menu.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
            }
        }
    }
}
