/* This file is part of KDevelop
    Copyright (C) 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#include "importprojectthread.h"
#include "projectmodel.h"
#include "interfaces/iprojectfilemanager.h"
#include "kdebug.h"
#include <QQueue>

namespace KDevelop
{

struct ImportProjectThreadPrivate
{
    ProjectFolderItem *m_folder;
    IProjectFileManager *m_importer;
    QList<ProjectFolderItem*> m_workingList;
};

ImportProjectThread::ImportProjectThread( QObject *parent )
    : ThreadWeaver::Job( parent )
//     : QThread(parent)
    , d( new ImportProjectThreadPrivate )
{}

ImportProjectThread::~ImportProjectThread()
{
    delete d;
}

void ImportProjectThread::setProjectFolderItem( ProjectFolderItem *folder )
{
    d->m_folder = folder;
}
void ImportProjectThread::setProjectFileManager( IProjectFileManager *importer )
{
    d->m_importer = importer;
}

void ImportProjectThread::run()
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

    kDebug() << "ImportProjectThread::run() returning" << endl;
}

}

#include "importprojectthread.moc"
