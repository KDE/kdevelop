/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CVSPROXY_H
#define KDEVPLATFORM_PLUGIN_CVSPROXY_H


#include <QUrl>

#include <KJob>
#include <vcs/vcsrevision.h>

class CvsJob;

namespace KDevelop
{
class IPlugin;
}

/**
 * This proxy acts as a single point of entry for most of the common cvs commands.
 * It is very easy to use, as the caller does not have to deal which the CvsJob class directly.
 * All the command line generation and job handling is done internally. The caller gets a CvsJob
 * object returned from the proxy and can then call it's start() method.
 *
 * Here is and example of how to user the proxy:
 * @code
 * CvsJob* job = proxy->editors( repo, urls );
 * if ( job ) {
 *     connect(job, SIGNAL( result(KJob*) ),
 *             this, SIGNAL( jobFinished(KJob*) ));
 *     job->start();
 * }
 * @endcode
 *
 * @note All actions that take a QList<QUrl> also need an url to the repository which
 *       must be a common base directory to all files from the QList<QUrl>.
 *       Actions that just take a single QUrl don't need a repository, the cvs command will be
 *       called directly in the directory of the given file
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsProxy : public QObject
{
    Q_OBJECT
public:
    CvsProxy(KDevelop::IPlugin* parent = 0);
    ~CvsProxy();

    bool isValidDirectory(QUrl dirPath) const;
    bool isVersionControlled(QUrl filePath) const;

    CvsJob* import(const QUrl &directory,
                const QString & server, const QString& repositoryName,
                const QString& vendortag, const QString& releasetag,
                const QString& message);
    CvsJob* log(const QUrl &file, const KDevelop::VcsRevision& rev);
    CvsJob* diff(const QUrl &url,
                 const KDevelop::VcsRevision& revA, 
                 const KDevelop::VcsRevision& revB,
                 const QString& diffOptions="");
    CvsJob* annotate(const QUrl &url, const KDevelop::VcsRevision& rev);
    CvsJob* edit(const QString& repo, const QList<QUrl>& files);
    CvsJob* unedit(const QString& repo, const QList<QUrl>& files);
    CvsJob* editors(const QString& repo, const QList<QUrl>& files);
    CvsJob* commit(const QString& repo, const QList<QUrl>& files,
                const QString& message);
    CvsJob* add(const QString& repo, const QList<QUrl>& files,
                bool recursiv = true, bool binary = false);
    CvsJob* remove(const QString& repo, const QList<QUrl>& files);
    CvsJob* update(const QString& repo, const QList<QUrl>& files,
                const KDevelop::VcsRevision& rev, 
                const QString& updateOptions,
                bool resursive = true, bool pruneDirs = true, bool createDirs = true);

    CvsJob* checkout(const QUrl &targetDir,
                const QString & server, const QString& module,
                const QString& checkoutOptions="",
                const QString& revision="",
                bool recursive = true,
                bool pruneDirs = true);

    CvsJob* status(const QString & repo, const QList<QUrl> & files,
                bool recursive=false, bool taginfo=false);

private:
    bool addFileList(CvsJob* job, const QString& repository, const QList<QUrl>& urls);
    QString convertVcsRevisionToString(const KDevelop::VcsRevision& rev);
    QString convertRevisionToPrevious(const KDevelop::VcsRevision& rev);

    enum RequestedOperation {
        NormalOperation,
        Import,
        CheckOut
    };
    bool prepareJob(CvsJob* job, const QString& repository,
                    enum RequestedOperation op = CvsProxy::NormalOperation);
    KDevelop::IPlugin* vcsplugin;
};

#endif
