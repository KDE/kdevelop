/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSPARTIMPL_H
#define CVSPARTIMPL_H

#include <qobject.h>
#include <qstringlist.h>
#include <qguardedptr.h>
#include <kurl.h>

#include <kdevversioncontrol.h>

class CvsServicePart;
class KDialogBase;
class KURL;
class KURL::List;
class CvsProcessWidget;
class KDevMainWindow;
class KDevCore;
class KDevDiffFrontend;
class QDir;
class JobScheduler;
class KDevVCSFileInfoProvider;
class CVSFileInfoProvider;

/**
* @short This is the base class for implementation of the core service.
*
* This is an attempt to separate the container part (CvsServicePart) and its implementation
* for reducing code complexity for module (cvspart.{h,cpp} was becoming too
* cumbersome). So a CvsServicePart can have several implementations, one directly wrapping
* 'cvs' command and another using cervisia's cvsservice.
*
* @author Mario Scalas
*/
class CvsServicePartImpl : public QObject
{
    friend class CvsServicePart;

    Q_OBJECT
public:
    //! Available Cvs operations
    enum CvsOperation
    {
        opFakeStub, opAdd, opCommit, opUpdate, opRevert, opRemove, opLog, opDiff, opTag, opUnTag, opEdit, opUnEdit, opEditors
    };

    /**
    * Costructor
    * @param part the CvsServicePart component
    * @param name
    */
    CvsServicePartImpl( CvsServicePart *part, const char *name=0 );
    /**
    * Destructor
    */
    virtual ~CvsServicePartImpl();

    /**
    * Do login into repository. The component will show a dialog requesting the
    * needed data to the user.
    */
    virtual void login();
    /**
    * Do logout. Of course one must be logged into repository first ;-)
    */
    virtual void logout();
    /**
    * Do checkout of module from some remote directory. Requested data will be
    * collected here.
	* @return true if the operation was successful
    */
    virtual bool checkout();
    /**
    * Commit the specified files (as KURL) to repository.
    * @param urlList
    */
    virtual void commit( const KURL::List& urlList );
    /**
    * Update the specified files (as KURL): files will be
    * updated if not locally modified.
    * @param urlList
    */
    virtual void update( const KURL::List& urlList );
    /**
    * Add the specified files (as KURL) to repository.
    * @param urlList
    * @param binary is the file binary or plain text
    */
    virtual void add( const KURL::List& urlList, bool binary = false );
    /**
    * Mark the specified files (as KURL) for beeing edited
    * @param urlList
    */
    virtual void edit( const KURL::List& urlList );
    /**
    * Remove editing mark from the specified files (as KURL)
    * @param urlList
    */
    virtual void unedit( const KURL::List& urlList );
    /**
    * Show list of editors for the specified files (as KURL)
    * @param urlList
    */
    virtual void editors( const KURL::List& urlList );
    /**
    * Remove the specified files (as KURL) from repository.
    * @param urlList
    */
    virtual void remove( const KURL::List& urlList );
    /**
    * Produce a log of changes about the specified files.
    * @param urlList
    */
    virtual void log( const KURL::List& urlList );
    /**
    * Produce a diff of the the specified files (as KURL). The diff could
    * be displayed in the diff frontend or in an ad-hoc container.
    * @param urlList
    */
    virtual void diff( const KURL::List& urlList );
    /**
    * Tag the specified files (as KURL) with a release or branch tag.
    * @param urlList
    */
    virtual void tag( const KURL::List& urlList );
    /**
    * Remove tag from the specified files (as KURL) in repository.
    * @param urlList
    */
    virtual void unTag( const KURL::List& urlList );
    /**
    * Remove tag from the specified files (as KURL) in repository.
    * @param urlList
    */
    virtual void removeStickyFlag( const KURL::List& urlList );
    /**
    * Add the specified files (as KURL) to the .cvsignore file.
    * @param urlList
    */
    virtual void addToIgnoreList( const KURL::List& urlList );
    /**
    * Commit the specified files (as KURL) to repository.
    * @param urlList
    */
    virtual void removeFromIgnoreList( const KURL::List& urlList );
    /**
    * Creates a new project with cvs support, that is will import the
    * generated sources in the repository.
    * @param dirName path to project directory on local system
    * @param cvsRsh value for the CVS_RSH env var (for accessing :ext:
    *        repositories)
    * @param location cvs server path (i.e. :pserver:marios@cvs.kde.org:/home/kde)
    * @param message an initial creation message for the project
    * @param module the module into repository where to put this source tree
    * @param vendor vendor string
    * @param release release tag
    * @param mustInitRoot if true will attempt to initialize $CVSROOT if not already prepared
    */
    virtual void createNewProject( const QString &dirName,
        const QString &cvsRsh, const QString &location,
        const QString &message, const QString &module, const QString &vendor,
        const QString &release, bool mustInitRoot );

