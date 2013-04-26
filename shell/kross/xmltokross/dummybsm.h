/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
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

#ifndef KDEVPLATFORM_DUMMYBSM_H
#define KDEVPLATFORM_DUMMYBSM_H

#include <interfaces/iplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>

class DummyBSM : public KDevelop::IPlugin, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )
    Q_INTERFACES( KDevelop::IProjectFileManager )
    public:
        explicit DummyBSM(QObject *parent=0, const QVariantList& args=QVariantList(), const KUrl::List& cf=KUrl::List());
        
        QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem* dom ) { Q_UNUSED( dom ); return QList<KDevelop::ProjectFolderItem*>(); }
        KDevelop::ProjectFolderItem* import(KDevelop::IProject *project );

        KDevelop::IProjectBuilder* builder() const { return 0; }
        KUrl buildDirectory(KDevelop::ProjectBaseItem*) const { return m_buildDir; }

        KUrl::List includeDirectories(KDevelop::ProjectBaseItem *) const { qDebug("jojooooooooooojo"); return m_includeDirectories; }
        QHash<QString,QString> defines(KDevelop::ProjectBaseItem *) const { return m_defines; }
        QList<KDevelop::ProjectTargetItem*> targets() const;
        QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem*) const;

        KDevelop::ProjectFolderItem* addFolder( const KUrl& /*folder */, KDevelop::ProjectFolderItem* /*parent*/ ) { return 0; }
        KDevelop::ProjectTargetItem* createTarget( const QString&, KDevelop::ProjectFolderItem* ) { return 0; }
        KDevelop::ProjectFileItem* addFile( const KUrl&, KDevelop::ProjectFolderItem* ) { return 0; }

        bool addFilesToTarget( const QList<KDevelop::ProjectFileItem*>&, KDevelop::ProjectTargetItem* ) { return false; }
        bool removeTarget( KDevelop::ProjectTargetItem* ) { return false; }
        bool removeFilesAndFolders( const QList< KDevelop::ProjectBaseItem* >& ) { return false; }
        bool removeFilesFromTargets( const QList<KDevelop::ProjectFileItem*>& ) { return false; }
        bool renameFile(KDevelop::ProjectFileItem*, const KUrl&) { return false; }
        bool renameFolder(KDevelop::ProjectFolderItem*, const KUrl&) { return false; }
        bool moveFilesAndFolders( const QList< KDevelop::ProjectBaseItem* >&, KDevelop::ProjectFolderItem* ) { return false; };
        bool copyFilesAndFolders( const KUrl::List &, KDevelop::ProjectFolderItem* ) { return false; };
        QHash<QString,QString> environment(KDevelop::ProjectBaseItem *) const { return QHash<QString, QString>(); }
        Features features() const { return Targets | Files | Folders; }
        
        void setBuildDir(const KUrl& buildDir) { m_buildDir=buildDir; }
        void setIncludeDirectories(const KUrl::List& id) { m_includeDirectories=id; }
        void setDefinesDirectories(const QHash<QString, QString>& defs) { m_defines=defs; }
        bool reload(KDevelop::ProjectFolderItem*) { return true; }
    private:
        KUrl::List m_controlledFiles;
        KUrl m_buildDir;
        KUrl::List m_includeDirectories;
        QHash<QString, QString> m_defines;
        KDevelop::ProjectTargetItem* m_target;
        KDevelop::ProjectFolderItem* m_folder;
};



#endif
