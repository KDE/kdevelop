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

#ifndef CVSPROXY_H
#define CVSPROXY_H

#include <QString>
#include <KUrl>
#include <KJob>

class CvsJob;

/**
 * This proxy acts as a single point of entry for most of the common cvs commands.
 * It is very easy to use, as the caller does not have to deal wich the CvsJob class directly.
 * All the command line generation and job handling is done internally. The caller gets a CvsJob
 * object returned from the proxy and can then call it's start() method.
 *
 * @note The caller does not have to worry about deleting the KJob. The proxy
 *       connects a default slot to all jobs that he created. See slotResult().
 *       This slot calls deleteLater() on the CvsJob that just finised.
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
 * @note All actions that take a KUrl::List also need an url to the repository which 
 *       must be a common base directory to all files from the KUrl::List.
 *       Actions that just take a single KUrl don't need a repository, the cvs command will be
 *       called directly in the directory of the given file
 * 
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsProxy : public QObject
{
    Q_OBJECT
public:
    CvsProxy(QObject* parent = 0);
    ~CvsProxy();

    bool isValidDirectory(const KUrl &dirPath) const;

    CvsJob* import(const KUrl& directory, 
                const QString & server, const QString& repositoryName,
                const QString& vendortag, const QString& releasetag,
                const QString& message);
    CvsJob* log(const KUrl& file);
    CvsJob* diff(const KUrl& url, const QString& diffOptions="", 
              const QString& revA="", const QString& revB="");
    CvsJob* edit(const QString& repo, const KUrl::List& files);
    CvsJob* unedit(const QString& repo, const KUrl::List& files);
    CvsJob* editors(const QString& repo, const KUrl::List& files);
    CvsJob* commit(const QString& repo, const KUrl::List& files, 
                const QString& message);
    CvsJob* add(const QString& repo, const KUrl::List& files, 
             bool binary = false);
    CvsJob* remove(const QString& repo, const KUrl::List& files);
    CvsJob* update(const QString& repo, const KUrl::List& files, 
                const QString& updateOptions,
                bool pruneDirs = true, bool createDirs = true);

private slots:
    void slotResult(KJob* job);

private:
    bool addFileList(CvsJob* job, const QString& repository, const KUrl::List& urls);

    enum RequestedOperation {
        NormalOperation,
        Import
    };
    bool prepareJob(CvsJob* job, const QString& repository, 
                    enum RequestedOperation op = CvsProxy::NormalOperation);
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
