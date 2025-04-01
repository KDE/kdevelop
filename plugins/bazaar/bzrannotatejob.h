/*
    SPDX-FileCopyrightText: 2013-2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BAZAAR_BZRANNOTATEJOB_H
#define BAZAAR_BZRANNOTATEJOB_H

#include <QStringList>
#include <QHash>
#include <QDir>
#include <QUrl>
#include <QPointer>

#include <vcs/vcsevent.h>
#include <vcs/vcsjob.h>

class QDir;
namespace KDevelop
{
class DVcsJob;
}

class BzrAnnotateJob : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    explicit BzrAnnotateJob(const QDir& workingDir, const QString& revisionSpec, const QUrl& localLocation,
                            KDevelop::IPlugin* parent = nullptr);

    QVariant fetchResults() override;
    void start() override;
    JobStatus status() const override;
    KDevelop::IPlugin* vcsPlugin() const override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void parseBzrAnnotateOutput(KDevelop::DVcsJob* job);
    void parseNextLine();
    void prepareCommitInfo(std::size_t revision);
    void parseBzrLog(KDevelop::DVcsJob* job);

private:
    QDir m_workingDir;
    QString m_revisionSpec;
    QUrl m_localLocation;
    KDevelop::IPlugin* m_vcsPlugin;

    JobStatus m_status;
    QPointer<KJob> m_job;

    QStringList m_outputLines;
    int m_currentLine;
    QHash<int, KDevelop::VcsEvent> m_commits;
    QVariantList m_results;
};

#endif // BAZAAR_BZRANNOTATEJOB_H
