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

import QtQuick 1.1

Text {
    id: text
    signal clicked;
    
    horizontalAlignment: Text.AlignHCenter
    font.pointSize: 20

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onClicked: text.clicked();
        onEntered: text.state="hovered"
        onExited: text.state="default"
    }
    
    states: [
         State {
             name: "default"
             PropertyChanges { target: text; color: "black" }
         },
         State {
             name: "hovered"
             PropertyChanges { target: text; color: "red" }
         }
     ]
}