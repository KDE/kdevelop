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

#include <projectmodel.h>
#include <iproject.h>
#include <kross/core/action.h>

#include "projectitemadaptors.h"

using namespace KDevelop;

KrossBuildSystemManager::KrossBuildSystemManager(const QVariantList& args)
    : action(0)
{}

ProjectFolderItem* KrossBuildSystemManager::import(KDevelop::IProject *project )
{
    kDebug() << "importing " << project->name();

    QVariant param;
    param.setValue((QObject*) project);
    QVariant result=action->callFunction( "importProject", QVariantList()<<param);
    KDevelop::ProjectFolderItem* m_rootFolder=new ProjectFolderItem(project, KUrl(result.toString()));
    m_folderPerUrl.insert(m_rootFolder->url(), m_rootFolder);
    return m_rootFolder;
}

QList<ProjectFolderItem*> KrossBuildSystemManager::parse( ProjectFolderItem* dom )
{
    ProjectFolderItemAdaptor *domadp=new ProjectFolderItemAdaptor(action, dom);
    kDebug() << "going to parse" << domadp->url();
    QVariant param;
    param.setValue((QObject*) domadp);
    QVariant result=action->callFunction( "parse", QVariantList()<<param);
    kDebug() << "end of parse";

    QList<ProjectFolderItem*> folders;
    foreach(const QString& afolder, result.toStringList())
    {
        folders.append(m_folderPerUrl[KUrl(afolder)]);
    }
    return folders;
}

IProjectBuilder* KrossBuildSystemManager::builder(ProjectFolderItem*) const
{
    return 0;
}

KUrl KrossBuildSystemManager::buildDirectory(ProjectBaseItem* it) const
{
    return KUrl();
}
KUrl::List KrossBuildSystemManager::includeDirectories(ProjectBaseItem *item) const
{
    qDebug() << "includeeees";
    ProjectBaseItemAdaptor *adapt=ProjectBaseItemAdaptor::createAdaptor(action, item);
    QVariant param;
    param.setValue((QObject*) adapt);
    QVariant result=action->callFunction( "includeDirectories", QVariantList()<<param);
    delete adapt;
    
    KUrl::List directories;
    foreach(const QString& adir, result.toStringList())
    {
        directories.append(KUrl(adir));
    }
    return directories;
}

QHash<QString,QString> KrossBuildSystemManager::defines(ProjectBaseItem *item) const
{
    QVariant param;
    ProjectBaseItemAdaptor *adapt=ProjectBaseItemAdaptor::createAdaptor(action, item);
    param.setValue((QObject*) adapt);
    QVariant result=action->callFunction( "defines", QVariantList()<<param);
    delete adapt;
    
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

QHash<QString,QString> KrossBuildSystemManager::environment(ProjectBaseItem *item) const
{
    QVariant param;
    ProjectBaseItemAdaptor *adapt=ProjectBaseItemAdaptor::createAdaptor(action, item);
    param.setValue((QObject*) adapt);
    QVariant result=action->callFunction( "environment", QVariantList()<<param);
    delete adapt;

    QMap<QString, QVariant> resultEnv= result.toMap();
    QHash<QString, QString> env;
    
    foreach(const QString& key, resultEnv.keys())
    {
        env[key]=resultEnv[key].toString();
    }
    
    return env;
}

IBuildSystemManager::Features KrossBuildSystemManager::features() const
{
    return Folders | Targets | Files;
}

QList<ProjectTargetItem*> KrossBuildSystemManager::targets() const
{
    QList<ProjectTargetItem*> targets;
    foreach(const QList<ProjectTargetItem*> &it, m_targets.values())
        targets += it;
    return targets;
}

QList<ProjectTargetItem*> KrossBuildSystemManager::targets(ProjectFolderItem* folder) const
{
    return m_targets[folder];
}

ProjectFolderItem* KrossBuildSystemManager::addFolder( const KUrl& folder, ProjectFolderItem* parent)
{
    return 0;
}

ProjectTargetItem* KrossBuildSystemManager::createTarget( const QString&, ProjectFolderItem* )
{
    return 0;
}

ProjectFileItem* KrossBuildSystemManager::addFile( const KUrl&, ProjectFolderItem* )
{
    return 0;
}

bool KrossBuildSystemManager::addFileToTarget( ProjectFileItem*, ProjectTargetItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeFolder( ProjectFolderItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeTarget( ProjectTargetItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeFile( ProjectFileItem* )
{
    return false;
}

bool KrossBuildSystemManager::removeFileFromTarget( ProjectFileItem*, ProjectTargetItem* )
{
    return false;
}

bool KrossBuildSystemManager::renameFile(ProjectFileItem*, const KUrl&)
{
    return false;
}

bool KrossBuildSystemManager::renameFolder(ProjectFolderItem*, const KUrl&)
{
    return false;
}

class ProjectManagerCallbacks : public QObject
{
    Q_OBJECT
    public:
        ProjectManagerCallbacks(KrossBuildSystemManager* pm, QObject* parent) : QObject(parent), m_interface(pm) {}
        
    public slots:
        void addFile(const QString& folder, const QString & targetName, const QString & filename)
            { m_interface->addFile(folder, targetName, filename); }
        
        void addTarget(const QString& folder, const QString & targetName)
            { m_interface->addTarget(folder, targetName); }
            
        void addFolder(const QString& folder)
            { m_interface->addFolder(folder); }
    private:
        KrossBuildSystemManager *m_interface;
};

void KrossBuildSystemManager::setAction(Kross::Action* anAction)
{
    action=anAction;
    ProjectManagerCallbacks* callbacks= new ProjectManagerCallbacks(this, action);
    action->addObject(callbacks, "IBuildSystemManager", Kross::ChildrenInterface::AutoConnectSignals);
}

void KrossBuildSystemManager::addFile(const QString& folder, const QString & targetName, const QString & filename)
{
    kDebug() << "adding file" << folder << targetName << filename;
    ProjectTargetItem* parent=m_targetPerName[targetName];
    new ProjectFileItem(parent->project(), KUrl(filename), parent);
}

void KrossBuildSystemManager::addTarget(const QString& folder, const QString & targetName)
{
    kDebug() << "adding target" << folder << targetName;
    ProjectFolderItem* parent=m_folderPerUrl[KUrl(folder)];
    ProjectTargetItem* newTarget = new ProjectTargetItem(parent->project(), targetName, parent);
    m_targetPerName[targetName]=newTarget;
}

void KrossBuildSystemManager::addFolder(const QString& folder)
{
    kDebug() << "adding folder" << folder << m_folderPerUrl << KUrl(folder).upUrl();
    ProjectFolderItem* parent=m_folderPerUrl[KUrl(folder).upUrl()];
    
    ProjectFolderItem* newFolder = new ProjectFolderItem(parent->project(), folder, parent);
    m_folderPerUrl.insert(newFolder->url(), newFolder);
}

#include "krossbuildsystemmanager.moc"
