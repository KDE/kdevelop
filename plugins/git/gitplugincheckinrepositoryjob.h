/*
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GITPLUGINCHECKINREPOSITORYJOB_H
#define GITPLUGINCHECKINREPOSITORYJOB_H

#include <vcs/interfaces/icontentawareversioncontrol.h>
#include <QProcess>

class GitPluginCheckInRepositoryJob : public KDevelop::CheckInRepositoryJob
{
    Q_OBJECT
public:
    GitPluginCheckInRepositoryJob(KTextEditor::Document* document, const QString& rootDirectory);
    ~GitPluginCheckInRepositoryJob() override;
    void start() override;

private Q_SLOTS:
    void repositoryQueryFinished(int);
    void processFailed(QProcess::ProcessError);

private:
    QProcess* m_hashjob;
    QProcess* m_findjob;
    QString m_rootDirectory;
};

#endif // GITPLUGINCHECKINREPOSITORYJOB_H
