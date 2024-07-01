/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPROJECTFILEMANAGER_H
#define KDEVPLATFORM_IPROJECTFILEMANAGER_H

#include <QObject>

#include <project/projectexport.h>
#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>
#include <util/path.h>

class KJob;

namespace KDevelop
{

class IProject;
class ProjectBaseItem;
class ProjectFolderItem;
class ProjectFileItem;

/**
 * @short An interface to project file management
 *
 * FileManager is the class you want to implement for integrating
 * a project manager in KDevelop.  For build systems, implement its
 * child class, BuildManager.
 *
 * These classes \e do \e not cause files, folders etc. to be created
 * or removed on disk.  They simply read from and write to the file(s)
 * which describe the structure (eg. CMakeLists.txt for cmake, Makefile.am for automake, etc).
 *
 * @author Roberto Raggi, Matt Rogers, Hamish Rodda, Milian Wolff
 */
class KDEVPLATFORMPROJECT_EXPORT IProjectFileManager
{
public:

    virtual ~IProjectFileManager();
    /** Features the file manager supports */
    enum Feature
    {
        None     = 0 ,     ///< This manager supports nothing
        Folders  = 1 << 0, ///< Folders are supported by the manager
        Targets  = 1 << 1, ///< Targets are supported by the manager
        Files    = 1 << 2  ///< Files are supported by the manager
    };
    Q_DECLARE_FLAGS( Features, Feature )

    /**
     * @return the Features supported by the filemanager
     */
    virtual Features features() const = 0;

    /**
     * This method initialize the model item @arg dom
     * @return The list of the sub folders
     */
    virtual QList<ProjectFolderItem*> parse(ProjectFolderItem *dom) = 0;

    /**
     * This method creates the root item from the file @arg fileName
     * @return The created item
     */
    virtual ProjectFolderItem *import(IProject *project) = 0;

    /**
     * @brief This method creates an import job for the given @p item
     *
     * @details The default implementation should be suitable for most needs,
     * it'll create an instance of class @ref ImportProjectJob
     *
     * @return a job that imports the project
     */
    virtual KJob* createImportJob(ProjectFolderItem* item);

    /**
     * Add a folder to the project and create it on disk.
     *
     * Adds the folder specified by @p folder to @p parent and modifies the
     * underlying build system if needed
     */
    virtual ProjectFolderItem* addFolder(const Path& folder, ProjectFolderItem* parent) = 0;

    /**
     * Add a file to a folder and create it on disk.
     *
     * Adds the file specified by @p file to the folder @p parent and modifies
     * the underlying build system if needed. The file is not added to a target
     */
    virtual ProjectFileItem* addFile(const Path& file, ProjectFolderItem *parent) = 0;

    /**
     * Remove files or folders from the project and delete them from disk
     *
     * Removes the files or folders specified by @p items and
     * modifies the underlying build system if needed.
     *
     * Note: Do not attempt to remove subitems along with their parents
     */
    virtual bool removeFilesAndFolders(const QList<ProjectBaseItem*> &items) = 0;

    /**
     * Move files and folders within a given project
     *
     * Moves the files or folders specified by @p items to @p newParent and
     * modifies the underlying build system as needed
     *
     * Note: Do not attempt to move subitems along with their parents
     */
    virtual bool moveFilesAndFolders(const QList< KDevelop::ProjectBaseItem* > &items, KDevelop::ProjectFolderItem* newParent) = 0;

    /**
     * Copy files and folders within a given project
     *
     * Copies the files or folders specified by @p items to @p newParent and
     * modifies the underlying build system as needed
     *
     * Note: Do not attempt to copy subitems along with their parents
     */
    virtual bool copyFilesAndFolders(const Path::List &items, KDevelop::ProjectFolderItem* newParent) = 0;

    /**
     * Rename a file in the project
     *
     * Renames the file specified by @p oldFile to @p newPath
     */
    virtual bool renameFile(ProjectFileItem* file, const Path& newPath) = 0;

    /**
     * Rename a folder in the project
     *
     * Renames the folder specified by @p oldFile to @p newPath
     */
    virtual bool renameFolder(ProjectFolderItem* oldFolder, const Path& newPath) = 0;

    /**
     * Reload an item in the project
     *
     * Reloads the item specified by @p item
     */
    virtual bool reload(ProjectFolderItem* item) = 0;

Q_SIGNALS:
    void folderAdded(KDevelop::ProjectFolderItem* folder);
    void folderRemoved(KDevelop::ProjectFolderItem* folder);
    void folderRenamed(const KDevelop::Path& oldFolder, KDevelop::ProjectFolderItem* newFolder);

    void fileAdded(KDevelop::ProjectFileItem* file);
    void fileRemoved(KDevelop::ProjectFileItem* file);
    void fileRenamed(const KDevelop::Path& oldFile, KDevelop::ProjectFileItem* newFile);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IProjectFileManager::Features)
}

Q_DECLARE_INTERFACE( KDevelop::IProjectFileManager, "org.kdevelop.IProjectFileManager")

#endif
