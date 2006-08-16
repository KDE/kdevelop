/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Matt Rogers <mattr@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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
#ifndef KDEVFILEMANAGER_H
#define KDEVFILEMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <kurl.h>
#include "kdevitemmodel.h"

class KDevProjectEditor;
class KDevProjectModel;
class KDevProject;
class KDevProjectItem;
class KDevProjectFolderItem;
class KDevProjectTargetItem;
class KDevProjectFileItem;
class KDialogBase;

/**
 * @short An interface to project file management
 *
 * KDevFileManager is the class you want to implement for integrating
 * a project manager in KDevelop.  For build systems, implement its
 * child class, KDevBuildManager.
 *
 * These classes \e do \e not cause files, folders etc. to be created
 * or removed on disk.  They simply read from and write to the file(s)
 * which describe the structure (eg. CMakeLists.txt for cmake, Makefile.am for automake, etc).
 *
 * @author Roberto Raggi, Matt Rogers, Hamish Rodda
 */
class KDEVINTERFACES_EXPORT KDevFileManager: public QObject
{
    Q_OBJECT

public:
    /**
     * Create a file manager object, and perform the import the project from disk.
     *
     * Ideally, the file manager should keep an eye on the file(s) which are used to determine
     * which files exist in this project, and update the in-memory structure accordingly
     * when a change is detected.
     *
     * @todo should forced refresh function exist?
     * @todo add error reporting mechanism
     */
    KDevFileManager(QObject *parent = 0);
    virtual ~KDevFileManager();

    /** Features the file manager supports */
    enum Feature
    {
        None,              ///< This project supports nothing
        Folders  = 0x0001, ///< Folders are supported by the project
        Targets  = 0x0002, ///< Targets are supported by the project
        Files    = 0x0003, ///< Files are supported by the project
    };
    Q_DECLARE_FLAGS( Features, Feature )

    /** @return The current project. */
    virtual KDevProject *project() const;

    /**
     * This method initialize the model item @arg dom
     * @return The list of the sub folders
     */
    virtual QList<KDevProjectFolderItem*> parse(KDevProjectFolderItem *dom);

    /**
     * This method creates the root item from the file @arg fileName
     * @return The created item
     */
    virtual KDevProjectItem *import(KDevProjectModel *model, const KUrl &fileName);

    /**
     * Return the top item in this heirachy, which represents the root folder of the source
     * hierarchy.
     *
     */
    virtual KDevProjectFolderItem* top();

    /**
     * Add a folder to the project
     *
     * Adds the folder specified by @p folder to @p parent and modifies the
     * underlying build system if needed
     */
    virtual KDevProjectFolderItem* addFolder(const KUrl& folder, KDevProjectFolderItem *parent);
    

    /**
     * Add a file to a folder
     *
     * Adds the file specified by @p file to the folder @p parent and modifies
     * the underlying build system if needed. The file is not added to a target
     */
    virtual KDevProjectFileItem* addFile(const KUrl& folder, KDevProjectFolderItem *parent);

    /**
     * Remove a folder from the project
     *
     * Removes the folder specified by @p folder from folder @p parent and
     * modifies the underlying build system if needed.
     */
    virtual bool removeFolder(KDevProjectFolderItem *folder); 

    /**
     * Remove a file from the project
     *
     * Removes the file specified by @p file and 
     * modifies the underlying build system if needed. If the file being 
     * removed is also part of a target, it should be removed from the target as well
     */
    virtual bool removeFile(KDevProjectFileItem *file);

    /**
     * Rename a file in the project
     * 
     * Renames the file specified by @p oldFile to @p newFile
     * 
     */
    virtual bool renameFile(KDevProjectFileItem* oldFile,
                            const KUrl& newFile);
    /**
     * Rename a folder in the project
     * 
     * Renames the folder specified by @p oldFile to @p newFile
     */
    virtual bool renameFolder(KDevProjectFolderItem* oldFolder,
                              const KUrl& newFolder );

signals:
    void projectItemConfigWidget(const QList<KDevProjectItem*> &dom, KDialogBase *dialog);

    void folderAdded( KDevProjectFolderItem* folder );
    void folderRemoved( KDevProjectFolderItem* folder );
    void folderRenamed( const KUrl& oldFolder,
                        KDevProjectFolderItem* newFolder );

    void fileAdded(KDevProjectFileItem* file);
    void fileRemoved(KDevProjectFileItem* file);
    void fileRenamed(const KUrl& oldFile,
                     KDevProjectFileItem* newFile);

private:
    class Private;
    Private* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KDevFileManager::Features )

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
