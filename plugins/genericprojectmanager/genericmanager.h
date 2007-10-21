/* This file is part of KDevelop
    Copyright 2004,2005 Roberto Raggi <roberto@kdevelop.org>

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
#ifndef KDEVGENERICMANAGER_H
#define KDEVGENERICMANAGER_H

#include "iprojectfilemanager.h"
#include "iplugin.h"
#include <QtCore/QVariant>

class QFileInfo;
class QStringList;
class KUrl;
template <typename T> class QList;
namespace KDevelop
{
class ProjectBaseItem;
class ProjectFolderItem;
class ProjectFileItem;
}

class KDialogBase;

class GenericProjectManager: public KDevelop::IPlugin, public KDevelop::IProjectFileManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectFileManager )
public:
    explicit GenericProjectManager( QObject *parent = 0,
                     const QVariantList &args = QVariantList() );
    virtual ~GenericProjectManager();

//
// IProjectFileManager interface
//
    virtual Features features() const
    {
        return Features( Folders | Files );
    }

    virtual KDevelop::ProjectFolderItem* addFolder( const KUrl& folder, KDevelop::ProjectFolderItem *parent );
    virtual KDevelop::ProjectFileItem* addFile( const KUrl& file, KDevelop::ProjectFolderItem *parent );
    virtual bool removeFolder( KDevelop::ProjectFolderItem *folder );
    virtual bool removeFile( KDevelop::ProjectFileItem *file );
    virtual bool renameFolder( KDevelop::ProjectFolderItem *folder, const KUrl& url );
    virtual bool renameFile( KDevelop::ProjectFileItem *file, const KUrl& url );

    virtual QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem *item );
    virtual KDevelop::ProjectFolderItem *import( KDevelop::IProject *project );

Q_SIGNALS:
    void projectItemConfigWidget(const QList<KDevelop::ProjectBaseItem*> &dom, KDialogBase *dialog);

    void folderAdded( KDevelop::ProjectFolderItem* folder );
    void folderRemoved( KDevelop::ProjectFolderItem* folder );
    void folderRenamed( const KUrl& oldFolder,
                        KDevelop::ProjectFolderItem* newFolder );

    void fileAdded(KDevelop::ProjectFileItem* file);
    void fileRemoved(KDevelop::ProjectFileItem* file);
    void fileRenamed(const KUrl& oldFile,
                     KDevelop::ProjectFileItem* newFile);

private:
    bool isValid( const QFileInfo &fileName ) const;

private:
    struct GenericProjectManagerPrivate* const d;
};

#endif // KDEVGENERICIMPORTER_H
