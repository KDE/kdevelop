/*
    SPDX-FileCopyrightText: 2013-2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BAZAAR_DIFFJOB_H
#define BAZAAR_DIFFJOB_H

#include <QVariant>
#include <QPointer>

#include <vcs/vcsjob.h>

namespace KDevelop
{
class DVcsJob;
}

class QDir;
class BazaarPlugin;

class DiffJob : public KDevelop::VcsJob
{
    Q_OBJECT

public:
    /**
     * Create a diff job.
     *
     * @param verbosity the verbosity of the internal output job instance that performs the actual work, not of this job
     *
     * @note DiffJob does not output anything itself and therefore is always Silent.
     */
    DiffJob(const QDir& workingDir, const QString& revisionSpecRange, const QUrl& fileOrDirectory, BazaarPlugin* parent = nullptr, OutputJobVerbosity verbosity = OutputJob::Silent);

    KDevelop::IPlugin* vcsPlugin() const override;
    KDevelop::VcsJob::JobStatus status() const override;
    QVariant fetchResults() override;
    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void prepareResult(KJob*);

private:
    BazaarPlugin* m_plugin;
    QVariant m_result;

    JobStatus m_status;
    QPointer<KDevelop::DVcsJob> m_job;
};

#endif // BAZAAR_DIFFJOB_H
