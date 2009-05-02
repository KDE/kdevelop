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
    }
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
        emitResult();
    }
}
        
bool ExecuteCompositeJob::doKill()
{
    d->m_killing = true;
    if(hasSubjobs()) {
        KJob* j = subjobs().first();
        if (j->kill()) {
            removeSubjob(j);
            return true;
        } else {
            return false;
        }
    }
    return true;
}

}

#include "executecompositejob.moc"
