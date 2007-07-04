/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#ifndef CMAKEMANAGER_H
#define CMAKEMANAGER_H

#include <QList>
#include <QString>

#include <ibuildsystemmanager.h>
#include <iplugin.h>

#include "cmakelistsparser.h"

class QDir;
class QObject;

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

class CMakeFolderItem;
class MacroAst;

class cmLocalGenerator;

class CMakeProjectManager : public KDevelop::IPlugin, public KDevelop::IBuildSystemManager
{
Q_OBJECT
Q_INTERFACES( KDevelop::IBuildSystemManager )
Q_INTERFACES( KDevelop::IProjectFileManager )
public:
    explicit CMakeProjectManager( QObject* parent = 0, const QStringList& args = QStringList() );

    virtual ~CMakeProjectManager();
    virtual Features features() const { return Features(Folders | Targets | Files); }
//     virtual KDevelop::IProject* project() const;
    virtual KDevelop::IProjectBuilder* builder(KDevelop::ProjectItem*) const { return 0; }
    virtual KUrl buildDirectory(KDevelop::ProjectItem*) const;
    virtual KUrl::List includeDirectories(KDevelop::ProjectBaseItem *) const;
    virtual KUrl::List preprocessorDefines(KDevelop::ProjectBaseItem *) const { return KUrl::List(); }

    virtual KDevelop::ProjectFolderItem* addFolder( const KUrl& /*folder */,
            KDevelop::ProjectFolderItem* /*parent*/ ) { return false; }

    virtual KDevelop::ProjectTargetItem* createTarget( const QString&,
            KDevelop::ProjectFolderItem* ) { return false; }

    virtual KDevelop::ProjectFileItem* addFile( const KUrl&,
                                          KDevelop::ProjectFolderItem* ) { return false; }

    virtual bool addFileToTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* ) { return false; }

    virtual bool removeFolder( KDevelop::ProjectFolderItem* ) { return false; }

    virtual bool removeTarget( KDevelop::ProjectTargetItem* ) { return false; }

    virtual bool removeFile( KDevelop::ProjectFileItem* ) { return false; }
    virtual bool removeFileFromTarget( KDevelop::ProjectFileItem*,
                                       KDevelop::ProjectTargetItem* ) { return false; }

    virtual bool renameFile(KDevelop::ProjectFileItem*, const KUrl&) { return false; }
    virtual bool renameFolder(KDevelop::ProjectFolderItem*, const KUrl&) { return false; }

    QList<KDevelop::ProjectTargetItem*> targets() const;

    virtual QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem* dom );
    virtual KDevelop::ProjectItem* import( KDevelop::IProject *project );
    virtual KUrl findMakefile( KDevelop::ProjectFolderItem* dom ) const;
    virtual KUrl::List findMakefiles( KDevelop::ProjectFolderItem* dom ) const;

    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectItem*) const { return QList<KDevelop::ProjectTargetItem*>(); }

private:
    QStringList resolveVariables(const QStringList & vars);
    KDevelop::IProject* m_project;
    CMakeFolderItem* m_rootItem;
    KDevelop::IProjectBuilder* m_builder;

    KUrl::List m_includeDirList;
    CMakeListsParser m_parser;
    QHash<QString, QStringList> m_vars;
    QHash<QString, MacroAst*> m_macros;
//     ProjectInfo m_projectInfo;
};

#endif


