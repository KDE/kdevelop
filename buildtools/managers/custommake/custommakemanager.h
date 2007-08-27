/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef CUSTOMMAKEMANAGER_H
#define CUSTOMMAKEMANAGER_H

#include <ibuildsystemmanager.h>
#include <iplugin.h>

template <typename T> class QList;
class QString;

namespace KDevelop
{
    class IProject;
    class ProjectItem;
    class ProjectFolderItem;
    class ProjectBaseItem;
    class ProjectFileItem;
    class ProjectTargetItem;
    class IProjectBuilder;
}

class CustomMakeManager : public KDevelop::IPlugin, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )
    Q_INTERFACES( KDevelop::IProjectFileManager )
public:

    explicit CustomMakeManager( QObject *parent = NULL, const QStringList& args = QStringList()  );

    virtual ~CustomMakeManager();

    virtual Features features() const { return Features(Folders | Targets | Files); }
    /**
     * Provide access to the builder
     */
    virtual KDevelop::IProjectBuilder* builder( KDevelop::ProjectItem*) const;

    /**
     * Provide a list of include directories.
     */
    virtual KUrl::List includeDirectories(KDevelop::ProjectBaseItem*) const;

    /**
     * Provide a list of files that contain the preprocessor defines for the
     * project
     */
    virtual QMap<QString,QString> preprocessorDefines(KDevelop::ProjectBaseItem*) const;

    /**
     * Create a new target
     *
     * Creates the target specified by @p target to the folder @p parent and
     * modifies the underlying build system if needed
     */
    virtual KDevelop::ProjectTargetItem* createTarget(const QString& target,
                                            KDevelop::ProjectFolderItem *parent);

    /**
     * Add a file to a target
     *
     * Adds the file specified by @p file to the target @p parent and modifies
     * the underlying build system if needed.
     */
    virtual bool addFileToTarget(KDevelop::ProjectFileItem *file, KDevelop::ProjectTargetItem *parent);

    /**
     * Remove a target
     *
     * Removes the target specified by @p target and
     * modifies the underlying build system if needed.
     */
    virtual bool removeTarget(KDevelop::ProjectTargetItem *target);

    /**
     * Remove a file from a target
     *
     * Removes the file specified by @p file from the folder @p parent and
     * modifies the underlying build system if needed. The file is not removed
     * from the folder it is in
     */
    virtual bool removeFileFromTarget(KDevelop::ProjectFileItem *file,
                                      KDevelop::ProjectTargetItem *parent);

    /**
     * Get the toplevel build directory for the project
     */
    virtual KUrl buildDirectory(KDevelop::ProjectItem*) const;

    /**
     * Get a list of all the targets in this project
     *
     * The list returned by this function should be checked to verify it is not
     * empty before using it
     *
     * @return The list of targets for this project
     * @todo implement
     */
    virtual QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectItem*) const;

///////////////////////////////////////////////////////////////////////

    /**
     * This method initialize the model item @arg dom
     * @return The list of the sub folders
     */
    virtual QList<KDevelop::ProjectFolderItem*> parse(KDevelop::ProjectFolderItem *dom);

    /**
     * This method creates the root item from the file @arg fileName
     * @return The created item
     */
    virtual KDevelop::ProjectItem *import(KDevelop::IProject *project);

    /**
     * Add a folder to the project
     *
     * Adds the folder specified by @p folder to @p parent and modifies the
     * underlying build system if needed
     */
    virtual KDevelop::ProjectFolderItem* addFolder(const KUrl& folder,
                                            KDevelop::ProjectFolderItem *parent);


    /**
     * Add a file to a folder
     *
     * Adds the file specified by @p file to the folder @p parent and modifies
     * the underlying build system if needed. The file is not added to a target
     */
    virtual KDevelop::ProjectFileItem* addFile(const KUrl& folder, KDevelop::ProjectFolderItem *parent);

    /**
     * Remove a folder from the project
     *
     * Removes the folder specified by @p folder from folder @p parent and
     * modifies the underlying build system if needed.
     */
    virtual bool removeFolder(KDevelop::ProjectFolderItem *folder);

    /**
     * Remove a file from the project
     *
     * Removes the file specified by @p file and
     * modifies the underlying build system if needed. If the file being
     * removed is also part of a target, it should be removed from the target as well
     */
    virtual bool removeFile(KDevelop::ProjectFileItem *file);

    /**
     * Rename a file in the project
     *
     * Renames the file specified by @p oldFile to @p newFile
     *
     */
    virtual bool renameFile(KDevelop::ProjectFileItem* oldFile,
                            const KUrl& newFile);
    /**
     * Rename a folder in the project
     *
     * Renames the folder specified by @p oldFile to @p newFile
     */
    virtual bool renameFolder(KDevelop::ProjectFolderItem* oldFolder,
                              const KUrl& newFolder );

    // IPlugin interface
    QPair<QString, QList<QAction*> > requestContextMenuActions( KDevelop::Context* );

    /**
     * Initialize targets by reading Makefile in @arg dir
     * @return Target lists in Makefile at @arg dir.
     */
    QStringList parseCustomMakeFile( const KUrl &makefile );

private Q_SLOTS:
    void slotCtxTriggered();
    void updateTargetMenu();

    /// Displays targets in top src dir
    void targetMenuActivated( QAction* );
    void slotBuilt( KDevelop::ProjectBaseItem* item );

private:
    class Private;
    Private *d;
};
#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
