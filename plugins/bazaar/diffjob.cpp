/*
    SPDX-FileCopyrightText: 2013-2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "diffjob.h"

#include <QDir>

#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsdiff.h>

#include "bazaarplugin.h"

using namespace KDevelop;

DiffJob::DiffJob(const QDir& workingDir, const QString& revisionSpecRange,
                 const QUrl& fileOrDirectory, BazaarPlugin* parent,
                 KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, OutputJob::Silent)
    , m_plugin(parent)
    , m_status(VcsJob::JobNotStarted)
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
        connect(m_job.data(), &DVcsJob::finished, this, &DiffJob::prepareResult);
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
    if (m_job->process()->exitStatus() == QProcess::NormalExit) {
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

#include "moc_diffjob.cpp"
