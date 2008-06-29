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

#include "krossprojectmanager.h"

#include "ikrossprojectmanager.h"

#include <KUrl>
#include <KProcess>
#include <KDebug>
#include <KStandardDirs>

#include <icore.h>
#include <iproject.h>
#include <iplugincontroller.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <projectmodel.h>


K_PLUGIN_FACTORY(KrossSupportFactory, registerPlugin<KrossProjectManager>(); );
K_EXPORT_PLUGIN(KrossSupportFactory("kdevkrossmanager"));


KrossProjectManager::KrossProjectManager( QObject* parent, const QVariantList& args )
    : KDevelop::IPlugin( KrossSupportFactory::componentData(), parent )
{
    kDebug() << "Krossing the krossed paths of this krossed world" << args;
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    
    m_name = args.first().toString();
}

KDevelop::IProjectFileManager::Features KrossProjectManager::features() const
{
    return Features(Folders | Targets | Files);
}

QList<KDevelop::ProjectFolderItem*> KrossProjectManager::parse( KDevelop::ProjectFolderItem* dom )
{
    return m_scripts[dom->project()]->parse(dom);
}

KDevelop::ProjectFolderItem* KrossProjectManager::import( KDevelop::IProject *project )
{
    kDebug() << "importiiiiing" << project << m_name;
//     QString file="/home/kde-devel/krosstest/vckross.py";
    
    KUrl file=KStandardDirs::installPath("kdedir");
    file.addPath("lib");
    file.addPath("kde4");
    file.addPath(m_name);
    file.addPath(m_name+".py");
    kDebug() << "importiiiiing" << project << m_name;
    
    IKrossProjectManager* kpm=new IKrossProjectManager(this);
    m_scripts[project] = kpm;
    
    return kpm->import(file, project);
}

KDevelop::IProjectBuilder* KrossProjectManager::builder(KDevelop::ProjectFolderItem*) const
{
    return 0;
}

KUrl KrossProjectManager::buildDirectory(KDevelop::ProjectBaseItem* item) const
{
    return m_scripts[item->project()]->buildDirectory(item);
}

KUrl::List KrossProjectManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return m_scripts[item->project()]->includeDirectories(item);
}

QHash<QString,QString> KrossProjectManager::defines(KDevelop::ProjectBaseItem *item) const
{
    return m_scripts[item->project()]->defines(item);
}

QList<KDevelop::ProjectTargetItem*> KrossProjectManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> targets;
    foreach(const IKrossProjectManager* it, m_scripts.values())
    {
        targets << it->targets();
    }
    return targets;
}

QList<KDevelop::ProjectTargetItem*> KrossProjectManager::targets(KDevelop::ProjectFolderItem* folder) const
{
    return folder->targetList();
}

KDevelop::ProjectFolderItem* KrossProjectManager::addFolder( const KUrl& folder, KDevelop::ProjectFolderItem* parent )
{
    return m_scripts[parent->project()]->addFolder(folder, parent);
}

KDevelop::ProjectTargetItem* KrossProjectManager::createTarget( const QString& name, KDevelop::ProjectFolderItem* parent)
{
    return m_scripts[parent->project()]->createTarget(name, parent);
}

KDevelop::ProjectFileItem* KrossProjectManager::addFile( const KUrl& path, KDevelop::ProjectFolderItem* parent)
{
    return m_scripts[parent->project()]->addFile(path, parent);
}

bool KrossProjectManager::addFileToTarget( KDevelop::ProjectFileItem* file, KDevelop::ProjectTargetItem* parent)
{
    return m_scripts[parent->project()]->addFileToTarget(file, parent);
}

bool KrossProjectManager::removeFolder( KDevelop::ProjectFolderItem* folder)
{
    return m_scripts[folder->project()]->removeFolder(folder);
}

bool KrossProjectManager::removeTarget( KDevelop::ProjectTargetItem* target)
{
    return m_scripts[target->project()]->removeTarget(target);
}

bool KrossProjectManager::removeFile( KDevelop::ProjectFileItem* file)
{
    return m_scripts[file->project()]->removeFile(file);
}

bool KrossProjectManager::removeFileFromTarget( KDevelop::ProjectFileItem* file, KDevelop::ProjectTargetItem* target)
{
    return m_scripts[file->project()]->removeFileFromTarget(file, target);
}

bool KrossProjectManager::renameFile(KDevelop::ProjectFileItem* file, const KUrl& newPath)
{
    return m_scripts[file->project()]->renameFile(file, newPath);
}

bool KrossProjectManager::renameFolder(KDevelop::ProjectFolderItem* folder, const KUrl& newPath)
{
    return m_scripts[folder->project()]->renameFolder(folder, newPath);
}

KrossProjectManager::~ KrossProjectManager()
{}

#include "krossprojectmanager.moc"
