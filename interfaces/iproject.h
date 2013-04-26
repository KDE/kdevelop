/* This file is part of the KDE project
   Copyright 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2002 Simon Hausmann <hausmann@kde.org>
   Copyright 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright 2006 Matt Rogers <mattr@kde.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef KDEVPLATFORM_IPROJECT_H
#define KDEVPLATFORM_IPROJECT_H

#include <QtCore/QObject>

#include <kurl.h>
#include <ksharedconfig.h>

#include "interfacesexport.h"

class KJob;

template<typename T> class QList;
template<typename T> class QSet;


namespace KDevelop
{

class IPlugin;
class IProjectFileManager;
class IBuildSystemManager;
class ProjectBaseItem;
class ProjectFileItem;
class ProjectFolderItem;
class IndexedString;

/**
 * \brief Object which represents a KDevelop project
 *
 * Provide better descriptions
 */
class KDEVPLATFORMINTERFACES_EXPORT IProject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kdevelop.Project")
    Q_PROPERTY(QString projectName READ name CONSTANT)
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
    Q_SCRIPTABLE virtual IProjectFileManager* projectFileManager() const = 0;

    /**
     * Get the build system manager for the project
     *
     * @return the build system manager for the project, if one exists; otherwise null
     */
    Q_SCRIPTABLE virtual IBuildSystemManager* buildSystemManager() const = 0;

    /**
     * Get the plugin that manages the project
     * This can be used to get other interfaces like IBuildSystemManager
     */
    Q_SCRIPTABLE virtual IPlugin* managerPlugin() const = 0;

    /**
     * Get the version control plugin for this project
     * This may return 0 if the project is not under version control
     * or version control has been disabled for this project
     */
    Q_SCRIPTABLE virtual IPlugin* versionControlPlugin() const = 0;

    /**
     * With this the top-level project item can be retrieved
     */
    Q_SCRIPTABLE virtual ProjectFolderItem* projectItem() const = 0;

    /** Get how much files are there in the project */
    Q_SCRIPTABLE virtual int fileCount() const = 0;

    /** Get the item corresponding to the @p pos file */
    Q_SCRIPTABLE virtual ProjectFileItem* fileAt( int pos) const = 0;

    /** Get a list of all files in the project */
    Q_SCRIPTABLE virtual QList<ProjectFileItem*> files() const = 0;

    /** Get all items corresponding to the @p folder url */
    Q_SCRIPTABLE virtual QList<ProjectBaseItem*> itemsForUrl( const KUrl& url ) const = 0;
    
    /** Get all file items corresponding to the @p file url */
    Q_SCRIPTABLE virtual QList<ProjectFileItem*> filesForUrl( const KUrl& file ) const = 0;

    /** Get all folder items corresponding to the @p folder url */
    Q_SCRIPTABLE virtual QList<ProjectFolderItem*> foldersForUrl( const KUrl& folder ) const = 0;

    /** Make the model to reload */
    Q_SCRIPTABLE virtual void reloadModel() = 0;
//     virtual KUrl projectConfigFile() const = 0;
//     virtual KUrl projectDefaultsConfigFile() const = 0;

    /** Get the url of the project file.*/
    Q_SCRIPTABLE virtual KUrl projectFileUrl() const = 0;
    virtual KSharedConfig::Ptr projectConfiguration() const = 0;

    virtual void addToFileSet( const IndexedString& ) = 0;
    virtual void removeFromFileSet( const IndexedString& ) = 0;
    virtual QSet<IndexedString> fileSet() const = 0;

    /** Returns whether the project is ready to be used or not.
        A project won't be ready for use when it's being reloaded or still loading
    */
    virtual bool isReady() const=0;

Q_SIGNALS:
    void fileAddedToSet( KDevelop::IProject* project, const KDevelop::IndexedString& file);
    void fileRemovedFromSet( KDevelop::IProject* project, const KDevelop::IndexedString& file);

public Q_SLOTS:
    /**
     * @brief Get the project folder
     * @return The canonical absolute directory of the project.
     */
    virtual Q_SCRIPTABLE const KUrl folder() const = 0;

    /** Returns the name of the project. */
    virtual QString name() const = 0;

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
     * Check if the url specified by @a url is part of the project.
     * @a url can be either a relative url (to the project directory) or
     * an absolute url.
     *
     * @param url the url to check
     *
     * @return true if the url @a url is a part of the project.
     */
    virtual bool inProject(const KUrl &url) const = 0;

};

}
#endif
