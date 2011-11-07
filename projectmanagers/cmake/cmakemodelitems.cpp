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

#include "cmakemodelitems.h"
#include <QString>
#include <kdebug.h>

#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <project/interfaces/ibuildsystemmanager.h>

CMakeFolderItem::CMakeFolderItem( KDevelop::IProject *project, const KUrl &folder, const QString& build,
                                  CMakeFolderItem* item)
    : KDevelop::ProjectBuildFolderItem( project, folder, item ), m_formerParent(item), m_buildDir(build)
{
    Q_ASSERT(folder.path().endsWith("/"));
}

QStringList CMakeFolderItem::includeDirectories() const
{
    QStringList urls(m_includeList);

    CMakeFolderItem *folder = formerParent();
    while(folder)
    {
        urls += folder->includeDirectories();
        folder = folder->formerParent();
    }
    return urls;
}

KUrl CMakeExecutableTargetItem::builtUrl() const
{
    KUrl ret;
    if(path.isEmpty())
        ret=project()->buildSystemManager()->buildDirectory(const_cast<CMakeExecutableTargetItem*>(this));
    else
        ret=path;
    
    ret.addPath(outputName);
    return ret;
}

bool isTargetType(Target::Type type, KDevelop::ProjectTargetItem* t)
{
    return
        (type == Target::Library && t->type()==KDevelop::ProjectBaseItem::LibraryTarget) ||
        (type == Target::Executable && t->type()==KDevelop::ProjectBaseItem::ExecutableTarget) ||
        (type == Target::Custom && t->type()==KDevelop::ProjectBaseItem::Target);
}

KDevelop::ProjectTargetItem* CMakeFolderItem::targetNamed(Target::Type type, const QString& targetName) const
{
    QList< KDevelop::ProjectTargetItem* > targets = targetList();
    foreach(KDevelop::ProjectTargetItem* t, targets) {
        if(isTargetType(type, t) && t->text()==targetName) {
            Q_ASSERT(dynamic_cast<KDevelop::ProjectTargetItem*>(t));
            return t;
        }
    }
    return 0;
}

KDevelop::ProjectFolderItem* CMakeFolderItem::folderNamed(const QString& name) const
{
    QList<KDevelop::ProjectFolderItem*> folders = folderList();
    foreach(KDevelop::ProjectFolderItem* folder, folders) {
        if(folder->text()==name)
            return folder;
    }
    return 0;
}

template <class T>
bool textInList(const QList<T>& list, KDevelop::ProjectBaseItem* item)
{
    foreach(const T& s, list) {
        if(item->text()==s.name)
            return true;
    }
    return false;
}

void CMakeFolderItem::cleanupBuildFolders(const QList< Subdirectory >& subs)
{
    QList<KDevelop::ProjectFolderItem*> folders = folderList();
    foreach(KDevelop::ProjectFolderItem* folder, folders) {
        CMakeFolderItem* cmfolder = dynamic_cast<CMakeFolderItem*>(folder);
        if(cmfolder && cmfolder->formerParent()==this && !textInList<Subdirectory>(subs, folder))
            delete folder;
    }
}

void CMakeFolderItem::cleanupTargets(const QList<CMakeTarget>& targets)
{
    QList<KDevelop::ProjectTargetItem*> targetl = targetList();
    foreach(KDevelop::ProjectTargetItem* target, targetl) {
        if(!textInList<CMakeTarget>(targets, target))
            delete target;
    }
}

CMakeExecutableTargetItem::CMakeExecutableTargetItem(KDevelop::IProject* project, const QString& name, CMakeFolderItem* parent, KDevelop::IndexedDeclaration c, const QString& _outputName, const KUrl& basepath)
    : KDevelop::ProjectExecutableTargetItem( project, name, parent)
    , DUChainAttatched(c)
    , outputName(_outputName)
    , path(basepath)
{}

CMakeLibraryTargetItem::CMakeLibraryTargetItem(KDevelop::IProject* project, const QString& name, CMakeFolderItem* parent, KDevelop::IndexedDeclaration c, const QString& _outputName, const KUrl&)
    : KDevelop::ProjectLibraryTargetItem( project, name, parent), DUChainAttatched(c), outputName(_outputName)
{}
