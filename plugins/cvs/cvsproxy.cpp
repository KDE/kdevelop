/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
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
#include <KLocale>
#include <KUrl>
#include <KMessageBox>
#include <kshell.h>
#include "cvsjob.h"

CvsProxy::CvsProxy(QObject* parent)
: QObject(parent)
{
}

CvsProxy::~CvsProxy()
{
    kDebug()<<  k_funcinfo <<endl;
}

void CvsProxy::slotResult(KJob* job)
{
    job->deleteLater();
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
        kDebug() << repository << " is not a valid CVS repository" << endl;
        return false;
    }

    // clear commands and args from a possible previous run
    job->clear();

    // setup the working directory for the new job
    job->setDirectory(repository);

    // each job that was connected by this proxy class will
    // automatically be delete after it has finished.
    // Therefor the slotResult() calls deleteLater() on the job
    connect(job, SIGNAL( result(KJob*) ),
            this, SLOT( slotResult(KJob*) ));

    return true;
}

bool CvsProxy::addFileList(CvsJob* job, const QString& repository, const KUrl::List& urls)
{
    foreach(KUrl url, urls) {
        //@todo this is ok for now, but what if some of the urls are not
        //      to the given repository
        QString file = KUrl::relativeUrl(repository + QDir::separator(), url);

        *job << KShell::quoteArg( file );
    }

    return true;
}

CvsJob* CvsProxy::log(const KUrl& url)
{
    kDebug() << k_funcinfo << endl;

    QFileInfo info(url.toLocalFile());
    if (!info.isFile())
        return false;

    CvsJob* job = new CvsJob(this);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "log";
        *job << KShell::quoteArg(info.fileName());

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob* CvsProxy::diff(const KUrl& url, const QString& diffOptions,
                    const QString& revA, const QString& revB)
{
    kDebug() << k_funcinfo << endl;

    QFileInfo info(url.toLocalFile());

    CvsJob* job = new CvsJob(this);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "diff";
        *job << diffOptions;

        if (!revA.isEmpty())
            *job << "-r"<<KShell::quoteArg(revA);
        if (!revB.isEmpty())
            *job << "-r"<<KShell::quoteArg(revB);

        *job << KShell::quoteArg(info.fileName());

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob * CvsProxy::annotate(const KUrl & url, const QString & revision)
{
    kDebug() << k_funcinfo << endl;

    QFileInfo info(url.toLocalFile());

    CvsJob* job = new CvsJob(this);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "annotate";

        if (!revision.isEmpty())
            *job << "-r"<<KShell::quoteArg(revision);

        *job << KShell::quoteArg(info.fileName());

        return job;
    }
    if (job) delete job;
    return NULL;
}

CvsJob* CvsProxy::edit(const QString& repo, const KUrl::List& files)
{
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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

CvsJob* CvsProxy::add(const QString & repo, const KUrl::List & files, bool binary)
{
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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

CvsJob* CvsProxy::update(const QString & repo, const KUrl::List & files,
                      const QString & updateOptions, bool pruneDirs, bool createDirs)
{
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "update";

        if (pruneDirs)
            *job << "-P";
        if (createDirs)
            *job << "-d";
        if (!updateOptions.isEmpty())
            *job << updateOptions;

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
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
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

CvsJob * CvsProxy::status(const KUrl & directory, bool recursive, bool taginfo)
{
    kDebug() << k_funcinfo << endl;

    CvsJob* job = new CvsJob(this);
    if ( prepareJob(job, directory.toLocalFile()) ) {
        *job << "cvs";
        *job << "status";

        if (!recursive)
            *job << "-l";
        if (taginfo)
            *job << "-v";

        return job;
    }
    if (job) delete job;
    return NULL;
}

#include "cvsproxy.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
