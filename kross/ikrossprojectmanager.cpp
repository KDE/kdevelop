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

#include "ikrossprojectmanager.h"
#include <KDebug>

#include <projectmodel.h>
#include <iproject.h>
#include <kross/core/action.h>

#include "projectitemadaptors.h"

using namespace KDevelop;

extern "C"
{
    KDE_EXPORT QObject* krossmodule()
    {
        return new IKrossProjectManager();
    }
}

IKrossProjectManager::IKrossProjectManager( QObject* parent, const QVariantList& args)
    : m_rootFolder(0), action(0)
{}

ProjectFolderItem* IKrossProjectManager::import( const KUrl& file, KDevelop::IProject *project )
{
    action = new Kross::Action(this, file);
    action->addObject(this, "IKrossProjectManager", Kross::ChildrenInterface::AutoConnectSignals);
    action->setFile(file.toLocalFile());
    action->trigger();
    
    QVariant param;
    param.setValue((QObject*) project);
    QVariant result=action->callFunction( "importProject", QVariantList()<<param);
    m_rootFolder=new ProjectFolderItem(project, KUrl(result.toString()));
    m_folderPerUrl.insert(m_rootFolder->url(), m_rootFolder);
    return m_rootFolder;
}

QList<ProjectFolderItem*> IKrossProjectManager::parse( ProjectFolderItem* dom )
{
    ProjectFolderItemAdaptor domadp(action, dom);
    
    QVariant param;
    param.setValue((QObject*) &domadp);
    QVariant result=action->callFunction( "parse", QVariantList()<<param);
    
    QList<ProjectFolderItem*> folders;
    foreach(const QString& afolder, result.toStringList())
    {
        folders.append(m_folderPerUrl[KUrl(afolder)]);
    }
    return folders;
}

IProjectBuilder* IKrossProjectManager::builder(ProjectFolderItem*) const
{
    return 0;
}

KUrl IKrossProjectManager::buildDirectory(ProjectBaseItem* it) const
{
    return KUrl();
}
KUrl::List IKrossProjectManager::includeDirectories(ProjectBaseItem *item) const
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

QHash<QString,QString> IKrossProjectManager::defines(ProjectBaseItem *item)
{
    QVariant param;
    ProjectBaseItemAdaptor *adapt=ProjectBaseItemAdaptor::createAdaptor(action, item);
    param.setValue((QObject*) adapt);
    QVariant result=action->callFunction( "defines", QVariantList()<<param);
    delete adapt;
    
    QMap<QString, QVariant> resultDefines= result.toMap();
    QList<QString> keys=resultDefines.keys();
    QHash<QString, QString> defs;
    
    QList<QString>::const_iterator it=keys.constBegin(), itEnd=keys.constEnd();
    for(; it!=itEnd; ++it)
    {
        defs[*it]=resultDefines[*it].toString();
    }
    
    return defs;
}

QList<ProjectTargetItem*> IKrossProjectManager::targets() const
{
    QList<ProjectTargetItem*> targets;
    foreach(const QList<ProjectTargetItem*> &it, m_targets.values())
        targets += it;
    return targets;
}

QList<ProjectTargetItem*> IKrossProjectManager::targets(ProjectFolderItem* folder) const
{
    return m_targets[folder];
}

ProjectFolderItem* IKrossProjectManager::addFolder( const KUrl& folder, ProjectFolderItem* parent)
{
    return 0;
}

ProjectTargetItem* IKrossProjectManager::createTarget( const QString&, ProjectFolderItem* )
{
    return 0;
}

ProjectFileItem* IKrossProjectManager::addFile( const KUrl&, ProjectFolderItem* )
{
    return 0;
}

bool IKrossProjectManager::addFileToTarget( ProjectFileItem*, ProjectTargetItem* )
{
    return false;
}

bool IKrossProjectManager::removeFolder( ProjectFolderItem* )
{
    return false;
}

bool IKrossProjectManager::removeTarget( ProjectTargetItem* )
{
    return false;
}

bool IKrossProjectManager::removeFile( ProjectFileItem* )
{
    return false;
}

bool IKrossProjectManager::removeFileFromTarget( ProjectFileItem*, ProjectTargetItem* )
{
    return false;
}

bool IKrossProjectManager::renameFile(ProjectFileItem*, const KUrl&)
{
    return false;
}

bool IKrossProjectManager::renameFolder(ProjectFolderItem*, const KUrl&)
{
    return false;
}

void IKrossProjectManager::addFile(const QString& folder, const QString & targetName, const QString & filename)
{
    kDebug() << "adding file" << folder << targetName << filename;
    ProjectTargetItem* parent=m_targetPerName[targetName];
    ProjectFileItem* newFile = new ProjectFileItem(parent->project(), KUrl(filename), parent);
}

void IKrossProjectManager::addTarget(const QString& folder, const QString & targetName)
{
    kDebug() << "adding target" << folder << targetName;
    ProjectFolderItem* parent=m_folderPerUrl[KUrl(folder)];
    ProjectTargetItem* newTarget = new ProjectTargetItem(parent->project(), targetName, parent);
    m_targetPerName[targetName]=newTarget;
}

void IKrossProjectManager::addFolder(const QString& folder)
{
    kDebug() << "adding folder" << folder << m_folderPerUrl << KUrl(folder).upUrl();
    ProjectFolderItem* parent=m_folderPerUrl[KUrl(folder).upUrl()];
    
    ProjectFolderItem* newFolder = new ProjectFolderItem(parent->project(), folder, parent);
    m_folderPerUrl.insert(newFolder->url(), newFolder);
}

#include "ikrossprojectmanager.moc"

