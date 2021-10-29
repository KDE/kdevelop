/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.kdevplatform 1.0
import org.kde.plasma.extras 2.0

Column
{
    Heading {
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Projects")
        level: 2
    }

    Repeater
    {
        width: parent.width
        model: ICore.projectController.projectCount()

        delegate: Column {
            id: del
            property variant project: ICore.projectController.projectAt(index)
            width: parent.width

            Heading {
                text: project.name()
                level: 3
            }
        }
    }
}
