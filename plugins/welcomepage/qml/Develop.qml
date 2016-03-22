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
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.3
import org.kdevelop.welcomepage 4.3

StandardPage
{
    RowLayout {
        id: toolBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 25
        }
        Link {
            iconName: "project-development-new-template"
            text: i18n("New Project")
            onClicked: kdev.retrieveMenuAction("project/project_new").trigger()
        }

        Link {
            text: i18n("Open Project")
            iconName: "project-development-open"
            onClicked: ICore.projectController().openProject()
        }

        Link {
            text: i18n("Fetch Project")
            iconName: "download"
            onClicked: kdev.retrieveMenuAction("project/project_fetch").trigger()
        }

        Link {
            iconName: "document-open-recent"
            text: i18n("Recent Projects")
            onClicked: kdev.showMenu("project/project_open_recent")
        }
        Item {
            Layout.fillWidth: true
        }
    }

    ScrollView {
        anchors {
            left: parent.left
            top: toolBar.bottom
            bottom: parent.bottom
            right: parent.right
            bottomMargin: 30
            leftMargin: 30
            rightMargin: 30
        }
        ListView {
            id: sessionsView
            clip: true

            delegate: MouseArea {
                        width: sessionsView.width
                        height: visible ? 30 : 0
                        visible: projects.length > 0
                        onClicked: sessions.loadSession(uuid)
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        Label {
                            width: parent.width
                            readonly property string projectNamesString: projectNames.join(", ").replace(/.kdev4/g, "")
                            text: display=="" ? projectNamesString : i18n("%1: %2", display, projectNamesString)
                            elide: Text.ElideRight
                            opacity: parent.containsMouse ? 0.8 : 1
                        }
                    }

            model: SessionsModel { id: sessions }

            header: Heading {
                height: 1.25*implicitHeight
                text: i18n("Sessions")
            }
        }
    }
}
