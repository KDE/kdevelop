/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ksequentialcompoundjob.h"
#include "ksequentialcompoundjob_p.h"

#include "debug.h"

using namespace KDevCoreAddons;

KSequentialCompoundJobPrivate::KSequentialCompoundJobPrivate() = default;
KSequentialCompoundJobPrivate::~KSequentialCompoundJobPrivate() = default;

bool KSequentialCompoundJobPrivate::isCurrentlyRunningSubjob(KJob *job) const
{
    return m_jobIndex >= 0 && !m_subjobs.empty() && job == m_subjobs.constFirst();
}

void KSequentialCompoundJobPrivate::startNextSubjob()
{
    ++m_jobIndex;
    Q_ASSERT(!m_subjobs.empty());
    auto *const job = m_subjobs.front();

    qCDebug(UTIL) << "starting subjob" << m_jobIndex + 1 << "of" << m_jobCount << ':' << job;
    job->start();
}

KSequentialCompoundJob::KSequentialCompoundJob(QObject *parent)
    : KSequentialCompoundJob(*new KSequentialCompoundJobPrivate, parent)
{
}

KSequentialCompoundJob::KSequentialCompoundJob(KSequentialCompoundJobPrivate &dd, QObject *parent)
    : KCompoundJob(dd, parent)
{
    setCapabilities(Killable);
}

KSequentialCompoundJob::~KSequentialCompoundJob() = default;

void KSequentialCompoundJob::setAbortOnSubjobError(bool abort)
{
    Q_D(KSequentialCompoundJob);
    d->m_abortOnSubjobError = abort;
}

void KSequentialCompoundJob::start()
{
    Q_D(KSequentialCompoundJob);
    if (d->m_subjobs.empty()) {
        qCDebug(UTIL) << "no subjobs, finishing in start()";
        emitResult();
        return;
    }

    d->startNextSubjob();
}

void KSequentialCompoundJob::subjobPercentChanged(KJob *job, unsigned long percent)
{
    Q_D(KSequentialCompoundJob);
    Q_ASSERT(d->m_jobIndex < d->m_jobCount); // invariant
    if (!d->isCurrentlyRunningSubjob(job)) {
        qCDebug(UTIL) << "ignoring percentChanged() signal emitted by an unstarted or finished subjob" << job;
        return;
    }
    Q_ASSERT(d->m_jobIndex >= 0);

    const unsigned long totalPercent = (100.0 * d->m_jobIndex + percent) / d->m_jobCount;
    qCDebug(UTIL) << "subjob percent:" << percent << "; total percent:" << totalPercent;
    setPercent(totalPercent);
}

void KSequentialCompoundJob::slotResult(KJob *job)
{
    Q_D(KSequentialCompoundJob);
    disconnect(job, &KJob::percentChanged, this, &KSequentialCompoundJob::subjobPercentChanged);

    Q_ASSERT(d->m_jobIndex < d->m_jobCount); // invariant
    const auto totalPercent = d->m_jobIndex == -1 ? 100 : static_cast<unsigned long>(100.0 * (d->m_jobIndex + 1) / d->m_jobCount);
    qCDebug(UTIL) << "subjob finished:" << job << "; total percent:" << totalPercent;
    setPercent(totalPercent);

    bool emitDone = false;
    if (d->m_abortOnSubjobError && job->error()) {
        qCDebug(UTIL) << "aborting on subjob error:" << job->error() << job->errorText();
        KCompoundJob::slotResult(job); // calls emitResult()
        emitDone = true;
    } else {
        removeSubjob(job);
    }

    if (!d->m_subjobs.empty() && !error() && !d->m_killing) {
        qCDebug(UTIL) << "remaining subjobs:" << d->m_subjobs;
        d->startNextSubjob();
    } else if (!emitDone) {
        setError(job->error());
        setErrorText(job->errorString());
        emitResult();
    }
}

bool KSequentialCompoundJob::addSubjob(KJob *job)
{
    Q_D(KSequentialCompoundJob);
    if (!KCompoundJob::addSubjob(job)) {
        return false;
    }
    ++d->m_jobCount;
    connect(job, &KJob::percentChanged, this, &KSequentialCompoundJob::subjobPercentChanged);
    return true;
}

bool KSequentialCompoundJob::doKill()
{
    Q_D(KSequentialCompoundJob);
    qCDebug(UTIL) << "killing subjobs:" << d->m_subjobs.size();
    d->m_killing = true;
    while (!d->m_subjobs.empty()) {
        auto *const job = d->m_subjobs.front();
        if (!job || job->kill()) {
            removeSubjob(job);
        } else {
            return false;
        }
    }
    return true;
}

#include "moc_ksequentialcompoundjob.cpp"
