/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS                                                      *
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

#include "idvcsexecutor.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QVariant>

#include <KDE/KDebug>
#include <KDE/KShell>

#include "dvcsjob.h"

using KDevelop::IDVCSexecutor;


bool IDVCSexecutor::prepareJob(DVCSjob* job, const QString& repository, enum RequestedOperation op)
{
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "git clone" isValidDirectory() would fail as the
    // directory is not yet under git control
    if (op == NormalOperation &&
        !isValidDirectory(repository)) {
        kDebug() << repository << " is not a valid repository";
        return false;
    }

    if (!QFileInfo(repository).isAbsolute())
    {
        //We don't want to have empty or non-absolute pathes for working dir
        return false;
    }

    // clear commands and args from a possible previous run
    job->clear();

    //repository is sent by ContextMenu, so we check if it is a file and use it's path
    QFileInfo repoInfo = QFileInfo(repository);
    if (repoInfo.isFile())
        job->setDirectory(repoInfo.path());
    else
        job->setDirectory(repository);

    return true;
}

bool IDVCSexecutor::addFileList(DVCSjob* job, const KUrl::List& urls)
{
    QStringList args;

    foreach(const KUrl &url, urls) {
        ///@todo this is ok for now, but what if some of the urls are not
        ///      to the given repository
        //all urls should be relative to the working directory!
        //if url is relative we rely on it's relative to job->getDirectory(), so we check if it's exists
        QString file;
        if (!QFileInfo(url.path()).isRelative() )
            file = KUrl::relativeUrl(job->getDirectory() + '/', url);
        //actually this shouldn't happen (only in Git tests)
        else if (QDir(job->getDirectory()).dirName() == url.path())
            file = ".";
        else
            file = url.path();
        args << file;
        kDebug() << "url is: " << url << "job->getDirectory(): " << job->getDirectory() << " file is: " << file;
    }

    *job << args;
    return true;
}


///TODO: imlement in HG and Bazaar!

bool IDVCSexecutor::isInRepo(const KUrl &path)
{
    Q_UNUSED(path)
    return true;
}

void IDVCSexecutor::parseOutput(const QString& jobOutput,
                                QList<DVCScommit>& revisions) const 
{
    Q_UNUSED(jobOutput)
    Q_UNUSED(revisions)
}

DVCSjob* IDVCSexecutor::checkout(const QString &repository, const QString &branch) 
{
    Q_UNUSED(repository)
    Q_UNUSED(branch)
    return empty_cmd();
}

DVCSjob* IDVCSexecutor::log(const KUrl& url)
{
    Q_UNUSED(url)
    return empty_cmd();
}

DVCSjob* IDVCSexecutor::branch(const QString &repository, const QString &basebranch, const QString &branch,
                               const QStringList &args)
{
    Q_UNUSED(repository)
    Q_UNUSED(branch)
    Q_UNUSED(basebranch)
    Q_UNUSED(args)
    return empty_cmd();
}

//parsers for branch:
QString IDVCSexecutor::curBranch(const QString &repository) 
{
    Q_UNUSED(repository)
    return QString();
}

QStringList IDVCSexecutor::branches(const QString &repository) 
{
    Q_UNUSED(repository)
    return QStringList();
}

//commit manager helpers:
QList<QVariant> IDVCSexecutor::getModifiedFiles(const QString &directory){
    Q_UNUSED(directory)
    return QList<QVariant>();
}

QList<QVariant> IDVCSexecutor::getCachedFiles(const QString &directory)
{
    Q_UNUSED(directory)
    return QList<QVariant>();
}

QList<QVariant> IDVCSexecutor::getOtherFiles(const QString &directory)
{
    Q_UNUSED(directory)
    return QList<QVariant>();
}

DVCSjob* IDVCSexecutor::reset(const QString &repository, const QStringList &args, const KUrl::List &files)
{
    Q_UNUSED(repository)
    Q_UNUSED(args)
    Q_UNUSED(files)
    return empty_cmd();
}

QList<DVCScommit> IDVCSexecutor::getAllCommits(const QString &repo)
{
    Q_UNUSED(repo)
    return QList<DVCScommit>();
}
