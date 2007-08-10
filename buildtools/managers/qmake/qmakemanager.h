/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef QMAKEMANAGER_H
#define QMAKEMANAGER_H

#include <ibuildsystemmanager.h>
#include <iplugin.h>

template <typename T> class QList;
class QString;

//#include "domutil.h"
//#include <projectmodel.h>

class QDir;
class QObject;
class KDialogBase;
class IQMakeBuilder;
class QMakeCache;
class QMakeMkSpecs;
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

class QMakeProjectManager : public KDevelop::IPlugin, public KDevelop::IBuildSystemManager
{
Q_OBJECT
Q_INTERFACES( KDevelop::IProjectFileManager )
Q_INTERFACES( KDevelop::IBuildSystemManager )
public:
    explicit QMakeProjectManager( QObject* parent = 0, const QStringList& args = QStringList() );

    virtual ~QMakeProjectManager();

    virtual Features features() const { return Features(Folders | Targets | Files); }
    virtual KDevelop::IProjectBuilder*  builder(KDevelop::ProjectItem*) const;
    virtual KUrl buildDirectory(KDevelop::ProjectItem*) const;
    virtual KUrl::List includeDirectories(KDevelop::ProjectBaseItem*) const;
    virtual KUrl::List preprocessorDefines(KDevelop::ProjectBaseItem*) const { return KUrl::List(); }

    virtual KDevelop::ProjectTargetItem* createTarget( const QString&,
            KDevelop::ProjectFolderItem* ) { return false; }

    virtual bool addFileToTarget( KDevelop::ProjectFileItem*,
                                  KDevelop::ProjectTargetItem* ) { return false; }

    virtual bool removeTarget( KDevelop::ProjectTargetItem* ) { return false; }

    virtual bool removeFileFromTarget( KDevelop::ProjectFileItem*,
                                       KDevelop::ProjectTargetItem* ) { return false; }

    virtual QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectItem*) const;

    virtual QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem* dom );
    virtual KDevelop::ProjectItem* import( KDevelop::IProject* );
    virtual KUrl findMakefile( KDevelop::ProjectFolderItem* folder ) const;
    virtual KUrl::List findMakefiles( KDevelop::ProjectFolderItem* folder ) const;

    virtual KDevelop::ProjectFolderItem* addFolder( const KUrl&,
            KDevelop::ProjectFolderItem* ) { return 0; }

    virtual KDevelop::ProjectFileItem* addFile( const KUrl&,
            KDevelop::ProjectFolderItem* ) { return 0; }

    virtual bool removeFile( KDevelop::ProjectFileItem* ) { return false; }
    virtual bool removeFolder( KDevelop::ProjectFolderItem* ) { return false; }
    virtual bool renameFile( KDevelop::ProjectFileItem*, const KUrl& ) { return false; }
    virtual bool renameFolder( KDevelop::ProjectFolderItem*, const KUrl&  ) { return false; }

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
    QHash<QString,QString> queryQMake( KDevelop::IProject* ) const;
    QString findBasicMkSpec( const QString& mkspecdir ) const;
    QMakeCache* findQMakeCache( const QString& projectfile ) const;
    IQMakeBuilder* m_builder;
};

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
