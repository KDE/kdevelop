/*
    SPDX-FileCopyrightText: 2011 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.15

import org.kdevelop.welcomepage 4.3

StandardPage
{
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20

        spacing: 20

        RowLayout {
            id: toolBar

            width: parent.width

            Button {
                icon.name: "project-development-new-template"
                text: i18n("New Project")
                onClicked: kdev.retrieveMenuAction("project/project_new").trigger()
            }

            Button {
                text: i18n("Open Project")
                icon.name: "project-open"
                onClicked: ICore.projectController.openProject()
            }

            Button {
                text: i18n("Fetch Project")
                icon.name: "edit-download"
                onClicked: kdev.retrieveMenuAction("project/project_fetch").trigger()
            }

            Button {
                icon.name: "document-open-recent"
                text: i18n("Recent Projects")
                onClicked: kdev.showMenu("project/project_open_recent")
            }
            Item {
                Layout.fillWidth: true
            }
        }

        Label {
            id: greetingLabel

            visible: !sessionsView.visible
            Layout.fillWidth: true
            Layout.fillHeight: true

            text: i18n("<h3>Welcome to KDevelop!</h3>\n" +
                "<p>You can start working on a project by opening an existing or creating a new one via the above buttons.</p>\n" +
                "<p>If you need help, please check out the <a href=\"https://userbase.kde.org/KDevelop\">User Manual.</a></p>") +

                (Qt.platform.os === "windows" ?
                    i18n("<br/>\n" +
                        "<h3>Note for Windows users</h3>\n" +
                        "<p>Note that KDevelop does NOT ship a C/C++ compiler on Windows!</p>\n" +
                        "<p>You need to install either GCC via MinGW or install a recent version of the Microsoft Visual Studio IDE and make sure the environment is setup correctly <i>before</i> starting KDevelop.</p>\n" +
                        "<p>If you need further assistance, please check out the <a href=\"https://userbase.kde.org/KDevelop4/Manual/WindowsSetup\">KDevelop under Windows instructions.</a></p>") :
                    "")
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)

            MouseArea {
                anchors.fill: parent

                acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }

        ListView {
            id: sessionsView

            Layout.fillHeight: true
            Layout.fillWidth: true

            visible: sessionsView.count > 1 // we always have at least one active session
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar { id: verticalScrollBar }

            delegate: Label {
                readonly property string projectNamesString: projectNames.join(", ").replace(/.kdev4/g, "")

                width: sessionsView.width - verticalScrollBar.width
                height: visible ? 30 : 0

                visible: projects.length > 0

                text: display == "" ? projectNamesString : i18n("%1: %2", display, projectNamesString)
                elide: Text.ElideRight
                opacity: labelMouseArea.containsMouse ? 0.8 : 1

                MouseArea {
                    id: labelMouseArea

                    anchors.fill: parent

                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: sessionsModel.loadSession(uuid)
                }
            }

            model: SessionsModel { id: sessionsModel }

            header: Heading {
                text: i18n("Sessions")
            }
        }
    }
}
