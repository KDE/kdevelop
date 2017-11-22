/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
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
#include <QUrl>

#include <KLocalizedString>
#include <KMessageBox>
#include <kshell.h>

#include "cvsjob.h"
#include "cvsannotatejob.h"
#include "cvslogjob.h"
#include "cvsstatusjob.h"
#include "cvsdiffjob.h"
#include <debug.h>

#include <interfaces/iplugin.h>

CvsProxy::CvsProxy(KDevelop::IPlugin* parent)
: QObject(parent), vcsplugin(parent)
{
}

CvsProxy::~CvsProxy()
{
}

bool CvsProxy::isValidDirectory(const QUrl& dirPath) const
{
    const QFileInfo fsObject( dirPath.toLocalFile() );
    QDir dir = fsObject.isDir() ? QDir(fsObject.filePath()) : fsObject.dir();

    return dir.exists(QStringLiteral("CVS"));
}

bool CvsProxy::isVersionControlled(const QUrl& filePath) const
{
    const QFileInfo fsObject( filePath.toLocalFile() );
    QDir dir = fsObject.isDir() ? QDir(fsObject.filePath()) : fsObject.dir();

    if( !dir.cd(QStringLiteral("CVS")) )
        return false;

    if( fsObject.isDir() )
        return true;

    QFile cvsEntries( dir.absoluteFilePath(QStringLiteral("Entries")) );
    cvsEntries.open( QIODevice::ReadOnly );
    QString cvsEntriesData = cvsEntries.readAll();
    cvsEntries.close();
    return ( cvsEntriesData.indexOf( fsObject.fileName() ) != -1 );
}


bool CvsProxy::prepareJob(CvsJob* job, const QString& repository, enum RequestedOperation op)
{
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "cvs import" isValidDirectory() would fail as the
    // directory is not yet under CVS control
    if (op == CvsProxy::NormalOperation &&
        !isValidDirectory(QUrl::fromLocalFile(repository))) {
        qCDebug(PLUGIN_CVS) << repository << " is not a valid CVS repository";
        return false;
    }

    // clear commands and args from a possible previous run
    job->clear();

    // setup the working directory for the new job
    job->setDirectory(repository);

    return true;
}

bool CvsProxy::addFileList(CvsJob* job, const QString& repository, const QList<QUrl>& urls)
{
    QStringList args;

    QDir repoDir(repository);
    foreach(const QUrl &url, urls) {
        ///@todo this is ok for now, but what if some of the urls are not
        ///      to the given repository
        const QString file = repoDir.relativeFilePath(url.toLocalFile());
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
                str = "-r"+rev.revisionValue().toString();
            break;

        case KDevelop::VcsRevision::Date:
            if (rev.revisionValue().isValid())
                str = "-D"+rev.revisionValue().toString();
            break;

        case KDevelop::VcsRevision::GlobalNumber: // !! NOT SUPPORTED BY CVS !!
        default:
            break;
    }

    return str;
}

QString CvsProxy::convertRevisionToPrevious(const KDevelop::VcsRevision& rev)
{
    QString str;

    // this only works if the revision is a real revisionnumber and not a date or special
    switch (rev.revisionType())
    {
        case KDevelop::VcsRevision::FileNumber:
            if (rev.revisionValue().isValid()) {
                QString orig = rev.revisionValue().toString();

                // First we need to find the base (aka branch-part) of the revision number which will not change
                QString base(orig);
                base.truncate(orig.lastIndexOf(QLatin1Char('.')));

                // next we need to cut off the last part of the revision number
                // this number is a count of revisions with a branch
                // so if we want to diff to the previous we just need to lower it by one
                int number = orig.midRef(orig.lastIndexOf(QLatin1Char('.'))+1).toInt();
                if (number > 1) // of course this is only possible if our revision is not the first on the branch
                    number--;

                str = QStringLiteral("-r") + base + '.' + QString::number(number);
                qCDebug(PLUGIN_CVS) << "Converted revision "<<orig<<" to previous revision "<<str;
            }
            break;

        default:
            break;
    }

    return str;
}

CvsJob* CvsProxy::log(const QUrl &url, const KDevelop::VcsRevision& rev)
{
    QFileInfo info(url.toLocalFile());
    // parent folder path for files, otherwise the folder path itself
    const QString repo = info.isFile() ? info.absolutePath() : info.absoluteFilePath();

    CvsLogJob* job = new CvsLogJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "log";

        QString convRev = convertVcsRevisionToString(rev);
        if (!convRev.isEmpty()) {
            convRev.replace(QLatin1String("-D"), QLatin1String("-d"));
            *job << convRev;
        }

        if (info.isFile()) {
            *job << KShell::quoteArg(info.fileName());
        }

        return job;
    }
    delete job;
    return nullptr;
}

