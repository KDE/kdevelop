/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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
#include <QtCore/QVariant>

#include <KDirWatch>

#include <ibuildsystemmanager.h>
#include <iplugin.h>

#include "cmakelistsparser.h"
#include "cmakeprojectvisitor.h"

class QDir;
class QObject;

namespace KDevelop
{
    class IProject;
    class ProjectFolderItem;
    class ProjectBaseItem;
    class ProjectFileItem;
    class ProjectTargetItem;
    class IProjectBuilder;
}

class CMakeFolderItem;
class ICMakeBuilder;

class CMakeProjectManager : public KDevelop::IPlugin, public KDevelop::IBuildSystemManager
{
Q_OBJECT
Q_INTERFACES( KDevelop::IBuildSystemManager )
Q_INTERFACES( KDevelop::IProjectFileManager )
public:
    explicit CMakeProjectManager( QObject* parent = 0, const QVariantList& args = QVariantList() );

    virtual ~CMakeProjectManager();
    virtual Features features() const { return Features(Folders | Targets | Files); }
//     virtual KDevelop::IProject* project() const;
    virtual KDevelop::IProjectBuilder* builder(KDevelop::ProjectFolderItem*) const;
    virtual KUrl buildDirectory(KDevelop::ProjectBaseItem*) const;
    virtual KUrl::List includeDirectories(KDevelop::ProjectBaseItem *) const;
    virtual QHash<QString, QString> defines(KDevelop::ProjectBaseItem *) const;
    virtual QHash<QString, QString> environment(KDevelop::ProjectBaseItem*) const { return QHash<QString,QString>(); }

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
    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem* folder) const;

    virtual QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem* dom );
    virtual KDevelop::ProjectFolderItem* import( KDevelop::IProject *project );
    
public slots:
    void dirtyFile(const QString& file);

private:
    void parseOnly(KDevelop::IProject* project, const KUrl &url);
    void reimport(CMakeFolderItem*);
    
    void initializeProject(KDevelop::IProject* project, const KUrl& baseUrl);
    
    static QStringList guessCMakeModulesDirectories(const QString& cmakeBin);
    static QString guessCMakeShare(const QString& cmakeBin);
    static QString guessCMakeRoot(const QString& cmakeBin);
    
    void includeScript(const QString& File, KDevelop::IProject * project);
    
    VariableMap m_varsDef;
    QStringList m_modulePathDef;
    QMap<KDevelop::IProject*, QStringList> m_modulePathPerProject;
    QMap<KDevelop::IProject*, VariableMap> m_varsPerProject;
    QMap<KDevelop::IProject*, MacroMap> m_macrosPerProject;
    QMap<KDevelop::IProject*, KDirWatch*> m_watchers;
    
    QMap<KUrl, CMakeFolderItem*> m_folderPerUrl;

    QStringList cmakeInitScripts;
    ICMakeBuilder* m_builder;
    QList<KDevelop::ProjectTargetItem*> m_targets;
};

#endif


