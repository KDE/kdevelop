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

#include "stashpatchsource.h"
#include "gitplugin.h"

#include "vcs/dvcs/dvcsjob.h"
#include "interfaces/icore.h"
#include "interfaces/iruncontroller.h"

#include <QTemporaryFile>
#include <QTextStream>

StashPatchSource::StashPatchSource(const QString& stashName, GitPlugin* plugin, const QDir & baseDir)
 : m_stashName(stashName), m_plugin(plugin), m_baseDir(baseDir)
{
    QTemporaryFile tempFile;

    tempFile.setAutoRemove(false);
    tempFile.open();
    m_patchFile = QUrl::fromLocalFile(tempFile.fileName());

    KDevelop::DVcsJob * job = m_plugin->gitStash(m_baseDir, QStringList() << "show" << "-u" << m_stashName, KDevelop::OutputJob::Silent);

    connect(job, SIGNAL(resultsReady(KDevelop::VcsJob*)), this, SLOT(updatePatchFile(KDevelop::VcsJob*)));
    KDevelop::ICore::self()->runController()->registerJob(job);
}

StashPatchSource::~StashPatchSource()
{
    QFile::remove(m_patchFile.toLocalFile());
}

QUrl StashPatchSource::baseDir() const
{
    return QUrl::fromLocalFile(m_baseDir.absolutePath());
}

QUrl StashPatchSource::file() const
{
    return m_patchFile;
}

void StashPatchSource::update()
{
}

bool StashPatchSource::isAlreadyApplied() const
{
    return false;
}

QString StashPatchSource::name() const
{
    return m_stashName;
}

void StashPatchSource::updatePatchFile(KDevelop::VcsJob* job)
{
    KDevelop::DVcsJob* dvcsJob = qobject_cast<KDevelop::DVcsJob*>(job);
    QFile f(m_patchFile.toLocalFile());
    QTextStream txtStream(&f);

    f.open(QIODevice::WriteOnly);
    txtStream << dvcsJob->rawOutput();
    f.close();

    emit patchChanged();
}
