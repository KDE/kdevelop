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

import QtQuick 1.0

StandardBackground
{
    id: root
    gradient: Gradient {
         GradientStop { position: 0.0; color: "#B85B5B" }
         GradientStop { position: 1.0; color: "#EEBABA" }
    }
    showGoCode: true
    pageIcon: "applications-engineering"

    StandardPage {
        id: startingPage
        anchors {
            fill: parent
            leftMargin: 256
            margins: 10
        }
        
        Column {
            anchors.margins: 30
            anchors.fill: parent
            spacing: 30
            
            Text {
                width: parent.width
                text: i18n("On the <em>Review</em> area you will be able to find the tools you need "+
                      "to review changes in your projects, either the ones you made or some external patch. ")
                wrapMode: Text.WordWrap
                
            }
            
            Link {
                text: i18n("Review a Patch")
                onClicked: {
                    kdev.raiseToolView("EditPatch")
                }
            }
        }
    }
}
