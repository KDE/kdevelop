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
#ifndef KDEVVERSIONCONTROL_H
#define KDEVVERSIONCONTROL_H

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <qmap.h>
#include <qvaluelist.h>

#include <kdevplugin.h>

/**
@file kdevversioncontrol.h
Version control system interface and utility classes.
*/

/**
Info about file state in VCS.

Used, for example in file views to display VCS related information about files.
*/
struct VCSFileInfo
{
    /**State of the file.*/
    enum FileState { 
        Unknown        /**<No VCS information about a file is known.*/, 
        Added          /**<File was added to the repository but not commited.*/, 
        Uptodate       /**<File was updated or it is already at up to date version.*/, 
        Modified       /**<File was modified locally.*/, 
        Conflict       /**<Local version conflicts with the one in a repository.*/, 
        Sticky         /**<File is sticky.*/, 
        NeedsPatch     /**<File needs a patch.*/, 
        NeedsCheckout  /**<File needs to be checkout again.*/, 
        Directory      /**<This is a directory.*/ 
    };

    /**Constructor.*/
    VCSFileInfo() {}
    /**Constructor.
    @param fn The file name (without a path).
    @param workRev The current working revision of a file.
    @param repoRev The last revision of a file in the repository.
    @param aState The state of a file.*/    
    VCSFileInfo( QString fn, QString workRev, QString repoRev, FileState aState )
        : fileName(fn), workRevision(workRev), repoRevision(repoRev), state(aState) {}

    /**The file name.*/
    QString fileName;   // Yeah, _just_ the file name ;-)
    /**The working revision number.*/
    QString workRevision;
    /**The repository revision number.*/
    QString repoRevision;
    /**The state of a file.*/
    FileState state;

    /**@return A descriptive string with all VCS related info about the file.*/
    QString toString() const
    {
        return "(" + fileName + ", " + workRevision + ", " + repoRevision + ", " + state2String( state ) + ")";
    }

    /**@return A textual VCS state description.*/
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

/**@class FileDom
Info for a bunch of files that got modified.
This is a type definition: @code QMap<QString,VCSFileInfo> VCSFileInfoMap; @endcode
*/
typedef QMap<QString,VCSFileInfo> VCSFileInfoMap;

class KDevVCSFileInfoProvider;


/**
KDevelop version control system interface.
This is the abstract base class which encapsulates everything
necessary for communicating with version control systems.
VCS support plugins should implement this interface.

Instances that implement this interface are available through extension architecture:
@code
KDevVersionControl *vcs = extension<KDevVersionControl>("KDevelop/VersionControl");
if (vcs) {
    // do something
} else {
    // fail
}
@endcode
@sa KDevPlugin::extension method documentation.
*/
class KDevVersionControl: public KDevPlugin
{
    Q_OBJECT

public:
    /**Constructs a VCS plugin.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.
    @param name The internal name which identifies the plugin.*/
    KDevVersionControl(const KDevPluginInfo *info, QObject *parent, const char *name )
        :KDevPlugin(info, parent, name ) {}

    /**Creates a new project in the passed path @p dir. This should instantiate
    VCS infrastructure and import a project into the VCS in that directory.
    @param dir The absolute path to the directory where VCS infrastructure should be
    created.*/
    virtual void createNewProject(const QString& dir) = 0;

    /**Fetches a module from remote repository, so it can be used for importing.
    @return true in case of success.*/
    virtual bool fetchFromRepository() = 0;

    /**@return The file info provider for this version control (0 if none is available).*/
    virtual KDevVCSFileInfoProvider *fileInfoProvider() const = 0;

    /**Checks if the directory is valid for this version control (for example
    CVS may check for the presence of "<dirPath>/CVS/" subdir and something else)
    @param dirPath The absolute path of the directory.
    @return true if the directory is valid for this version control
    <b>warning</b>: this returns false by default.*/
    virtual bool isValidDirectory(const QString &dirPath) const = 0;


signals:
    /**Emitted when the Version Control has finished importing a module from remote
    repository
    @param destinationDir The directory where the module has been fetched.*/
    void finishedFetching(QString destinationDir);

};

/**
Basic interface for providing info on file registered in a version control repository repository.
*/
class KDevVCSFileInfoProvider: public QObject
{
    Q_OBJECT
public:
    /**Constructor.
    @param parent The parent VCS plugin.
    @param name The name of a provider object.*/
    KDevVCSFileInfoProvider(KDevVersionControl *parent, const char *name)
        : QObject( parent, name ), m_owner(parent) {}

    /**Gets the status for local files in the specified directory: 
    the info are collected locally so they are necessarily in sync with the repository
    
    This is a <b>synchronous operation</b> (blocking).
    @param dirPath The relative (to project dir) directory path to stat.
    @return Status for all <b>registered</b> files.*/
    virtual const VCSFileInfoMap *status(const QString &dirPath) = 0;

    /**Starts a request for directory status to the remote repository.
    Requests and answers are asynchronous.
    
    This is an <b>asynchronous operation for requesting data</b>, so
    for obvious reasons: the caller must connect the statusReady() signal and 
    check for the return value of this method.
    @param dirPath The (relative to project directory) directory which status you are asking for.
    @param callerData The pointer to some data you want the provider will return 
    to you when it has done.
    @return true if the request has been successfully started, false otherwise.*/
    virtual bool requestStatus( const QString &dirPath, void *callerData ) = 0;

signals:
    /**Emitted when the status request to remote repository has finished.
    @param fileInfoMap The status for <b>registered in repository</b> files.
    @param callerData The pointer to some data you want the provider will return 
    to you when it has done
    @see requestStatus for to find out when this signal should be used.*/
    void statusReady(const VCSFileInfoMap &fileInfoMap, void *callerData);

protected:
    /**@return The version control which owns this provider.*/
    KDevVersionControl *owner() const { return m_owner; }

private:
    KDevVersionControl *m_owner;

private:
    KDevVCSFileInfoProvider( const KDevVCSFileInfoProvider & );
    KDevVCSFileInfoProvider &operator=( const KDevVCSFileInfoProvider & );
};

#endif
