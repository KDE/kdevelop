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

#include "gitexecutor.h"


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

GitExecutor::GitExecutor(KDevelop::IPlugin* parent)
    : QObject(parent), vcsplugin(parent)
{
}

GitExecutor::~GitExecutor()
{
}

QString GitExecutor::name() const
{
    return QLatin1String("Git");
}

//TODO: write tests for this method!
//maybe func()const?
bool GitExecutor::isValidDirectory(const KUrl & dirPath)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
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

bool GitExecutor::prepareJob(DVCSjob* job, const QString& repository, enum RequestedOperation op)
{
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "git clone" isValidDirectory() would fail as the
    // directory is not yet under git control
    if (op == GitExecutor::NormalOperation &&
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

bool GitExecutor::addFileList(DVCSjob* job, const QString& repository, const KUrl::List& urls)
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

// QString GitExecutor::convertVcsRevisionToString(const KDevelop::VcsRevision & rev)
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

DVCSjob* GitExecutor::init(const KUrl &directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), GitExecutor::Init) ) {
        *job << "git-init";
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::clone(const KUrl &repository, const KUrl directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), GitExecutor::Init) ) {
        *job << "git-clone";
        *job << repository.path();
//         addFileList(job, repository.path(), directory); //TODO it's temp, should work only with local repos
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::add(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
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
DVCSjob* GitExecutor::commit(const QString& repository,
                         const QString &message, /*= "KDevelop didn't provide any message, it may be a bug"*/
                         const KUrl::List &args /*= QStringList("")*/)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-commit";
        foreach(KUrl arg, args)
            *job<<arg.path();  //TODO much better to use QStringlist, but IBasicVS...
        *job << "-m";
        *job << KShell::quoteArg( message );
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::remove(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-rm";
        addFileList(job, repository, files);
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::status(const QString & repository, const KUrl::List & files, bool recursive, bool taginfo)
{
    Q_UNUSED(files)
    Q_UNUSED(recursive)
    Q_UNUSED(taginfo)
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "status";

        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::log(const KUrl& url)
{
    QFileInfo info(url.toLocalFile());
    if (!info.isFile())
        return false;

    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, info.absolutePath()) ) {
        *job << "git";
        *job << "log";
        *job << info.fileName();
        return job;
    }
    if (job) delete job;
    return NULL;
}

// DVCSjob* GitExecutor::is_inside_work_tree(const QString& repository)
// {
// 
//     return NULL;
// }

DVCSjob* GitExecutor::var(const QString & repository)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-var";
        *job << "-l";
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::empty_cmd() const
{
    ///TODO: maybe just "" command?
    DVCSjob* job = new DVCSjob(vcsplugin);
    *job << "echo";
    *job << "-n";
    return job;
}

//Actually we can just copy the outpuc without parsing. So it's a kind of draft for future
void GitExecutor::parseOutput(const QString& jobOutput, QList<DVCScommit>& commits) const
{
//     static QRegExp rx_sep( "[-=]+" );
//     static QRegExp rx_date( "date:\\s+([^;]*);\\s+author:\\s+([^;]*).*" );

    static QRegExp rx_com( "commit \\w{1,40}" );

    QStringList lines = jobOutput.split("\n");

    DVCScommit item;

    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];
        kDebug(9500) << "line:" << s ;

        if (rx_com.exactMatch(s)) {
            kDebug(9500) << "MATCH COMMIT";
            item.commit = s;
            s = lines[++i];
            item.author = s;
            s = lines[++i];
            item.date = s;
            commits.append(item);
        }
        else 
        {
            item.log += s+'\n';
        }
    }
}

// #include "hgexetor.moc"
