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
#include <QThread>
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

static KDevelop::ProjectBaseItem* getRealCMakeParent(KDevelop::ProjectBaseItem* baseItem)
{
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( baseItem );
    if(folder) {
        return folder->formerParent();
    }
    return baseItem->parent();
}

QStringList IncludesAttached::includeDirectories(KDevelop::ProjectBaseItem* placeInHierarchy) const
{
    QStringList ret(m_includeList);
    if (!placeInHierarchy)
        return ret;

    placeInHierarchy = getRealCMakeParent(placeInHierarchy);
    while(placeInHierarchy)
    {
        IncludesAttached* includer = dynamic_cast<IncludesAttached*>( placeInHierarchy );
        if(includer) {
            ret += includer->includeDirectories(placeInHierarchy);
            return ret;
        }

        placeInHierarchy = getRealCMakeParent(placeInHierarchy);
    }
    return ret;
}

CMakeDefinitions DefinesAttached::definitions(CMakeFolderItem* parentFolder) const
{
    CMakeDefinitions result = m_defines;

    // This goes up recursively through the hierarchy of cmake-parent-dirs 
    // and fetches their definitions too. This makes sure that defines set in a parent CMakeLists.txt
    // are also applied in this subdirectory.
    //
    // This is not 100% correct, since the current CMakeLists.txt might have removed one of the parent
    // defines again, but at this point we cannot take care of that anymore, this would need to be
    // fixed in the parser. In addition this does not take into account wether a define in a parent
    // was added before or after the add_subdirectory for this cmakelists.txt. And last but not least
    // CMake actually adds all defines as-is, even if two add_definitions are adding the same define
    // with the same or different values. Our code will only take the 'last' value.
    if( parentFolder ) {
        QHash<QString,QString> parentDefs = parentFolder->definitions(parentFolder->formerParent());
        for( QHash<QString,QString>::const_iterator it = parentDefs.constBegin(); it != parentDefs.constEnd(); it++ ) {
            if( !result.contains( it.key() ) ) {
                result[it.key()] = it.value();
            }
        }
    }
    return result;
}

void DefinesAttached::defineVariables(const QStringList& vars)
{
    foreach(const QString& v, vars)
        m_defines.insert(v.section('=', 0, 0), v.section('=', 1, -1));
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

QList<KDevelop::ProjectBaseItem*> CMakeFolderItem::cleanupBuildFolders(const QList< Subdirectory >& subs)
{
    QList<ProjectBaseItem*> ret;
    QList<KDevelop::ProjectFolderItem*> folders = folderList();
    foreach(KDevelop::ProjectFolderItem* folder, folders) {
        CMakeFolderItem* cmfolder = dynamic_cast<CMakeFolderItem*>(folder);
        if(cmfolder && cmfolder->formerParent()==this && !textInList<Subdirectory>(subs, folder))
            ret += folder;
    }
    return ret;
}

QList<KDevelop::ProjectBaseItem*> CMakeFolderItem::cleanupTargets(const QList<CMakeTarget>& targets)
{
    QList<ProjectBaseItem*> ret;
    QList<KDevelop::ProjectTargetItem*> targetl = targetList();
    foreach(KDevelop::ProjectTargetItem* target, targetl) {
        if(!textInList<CMakeTarget>(targets, target))
            ret += target;
    }
    return ret;
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

CMakeFolderItem::~CMakeFolderItem() {
}

DescriptorAttatched::~DescriptorAttatched() {
}

DUChainAttatched::~DUChainAttatched() {
}

DefinesAttached::~DefinesAttached() {
}

IncludesAttached::~IncludesAttached() {
}

KUrl CMakeExecutableTargetItem::installedUrl() const {
    return KUrl();
}