    /**
    * @return true if the directory is valid as CVS directory (has the /CVS/ dir inside) (FORWARDER)
    */
    virtual bool isValidDirectory( const QDir &dir ) const;
    /**
    * @return a reference to the custom FileInforProvider object (FORWARDER)
    */
    KDevVCSFileInfoProvider *fileInfoProvider() const;


// Helpers
public:
    /**
    * Stops the CVS job, both currently executing and queued.
    * @todo queuing is not yet implemented
    */
    void flushJobs();
    /**
    * @return a reference to the process widget: many worker methods
    * display their output in it and the CvsServicePart will embed it in the
    * bottom embedded view.
    */
    CvsProcessWidget *processWidget() const;

signals:
    void warning( const QString &msg );
    /**
    * Emitted when the component has terminated checkout operation
    * @param checkedDir directory where the module has been checked out
    *        (will be empty if the operation failed)
    */
    void checkoutFinished( QString checkedDir );

private slots:
    void slotJobFinished( bool normalExit, int exitStatus );
    void slotDiffFinished( bool normalExit, int exitStatus );
    void slotCheckoutFinished( bool normalExit, int exitStatus );
    void slotProjectOpened();

private:
    /**
    * Call this every time a slot for cvs operations starts!! (It will setup the
    * state (file/dir URL, ...).
    * It will also display proper error messages so the caller must only exit if
    * it fails (return false); if return true than basic requisites for cvs operation
    * are satisfied.
    * @return true and the valid URLs paths in m_fileList if the operation can be performed,
    *         false otherwise.
    */
    bool prepareOperation( const KURL::List &someUrls, CvsOperation op );
    /**
    * Call this every time a slot for cvs operations ends!! (It will restore the state for a new
    * operation) and notify clients about changes.
    */
    void doneOperation( const KURL::List &someUrls = KURL::List(), CvsOperation op = opFakeStub );

    void emitFileStateModified( const KURL::List &urls, VCSFileInfo::FileState &commonState );

    /**
    *   @return true if the @p url is present in CVS/Entry file
    */
    static bool isRegisteredInRepository( const QString &projectDirectory, const KURL &url );
    /**
    * Ideally this function will take a bunch of URLs and validate them (they are valid files,
    * are files registered in CVS, are on a supported filesystem, ...). Currently checks
    * only for files belonging to the repository ;)
    * @param projectDirectory
    * @param urls list of KURL to check (the list can be modified during the operation)
    * @param op type of cvs operation, as pecified in @see CvsOperation enum
    */
    static void validateURLs( const QString &projectDirectory, KURL::List &urls, CvsOperation op );

    /**
    * Add file to it's respective ignore list. This means that, for example, if you
    * add '/home/mario/src/myprj/mylib/module1/bad.cpp' then the string 'bad.cpp' will be
    * appended to file '/home/mario/src/myprj/mylib/module1/.cvsignore'.
    * @param projectDirectory
    * @param url url to be added to the check list.
    */
    static void addToIgnoreList( const QString &projectDirectory, const KURL &url );

