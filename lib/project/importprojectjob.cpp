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
#include "importprojectthread.h"

#include "interfaces/iprojectfilemanager.h"
#include "projectmodel.h"

#include <kglobal.h>
#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>

namespace KDevelop
{

struct ImportProjectJobPrivate
{
    ThreadWeaver::Weaver *m_weaver;
    ImportProjectThread *m_weaverJob;
};

ImportProjectJob::ImportProjectJob(QStandardItem *folder, IProjectFileManager *importer)
    : KJob(0), d(new ImportProjectJobPrivate)

{
    d->m_weaver = new ThreadWeaver::Weaver( this );
    d->m_weaverJob = new ImportProjectThread( this );

    d->m_weaverJob->setProjectFileManager( importer );
    ProjectFolderItem *folderItem = 0;
    if ( folder->type() == ProjectBaseItem::Folder ||
         folder->type() == ProjectBaseItem::BuildFolder ||
         folder->type() == ProjectBaseItem::Project )
    {
        folderItem = dynamic_cast<ProjectFolderItem*>( folder );
    }
    d->m_weaverJob->setProjectFolderItem( folderItem );
}

ImportProjectJob::~ImportProjectJob()
{
    delete d;
}

void ImportProjectJob::start()
{
    if( !(d->m_weaverJob) )
        return;

    d->m_weaver->enqueue( d->m_weaverJob );
    connect( d->m_weaverJob, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(slotDone(ThreadWeaver::Job*)) );

//     d->m_weaverJob->start();
//     connect( d->m_weaverJob, SIGNAL(finished()), this, SLOT(slotFinished()) );
}

void ImportProjectJob::slotDone(ThreadWeaver::Job*)
{
    emitResult();
}

// void ImportProjectJob::slotFinished()
// {
//     emitResult();
// }

}
#include "importprojectjob.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
