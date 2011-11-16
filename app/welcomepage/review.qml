/* KDevelop CMake Support
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

Rectangle
{
    id: root
    gradient: Gradient {
         GradientStop { position: 0.0; color: "#B85B5B" }
         GradientStop { position: 1.0; color: "#EEBABA" }
    }
    
    Column {
        id: tabs
        width: 250
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 30
        spacing: 30

        Link {
            width: parent.width

            text: "Back to code"
            onClicked: kdev.setArea("code")
        }
    }

    StandardPage {
        id: startingPage
        anchors {
            left: tabs.right
            right: root.right
            top: root.top
            bottom: root.bottom
            margins: 10
        }
        
        color: "#F1D7D7"
        border.color: "#66191A"

//         anchors.fill: parent
        
        Column {
            anchors.margins: 30
            anchors.fill: parent
            spacing: 30
            
            Text { text: "Area explaination..." }
            
            Link {
                text: "Import a patch from URL"
                onClicked: {
                    console.log("fefefe "+ICore.pluginController().loadPlugin)
                    var p = ICore.pluginController().loadPlugin("kdevpatchreview")
                    console.log("fififi "+p)
                    console.log("lalala "+p.objectName())
                }
            }
        }
    }
}
