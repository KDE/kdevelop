/*
    SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2002-2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
    SPDX-FileCopyrightText: 2003 Mario Scalas <mario.scalas@libero.it>
    SPDX-FileCopyrightText: 2003-2004 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPROJECT_H
#define KDEVPLATFORM_IPROJECT_H

#include <QObject>

#include <KSharedConfig>

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
    Q_PROPERTY(QString projectName READ name CONSTANT)
public:
    /**
     * Constructs a project.
     *
     * @param parent The parent object for the plugin.
     */
    explicit IProject(QObject *parent = nullptr);

    /// Destructor.
    ~IProject() override;

    /**
     * Get the file manager for the project
     *
     * @return the file manager for the project, if one exists; otherwise null
     */
    virtual IProjectFileManager* projectFileManager() const = 0;

    /**
     * Get the build system manager for the project
     *
     * @return the build system manager for the project, if one exists; otherwise null
     */
    virtual IBuildSystemManager* buildSystemManager() const = 0;

    /**
     * Get the plugin that manages the project
     * This can be used to get other interfaces like IBuildSystemManager
     */
    virtual IPlugin* managerPlugin() const = 0;

    /**
     * Get the version control plugin for this project
     * This may return 0 if the project is not under version control
     * or version control has been disabled for this project
     */
    virtual IPlugin* versionControlPlugin() const = 0;

    /**
     * With this the top-level project item can be retrieved
     */
    virtual ProjectFolderItem* projectItem() const = 0;

    /**
     * @return all items with the corresponding @p path
     */
    virtual QList<ProjectBaseItem*> itemsForPath( const IndexedString& path ) const = 0;

    /**
     * @return all file items with the corresponding @p file path
     */
    virtual QList<ProjectFileItem*> filesForPath( const IndexedString& file ) const = 0;

    /**
     * @return all folder items with the corresponding @p folder path
     */
    virtual QList<ProjectFolderItem*> foldersForPath( const IndexedString& folder ) const = 0;

    /**
     * @return the path to the project file
     */
    virtual Path projectFile() const = 0;
    /** Get the url of the project file.*/
    virtual KSharedConfigPtr projectConfiguration() const = 0;

    virtual void addToFileSet( ProjectFileItem* item ) = 0;
    virtual void removeFromFileSet( ProjectFileItem* item ) = 0;
    virtual QSet<IndexedString> fileSet() const = 0;

    /** Returns whether the project is ready to be used or not.
        A project won't be ready for use when it's being reloaded or still loading
    */
    virtual bool isReady() const=0;

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
    virtual bool inProject(const IndexedString &path) const = 0;

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

    /** This method is invoked when the project needs to be closed. */
    virtual void close() = 0;
};

}
#endif
