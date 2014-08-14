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
class Path;
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

    /**
     * @return all items with the corresponding @p path
     */
    Q_SCRIPTABLE virtual QList<ProjectBaseItem*> itemsForPath( const IndexedString& path ) const = 0;
    KDEVPLATFORMINTERFACES_DEPRECATED Q_SCRIPTABLE virtual QList<ProjectBaseItem*> itemsForUrl( const KUrl& url ) const = 0;

    /**
     * @return all file items with the corresponding @p file path
     */
    Q_SCRIPTABLE virtual QList<ProjectFileItem*> filesForPath( const IndexedString& file ) const = 0;
    KDEVPLATFORMINTERFACES_DEPRECATED Q_SCRIPTABLE virtual QList<ProjectFileItem*> filesForUrl( const KUrl& file ) const = 0;

    /**
     * @return all folder items with the corresponding @p folder path
     */
    Q_SCRIPTABLE virtual QList<ProjectFolderItem*> foldersForPath( const IndexedString& folder ) const = 0;
    KDEVPLATFORMINTERFACES_DEPRECATED Q_SCRIPTABLE virtual QList<ProjectFolderItem*> foldersForUrl( const KUrl& folder ) const = 0;

    /**
     * @return the path to the project file
     */
    virtual Path projectFile() const = 0;
    /** Get the url of the project file.*/
    KDEVPLATFORMINTERFACES_DEPRECATED Q_SCRIPTABLE virtual KUrl projectFileUrl() const = 0;
    virtual KSharedConfig::Ptr projectConfiguration() const = 0;

    virtual void addToFileSet( ProjectFileItem* item ) = 0;
    virtual void removeFromFileSet( ProjectFileItem* item ) = 0;
    virtual QSet<IndexedString> fileSet() const = 0;

    /** Returns whether the project is ready to be used or not.
        A project won't be ready for use when it's being reloaded or still loading
    */
    virtual bool isReady() const=0;

    /**
     * @brief Get the project folder
     * @return The canonical absolute directory of the project.
     */
    KDEVPLATFORMINTERFACES_DEPRECATED virtual Q_SCRIPTABLE const KUrl folder() const = 0;

    /**
     * @brief Get the project path
     * @return The canonical absolute directory of the project.
     */
    virtual Path path() const = 0;

    /** Returns the name of the project. */
    virtual Q_SCRIPTABLE QString name() const = 0;

    /**
     * @brief Check if the project contains an item with the given @p path.
     *
     * @param path the path to check
     *
     * @return true if the path @a path is a part of the project.
     */
    virtual Q_SCRIPTABLE bool inProject(const IndexedString &path) const = 0;

    /**
     * @brief Tells the project what job is reloading it
     *
     * It's useful so that we can tell whether the project manager is busy or not.
     */
    virtual void setReloadJob(KJob* job) = 0;

Q_SIGNALS:
    /**
     * Gets emitted whenever a file was added to the project.
     */
    void fileAddedToSet( KDevelop::ProjectFileItem* item );
    /**
     * Gets emitted whenever a file was removed from the project.
     */
    void fileRemovedFromSet( KDevelop::ProjectFileItem* item );

public Q_SLOTS:
    /** Make the model to reload */
    virtual void reloadModel() = 0;
};

}
#endif
