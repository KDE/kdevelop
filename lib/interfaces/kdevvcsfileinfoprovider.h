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

#ifndef KDEVVCSFILEINFOPROVIDER_H
#define KDEVVCSFILEINFOPROVIDER_H

#include <qobject.h>
#include <qmap.h>

/**
* Info about file state
*/
struct VCSFileInfo
{
    enum FileState { Unknown, Added, Uptodate, Modified, Conflict, Sticky, NeedsPatch, NeedsCheckout, Directory };

    VCSFileInfo() {}
    VCSFileInfo( QString fn, QString workRev, QString repoRev, FileState aState )
        : fileName(fn), workRevision(workRev), repoRevision(repoRev), state(aState) {}

    QString fileName;   // Yeah, _just_ the file name ;-)
    QString workRevision;
    QString repoRevision;
    FileState state;

    QString toString() const;

    static QString state2String( FileState state );
};

/**
* Info for a bunch of files that got modified
*/
typedef QMap<QString,VCSFileInfo> VCSFileInfoMap;

class KDevVersionControl;

/**
*  A basic interface for providing info on file registered in a version control repository repository
*/
class KDevVCSFileInfoProvider : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor
    */
    KDevVCSFileInfoProvider( KDevVersionControl *parent, const char *name );
    /**
    * Destructor
    */
    virtual ~KDevVCSFileInfoProvider();

    /**
    * <b>Sync interface</b>
    * Status for the local files in the specified directory: the info are collected locally so they are
    * necessarly in sync with the repository
    * @param dirPath relative (to project dir) directory path to stat
    * @return status for all <b>registered</b> files
    */
    virtual const VCSFileInfoMap *status( const QString &dirPath ) const = 0;

    /**
    * <b>Async interface for requesting data</b>
    * Start a request for directory status to the remote repository. Requests and answers are asynchronous
    * for obvious reasons: the caller must connect the statusReady() signal and check for the return
    * value of this method.
    * @param dirPath the (relative to project directory) directory which status you are asking for
    * @param callerData a pointer to some data you want the provider will return to you when it has done
    * @return true if the request has been successfully started, false otherwise
    */
    virtual bool requestStatus( const QString &dirPath, void *callerData ) = 0;
signals:
    /**
    * Emitted when the status request to remote repository has finished
    * @param fileInfoMap status for <b>registered in repository</b> files
    * @param callerData @see requestStatus.
    */
    void statusReady( const VCSFileInfoMap &fileInfoMap, void *callerData );

protected:
    /**
    * @return the version control which owns this version control
    */
    KDevVersionControl *owner() const;

private:
    struct Private;
    Private *d;

    //! Verboten!
private:
    KDevVCSFileInfoProvider( const KDevVCSFileInfoProvider & );
    KDevVCSFileInfoProvider &operator=( const KDevVCSFileInfoProvider & );
};

#endif // KDEVVCSFILEINFOPROVIDER_H
