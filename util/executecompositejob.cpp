/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

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

#include "executecompositejob.h"
#include "debug.h"

namespace KDevelop
{
class ExecuteCompositeJobPrivate
{
public:
    bool m_killing;
};

ExecuteCompositeJob::ExecuteCompositeJob(QObject* parent, const QList<KJob*>& jobs)
: KCompositeJob(parent), d(new ExecuteCompositeJobPrivate)
{
    d->m_killing = false;
    setCapabilities(Killable);

    qCDebug(UTIL) << "execute composite" << jobs;
    foreach(KJob* job, jobs) {
        addSubjob(job);
        if (objectName().isEmpty()) setObjectName(job->objectName());
    }
}

ExecuteCompositeJob::~ExecuteCompositeJob()
{
    delete d;
}

void ExecuteCompositeJob::start()
{
    if(hasSubjobs()) {
        auto first = subjobs().first();
        qCDebug(UTIL) << "starting:" << first;
        first->start();
    } else {
        emitResult();
    }
}

void ExecuteCompositeJob::slotResult(KJob* job)
{
    qCDebug(UTIL) << "finished: "<< job;
    if (job->error()) {
        qCDebug(UTIL) << "JOB ERROR:" << job->error() << job->errorString();
    }

    KCompositeJob::slotResult(job);

    if (hasSubjobs() && !error() && !d->m_killing) {
        qCDebug(UTIL) << "remaining: " << subjobs().count() << subjobs();
        KJob* nextJob = subjobs().first();
        qCDebug(UTIL) << "starting:" << nextJob;
        nextJob->start();
    } else {
        setError(job->error());
        setErrorText(job->errorString());
        emitResult();
    }
}

bool ExecuteCompositeJob::doKill()
{
    qDebug() << "Killing subjobs:" << subjobs().size();
    d->m_killing = true;
    while(hasSubjobs()) {
        KJob* j = subjobs().first();
        if( !j ) {
            removeSubjob(j);
            continue;
        }
        if (j->kill()) {
            removeSubjob(j);
        } else {
            return false;
        }
    }
    return true;
}

}

