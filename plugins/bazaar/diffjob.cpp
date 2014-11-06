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

#include "diffjob.h"

#include <QtCore/QDir>

#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsdiff.h>

#include "bazaarplugin.h"

DiffJob::DiffJob(const QDir& workingDir, const QString& revisionSpecRange,
                 const QUrl& fileOrDirectory, BazaarPlugin* parent,
                 KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity), m_plugin(parent),
      m_status(KDevelop::VcsJob::JobNotStarted)
{
    setType(KDevelop::VcsJob::Diff);
    setCapabilities(Killable);
    m_job = new KDevelop::DVcsJob(workingDir, parent, verbosity);
    m_job->setType(VcsJob::Diff);
    *m_job << "bzr" << "diff" << "-p1" << revisionSpecRange << fileOrDirectory;
}

KDevelop::IPlugin* DiffJob::vcsPlugin() const
{
    return m_plugin;
}

KDevelop::VcsJob::JobStatus DiffJob::status() const
{
    return m_status;
}

QVariant DiffJob::fetchResults()
{
    return m_result;
}

void DiffJob::start()
{
    if (m_status != KDevelop::VcsJob::JobNotStarted)
        return;
    if (m_job) {
        connect(m_job.data(), &KDevelop::DVcsJob::finished, this, &DiffJob::prepareResult);
        m_status = KDevelop::VcsJob::JobRunning;
        m_job->start();
    }
}

bool DiffJob::doKill()
{
    m_status = KDevelop::VcsJob::JobCanceled;
    if (m_job)
        return m_job->kill(KJob::Quietly);
    else
        return true;
}

void DiffJob::prepareResult(KJob*)
{
    if (m_job->process()->exitStatus() <= 2) {
        KDevelop::VcsDiff diff;
        diff.setDiff(m_job->output());
        diff.setBaseDiff(QUrl::fromLocalFile(m_job->directory().absolutePath()));
        m_result.setValue(diff);
        m_status = KDevelop::VcsJob::JobSucceeded;
    } else {
        setError(m_job->process()->exitStatus());
        m_status = KDevelop::VcsJob::JobFailed;
    }
    emitResult();
    emit resultsReady(this);
}