CvsJob* CvsProxy::diff(const QUrl& url,
            const KDevelop::VcsRevision& revA,
            const KDevelop::VcsRevision& revB,
            const QString& diffOptions)
{
    QFileInfo info(url.toLocalFile());

    CvsDiffJob* job = new CvsDiffJob(vcsplugin);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "diff";

        if (!diffOptions.isEmpty())
            *job << diffOptions;

        QString rA;
        if (revA.revisionType() == KDevelop::VcsRevision::Special) {
            // We only support diffing to previous for now
            // Other special types might be added later
            KDevelop::VcsRevision::RevisionSpecialType specialtype =
                    revA.revisionValue().value<KDevelop::VcsRevision::RevisionSpecialType>();
            if (specialtype == KDevelop::VcsRevision::Previous) {
                rA = convertRevisionToPrevious(revB);
            }
        } else {
            rA = convertVcsRevisionToString(revA);
        }
        if (!rA.isEmpty())
            *job << rA;

        QString rB = convertVcsRevisionToString(revB);
        if (!rB.isEmpty())
            *job << rB;

        // in case the QUrl is a directory there is no filename
        if (!info.fileName().isEmpty())
            *job << KShell::quoteArg(info.fileName());

        return job;
    }
    delete job;
    return nullptr;
}

CvsJob * CvsProxy::annotate(const QUrl& url, const KDevelop::VcsRevision& rev)
{
    QFileInfo info(url.toLocalFile());

    CvsAnnotateJob* job = new CvsAnnotateJob(vcsplugin);
    if ( prepareJob(job, info.absolutePath()) ) {
        *job << "cvs";
        *job << "annotate";

        QString revision = convertVcsRevisionToString(rev);
        if (!revision.isEmpty())
            *job << revision;

        *job << KShell::quoteArg(info.fileName());

        return job;
    }
    delete job;
    return nullptr;
}

CvsJob* CvsProxy::edit(const QString& repo, const QList<QUrl>& files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "edit";

        addFileList(job, repo, files);

        return job;
    }
    delete job;
    return nullptr;
}


CvsJob* CvsProxy::unedit(const QString& repo, const QList<QUrl>& files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "unedit";

        addFileList(job, repo, files);

        return job;
    }
    delete job;
    return nullptr;
}

CvsJob* CvsProxy::editors(const QString& repo, const QList<QUrl>& files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "editors";

        addFileList(job, repo, files);

        return job;
    }
    delete job;
    return nullptr;
}

CvsJob* CvsProxy::commit(const QString& repo, const QList<QUrl>& files, const QString& message)
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
    delete job;
    return nullptr;
}

CvsJob* CvsProxy::add(const QString& repo, const QList<QUrl>& files,
                      bool recursiv, bool binary)
{
    Q_UNUSED(recursiv);
    // FIXME recursiv is not implemented yet
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
    delete job;
    return nullptr;
}

CvsJob * CvsProxy::remove(const QString& repo, const QList<QUrl>& files)
{
    CvsJob* job = new CvsJob(vcsplugin);
    if ( prepareJob(job, repo) ) {
        *job << "cvs";
        *job << "remove";

        *job << "-f"; //existing files will be deleted

        addFileList(job, repo, files);

        return job;
    }
    delete job;
    return nullptr;
}

CvsJob * CvsProxy::update(const QString& repo, const QList<QUrl>& files,
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
    delete job;
    return nullptr;
}

CvsJob * CvsProxy::import(const QUrl& directory,
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
    delete job;
    return nullptr;
}

CvsJob * CvsProxy::checkout(const QUrl& targetDir,
                            const QString & server, const QString & module,
                            const QString & checkoutOptions,
                            const QString & revision,
                            bool recursive,
                            bool pruneDirs)
{
    CvsJob* job = new CvsJob(vcsplugin);
    ///@todo when doing a checkout we don't have the targetdir yet,
    ///      for now it'll work to just run the command from the root
    if ( prepareJob(job, QStringLiteral("/"), CvsProxy::CheckOut) ) {
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

        *job << "-d" << targetDir.toString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);

        *job << module;

        return job;
    }
    delete job;
    return nullptr;
}

CvsJob * CvsProxy::status(const QString & repo, const QList<QUrl> & files, bool recursive, bool taginfo)
{
    CvsStatusJob* job = new CvsStatusJob(vcsplugin);
    job->setCommunicationMode( KProcess::MergedChannels );
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
    delete job;
    return nullptr;
}

