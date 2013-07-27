/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
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
                                        const QVariantList &args = QVariantList() );
    virtual ~AbstractFileManagerPlugin();

//
// IProjectFileManager interface
//
    virtual Features features() const;

    virtual ProjectFolderItem* addFolder( const KUrl& folder, ProjectFolderItem *parent );
    virtual ProjectFileItem* addFile( const KUrl& file, ProjectFolderItem *parent );
    virtual bool removeFilesAndFolders( const QList<ProjectBaseItem*> &items );
    virtual bool moveFilesAndFolders(const QList< ProjectBaseItem* >& items, ProjectFolderItem* newParent);
    virtual bool copyFilesAndFolders(const KUrl::List& items, ProjectFolderItem* newParent);
    virtual bool renameFolder( ProjectFolderItem *folder, const KUrl& url );
    virtual bool renameFile( ProjectFileItem *file, const KUrl& url );

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
     * The default implementation will query all IProjectFilter plugins and ask them
     * whether a given url should be included or not.
     *
     * @return True when @p url should belong to @p project, false otherwise.
     */
    virtual bool isValid(const KUrl& url, const bool isFolder, IProject* project) const;

    /**
     * Customization hook enabling you to create custom FolderItems if required.
     *
     * The default implementation will return a simple @c ProjectFolderItem
     */
    virtual ProjectFolderItem* createFolderItem( KDevelop::IProject* project, const KUrl& url,
                                                 KDevelop::ProjectBaseItem* parent = 0);

    /**
     * Customization hook enabling you to create custom FileItems if required.
     *
     * The default implementation will return a simple @c ProjectFileItem
     */
    virtual ProjectFileItem* createFileItem( KDevelop::IProject* project, const KUrl& url,
                                             KDevelop::ProjectBaseItem* parent);

    /**
     * @return the @c KDirWatch for the given @p project.
     */
    KDirWatch* projectWatcher( KDevelop::IProject* project ) const;

Q_SIGNALS:
    void folderAdded(KDevelop::ProjectFolderItem* folder);
    void folderRemoved(KDevelop::ProjectFolderItem* folder);
    void folderRenamed(const KUrl& oldFolder, KDevelop::ProjectFolderItem* newFolder);

    void fileAdded(KDevelop::ProjectFileItem* file);
    void fileRemoved(KDevelop::ProjectFileItem* file);
    void fileRenamed(const KUrl& oldFile, KDevelop::ProjectFileItem* newFile);

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

    Q_PRIVATE_SLOT(d, void pluginLoaded( KDevelop::IPlugin* ) )
    Q_PRIVATE_SLOT(d, void unloadingPlugin( KDevelop::IPlugin* ) )
};

}

#endif // KDEVPLATFORM_ABSTRACTGENERICMANAGER_H
