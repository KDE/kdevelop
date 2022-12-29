/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompositejob.h"
#include "kcompositejob_p.h"

KCompositeJobPrivate::KCompositeJobPrivate()
{
}

KCompositeJobPrivate::~KCompositeJobPrivate()
{
}

KCompositeJob::KCompositeJob(QObject *parent)
    : KJob(*new KCompositeJobPrivate, parent)
{
}

KCompositeJob::KCompositeJob(KCompositeJobPrivate &dd, QObject *parent)
    : KJob(dd, parent)
{
}

KCompositeJob::~KCompositeJob()
{
}

bool KCompositeJob::addSubjob(KJob *job)
{
    Q_D(KCompositeJob);
    if (job == nullptr || d->subjobs.contains(job)) {
        return false;
    }

    job->setParent(this);
    d->subjobs.append(job);
    connect(job, &KJob::result, this, &KCompositeJob::slotResult);

    // Forward information from that subjob.
    connect(job, &KJob::infoMessage, this, &KCompositeJob::slotInfoMessage);

    return true;
}

bool KCompositeJob::removeSubjob(KJob *job)
{
    Q_D(KCompositeJob);
    // remove only Subjobs that are on the list
    if (d->subjobs.removeAll(job) > 0) {
        job->setParent(nullptr);
        disconnect(job, &KJob::result, this, &KCompositeJob::slotResult);
        disconnect(job, &KJob::infoMessage, this, &KCompositeJob::slotInfoMessage);
        return true;
    }
    return false;
}

bool KCompositeJob::hasSubjobs() const
{
    return !d_func()->subjobs.isEmpty();
}

const QList<KJob *> &KCompositeJob::subjobs() const
{
    return d_func()->subjobs;
}

void KCompositeJob::clearSubjobs()
{
    Q_D(KCompositeJob);
    for (KJob *job : std::as_const(d->subjobs)) {
        job->setParent(nullptr);
        disconnect(job, &KJob::result, this, &KCompositeJob::slotResult);
        disconnect(job, &KJob::infoMessage, this, &KCompositeJob::slotInfoMessage);
    }
    d->subjobs.clear();
}

void KCompositeJob::slotResult(KJob *job)
{
    // Did job have an error ?
    if (job->error() && !error()) {
        // Store it in the parent only if first error
        setError(job->error());
        setErrorText(job->errorText());
        // Finish this job
        emitResult();
    }
    // After a subjob is done, we might want to start another one.
    // Therefore do not emitResult
    removeSubjob(job);
}

void KCompositeJob::slotInfoMessage(KJob *job, const QString &plain, const QString &rich)
{
    Q_EMIT infoMessage(job, plain, rich);
}

#include "moc_kcompositejob.cpp"
