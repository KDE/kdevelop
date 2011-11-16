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
         GradientStop { position: 0.0; color: "#21257C" }
         GradientStop { position: 1.0; color: "#62B6F1" }
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
        
        color: "#5BD3A1"
        border.color: "#1F2374"

//         anchors.fill: parent
        
        Column {
            anchors.margins: 30
            anchors.fill: parent
            spacing: 30
            
            Text {
                width: parent.width
                text: "On the <em>Debug</em> area you will be able to see and analyze how your program works on execution. "+
                      "On the <em>Run</em> menu you will find all the possible options.<p/><p/>"+
                      "As you can see, here you can just execute your application or debug it if you need "+
                      "further runtime information. You can select what is going to be run by configuring "+
                      "the launches and selecting the one you want to use in the <em>Current Launch Configuration</em> sub-menu."
                wrapMode: Text.WordWrap
            }
            Link { text: "Configure a new Target"; onClicked: kdev.retrieveMenuAction("run/configure_launches") }
            Link { text: "Execute"; onClicked: kdev.retrieveMenuAction("run/run_execute") }
            Link { text: "Debug"; onClicked: kdev.retrieveMenuAction("run/run_debug") }
        }
    }
}
