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

StandardPage
{
    color: "#FFF3EC"
    
    Column {
        id: column1
        height: parent.height
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width/2
        anchors.margins: 30
        spacing: 50

        Link {
            text: i18n("New Project")
            onClicked: kdev.retrieveMenuAction("project/project_new").trigger()
        }

        Link {
            text: i18n("Import project")
            onClicked: ICore.projectController().openProject()
        }
    }

    ListView {
        id: projectsView
        anchors {
            left: column1.right
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        
        anchors.margins: 30

        delegate: Row {
                spacing: 10

                Link {
                    function justName(str) {
                        var idx = str.indexOf(" [")
                        
                        return str.substr(0, idx);
                    }
                    font.pixelSize: 15
                    width: projectsView.width
                    
                    text: justName(modelData["text"])
                    onClicked: modelData.trigger()
                }
            }

        model: kdev.recentProjects()
        spacing: 5
        
        header: Text {
            font.pixelSize: 25
            text: i18n("Recent Projects:")
        }
    }
}
