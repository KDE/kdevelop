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

KDevelop::ProjectFolderItem* IKrossProjectManager::import( const KUrl& file, KDevelop::IProject *project )
{
    action = new Kross::Action(this, file);
    action->addObject(this, "IKrossProjectManager", Kross::ChildrenInterface::AutoConnectSignals);
    action->setFile(file.toLocalFile());
    action->trigger();
    
    QVariant param;
    param.setValue((QObject*) project);
    QVariant result=action->callFunction( "importProject", QVariantList()<<param);
    m_rootFolder=new KDevelop::ProjectFolderItem(project, KUrl(result.toString()));
    m_folderPerUrl.insert(m_rootFolder->url(), m_rootFolder);
    return m_rootFolder;
}

QList<KDevelop::ProjectFolderItem*> IKrossProjectManager::parse( KDevelop::ProjectFolderItem* dom )
{
    QVariant param;
    param.setValue(dom->url().toLocalFile());
    QVariant result=action->callFunction( "parse", QVariantList()<<param);
    
    QList<KDevelop::ProjectFolderItem*> folders;
    foreach(const QString& afolder, result.toStringList())
    {
        folders.append(m_folderPerUrl[KUrl(afolder)]);
    }
    return folders;
}

KDevelop::IProjectBuilder* IKrossProjectManager::builder(KDevelop::ProjectFolderItem*) const
{
    return 0;
}

KUrl IKrossProjectManager::buildDirectory(KDevelop::ProjectBaseItem* it) const
{
    return KUrl();
}

KUrl::List IKrossProjectManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return m_includesPerItem[item];
}

QHash<QString,QString> IKrossProjectManager::defines(KDevelop::ProjectBaseItem *item)
{
    return m_defines[item];
}

QList<KDevelop::ProjectTargetItem*> IKrossProjectManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> targets;
    foreach(const QList<KDevelop::ProjectTargetItem*> &it, m_targets.values())
        targets += it;
    return targets;
}

QList<KDevelop::ProjectTargetItem*> IKrossProjectManager::targets(KDevelop::ProjectFolderItem* folder) const
{
    return m_targets[folder];
}

KDevelop::ProjectFolderItem* IKrossProjectManager::addFolder( const KUrl& folder, KDevelop::ProjectFolderItem* parent)
{
    return 0;
}

KDevelop::ProjectTargetItem* IKrossProjectManager::createTarget( const QString&, KDevelop::ProjectFolderItem* )
{
    return 0;
}

KDevelop::ProjectFileItem* IKrossProjectManager::addFile( const KUrl&, KDevelop::ProjectFolderItem* )
{
    return 0;
}

bool IKrossProjectManager::addFileToTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* )
{
    return false;
}

bool IKrossProjectManager::removeFolder( KDevelop::ProjectFolderItem* )
{
    return false;
}

bool IKrossProjectManager::removeTarget( KDevelop::ProjectTargetItem* )
{
    return false;
}

bool IKrossProjectManager::removeFile( KDevelop::ProjectFileItem* )
{
    return false;
}

bool IKrossProjectManager::removeFileFromTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* )
{
    return false;
}

bool IKrossProjectManager::renameFile(KDevelop::ProjectFileItem*, const KUrl&)
{
    return false;
}

bool IKrossProjectManager::renameFolder(KDevelop::ProjectFolderItem*, const KUrl&)
{
    return false;
}

void IKrossProjectManager::addFile(const QString& folder, const QString & targetName, const QString & filename)
{
    kDebug() << "adding file" << folder << targetName << filename;
    KDevelop::ProjectTargetItem* parent=m_targetPerName[targetName];
    KDevelop::ProjectFileItem* newFile = new KDevelop::ProjectFileItem(parent->project(), KUrl(filename), parent);
}

void IKrossProjectManager::addTarget(const QString& folder, const QString & targetName)
{
    kDebug() << "adding target" << folder << targetName;
    KDevelop::ProjectFolderItem* parent=m_folderPerUrl[KUrl(folder)];
    KDevelop::ProjectTargetItem* newTarget = new KDevelop::ProjectTargetItem(parent->project(), targetName, parent);
    m_targetPerName[targetName]=newTarget;
}

void IKrossProjectManager::addFolder(const QString& folder)
{
    kDebug() << "adding folder" << folder << m_folderPerUrl << KUrl(folder).upUrl();
    KDevelop::ProjectFolderItem* parent=m_folderPerUrl[KUrl(folder).upUrl()];
    
    KDevelop::ProjectFolderItem* newFolder = new KDevelop::ProjectFolderItem(parent->project(), folder, parent);
    m_folderPerUrl.insert(newFolder->url(), newFolder);
}

#include "ikrossprojectmanager.moc"

