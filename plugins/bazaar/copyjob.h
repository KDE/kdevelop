/*
    SPDX-FileCopyrightText: 2013-2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BAZAAR_COPYJOB_H
#define BAZAAR_COPYJOB_H

#include <QUrl>
#include <QPointer>

#include <vcs/vcsjob.h>

#include <ctime>

namespace KIO
{
class Job;
}

class BazaarPlugin;

class CopyJob : public KDevelop::VcsJob
{
    Q_OBJECT

public:
    explicit CopyJob(const QUrl& localLocationSrc, const QUrl& localLocationDstn, BazaarPlugin* parent = nullptr);

    KDevelop::IPlugin* vcsPlugin() const override;
    KDevelop::VcsJob::JobStatus status() const override;
    QVariant fetchResults() override;
    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void finish(KJob*);
    void addToVcs(KIO::Job* job, const QUrl& from, const QUrl& to, const QDateTime& mtime, bool directory, bool renamed);

private:
    BazaarPlugin* m_plugin;
    QUrl m_source;
    QUrl m_destination;

    JobStatus m_status;
    QPointer<KJob> m_job;
};

#endif // BAZAAR_COPYJOB_H
