/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Adapted for Hg                                                        *
 *   Copyright 2008 Tom Burdick <thomas.burdick@gmail.com>                 *
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

#include "hgproxy.h"

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

#include "hgjob.h"

#include <iplugin.h>

HgProxy::HgProxy(KDevelop::IPlugin* parent)
    : QObject(parent), vcsplugin(parent)
{
}

HgProxy::~HgProxy()
{
}

//TODO: write tests for this method!
//maybe func()const?
bool HgProxy::isValidDirectory(const KUrl & dirPath)
{
    HgJob* job = new HgJob(vcsplugin);
    if (job)
    {
        job->clear();
        *job << "hg-rev-parse";
        *job << "--is-inside-work-tree";
        QString path = dirPath.path();
        QFileInfo fsObject(path);
        if (fsObject.isFile())
            path = fsObject.path();
        job->setDirectory(path);
        job->exec();
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
        {
            kDebug(9500) << "Dir:" << path << " is is inside work tree of hg" ;
            return true;
        }
    }
    kDebug(9500) << "Dir:" << dirPath.path() << " is is not inside work tree of hg" ;
    return false;
}

bool HgProxy::prepareJob(HgJob* job, const QString& repository, enum RequestedOperation op)
{
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "hg clone" isValidDirectory() would fail as the
    // directory is not yet under hg control
    if (op == HgProxy::NormalOperation &&
        !isValidDirectory(repository)) {
        kDebug(9500) << repository << " is not a valid hg repository";
        return false;
        }

    // clear commands and args from a possible previous run
        job->clear();

    // setup the working directory for the new job
        job->setDirectory(repository);

        return true;
}

bool HgProxy::addFileList(HgJob* job, const QString& repository, const KUrl::List& urls)
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

// QString HgProxy::convertVcsRevisionToString(const KDevelop::VcsRevision & rev)
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

HgJob* HgProxy::init(const KUrl &directory)
{
    HgJob* job = new HgJob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), HgProxy::Init) ) {
        *job << "hg init";
        return job;
    }
    if (job) delete job;
    return NULL;
}

HgJob* HgProxy::clone(const KUrl &repository, const KUrl directory)
{
    HgJob* job = new HgJob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), HgProxy::Init) ) {
        *job << "hg clone";
        *job << repository.path();
//         addFileList(job, repository.path(), directory); //TODO it's temp, should work only with local repos
        return job;
    }
    if (job) delete job;
    return NULL;
}

HgJob* HgProxy::add(const QString& repository, const KUrl::List &files)
{
    HgJob* job = new HgJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg";
        *job << "add";

        addFileList(job, repository, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

//TODO: hg doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
HgJob* HgProxy::commit(const QString& repository,
                         const QString &message, /*= "KDevelop didn't provide any message, it may be a bug"*/
                         const KUrl::List &files /*= QStringList("-a")*/)
{
    HgJob* job = new HgJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg commit";
        *job << "-m";
        *job << KShell::quoteArg( message );
        return job;
    }
    if (job) delete job;
    return NULL;
}

HgJob* HgProxy::remove(const QString& repository, const KUrl::List &files)
{
    HgJob* job = new HgJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg rm";
        addFileList(job, repository, files);
        return job;
    }
    if (job) delete job;
    return NULL;
}

// //TODO: now just only "hg log" or "hg log file", no extensions
// HgJob* HgProxy::log(const KUrl& url, const KDevelop::VcsRevision& rev)
// {
//     QFileInfo info(url.toLocalFile());
//     if (!info.isFile())
//         return false;
// 
//     HgJob* job = new HgJob(vcsplugin);
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
// HgJob* HgProxy::diff(const KUrl& url, 
//                        const KDevelop::VcsRevision& revA, 
//                        const KDevelop::VcsRevision& revB,
//                        const QString& diffOptions)
// {
//     QFileInfo info(url.toLocalFile());
// 
//     HgJob* job = new HgJob(vcsplugin);
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
// HgJob* HgProxy::annotate(const KUrl & url, const KDevelop::VcsRevision& rev)
// {
//     QFileInfo info(url.toLocalFile());
// 
//     HgJob* job = new HgJob(vcsplugin);
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

HgJob* HgProxy::status(const QString & repository, const KUrl::List & files, bool recursive, bool taginfo)
{
    HgJob* job = new HgJob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "hg";
        *job << "status";
        addFileList(job, repository, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

// HgJob* HgProxy::is_inside_work_tree(const QString& repository)
// {
// 
//     return NULL;
// }

HgJob* HgProxy::empty_cmd() const
{
    ///TODO: maybe just "" command?
    HgJob* job = new HgJob(vcsplugin);
    *job << "echo";
    *job << "-n";
    return job;
}

#include "hgproxy.moc"
