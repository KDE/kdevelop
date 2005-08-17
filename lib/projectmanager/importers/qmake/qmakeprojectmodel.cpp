/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "qmakeprojectmodel.h"

#include <qmakeast.h>

//QMakeFolderModel

QMakeFolderModel::QMakeFolderModel(ProjectModel* projectModel)
    :ProjectFolderModel(projectModel), ast(0)
{
}

QMakeFolderModel::~QMakeFolderModel()
{
//     delete ast;
}


//QMakeFileModel

QMakeFileModel::QMakeFileModel(ProjectModel *projectModel)
    :ProjectFileModel(projectModel)
{
}


//QMakeTargetModel

QMakeTargetModel::QMakeTargetModel(ProjectModel *projectModel)
    :ProjectTargetModel(projectModel), ast(0)
{
}

QMakeTargetModel::~QMakeTargetModel()
{
//     delete ast;
}
