/*
    SPDX-FileCopyrightText: 2013 David E. Narvaez <david@piensalibre.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STASHPATCHSOURCE_H
#define STASHPATCHSOURCE_H

#include <interfaces/ipatchsource.h>

#include <QDir>

namespace KDevelop
{
    class VcsJob;
}

class GitPlugin;

class StashPatchSource : public KDevelop::IPatchSource
{
    Q_OBJECT

public:
    StashPatchSource(const QString & stashName, GitPlugin * patch, const QDir & baseDir);
    ~StashPatchSource() override;
    QUrl baseDir() const override;
    QUrl file() const override;
    void update() override;
    bool isAlreadyApplied() const override;
    QString name() const override;
    bool canSelectFiles() const override { return true; };
    uint depth() const override { return 1; }

private Q_SLOTS:
    void updatePatchFile(KDevelop::VcsJob* job);

private:
    QString m_stashName;
    GitPlugin * m_plugin;
    QDir m_baseDir;
    QUrl m_patchFile;
};

#endif // STASHPATCHSOURCE_H
