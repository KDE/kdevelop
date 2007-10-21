/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#include "projectmodel.h"

#include <kglobal.h>
#include <kdebug.h>
#include <QQueue>

namespace KDevelop
{

struct ImportProjectJobPrivate
{
    ProjectFolderItem *m_folder;
    IProjectFileManager *m_importer;
};

ImportProjectJob::ImportProjectJob(QStandardItem *folder, IProjectFileManager *importer)
    : KJob(0), d(new ImportProjectJobPrivate)

{
    d->m_importer = importer;
    ProjectFolderItem *folderItem = 0;
    if ( folder->type() == ProjectBaseItem::Folder ||
         folder->type() == ProjectBaseItem::BuildFolder)
    {
        folderItem = dynamic_cast<ProjectFolderItem*>( folder );
    }
    d->m_folder = folderItem;
}

ImportProjectJob::~ImportProjectJob()
{
    delete d;
}

void ImportProjectJob::start()
{
    QQueue< QList<KDevelop::ProjectFolderItem*> > workQueue;
    QList<KDevelop::ProjectFolderItem*> initial;
    initial.append( d->m_folder );
    workQueue.enqueue( initial );

    while( workQueue.count() > 0 )
    {
        QList<KDevelop::ProjectFolderItem*> front = workQueue.dequeue();
        Q_FOREACH( KDevelop::ProjectFolderItem* _item, front )
        {
            QList<KDevelop::ProjectFolderItem*> workingList = d->m_importer->parse( _item );
            if( workingList.count() > 0 )
                workQueue.enqueue( workingList );
        }
    }

    emitResult();
    kDebug(9503) << "ImportProjectThread::run() returning";
}

}
#include "importprojectjob.moc"

