/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompoundjob.h"
#include "kcompoundjob_p.h"

KCompoundJobPrivate::KCompoundJobPrivate()
{
}

KCompoundJobPrivate::~KCompoundJobPrivate()
{
}

KCompoundJob::KCompoundJob(QObject *parent)
    : KJob(*new KCompoundJobPrivate, parent)
{
}

KCompoundJob::KCompoundJob(KCompoundJobPrivate &dd, QObject *parent)
    : KJob(dd, parent)
{
}

KCompoundJob::~KCompoundJob()
{
}

bool KCompoundJob::addSubjob(KJob *job)
{
    Q_D(KCompoundJob);
    if (job == nullptr || d->subjobs.contains(job)) {
        return false;
    }

    job->setParent(this);
    d->subjobs.append(job);
    connect(job, &KJob::result, this, &KCompoundJob::slotResult);

    // Forward information from that subjob.
    connect(job, &KJob::infoMessage, this, &KCompoundJob::slotInfoMessage);

    return true;
}

bool KCompoundJob::removeSubjob(KJob *job)
{
    Q_D(KCompoundJob);
    // remove only Subjobs that are on the list
    if (d->subjobs.removeAll(job) > 0) {
        job->setParent(nullptr);
        disconnect(job, &KJob::result, this, &KCompoundJob::slotResult);
        disconnect(job, &KJob::infoMessage, this, &KCompoundJob::slotInfoMessage);
        return true;
    }
    return false;
}

bool KCompoundJob::hasSubjobs() const
{
    return !d_func()->subjobs.isEmpty();
}

const QList<KJob *> &KCompoundJob::subjobs() const
{
    return d_func()->subjobs;
}

void KCompoundJob::clearSubjobs()
{
    Q_D(KCompoundJob);
    for (KJob *job : std::as_const(d->subjobs)) {
        job->setParent(nullptr);
        disconnect(job, &KJob::result, this, &KCompoundJob::slotResult);
        disconnect(job, &KJob::infoMessage, this, &KCompoundJob::slotInfoMessage);
    }
    d->subjobs.clear();
}

void KCompoundJob::slotResult(KJob *job)
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

void KCompoundJob::slotInfoMessage(KJob *job, const QString &plain, const QString &rich)
{
    Q_EMIT infoMessage(job, plain, rich);
}

#include "moc_kcompoundjob.cpp"
