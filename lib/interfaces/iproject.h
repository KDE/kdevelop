/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright     2006 Matt Rogers <mattr@kde.org>
   Copyright     2007 Andreas Pakulat <apaku@gmx.de>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef IPROJECT_H
#define IPROJECT_H

#include <QtCore/QObject>

#include <kurl.h>

#include "interfacesexport.h"
#include <ksharedconfig.h>
template<typename T> class QList;

class KJob;


namespace KDevelop
{

class IPlugin;
class IProjectFileManager;
class ProjectModel;
class ProjectItem;
class ProjectFileItem;
class ProjectFolderItem;

/**
 * \brief Object which represents a KDevelop project
 *
 * Provide better descriptions
 */
class KDEVPLATFORMINTERFACES_EXPORT IProject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kdevelop.Project")
public:
    /**
     * Constructs a project.
     *
     * @param parent The parent object for the plugin.
     */
    IProject(QObject *parent = 0);

    /// Destructor.
    virtual ~IProject();

    /**
     * Get the file manager for the project
     *
     * @return the file manager for the project, if one exists; otherwise null
     */
    virtual IProjectFileManager* fileManager() const = 0;

    /**
     * Get the plugin that manages the project
     * This can be used to get other interfaces like IBuildSystemManager
     */
    virtual IPlugin* managerPlugin() const = 0;

    /**
     * With this the top-level project item can be retrieved
     */
    virtual ProjectItem* projectItem() const = 0;

    /** Get a list of all files in the project */
    virtual int fileCount() const = 0;
    virtual ProjectFileItem* fileAt( int ) const = 0;

    virtual QList<ProjectFileItem*> files() const = 0;

    virtual ProjectFileItem *fileForUrl( const KUrl& ) const = 0;

//     virtual KUrl projectConfigFile() const = 0;
//     virtual KUrl projectDefaultsConfigFile() const = 0;
    virtual KUrl projectFileUrl() const = 0;
    virtual KSharedConfig::Ptr projectConfiguration() const = 0;
//     virtual void setLocalFile( const KUrl& ) = 0;
//     virtual void setGlobalFile( const KUrl& ) = 0;

public Q_SLOTS:
    /**
     * @brief Open a project
     * This method opens a project and starts the process of loading the
     * data for the project from disk.
     * @param projectFileUrl The url pointing to the location of the project
     * file to load
     * The project name is taken from the Name key in the project file in
     * the 'General' group
     */
    virtual Q_SCRIPTABLE bool open(const KUrl &projectFileUrl) = 0;

    /** This method is invoked when the project needs to be closed. */
    virtual Q_SCRIPTABLE void close() = 0;

    /**
     * @brief Get the project folder
     * @return The canonical absolute directory of the project.
     */
    virtual Q_SCRIPTABLE const KUrl& folder() const = 0;

    /** Returns the name of the project. */
    virtual Q_SCRIPTABLE QString name() const = 0;

    /**
     * Find the url relative to the project directory equivalent to @a absoluteUrl.
     * This function does not check to see if the file is contained within the
     * project; for that, use inProject().
     *
     * @param absoluteUrl Absolute url to convert
     * @deprecated use KUrl::relativeUrl instead
     * @returns absoluteUrl relative to projectDirectory()
     **/
    virtual KUrl relativeUrl(const KUrl& absoluteUrl) const = 0;

    /**
     * Returns the absolute url corresponding to the given \a relativeUrl and
     * the project directory.
     *
     * @param relativeUrl Relative url to convert
     *
     * @returns the absolute URL relative to projectDirectory()
     **/
    virtual KUrl urlRelativeToProject(const KUrl& relativeUrl) const = 0;

    /**
     * Check if the url specified by @a url is part of the project.
     * @a url can be either a relative url (to the project directory) or
     * an absolute url.
     *
     * @param url the url to check
     *
     * @return true if the url @a url is a part of the project.
     */
    virtual bool inProject(const KUrl &url) const = 0;

    /**
     * The persistent AST storage for this project.
     */
//     virtual IPersistentHash *persistentHash() const = 0;

};

}
#endif
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
