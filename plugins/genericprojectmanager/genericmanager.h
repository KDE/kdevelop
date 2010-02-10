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

#include "igenericprojectmanager.h"
#include <interfaces/iplugin.h>
#include <QtCore/QVariant>

class QFileInfo;
class QStringList;
class KUrl;
class KDirWatch;
template <typename T> class QList;

///FIXME: remove once the hack is gone
class KJob;

namespace KIO
{
class Job;
class UDSEntry;
typedef QList<UDSEntry> UDSEntryList;
}

namespace KDevelop
{
class ProjectBaseItem;
class ProjectFolderItem;
class ProjectFileItem;
}

class KDialogBase;

class GenericProjectManager: public KDevelop::IPlugin, public KDevelop::IGenericProjectManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectFileManager )
    Q_INTERFACES( KDevelop::IGenericProjectManager )
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
    virtual bool reload(KDevelop::ProjectFolderItem* item);
    virtual KJob* createImportJob(KDevelop::ProjectFolderItem* item);

    /// first item is includes, second excludes
    typedef QPair<QStringList, QStringList> IncludeRules;

Q_SIGNALS:
    void appendSubDir( KDevelop::ProjectFolderItem* item );

private Q_SLOTS:
    /// @p forceRecursion if true, existing folders will be re-read no matter what
    KJob* eventuallyReadFolder( KDevelop::ProjectFolderItem* item, const bool forceRecursion = false );
    void addJobItems(KDevelop::ProjectFolderItem* baseItem, const KIO::UDSEntryList& entries, const bool forceRecursion);

    void deleted(const QString &path);
    void created(const QString &path);

    void projectClosing(KDevelop::IProject* project);

private:
    /// Stops watching the given folder for changes, only useful for local files.
    void stopWatcher(KDevelop::ProjectFolderItem* folder);
    /// Continues watching the given folder for changes.
    void continueWatcher(KDevelop::ProjectFolderItem* folder);
    /// Common renaming function.
    bool rename( KDevelop::ProjectBaseItem* item, const KUrl& source, const KUrl& destination);

private:
    bool isValid( const KUrl& url, const bool isFolder, KDevelop::IProject* project,
                  const IncludeRules& rules ) const;
    QMap<KDevelop::IProject*, KDirWatch*> m_watchers;
};

#endif // KDEVGENERICIMPORTER_H
