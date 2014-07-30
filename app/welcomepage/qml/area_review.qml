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
         GradientStop { position: 0.0; color: "#B85B5B" }
         GradientStop { position: 1.0; color: "#EEBABA" }
    }
    tools: Link {
            id: goCode

            iconSource: "go-previous"
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
            
            ToolBar {
                tools: Flow {
                    Link {
                        iconSource: "kompare"
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
                text: i18n("On the <em>Review</em> area you will be able to find the tools you need "+
                      "to review changes in your projects, either the ones you made or some external patch.<br/> Also it will help you send "+
                      "the changes to the community you're contributing to, either by committing the changes, sending them by e-mail or "+
                      "putting them on a ReviewBoard service.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
            }
        }
    }
}
