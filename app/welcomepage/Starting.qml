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
import org.kdevelop.welcomepage 4.3

StandardPage
{
    ToolBar {
        id: toolBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 25
        }
        tools: Row {
            spacing: 50

            Link {
                iconSource: "project-development-new-template"
                text: i18n("New Project")
                onClicked: kdev.retrieveMenuAction("project/project_new").trigger()
            }

            Link {
                iconSource: "project-development-open"
                text: i18n("Import project")
                onClicked: ICore.projectController().openProject()
            }
        }
    }

    ListView {
        id: sessionsView
        anchors {
            left: parent.left
            top: toolBar.bottom
            bottom: parent.bottom
            right: projectsView.left
            margins: 30
        }

        delegate: ListItem {
                    width: sessionsView.width
                    height: visible ? 30 : 0
                    visible: projects.length>0
                    onClicked: sessions.loadSession(uuid)
                    enabled: true
                    
                    Label {
                        width: parent.width
                        text: (display=="" ? projectNames.join(", ") : i18n("%1: %2", display, projectNames.join(", ")))
                        elide: Text.ElideRight
                    }
                }

        model: SessionsModel { id: sessions }
        
        header: Text {
            font.pixelSize: 25
            text: i18n("Sessions:")
        }
    }

    ListView {
        id: projectsView
        anchors {
            top: toolBar.bottom
            bottom: parent.bottom
            right: parent.right
            margins: 30
        }
        
        width: parent.width/3

        delegate: ListItem {
                    enabled: true
                    function justName(str) {
                        var idx = str.indexOf(" [")
                        
                        return str.substr(0, idx);
                    }
                    width: projectsView.width
                    height: 30
                    
                    Label {
                        anchors.fill: parent
                        font.pixelSize: 15
                        text: justName(modelData["text"])
                    }
                    onClicked: modelData.trigger()
                }

        model: kdev.recentProjects()
        
        header: Text {
            font.pixelSize: 25
            text: i18n("Recent Projects:")
        }
    }
}
