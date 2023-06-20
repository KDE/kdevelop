/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executecompositejob.h"
#include "debug.h"

namespace KDevelop {
class ExecuteCompositeJobPrivate
{
public:
    void startNextJob(KJob* job);

    bool m_killing = false;
    bool m_abortOnError = true;

    int m_jobIndex = -1;
    int m_jobCount = 0;
};

ExecuteCompositeJob::ExecuteCompositeJob(QObject* parent, const QList<KJob*>& jobs)
    : KCompositeJob(parent)
    , d_ptr(new ExecuteCompositeJobPrivate)
{
    setCapabilities(Killable);

    qCDebug(UTIL) << "execute composite" << jobs;
    for (KJob* job : jobs) {
        if (!job) {
            qCWarning(UTIL) << "Added null job!";
            continue;
        }
        addSubjob(job);
        if (objectName().isEmpty())
            setObjectName(job->objectName());
    }
}

ExecuteCompositeJob::~ExecuteCompositeJob() = default;

void ExecuteCompositeJobPrivate::startNextJob(KJob* job)
{
    ++m_jobIndex;

    qCDebug(UTIL) << "starting:" << job;
    job->start();
}

void ExecuteCompositeJob::start()
{
    Q_D(ExecuteCompositeJob);

    if (hasSubjobs()) {
        d->startNextJob(subjobs().first());
    } else {
        emitResult();
    }
}

bool ExecuteCompositeJob::addSubjob(KJob* job)
{
    Q_D(ExecuteCompositeJob);

    const bool success = KCompositeJob::addSubjob(job);
    if (!success)
        return false;

    ++d->m_jobCount;

    connect(job, &KJob::percentChanged,
            this, &ExecuteCompositeJob::slotPercent);
    return true;
}

void ExecuteCompositeJob::slotPercent(KJob* job, unsigned long percent)
{
    Q_D(ExecuteCompositeJob);

    Q_UNUSED(job);

    Q_ASSERT(d->m_jobCount > 0);
    Q_ASSERT(d->m_jobIndex >= 0 && d->m_jobIndex < d->m_jobCount);

    const float ratio = ( float )d->m_jobIndex / d->m_jobCount;
    const unsigned long totalPercent = ratio * 100 + (( float )percent / d->m_jobCount);

    emitPercent(totalPercent, 100);
}

void ExecuteCompositeJob::slotResult(KJob* job)
{
    Q_D(ExecuteCompositeJob);

    disconnect(job, &KJob::percentChanged,
               this, &ExecuteCompositeJob::slotPercent);

    // jobIndex + 1 because this job just finished
    const float ratio = d->m_jobIndex != -1 ? (d->m_jobIndex + 1.0) / d->m_jobCount : 1.0;
    emitPercent(ratio * 100, 100);

    qCDebug(UTIL) << "finished: " << job << job->error() << "percent:" << ratio * 100;
    bool emitDone = false;
    if (d->m_abortOnError && job->error()) {
        qCDebug(UTIL) << "JOB ERROR:" << job->error() << job->errorString();
        KCompositeJob::slotResult(job); // calls emitResult()
        emitDone = true;
    } else
        removeSubjob(job);

    if (hasSubjobs() && !error() && !d->m_killing) {
        qCDebug(UTIL) << "remaining: " << subjobs().count() << subjobs();
        d->startNextJob(subjobs().first());
    } else if (!emitDone) {
        setError(job->error());
        setErrorText(job->errorString());
        emitResult();
    }
}

bool ExecuteCompositeJob::doKill()
{
    Q_D(ExecuteCompositeJob);

    qCDebug(UTIL) << "Killing subjobs:" << subjobs().size();
    d->m_killing = true;
    while (hasSubjobs()) {
        KJob* j = subjobs().first();

        if (!j || j->kill()) {
            removeSubjob(j);
        } else {
            return false;
        }
    }
    return true;
}

void ExecuteCompositeJob::setAbortOnError(bool abort)
{
    Q_D(ExecuteCompositeJob);

    d->m_abortOnError = abort;
}

}

#include "moc_executecompositejob.cpp"
