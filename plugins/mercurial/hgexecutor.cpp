/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Mercurial                                                 *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

#include "hgexecutor.h"


#include <QFileInfo>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QDateTime>
#include <KLocale>
#include <KUrl>
#include <KMessageBox>
#include <kshell.h>
#include <KDebug>

#include <dvcsjob.h>
#include <iplugin.h>

HgExecutor::HgExecutor(KDevelop::IPlugin* parent)
    : QObject(parent), vcsplugin(parent)
{
}

HgExecutor::~HgExecutor()
{
}

QString HgExecutor::name() const
{
    return QLatin1String("Hg");
}

//TODO: write tests for this method!
//maybe func()const?
bool HgExecutor::isValidDirectory(const KUrl & dirPath)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (job)
    {
        QString path = dirPath.path();
        QFileInfo fsObject(path);
        if (fsObject.isFile())
            path = fsObject.path();

        job->clear();
        job->setDirectory(path);
        *job << "hg";
        *job << "root";
        job->exec();
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
        {
            kDebug(9500) << "Dir:" << path << " is inside work tree of hg" ;
            return true;
        }
    }
    kDebug(9500) << "Dir:" << dirPath.path() << " is not inside work tree of hg" ;
    return false;
}

DVCSjob* HgExecutor::init(const KUrl &directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), HgExecutor::Init) ) {
        *job << "hg";
        *job << "init";
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* HgExecutor::clone(const KUrl &repository, const KUrl directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), HgExecutor::Init) ) {
        *job << "hg";
        *job << "clone";
        *job << repository.path();
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* HgExecutor::add(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg"; 
        *job << "add";
        addFileList(job, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

//TODO: hg doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
DVCSjob* HgExecutor::commit(const QString& repository,
                            const QString &message, /*= "KDevelop didn't provide any message, it may be a bug"*/
                            const KUrl::List &files /*= QStringList("-a")*/)
{
    Q_UNUSED(files)
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg"; 
        *job << "commit";
        *job << "-m";
        *job << KShell::quoteArg( message );
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* HgExecutor::remove(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg"; 
        *job << "rm";
        addFileList(job, files);
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* HgExecutor::status(const QString & repository, const KUrl::List & files, bool recursive, bool taginfo)
{
    Q_UNUSED(recursive)
    Q_UNUSED(taginfo)
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg";
        *job << "status";
        addFileList(job, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* HgExecutor::empty_cmd() const
{
    ///TODO: maybe just "" command?
    DVCSjob* job = new DVCSjob(vcsplugin);
    *job << "echo";
    *job << "-n";
    return job;
}

// #include "hgexecutor.moc"
