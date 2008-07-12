/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Bazaar                                                    *
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

#include "bzrexecutor.h"

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

BzrExecutor::BzrExecutor(KDevelop::IPlugin* parent)
    : QObject(parent), vcsplugin(parent)
{
}

BzrExecutor::~BzrExecutor()
{
}

//TODO: write tests for this method!
//maybe func()const?
bool BzrExecutor::isValidDirectory(const KUrl & dirPath)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (job)
    {
        job->clear();
        *job << "bzr";
        *job << "root";
        QString path = dirPath.path();
        QFileInfo fsObject(path);
        if (fsObject.isFile())
            path = fsObject.path();
        job->setDirectory(path);
        job->exec();
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
        {
            kDebug(9500) << "Dir:" << path << " is inside work tree of bzr" ;
            return true;
        }
    }
    kDebug(9500) << "Dir:" << dirPath.path() << " is not inside work tree of bzr" ;
    return false;
}

QString BzrExecutor::name() const
{
    return QLatin1String("Bazaar");
}

bool BzrExecutor::prepareJob(DVCSjob* job, const QString& repository, enum RequestedOperation op)
{
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "bzr clone" isValidDirectory() would fail as the
    // directory is not yet under bzr control
    if (op == BzrExecutor::NormalOperation &&
       !isValidDirectory(repository)) {
        kDebug(9500) << repository << " is not a valid bzr repository";
        return false;
    }

    // clear commands and args from a possible previous run
    job->clear();

    // setup the working directory for the new job
    job->setDirectory(repository);

    return true;
}

bool BzrExecutor::addFileList(DVCSjob* job, const QString& repository, const KUrl::List& urls)
{
    QStringList args;

    foreach(KUrl url, urls) {
        ///@todo this is ok for now, but what if some of the urls are not
        ///      to the given repository
        QString file = KUrl::relativeUrl(repository + QDir::separator(), url);

        args << KShell::quoteArg( file );
    }

    *job << args;

    return true;
}

// QString BzrExecutor::convertVcsRevisionToString(const KDevelop::VcsRevision & rev)
// {
//     QString str;
// 
//     switch (rev.revisionType())
//     {
//         case KDevelop::VcsRevision::Special:
//             break;
// 
//         case KDevelop::VcsRevision::FileNumber:
//             if (rev.revisionValue().isValid())
//                 str = "-r"+rev.revisionValue().toString();
//             break;
// 
//         case KDevelop::VcsRevision::Date:
//             if (rev.revisionValue().isValid())
//                 str = "-D"+rev.revisionValue().toString();
//             break;
// 
//             case KDevelop::VcsRevision::GlobalNumber: // !! NOT SUPPORTED BY CVS !!
//         default:
//             break;
//     }
// 
//     return str;
// }

DVCSjob* BzrExecutor::init(const KUrl &directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), BzrExecutor::Init) ) {
        *job << "bzr";
        *job << "init";
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* BzrExecutor::clone(const KUrl &repository, const KUrl directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), BzrExecutor::Init) ) {
        *job << "bzr";
        *job << "clone";
        *job << repository.path();
//         addFileList(job, repository.path(), directory); //TODO it's temp, should work only with local repos
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* BzrExecutor::add(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "bzr"; 
        *job << "add";
        addFileList(job, repository, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

//TODO: bzr doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
DVCSjob* BzrExecutor::commit(const QString& repository,
                         const QString &message, /*= "KDevelop didn't provide any message, it may be a bug"*/
                         const KUrl::List &files /*= QStringList("-a")*/)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "bzr"; 
        *job << "commit";
        *job << "-m";
        *job << KShell::quoteArg( message );
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* BzrExecutor::remove(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "bzr"; 
        *job << "rm";
        addFileList(job, repository, files);
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* BzrExecutor::status(const QString & repository, const KUrl::List & files, bool recursive, bool taginfo)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "bzr";
        *job << "status";
        addFileList(job, repository, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

// DVCSjob* BzrExecutor::is_inside_work_tree(const QString& repository)
// {
// 
//     return NULL;
// }

DVCSjob* BzrExecutor::empty_cmd() const
{
    ///TODO: maybe just "" command?
    DVCSjob* job = new DVCSjob(vcsplugin);
    *job << "echo";
    *job << "-n";
    return job;
}

// #include "bzrexecutor.moc"
