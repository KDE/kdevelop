/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsproxy.h"

#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <KLocale>
#include <KUrl>
#include <KMessageBox>
#include <kshell.h>
#include <KDebug>

#include "cvsjob.h"

#include <iplugin.h>

CvsProxy::CvsProxy(KDevelop::IPlugin* parent)
: QObject(parent), vcsplugin(parent)
{
}

CvsProxy::~CvsProxy()
{
}

void CvsProxy::slotResult(KJob* job)
{
//     job->deleteLater();
}

bool CvsProxy::isValidDirectory(const KUrl & dirPath) const
{
    QString path = dirPath.toLocalFile() + QDir::separator() + "CVS";
    return QFileInfo(path).exists();
}

bool CvsProxy::prepareJob(CvsJob* job, const QString& repository, enum RequestedOperation op)
{
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "cvs import" isValidDirectory() would fail as the
    // directory is not yet under CVS control
    if (op == CvsProxy::NormalOperation &&
        !isValidDirectory(repository)) {
        kDebug(9500) << repository << " is not a valid CVS repository";
        return false;
    }

    // clear commands and args from a possible previous run
    job->clear();

    // setup the working directory for the new job
    job->setDirectory(repository);

    // each job that was created by this proxy will
    // automatically be delete after it has finished.
    // Therefor the slotResult() calls deleteLater() on the job
    connect(job, SIGNAL( result(KJob*) ),
            this, SLOT( slotResult(KJob*) ));

    return true;
}

bool CvsProxy::addFileList(CvsJob* job, const QString& repository, const KUrl::List& urls)
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

QString CvsProxy::convertVcsRevisionToString(const KDevelop::VcsRevision & rev)
{
    QString str;

    switch (rev.revisionType())
    {
        case KDevelop::VcsRevision::Special:
            break;

        case KDevelop::VcsRevision::FileNumber:
            if (rev.revisionValue().isValid())
                str = "-r"+rev.revisionValue().toInt();
            break;

        case KDevelop::VcsRevision::Date:
            if (rev.revisionValue().isValid())
                str = "-D"+rev.revisionValue().toDateTime().toString( Qt::ISODate );
            break;

        case KDevelop::VcsRevision::GlobalNumber: // !! NOT SUPPORTED BY CVS !!
        default:
            break;
    }

    return str;
}

CvsJob* CvsProxy::log(const KUrl& url, const KDevelop::VcsRevision& rev)
{
    QFileInfo info(url.toLocalFile());
    if (!info.isFile())
        return false;

    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "log";

        QString convRev = convertVcsRevisionToString(rev);
        if (!convRev.isEmpty()) {
            convRev.replace("-D", "-d");
            *job << convRev;
        }

        *job << KShell::quoteArg(info.fileName());

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob* CvsProxy::diff(const KUrl& url, 
            const KDevelop::VcsRevision& revA, 
            const KDevelop::VcsRevision& revB,
            const QString& diffOptions)
{
    QFileInfo info(url.toLocalFile());

    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "diff";
        *job << diffOptions;

        QString rA = convertVcsRevisionToString(revA);
        if (!rA.isEmpty())
            *job << rA;
        QString rB = convertVcsRevisionToString(revB);
        if (!rB.isEmpty())
            *job << rB;

        *job << KShell::quoteArg(info.fileName());

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob * CvsProxy::annotate(const KUrl & url, const KDevelop::VcsRevision& rev)
{
    QFileInfo info(url.toLocalFile());

    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "annotate";

        QString revision = convertVcsRevisionToString(rev);
        if (!revision.isEmpty())
            *job << revision;

        *job << KShell::quoteArg(info.fileName());

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob* CvsProxy::edit(const QString& repo, const KUrl::List& files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "edit";

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}


CvsJob* CvsProxy::unedit(const QString& repo, const KUrl::List& files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "unedit";

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob* CvsProxy::editors(const QString& repo, const KUrl::List& files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "editors";

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob* CvsProxy::commit(const QString& repo, const KUrl::List& files, const QString& message)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "commit";

        *job << "-m";
        *job << KShell::quoteArg( message );

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob* CvsProxy::add(const QString & repo, const KUrl::List & files, 
                      bool recursiv, bool binary)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "add";

        if (binary) {
            *job << "-kb";
        }

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob * CvsProxy::remove(const QString & repo, const KUrl::List & files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "remove";

        *job << "-f"; //existing files will be deleted

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob * CvsProxy::update(const QString & repo, const KUrl::List & files, 
                          const KDevelop::VcsRevision & rev, 
                          const QString & updateOptions, 
                          bool recursive, bool pruneDirs, bool createDirs)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "update";

        if (recursive)
            *job << "-R";
        else 
            *job << "-L";
        if (pruneDirs)
            *job << "-P";
        if (createDirs)
            *job << "-d";
        if (!updateOptions.isEmpty())
            *job << updateOptions;

        QString revision = convertVcsRevisionToString(rev);
        if (!revision.isEmpty())
            *job << revision;

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob * CvsProxy::import(const KUrl & directory,
                          const QString & server, const QString & repositoryName,
                          const QString & vendortag, const QString & releasetag,
                          const QString& message)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, directory.toLocalFile(), CvsProxy::Import) ) {
        *job << "cvs";
        *job << "-q"; // don't print directory changes
        *job << "-d";
        *job << server;
        *job << "import";

        *job << "-m";
        *job << KShell::quoteArg( message );

        *job << repositoryName;
        *job << vendortag;
        *job << releasetag;

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob * CvsProxy::checkout(const KUrl & targetDir,
                            const QString & server, const QString & module,
                            const QString & checkoutOptions,
                            const QString & revision,
                            bool recursive,
                            bool pruneDirs)
{
    CvsJob* job = new CvsJob(vcsplugin);
    ///@todo when doing a checkout we don't have the targetdir yet,
    ///      for now it'll work to just run the command from the root
    if ( prepareJob(job, "/", CvsProxy::CheckOut) ) {
        *job << "cvs";
        *job << "-q"; // don't print directory changes
        *job << "-d" << server;
        *job << "checkout";

        if (!checkoutOptions.isEmpty())
            *job << checkoutOptions;

        if (!revision.isEmpty()) {
            *job << "-r" << revision;
        }

        if (pruneDirs)
            *job << "-P";

        if (!recursive)
            *job << "-l";

        *job << "-d" << targetDir.toLocalFile();

        *job << module;

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob * CvsProxy::status(const QString & repo, const KUrl::List & files, bool recursive, bool taginfo)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "status";

        if (recursive)
            *job << "-R";
        else
            *job << "-l";
        if (taginfo)
            *job << "-v";

        addFileList(job, repo, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

#include "cvsproxy.moc"
