/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>

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
#include "kdevvcsfileinfoprovider.h"

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
    KDevVersionControl( const QString& pluginName, const QString& icon, QObject *parent, const char *name );

    /**
    * Destructor. Removes current VCS from the list of available ones.
    */
    virtual ~KDevVersionControl();

    /**
    * Return a pointer to the associated configuration widget for this vcs.
    */
    virtual QWidget* newProjectWidget( QWidget */*parent*/ );

    /**
    * Creates a new project in the passed path @p dir.
    */
    virtual void createNewProject( const QString&/* dir*/ );

    /**
    * Fetch a module from remote repository, so it can be used for importing
    */
    virtual bool fetchFromRepository();

    /**
    * @return the file info provider for this version control (0 if none is available)
    */
    virtual KDevVCSFileInfoProvider *fileInfoProvider() const;

    /**
    * Checks if the directory is valid for this version control (for example
    * CVS may check for the presence of "<dirPath>/CVS/" subdir and something else)
    * @param dirPath absolute path of the directory
    * @return true if the directory is valid for this version control
    *     <b>warning</b>: this returns false by default
    */
    virtual bool isValidDirectory( const QString &dirPath ) const;

    /**
    * @return the unique identifier for this plugin (so it can be used for retrieving
    * it from collections).
    */
    QString uid() const;

    /**
     * Add @p vcs to the list of the registered VCS.
     */
    void registerVersionControl( KDevVersionControl *vcs );

    /**
     * Use the specified version control object as default vcs
     * @param vcsToUse
     */
    void setVersionControl( KDevVersionControl *vcsToUse );

    /**
     * Returns the Version Control System having the specified uinque identifier @p uid.
     */
    KDevVersionControl *versionControlByName( const QString &uid ) const;

    /**
     * Returns a list with unique identifiers among the version control systems.
     */
    QStringList registeredVersionControls() const;

    /**
     * Remove (<B>not</B> delete) @p vcs from the list of the registered VCS.
     */
    void unregisterVersionControl( KDevVersionControl *vcs );


signals:
    /**
    * Emitted when the Version Control has finished importing a module from remote
    * repository
    * @param destinationDir is the directory where the module has been fetched
    */
    void finishedFetching( QString destinationDir );

private:
    //@fixme - This is the same pointer as in the KDevPlugin baseclass, it is duplicated here 
	// to maintain the encapsulation without redesigning the whole thing... // teatime
    class KDevApi * m_api;

};

#endif
