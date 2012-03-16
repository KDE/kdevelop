/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
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

#include "krossbuildsystemmanager.h"
#include <KDebug>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <kross/core/action.h>
#include <kross/core/manager.h>
#include "wrappers/krossprojectmodel.h"
#include "wrappers/krossprojectmodelimpl.h"

class ProjectManagerCallbacks : public QObject
{
    Q_OBJECT
    public:
        ProjectManagerCallbacks(QObject* p, KrossBuildSystemManager* pm) : QObject(p), m_interface(pm) {}
        
    public slots:
        KDevelop::ProjectFileItem* addFile(Kross::Object::Ptr o, KDevelop::IProject* p, const KUrl& url, QStandardItem* parent=0)
            { return new KrossImplKDevelopProjectFileItem(o, p, url, parent); }
        
        KDevelop::ProjectTargetItem* addTarget(Kross::Object::Ptr o, KDevelop::IProject* p, const QString& name, QStandardItem* parent)
            { return new KrossImplKDevelopProjectTargetItem(o, p, name, parent); }
        KDevelop::ProjectLibraryTargetItem* addLibrary(Kross::Object::Ptr o, KDevelop::IProject* p, const QString& name, QStandardItem* parent)
            { return new KrossImplKDevelopProjectLibraryTargetItem(o, p, name, parent); }
        KDevelop::ProjectExecutableTargetItem* addExecutable(Kross::Object::Ptr o, KDevelop::IProject* p, const QString& name, QStandardItem* parent)
            { return new KrossImplKDevelopProjectExecutableTargetItem(o, p, name, parent); }
        KDevelop::ProjectFolderItem* addFolder(Kross::Object::Ptr o, KDevelop::IProject* p, const KUrl& url, QStandardItem* parent)
            { return new KrossImplKDevelopProjectFolderItem(o, p, url, parent); }
    private:
        KrossBuildSystemManager* m_interface;
};

class KrossProjectExecutableTargetItem : public KDevelop::ProjectExecutableTargetItem
{
    public:
        KrossProjectExecutableTargetItem(KDevelop::IProject* project, const QString &name, QStandardItem *parent = 0)
            : KDevelop::ProjectExecutableTargetItem(project, name, parent) {}
            
        virtual KUrl builtUrl() const { return KUrl(); }
        virtual KUrl installedUrl() const { return KUrl(); }
};

KrossBuildSystemManager::KrossBuildSystemManager(const QVariantList& )
    : action(0)
{}

KDevelop::ProjectFolderItem* KrossBuildSystemManager::import(KDevelop::IProject *project )
{
    kDebug() << "importing " << project->name();

    QVariant param;
    param.setValue((QObject*) project);
    QVariant result=action->callFunction( "importProject", QVariantList()<<param);
    KDevelop::ProjectFolderItem* m_rootFolder=new KDevelop::ProjectFolderItem(project, KUrl(result.toString()));
    m_folderPerUrl.insert(m_rootFolder->url(), m_rootFolder);
    return m_rootFolder;
}

QList<KDevelop::ProjectFolderItem*> KrossBuildSystemManager::parse( KDevelop::ProjectFolderItem* dom )
{
    QVariant result=action->callFunction( "parse", QVariantList()<< Handlers::kDevelopProjectFolderItemHandler(dom));
    kDebug() << "end of parse";

    QList<KDevelop::ProjectFolderItem*> folders;
    foreach(const QString& afolder, result.toStringList())
    {
        Q_ASSERT( m_folderPerUrl.contains(KUrl(afolder)) );
        folders.append(m_folderPerUrl[KUrl(afolder)]);
    }
    return folders;
}

bool KrossBuildSystemManager::reload(KDevelop::ProjectFolderItem */*item*/)
{
    return false;
}

KDevelop::IProjectBuilder* KrossBuildSystemManager::builder() const
{
    return 0;
}

KUrl KrossBuildSystemManager::buildDirectory(KDevelop::ProjectBaseItem* it) const
{
    QVariant result=action->callFunction( "buildDirectory", QVariantList() << Handlers::kDevelopProjectBaseItemHandler(it));
    return result.toUrl();
}
KUrl::List KrossBuildSystemManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    qDebug() << "includeeees";
    QVariant param=Handlers::kDevelopProjectBaseItemHandler(item);
    QVariant result=action->callFunction( "includeDirectories", QVariantList()<<param);
    
    KUrl::List directories;
    foreach(const QString& adir, result.toStringList())
    {
        directories.append(KUrl(adir));
    }
    return directories;
}

QHash<QString,QString> KrossBuildSystemManager::defines(KDevelop::ProjectBaseItem *item) const
{
    QVariant param=Handlers::kDevelopProjectBaseItemHandler(item);
    QVariant result=action->callFunction( "defines", QVariantList()<<param);
    
    QMap<QString, QVariant> resultDefines= result.toMap();
    QStringList keys=resultDefines.keys();
    QHash<QString, QString> defs;
    
    QStringList::const_iterator it=keys.constBegin(), itEnd=keys.constEnd();
    for(; it!=itEnd; ++it)
    {
        defs[*it]=resultDefines[*it].toString();
    }
    
    return defs;
}

KDevelop::IBuildSystemManager::Features KrossBuildSystemManager::features() const
{
    return Folders | Targets | Files;
}

QList<KDevelop::ProjectTargetItem*> KrossBuildSystemManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> targets;
    foreach(const QList<KDevelop::ProjectTargetItem*>& it, m_targets)
        targets += it;
    return targets;
}

QList<KDevelop::ProjectTargetItem*> KrossBuildSystemManager::targets(KDevelop::ProjectFolderItem* folder) const
{
    return m_targets[folder];
}

KDevelop::ProjectFolderItem* KrossBuildSystemManager::addFolder( const KUrl& folder, KDevelop::ProjectFolderItem* parent)
{
    Q_UNUSED( folder );
    Q_UNUSED( parent );
    return 0;
}

KDevelop::ProjectTargetItem* KrossBuildSystemManager::createTarget( const QString&, KDevelop::ProjectFolderItem* )
{
    return 0;
}

KDevelop::ProjectFileItem* KrossBuildSystemManager::addFile( const KUrl&, KDevelop::ProjectFolderItem* )
{
    return 0;
}

bool KrossBuildSystemManager::addFileToTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeFolder( KDevelop::ProjectFolderItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeTarget( KDevelop::ProjectTargetItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeFile( KDevelop::ProjectFileItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeFileFromTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* )
{
    return false;
}

bool KrossBuildSystemManager::renameFile(KDevelop::ProjectFileItem*, const KUrl&)
{
    return false;
}

bool KrossBuildSystemManager::renameFolder(KDevelop::ProjectFolderItem*, const KUrl&)
{
    return false;
}

void KrossBuildSystemManager::setAction(Kross::Action* anAction)
{
    action=anAction;
    ProjectManagerCallbacks* callbacks= new ProjectManagerCallbacks(action, this);
    action->addObject(callbacks, "IBuildSystemManager" , Kross::ChildrenInterface::AutoConnectSignals);
    
//     Kross::Manager::self().registerMetaTypeHandler("KDevelop::ProjectFileItem*", fileItemHandler);
}

#include "krossbuildsystemmanager.moc"
