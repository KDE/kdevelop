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
#include "projectmodel.h"

#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>


#include <kglobal.h>
#include <kdebug.h>

#include <interfaces/iprojectfilemanager.h>

namespace KDevelop
{

class ImportProjectJobPrivate
{
public:
    ImportProjectJobPrivate( ImportProjectJob* job ) : q(job) {}
    ProjectFolderItem *m_folder;
    IProjectFileManager *m_importer;
    ImportProjectJob* q;

    void import(ProjectFolderItem* folder)
    {
        QList<KDevelop::ProjectFolderItem*> subFolders = m_importer->parse(folder);
        foreach(KDevelop::ProjectFolderItem* sub, subFolders)
        {
            import(sub);
        }  
    }


};

ImportProjectJob::ImportProjectJob(QStandardItem *folder, IProjectFileManager *importer)
    : KJob(0), d(new ImportProjectJobPrivate( this ) )

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
    QFutureWatcher<void>* watcher = new QFutureWatcher<void>();
    connect(watcher, SIGNAL(finished()), SLOT(importDone()));
    QFuture<void> f = QtConcurrent::run(d, &ImportProjectJobPrivate::import, d->m_folder);
    watcher->setFuture(f);
}

void ImportProjectJob::importDone()
{
    if ( sender() )
        sender()->deleteLater(); /* Goodbye to the QFutureWatcher */

    emitResult();
}


}
#include "importprojectjob.moc"

