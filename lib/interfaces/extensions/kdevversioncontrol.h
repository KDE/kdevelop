/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KDEVVERSIONCONTROL_H_
#define _KDEVVERSIONCONTROL_H_

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "kdevplugin.h"


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

    QString toString() const
    {
        return "(" + fileName + ", " + workRevision + ", " + repoRevision + ", " + state2String( state ) + ")";
    }

    static QString state2String( FileState state )
    {
        switch (state)
        {
            case Added: return "added";
            case Uptodate: return "up-to-date";
            case Modified: return "modified";
            case Conflict: return "conflict";
            case Sticky: return "sticky";
            case NeedsPatch: return "needs patch";
            case NeedsCheckout: return "needs check-out";
            case Directory: return "directory";
            case Unknown:
            default:
                return "unknown";
        }
    }

};

/**
* Info for a bunch of files that got modified
*/
typedef QMap<QString,VCSFileInfo> VCSFileInfoMap;

class KDevVCSFileInfoProvider;


/**
* This is the abstract base class which encapsulates everything
* necessary for communicating with version control systems.
*/
class KDevVersionControl : public KDevPlugin
{
    Q_OBJECT

public:
    /**
    * Builds the object.
    *   @p pluginName   this _is_ *** unique *** identintifier between all available version controls.
    *   @p icon     symbolic name for the icon
    *   @p parent   the parent QObject for this version control system
    *   @p name     a name for this object; it may be null since it is usefull for debugging
    */
    KDevVersionControl(const KDevPluginInfo *info, QObject *parent, const char *name )
        :KDevPlugin(info, parent, name ) {}

    /**
    * Creates a new project in the passed path @p dir.
    */
    virtual void createNewProject( const QString&/* dir*/ ) = 0;

    /**
    * Fetch a module from remote repository, so it can be used for importing
    */
    virtual bool fetchFromRepository() = 0;

    /**
    * @return the file info provider for this version control (0 if none is available)
    */
    virtual KDevVCSFileInfoProvider *fileInfoProvider() const = 0;

    /**
    * Checks if the directory is valid for this version control (for example
    * CVS may check for the presence of "<dirPath>/CVS/" subdir and something else)
    * @param dirPath absolute path of the directory
    * @return true if the directory is valid for this version control
    *     <b>warning</b>: this returns false by default
    */
    virtual bool isValidDirectory( const QString &dirPath ) const = 0;


signals:
    /**
    * Emitted when the Version Control has finished importing a module from remote
    * repository
    * @param destinationDir is the directory where the module has been fetched
    */
    void finishedFetching( QString destinationDir );

};

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
    KDevVCSFileInfoProvider( KDevVersionControl *parent, const char *name )
        : QObject( parent, name ), m_owner(parent) {}

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
    KDevVersionControl *owner() const { return m_owner; }

private:
    KDevVersionControl *m_owner;

private:
    KDevVCSFileInfoProvider( const KDevVCSFileInfoProvider & );
    KDevVCSFileInfoProvider &operator=( const KDevVCSFileInfoProvider & );
};

#endif
