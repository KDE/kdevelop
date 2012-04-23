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
import org.kde.plasma.components 0.1

Rectangle {
    id: root
    gradient: Gradient {
         GradientStop { position: 0.0; color: "#61B056" }
         GradientStop { position: 1.0; color: "#A3D69B" }
    }
    
    Component.onCompleted: root.state="starting"

    ButtonColumn {
        id: tabs
        width: 250
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.margins: 30
        spacing: 30

        Link {
            width: parent.width

            text: i18n("Starting")
            onClicked: root.state = "starting"
        }

        Link {
            width: parent.width

            text: i18n("Learn KDevelop!")
            onClicked: root.state = "learning"
        }
    }

    Item {
        id: item1
        x: tabs.width+tabs.x
        height: parent.height
        width: parent.width-x

        Starting {
            id: startingPage
            
            anchors.fill: parent
            visible: root.state=="starting"
        }

        UseKDevelop {
            id: usePage

            anchors.fill: parent
            visible: root.state=="learning"
        }
    }
    states: [
        State { name: "learning" },
        State { name: "starting" }
    ]
}
