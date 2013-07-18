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

#ifndef KDEVPLATFORM_ABSTRACTGENERICMANAGER_H
#define KDEVPLATFORM_ABSTRACTGENERICMANAGER_H

#include "projectexport.h"

#include "interfaces/iprojectfilemanager.h"

#include <QVariant>

#include <interfaces/iplugin.h>

class KDirWatch;

namespace KDevelop {

class FileManagerListJob;

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
    explicit AbstractFileManagerPlugin( const KComponentData& instance, QObject *parent = 0,
                                        const QVariantList& args = QVariantList() );
    virtual ~AbstractFileManagerPlugin();

//
// IProjectFileManager interface
//
    virtual Features features() const;

    virtual ProjectFolderItem* addFolder( const Path& folder, ProjectFolderItem *parent );
    virtual ProjectFileItem* addFile( const Path& file, ProjectFolderItem *parent );
    virtual bool removeFilesAndFolders( const QList<ProjectBaseItem*> &items );
    virtual bool moveFilesAndFolders(const QList< ProjectBaseItem* >& items, ProjectFolderItem* newParent);
    virtual bool copyFilesAndFolders(const Path::List& items, ProjectFolderItem* newParent);
    virtual bool renameFolder(ProjectFolderItem* folder, const Path& newPath);
    virtual bool renameFile(ProjectFileItem* file, const Path& newPath);

    virtual QList<ProjectFolderItem*> parse( ProjectFolderItem *item );
    virtual ProjectFolderItem *import( IProject *project );
    virtual bool reload(ProjectFolderItem* item);
    virtual KJob* createImportJob(ProjectFolderItem* item);

protected:
//
// AbstractFileManagerPlugin interface
//
    /**
     * Filter interface making it possible to hide files and folders from a project.
     *
     * The default implementation will show all files.
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
                                                 ProjectBaseItem* parent = 0);

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

    /**
     * Sets a list of filenames to ignore when importing a project
     * the filter is applied to files and folders, so both will be ignored.
     *
     * The filenames are matched via QString::operator==(), so no wildcard or
     * regex-matching for now
     * 
     * This can be used for things like VCS-folders/files or other things a
     * plugin might want to hide.
     */
    void setImportFileNameFilter( const QStringList& filterNames );

private:
    struct Private;
//     friend class Private;
    Private* const d;

    Q_PRIVATE_SLOT(d, KJob* eventuallyReadFolder( ProjectFolderItem* item,
                                                  const bool forceRecursion = false ))
    Q_PRIVATE_SLOT(d, void addJobItems(FileManagerListJob* job,
                                       ProjectFolderItem* baseItem,
                                       const KIO::UDSEntryList& entries,
                                       const bool forceRecursion))

    Q_PRIVATE_SLOT(d, void deleted(const QString &path))
    Q_PRIVATE_SLOT(d, void created(const QString &path))

    Q_PRIVATE_SLOT(d, void projectClosing(KDevelop::IProject* project))
    Q_PRIVATE_SLOT(d, void jobFinished(KJob* job))

Q_SIGNALS:
    void folderAdded(KDevelop::ProjectFolderItem* folder);
    void folderRemoved(KDevelop::ProjectFolderItem* folder);
    void folderRenamed(const KDevelop::Path& oldFolder, KDevelop::ProjectFolderItem* newFolder);

    void fileAdded(KDevelop::ProjectFileItem* file);
    void fileRemoved(KDevelop::ProjectFileItem* file);
    void fileRenamed(const KDevelop::Path& oldFile, KDevelop::ProjectFileItem* newFile);
};

}

#endif // KDEVPLATFORM_ABSTRACTGENERICMANAGER_H
