/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iprojectfilemanager.h"
#include "projectmodel.h"
#include "importprojectjob.h"

namespace KDevelop
{
IProjectFileManager::~IProjectFileManager()
{
}

KJob* IProjectFileManager::createImportJob(ProjectFolderItem* item)
{
    return new ImportProjectJob( item, this );
}


}
