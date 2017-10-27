/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2010-2012 Milian Wolff <mail@milianw.de>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_ABSTRACTFILEMANAGERPLUGIN_H
#define KDEVPLATFORM_ABSTRACTFILEMANAGERPLUGIN_H

#include "projectexport.h"

#include "interfaces/iprojectfilemanager.h"

#include <QVariant>

#include <interfaces/iplugin.h>

class KDirWatch;

namespace KDevelop {

class AbstractFileManagerPluginPrivate;
class AbstractFileManagerPluginImportBenchmark;

/**
 * This class can be used as a common base for file managers.
 *
 * It supports remote files using KIO and uses KDirWatch to synchronize with on-disk changes.
 */
class KDEVPLATFORMPROJECT_EXPORT AbstractFileManagerPlugin : public IPlugin, public virtual IProjectFileManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectFileManager )

public:
    explicit AbstractFileManagerPlugin( const QString& componentName, QObject *parent = nullptr,
                                        const QVariantList &args = QVariantList() );
    ~AbstractFileManagerPlugin() override;

//
// IProjectFileManager interface
//
    Features features() const override;

    ProjectFolderItem* addFolder( const Path& folder, ProjectFolderItem *parent ) override;
    ProjectFileItem* addFile( const Path& file, ProjectFolderItem *parent ) override;
    bool removeFilesAndFolders( const QList<ProjectBaseItem*> &items ) override;
    bool moveFilesAndFolders(const QList< ProjectBaseItem* >& items, ProjectFolderItem* newParent) override;
    bool copyFilesAndFolders(const Path::List& items, ProjectFolderItem* newParent) override;
    bool renameFolder(ProjectFolderItem* folder, const Path& newPath) override;
    bool renameFile(ProjectFileItem* file, const Path& newPath) override;

    QList<ProjectFolderItem*> parse( ProjectFolderItem *item ) override;
    ProjectFolderItem *import( IProject *project ) override;
    bool reload(ProjectFolderItem* item) override;
    KJob* createImportJob(ProjectFolderItem* item) override;

protected:
//
// AbstractFileManagerPlugin interface
//
    /**
     * Filter interface making it possible to hide files and folders from a project.
     *
     * The default implementation will query all IProjectFilter plugins and ask them
     * whether a given url should be included or not.
     *
     * @return True when @p path should belong to @p project, false otherwise.
     */
    virtual bool isValid(const Path& path, const bool isFolder, IProject* project) const;

    /**
     * Customization hook enabling you to create custom FolderItems if required.
     *
     * The default implementation will return a simple @c ProjectFolderItem
     */
    virtual ProjectFolderItem* createFolderItem( IProject* project, const Path& path,
                                                 ProjectBaseItem* parent = nullptr);

    /**
     * Customization hook enabling you to create custom FileItems if required.
     *
     * The default implementation will return a simple @c ProjectFileItem
     */
    virtual ProjectFileItem* createFileItem( IProject* project, const Path& path,
                                             ProjectBaseItem* parent);

    /**
     * @return the @c KDirWatch for the given @p project.
     */
    KDirWatch* projectWatcher( IProject* project ) const;

Q_SIGNALS:
    void reloadedFileItem(KDevelop::ProjectFileItem* file);
    void reloadedFolderItem(KDevelop::ProjectFolderItem* folder);

    void folderAdded(KDevelop::ProjectFolderItem* folder);
    void folderRemoved(KDevelop::ProjectFolderItem* folder);
    void folderRenamed(const KDevelop::Path& oldFolder, KDevelop::ProjectFolderItem* newFolder);

    void fileAdded(KDevelop::ProjectFileItem* file);
    void fileRemoved(KDevelop::ProjectFileItem* file);
    void fileRenamed(const KDevelop::Path& oldFile, KDevelop::ProjectFileItem* newFile);

private:
    const QScopedPointer<class AbstractFileManagerPluginPrivate> d;
    friend class AbstractFileManagerPluginPrivate;
public:
    friend class AbstractFileManagerPluginImportBenchmark;
};

}

#endif // KDEVPLATFORM_ABSTRACTFILEMANAGERPLUGIN_H
