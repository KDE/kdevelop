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

#include <kdebug.h>

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

    qDebug() << "execute composite" << jobs;
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
    if(hasSubjobs())
        subjobs().first()->start();
    else
        emitResult();
}

void ExecuteCompositeJob::slotResult(KJob* job)
{
    kDebug() << "finished: "<< job << job->error() << error();
    KCompositeJob::slotResult(job);

    if(hasSubjobs() && !error() && !d->m_killing)
    {
        kDebug() << "remaining: " << subjobs().count() << subjobs();
        KJob* nextJob=subjobs().first();
        nextJob->start();
    } else {
        setError(job->error());
        setErrorText(job->errorString());
        emitResult();
    }
}

bool ExecuteCompositeJob::doKill()
{
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

bool ExecuteCompositeJob::addSubjob(KJob* job)
{
    // work-around for kdelibs bug: https://bugs.kde.org/show_bug.cgi?id=230692
    // (fixed in 4.11)
    // TODO: remove this method overwrite as soon as we depend on kdelibs 4.11
    bool success = KCompositeJob::addSubjob(job);
    // if job's parent != this => we're using kdelibs <= 4.10
    if (job->parent() != this) {
        // set the parent to 0, so the original parent won't delete this job
        // KCompositeJob takes care of deletion
        job->setParent(0);
    }
    return success;
}

const QList< KJob* >& ExecuteCompositeJob::subjobs() const
{
    return KCompositeJob::subjobs();
}

}

#include "executecompositejob.moc"
