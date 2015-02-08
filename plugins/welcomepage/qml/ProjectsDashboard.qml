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
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.2

import "plugins"

StandardPage {
    ScrollView {
        anchors.fill: parent
        GridView {
            id: grid
            width: parent.width
            cellWidth: grid.width/2
            cellHeight: grid.height/3

            model: [ "qrc:/qml/plugins/Branches.qml", "qrc:/qml/plugins/Projects.qml" ]
//          TODO: make this the plugin
            delegate: Loader {
                x: 5
                y: 5
                width: grid.cellWidth-5
                height: grid.cellHeight-5
                source: modelData
            }
        }
    }
}