    /**
    * Add files to their respective ignore list. This means that, for example, if you
    * add '/home/mario/src/myprj/mylib/module1/bad.cpp' then the string 'bad.cpp' will be
    * appended to file '/home/mario/src/myprj/mylib/module1/.cvsignore'.
    * @param projectDirectory
    * @param urls list of urls to be added to the check list.
    */
    static void addToIgnoreList( const QString &projectDirectory, const KURL::List &urls );

    /**
    * Remove file from it's respective .ignore files. As specified for @see addToIgnoreList
    * function, this means that, for example, if you remove '/home/mario/src/myprj/mylib/module1/bad.cpp'
    * then a search for the string 'bad.cpp' will be performed on file
    * '/home/mario/src/myprj/mylib/module1/.cvsignore': if found, it will be removed, otherwise
    * nothing will be removed.
    * @param projectDirectory
    * @param url url to be removed from the check list.
    */
    static void removeFromIgnoreList( const QString &projectDirectory, const KURL &url );

    /**
    * Remove files from their respective .ignore files. As specified for @see addToIgnoreList
    * function, this means that, for example, if you remove '/home/mario/src/myprj/mylib/module1/bad.cpp'
    * then a search for the string 'bad.cpp' will be performed on file
    * '/home/mario/src/myprj/mylib/module1/.cvsignore': if found, it will be removed, otherwise
    * nothing will be removed.
    * @param projectDirectory
    * @param urls list of urls to be removed from the check list.
    */
    static void removeFromIgnoreList( const QString &projectDirectory, const KURL::List &urls );
    /**
    * Implementation for requesting user input when files are added to project
    */
    void addFilesToProject( const QStringList &filesToAdd );
    /**
    * Implementation for requesting user input when files are removed from project
    */
    void removedFilesFromProject(const QStringList &filesToRemove);
    /**
    * Check each file in the list against CVS and returns a new list with the files
    * currently registered in the repository: if none is registered the returned list
    * is (quite rightly) empty.
    */
    QStringList checkFileListAgainstCVS( const QStringList &filesToCheck ) const;

    //! Changelog filename (currently "CHANGELOG" )
    static const QString changeLogFileName;
    //! Four spaces for every log line (except the first which includes the
    //! developers name)
    static const QString changeLogPrependString;

    // Internal short-cuts
    KDevMainWindow *mainWindow() const;
    KDevCore *core() const;
    QString projectDirectory() const;
    KDevDiffFrontend *diffFrontend() const;

    /** Locate and setup DCOP CvsService */
    bool requestCvsService();
    /** De-initialize and release CvsService */
    void releaseCvsService();

    CvsService_stub *m_cvsService;
    Repository_stub *m_repository;

    /** Used for storing module path between start and ending of check-out */
    QString modulePath;

    CVSFileInfoProvider *m_fileInfoProvider;
    JobScheduler *m_scheduler;
    /** Reference to owner part */
    CvsServicePart *m_part;

    //! Reference to widget integrated in the "bottom tabbar" (IDEAL)
    //! (_Must_ be initialized by derived class)
    QGuardedPtr<CvsProcessWidget> m_widget;

    //! Urls which to work upon
    const KURL::List &urlList() const;
    /**
    * @param relativeToProjectDir if true paths will be provided as relative to project directory,
    * as absolute paths otherwise
    * @return These are the file path contained in the urls provided for convenience
    * has been requested for.
    */
    QStringList fileList( bool relativeToProjectDir = true ) const;
    /** Last operation type: we save it so we can retrieve and use in slot*Exited() */
    CvsOperation lastOperation() const;

    // Both this data members are set by prepareOperation() method
    KURL::List m_urlList;
    CvsOperation m_lastOperation;
};

#endif
