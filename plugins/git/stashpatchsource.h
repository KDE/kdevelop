/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  David E. Narvaez <david@piensalibre.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
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
    virtual ~StashPatchSource();
    virtual QUrl baseDir() const override;
    virtual QUrl file() const override;
    virtual void update() override;
    virtual bool isAlreadyApplied() const override;
    virtual QString name() const override;
    virtual bool canSelectFiles() const override { return true; };
    virtual uint depth() const override { return 1; }

private slots:
    void updatePatchFile(KDevelop::VcsJob* job);

private:
    QString m_stashName;
    GitPlugin * m_plugin;
    QDir m_baseDir;
    QUrl m_patchFile;
};

#endif // STASHPATCHSOURCE_H
