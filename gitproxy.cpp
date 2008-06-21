/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
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

#include "gitproxy.h"

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

#include "gitjob.h"

#include <iplugin.h>

GitProxy::GitProxy(KDevelop::IPlugin* parent)
    : QObject(parent), vcsplugin(parent)
{
}

GitProxy::~GitProxy()
{
}

//TODO: write tests for this method!
//maybe func()const?
bool GitProxy::isValidDirectory(const KUrl & dirPath)
{
    GitJob* job = new GitJob(vcsplugin);
    if (job)
    {
        job->clear();
        *job << "git-rev-parse";
        *job << "--is-inside-work-tree";
        QString path = dirPath.path();
        QFileInfo fsObject(path);
        if (fsObject.isFile())
            path = fsObject.path();
        job->setDirectory(path);
        job->exec();
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
        {
            kDebug(9500) << "Dir:" << path << " is is inside work tree of git" ;
            return true;
        }
    }
    kDebug(9500) << "Dir:" << dirPath.path() << " is is not inside work tree of git" ;
    return false;
}

bool GitProxy::prepareJob(GitJob* job, const QString& repository, enum RequestedOperation op)
{
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "git clone" isValidDirectory() would fail as the
    // directory is not yet under git control
    if (op == GitProxy::NormalOperation &&
        !isValidDirectory(repository)) {
        kDebug(9500) << repository << " is not a valid git repository";
        return false;
        }

    // clear commands and args from a possible previous run
        job->clear();

    // setup the working directory for the new job
        job->setDirectory(repository);

        return true;
}

bool GitProxy::addFileList(GitJob* job, const QString& repository, const KUrl::List& urls)
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

// QString GitProxy::convertVcsRevisionToString(const KDevelop::VcsRevision & rev)
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

GitJob* GitProxy::init(const KUrl &directory)
{
    GitJob* job = new GitJob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), GitProxy::Init) ) {
        *job << "git-init";
        return job;
    }
    if (job) delete job;
    return NULL;
}

GitJob* GitProxy::clone(const KUrl &repository, const KUrl directory)
{
    GitJob* job = new GitJob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), GitProxy::Init) ) {
        *job << "git-clone";
        *job << repository.path();
//         addFileList(job, repository.path(), directory); //TODO it's temp, should work only with local repos
        return job;
    }
    if (job) delete job;
    return NULL;
}

GitJob* GitProxy::add(const QString& repository, const KUrl::List &files)
{
    GitJob* job = new GitJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "add";

        addFileList(job, repository, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

//TODO: git doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
GitJob* GitProxy::commit(const QString& repository,
                         const QString &message, /*= "KDevelop didn't provide any message, it may be a bug"*/
                         const KUrl::List &files /*= QStringList("-a")*/)
{
    GitJob* job = new GitJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-commit";
        addFileList(job, repository, files);
        *job << "-m";
        *job << KShell::quoteArg( message );
        return job;
    }
    if (job) delete job;
    return NULL;
}

GitJob* GitProxy::remove(const QString& repository, const KUrl::List &files)
{
    GitJob* job = new GitJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-rm";
        addFileList(job, repository, files);
        return job;
    }
    if (job) delete job;
    return NULL;
}

// //TODO: now just only "git log" or "git log file", no extensions
// GitJob* GitProxy::log(const KUrl& url, const KDevelop::VcsRevision& rev)
// {
//     QFileInfo info(url.toLocalFile());
//     if (!info.isFile())
//         return false;
// 
//     GitJob* job = new GitJob(vcsplugin);
//     if ( prepareJob(job, info.absolutePath()) ) {
//         *job << "cvs";
//         *job << "log";
// 
//         QString convRev = convertVcsRevisionToString(rev);
//         if (!convRev.isEmpty()) {
//             convRev.replace("-D", "-d");
//             *job << convRev;
//         }
// 
//         *job << KShell::quoteArg(info.fileName());
// 
//         return job;
//     }
//     if (job) delete job;
//     return NULL;
// }
// 
// GitJob* GitProxy::diff(const KUrl& url, 
//                        const KDevelop::VcsRevision& revA, 
//                        const KDevelop::VcsRevision& revB,
//                        const QString& diffOptions)
// {
//     QFileInfo info(url.toLocalFile());
// 
//     GitJob* job = new GitJob(vcsplugin);
//     if ( prepareJob(job, info.absolutePath()) ) {
//         *job << "cvs";
//         *job << "diff";
// 
//         if (!diffOptions.isEmpty())
//             *job << diffOptions;
// 
//         QString rA = convertVcsRevisionToString(revA);
//         if (!rA.isEmpty())
//             *job << rA;
//         QString rB = convertVcsRevisionToString(revB);
//         if (!rB.isEmpty())
//             *job << rB;
// 
//         *job << KShell::quoteArg(info.fileName());
// 
//         return job;
//     }
//     if (job) delete job;
//     return NULL;
// }
// 
// GitJob* GitProxy::annotate(const KUrl & url, const KDevelop::VcsRevision& rev)
// {
//     QFileInfo info(url.toLocalFile());
// 
//     GitJob* job = new GitJob(vcsplugin);
//     if ( prepareJob(job, info.absolutePath()) ) {
//         *job << "cvs";
//         *job << "annotate";
// 
//         QString revision = convertVcsRevisionToString(rev);
//         if (!revision.isEmpty())
//             *job << revision;
// 
//         *job << KShell::quoteArg(info.fileName());
// 
//         return job;
//     }
//     if (job) delete job;
//     return NULL;
// }

GitJob* GitProxy::status(const QString & repository, const KUrl::List & files, bool recursive, bool taginfo)
{
    GitJob* job = new GitJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "status";
        addFileList(job, repository, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

// GitJob* GitProxy::is_inside_work_tree(const QString& repository)
// {
// 
//     return NULL;
// }

GitJob* GitProxy::empty_cmd() const
{
    ///TODO: maybe just "" command?
    GitJob* job = new GitJob(vcsplugin);
    *job << "echo";
    *job << "-n";
    return job;
}

#include "gitproxy.moc"
