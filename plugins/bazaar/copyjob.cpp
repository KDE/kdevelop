/*
    SPDX-FileCopyrightText: 2013-2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "copyjob.h"

#include <QVariant>

#include <KIO/CopyJob>

#include <interfaces/iplugin.h>

#include "bazaarplugin.h"

using namespace KDevelop;

CopyJob::CopyJob(const QUrl& localLocationSrc, const QUrl& localLocationDstn, BazaarPlugin* parent)
    // this job does not output anything itself, so pass Silent to VcsJob()
    : VcsJob(parent, OutputJob::Silent)
    , m_plugin(parent)
    , m_source(localLocationSrc)
    , m_destination(localLocationDstn)
    , m_status(VcsJob::JobNotStarted)
{
    setType(JobType::Copy);
    setCapabilities(Killable);
}


KDevelop::IPlugin* CopyJob::vcsPlugin() const
{
    return m_plugin;
}

KDevelop::VcsJob::JobStatus CopyJob::status() const
{
    return m_status;
}

QVariant CopyJob::fetchResults()
{
    return QVariant();
}

void CopyJob::start()
{
    if (m_status != KDevelop::VcsJob::JobNotStarted)
        return;
    KIO::CopyJob* job = KIO::copy(m_source, m_destination, KIO::HideProgressInfo);
    connect(job, &KIO::CopyJob::copyingDone, this, &CopyJob::addToVcs);
    m_status = KDevelop::VcsJob::JobRunning;
    m_job = job;
    job->start();
}

bool CopyJob::doKill()
{
    m_status = KDevelop::VcsJob::JobCanceled;
    if (m_job)
        return m_job->kill(KJob::Quietly);
    else
        return true;
}

void CopyJob::addToVcs(KIO::Job* job, const QUrl& from, const QUrl& to, const QDateTime& mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(from);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);
    if (m_status != KDevelop::VcsJob::JobRunning)
        return;
    KDevelop::VcsJob* job2 = m_plugin->add(QList<QUrl>() << to, KDevelop::IBasicVersionControl::Recursive);
    connect(job2, &VcsJob::result, this, &CopyJob::finish);
    m_job = job2;
    job2->start();
}

void CopyJob::finish(KJob*)
{
    m_status = KDevelop::VcsJob::JobSucceeded;
    emitResult();
    emit resultsReady(this);
}

#include "moc_copyjob.cpp"
