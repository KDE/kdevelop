/***************************************************************************
 *   Copyright 2013-2014 Maciej Poleski                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "copyjob.h"

#include <QtCore/QVariant>

#include <KIO/CopyJob>

#include <interfaces/iplugin.h>
#include <vcs/dvcs/dvcsjob.h>

#include "bazaarplugin.h"

CopyJob::CopyJob(const QUrl& localLocationSrc, const QUrl& localLocationDstn, BazaarPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity), m_plugin(parent), m_source(localLocationSrc),
      m_destination(localLocationDstn), m_status(KDevelop::VcsJob::JobNotStarted)
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
    connect(job, SIGNAL(copyingDone(KIO::Job*, QUrl, QUrl, time_t, bool, bool)), this, SLOT(addToVcs(KIO::Job*, QUrl, QUrl, time_t, bool, bool)));
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

void CopyJob::addToVcs(KIO::Job* job, const QUrl& from, const QUrl& to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(from);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);
    if (m_status != KDevelop::VcsJob::JobRunning)
        return;
    KDevelop::VcsJob* job2 = m_plugin->add(QList<QUrl>() << to, KDevelop::IBasicVersionControl::Recursive);
    connect(job2, SIGNAL(result(KJob*)), this, SLOT(finish(KJob*)));
    m_job = job2;
    job2->start();
}

void CopyJob::finish(KJob*)
{
    m_status = KDevelop::VcsJob::JobSucceeded;
    emitResult();
    emit resultsReady(this);
}


