/* This file is part of KDevelop
    Copyright 2009  Radu Benea <radub82@gmail.com>

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

#include "genericmanagerlistjob.h"

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include <KDebug>

GenericManagerListJob::GenericManagerListJob(KDevelop::ProjectFolderItem* item, const bool forceRecursion)
    : KIO::Job(), m_item(0), m_forceRecursion(forceRecursion)
{
    /* the following line is not an error in judgement, apparently starting a
     * listJob while the previous one hasn't self-destructed takes a lot of time,
     * so we give the job a chance to selfdestruct first */
    connect( this, SIGNAL(nextJob()), SLOT(startNextJob()), Qt::QueuedConnection );

    addSubDir(item);
    startNextJob();
}

void GenericManagerListJob::addSubDir( KDevelop::ProjectFolderItem* item )
{
    m_listQueue.enqueue(item);
}

void GenericManagerListJob::slotEntries(KIO::Job* job, const KIO::UDSEntryList& entriesIn)
{
    Q_UNUSED(job);
    entryList.append(entriesIn);
}

void GenericManagerListJob::startNextJob()
{
    if ( m_listQueue.isEmpty() ) {
        return;
    }
    m_item = m_listQueue.dequeue();
    KIO::ListJob* job = KIO::listDir( m_item->url(), KIO::HideProgressInfo );
    connect( job, SIGNAL(entries(KIO::Job*, KIO::UDSEntryList)),
             this, SLOT(slotEntries(KIO::Job*, KIO::UDSEntryList)) );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)) );
}

void GenericManagerListJob::slotResult(KJob* job)
{
    emit entries(m_item, entryList, m_forceRecursion);
    entryList.clear();

    if( job->error() ) {
        kDebug() << "error in list job:" << job->error() << job->errorString();
    }

    if( m_listQueue.isEmpty() ) {
        emitResult();
    } else {
        emit nextJob();
    }
}

#include "genericmanagerlistjob.moc"
