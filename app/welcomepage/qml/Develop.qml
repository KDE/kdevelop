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
import org.kde.plasma.core 0.1 as PlasmaCore
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
        tools: Flow {
            Link {
                iconSource: "project-development-new-template"
                text: i18n("New Project")
                onClicked: kdev.retrieveMenuAction("project/project_new").trigger()
            }

            Link {
                iconSource: "project-development-open"
                text: i18n("Open project")
                onClicked: ICore.projectController().openProject()
            }

            Link {
                iconSource: "download"
                text: i18n("Fetch project")
                onClicked: kdev.retrieveMenuAction("project/project_fetch").trigger()
            }
            
            Link {
                iconSource: "document-open-recent"
                text: i18n("Recent Projects")
                onClicked: kdev.showMenu("project/project_open_recent")
            }
        }
    }

    ListView {
        id: sessionsView
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        anchors {
            left: parent.left
            top: toolBar.bottom
            bottom: parent.bottom
            right: parent.right
            bottomMargin: 30
            leftMargin: 30
            rightMargin: 30
        }

        delegate: ListItem {
                    width: sessionsView.width
                    height: visible ? 30 : 0
                    visible: projects.length>0
                    onClicked: sessions.loadSession(uuid)
                    enabled: true
                    
                    Label {
                        width: parent.width
                        text: (display=="" ?
                                    projectNames.join(", ").replace(/.kdev4/g, "")
                                  :
                                  i18n("%1: %2", display, projectNames.join(", ").replace(/.kdev4/g, "")))
                        elide: Text.ElideRight
                    }
                }

        model: PlasmaCore.SortFilterModel {
            sourceModel: SessionsModel { id: sessions }
            sortRole: "identifier"
            sortOrder: Qt.AscendingOrder
        }
        
        header: Heading {
            text: i18n("Sessions")
            height: implicitHeight*2
        }
    }
}
