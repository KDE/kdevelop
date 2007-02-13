/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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

#include "importprojectjob.h"

#include "interfaces/iprojectfilemanager.h"

#include <kglobal.h>
#include <kdebug.h>

namespace KDevelop
{

struct ImportProjectJobPrivate
{
    ProjectFolderItem *m_folder;
    IProjectFileManager *m_importer;
    QList<ProjectFolderItem*> m_workingList;
};

ImportProjectJob::ImportProjectJob(QStandardItem *folder, IProjectFileManager *importer)
    : KJob(0), d(new ImportProjectJobPrivate)

{
    d->m_importer = importer;
    d->m_folder = 0;
    if ( folder->type() == ProjectBaseItem::Folder ||
         folder->type() == ProjectBaseItem::BuildFolder ||
         folder->type() == ProjectBaseItem::Project )
    {
        d->m_folder = dynamic_cast<ProjectFolderItem*>( folder );
    }
}

ImportProjectJob::~ImportProjectJob()
{
    delete d;
}

void ImportProjectJob::start()
{
    if ( d->m_folder )
        startNextJob( d->m_folder );

    emitResult();
}

void ImportProjectJob::startNextJob(ProjectFolderItem *dom)
{
    d->m_workingList += d->m_importer->parse(dom);
    while ( !d->m_workingList.isEmpty() )
    {
      ProjectFolderItem *folder = d->m_workingList.first();
      d->m_workingList.pop_front();

      startNextJob(folder);
    }
}

void ImportProjectJob::processList()
{
}

}
#include "importprojectjob.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
