/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompoundjob.h"
#include "kcompoundjob_p.h"

using namespace KDevCoreAddons;

KCompoundJobPrivate::KCompoundJobPrivate() = default;
KCompoundJobPrivate::~KCompoundJobPrivate() = default;

void KCompoundJobPrivate::disconnectSubjob(KJob *job)
{
    Q_Q(KCompoundJob);
    job->setParent(nullptr);
    QObject::disconnect(job, &KJob::finished, q, &KCompoundJob::subjobFinished);
    QObject::disconnect(job, &KJob::infoMessage, q, &KCompoundJob::subjobInfoMessage);
}

KCompoundJob::KCompoundJob(QObject *parent)
    : KCompoundJob(*new KCompoundJobPrivate, parent)
{
}

KCompoundJob::KCompoundJob(KCompoundJobPrivate &dd, QObject *parent)
    : KJob(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

KCompoundJob::~KCompoundJob() = default;

bool KCompoundJob::addSubjob(KJob *job)
{
    Q_D(KCompoundJob);
    if (job == nullptr || d->m_subjobs.contains(job)) {
        return false;
    }

    job->setParent(this);
    d->m_subjobs.append(job);
    connect(job, &KJob::finished, this, &KCompoundJob::subjobFinished);

    // Forward information from that subjob.
    connect(job, &KJob::infoMessage, this, &KCompoundJob::subjobInfoMessage);

    return true;
}

bool KCompoundJob::removeSubjob(KJob *job)
{
    Q_D(KCompoundJob);
    // remove only Subjobs that are on the list
    if (d->m_subjobs.removeAll(job) > 0) {
        d->disconnectSubjob(job);
        return true;
    }
    return false;
}

bool KCompoundJob::hasSubjobs() const
{
    return !d_func()->m_subjobs.isEmpty();
}

const QList<KJob *> &KCompoundJob::subjobs() const
{
    return d_func()->m_subjobs;
}

void KCompoundJob::clearSubjobs()
{
    Q_D(KCompoundJob);
    for (KJob *job : std::as_const(d->m_subjobs)) {
        d->disconnectSubjob(job);
    }
    d->m_subjobs.clear();
}

void KCompoundJob::subjobFinished(KJob *job)
{
    // Did job have an error ?
    if (job->error() && !isFinished() && !error()) {
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

void KCompoundJob::subjobInfoMessage(KJob *job, const QString &message)
{
    Q_EMIT infoMessage(job, message);
}

#include "moc_kcompoundjob.cpp"
