/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol Gonzalez <aleixpol@kde.org>                      *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

import QtQuick 2.1
import org.kde.kdevplatform 1.0
import org.kde.plasma.components 2.0
import org.kde.plasma.extras 2.0

Column
{
    Heading {
        anchors.horizontalCenter: parent.horizontalCenter
        text: i18n("Branches")
        level: 2
    }

    Repeater
    {
        width: parent.width
        model: ICore.projectController().projectCount()

        delegate: Column {
            id: del
            property variant project: ICore.projectController().projectAt(index)
            width: parent.width

            Heading {
                text: project.name()
                level: 3
            }

            Repeater {
                delegate: Button {
                    text: display
                    width: parent.width
                    enabled: !isCurrent
                    onClicked: branchesModel.currentBranch=display
                }

                model: BranchesListModel {
                    id: branchesModel
                    project: del.project
                }
            }
        }
    }
}
