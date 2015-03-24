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

import QtQuick 1.1
import org.kde.kdevplatform 1.0
import org.kde.plasma.components 0.1
import org.kde.plasma.core 0.1 as PlasmaCore

ListView
{
	property real preferredHeight: 200
	property real preferredWidth: 200
	clip: true
	
	delegate: Button {
		text: display
		width: parent.width
		enabled: !isCurrent
		height: 40
		onClicked: branchesModel.currentBranch=display
	}
	
	ICore { id: core }
	
	model: BranchesListModel {
		id: branchesModel
		project: core.self.projectController().findProjectByName(dataSource.data[plasmoid.currentActivity]["projectName"])
	}
	
	PlasmaCore.DataSource {
		id: dataSource
		engine: "org.kdevelop.projects"
		connectedSources: [ plasmoid.currentActivity ]
	}
	
	Component.onCompleted: {
		var d = dataSource.data[plasmoid.currentActivity];
		var p = core.self.projectController().findProjectByName(d["projectName"])
	}
}
