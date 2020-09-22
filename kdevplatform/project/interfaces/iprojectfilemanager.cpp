/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Matt Rogers <mattr@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "iprojectfilemanager.h"
#include "projectmodel.h"
#include "importprojectjob.h"

namespace KDevelop
{
IProjectFileManager::~IProjectFileManager()
{
}

// KJob* IProjectFileManager::createImportJob(ProjectFolderItem* item)
// {
//     return new ImportProjectJob( item, this );
// }


}
