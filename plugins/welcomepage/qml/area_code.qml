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

StandardBackground {
    id: root
    
    state: "develop"

    tools: ColumnLayout {
        spacing: 10

        Link {
            Layout.fillWidth: true
            iconName: "applications-development"
            text: i18n("Develop")
            onClicked: root.state = "develop"
        }
        Link {
            Layout.fillWidth: true
            iconName: "project-development"
            text: i18n("Projects")
            onClicked: root.state = "projects"
            visible: false //FIXME: removed until it makes sense
        }
        Link {
            Layout.fillWidth: true
            iconName: "help-contents"
            text: i18n("Getting Started")
            onClicked: root.state = "gettingstarted"
        }
    }

    Loader {
        id: codeContents
        anchors {
            fill: parent
            leftMargin: root.marginLeft+root.margins
            margins: root.margins
        }
    }
    states: [
        State { name: "gettingstarted"
            PropertyChanges { target: codeContents; source: "qrc:/qml/GettingStarted.qml"}
        },
        State { name: "develop"
            PropertyChanges { target: codeContents; source: "qrc:/qml/Develop.qml"}
        },
        State { name: "projects"
            PropertyChanges { target: codeContents; source: "qrc:/qml/ProjectsDashboard.qml"}
        }
    ]
}
