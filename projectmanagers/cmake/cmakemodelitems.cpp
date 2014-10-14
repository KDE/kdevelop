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
#include <cmakeparserutils.h>
#include <QString>
#include <QThread>

#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>

#include <project/interfaces/ibuildsystemmanager.h>

CMakeFolderItem::CMakeFolderItem( KDevelop::IProject *project, const KDevelop::Path &path, const QString& build,
                                  CMakeFolderItem* item)
    : KDevelop::ProjectBuildFolderItem( project, path, item ), m_formerParent(item), m_buildDir(build)
{
}

static KDevelop::ProjectBaseItem* getRealCMakeParent(KDevelop::ProjectBaseItem* baseItem)
{
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( baseItem );
    if(folder) {
        return folder->formerParent();
    }
    return baseItem->parent();
}

QStringList CompilationDataAttached::includeDirectories(KDevelop::ProjectBaseItem* placeInHierarchy) const
{
    QStringList ret(m_includeList);
    if (!placeInHierarchy)
        return ret;

    placeInHierarchy = getRealCMakeParent(placeInHierarchy);
    while(placeInHierarchy)
    {
        CompilationDataAttached* includer = dynamic_cast<CompilationDataAttached*>( placeInHierarchy );
        if(includer) {
            ret += includer->includeDirectories(placeInHierarchy);
            return ret;
        }

        placeInHierarchy = getRealCMakeParent(placeInHierarchy);
    }
    return ret;
}

CMakeDefinitions CompilationDataAttached::definitions(CMakeFolderItem* parentFolder) const
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

void CompilationDataAttached::addDefinitions(const QStringList& vars)
{
    CMakeParserUtils::addDefinitions(vars, &m_defines);
}


QUrl CMakeExecutableTargetItem::builtUrl() const
{
    if(!path.isValid()) {
        KDevelop::Path ret = project()->buildSystemManager()->buildDirectory(const_cast<CMakeExecutableTargetItem*>(this));
        ret.addPath(outputName);
        return ret.toUrl();
    } else
        return path.toUrl();
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

CMakeExecutableTargetItem::CMakeExecutableTargetItem(KDevelop::IProject* project, const QString& name, CMakeFolderItem* parent, const QString& _outputName, const KDevelop::Path& basepath)
    : KDevelop::ProjectExecutableTargetItem( project, name, parent)
    , outputName(_outputName)
    , path(basepath)
{}

CMakeLibraryTargetItem::CMakeLibraryTargetItem(KDevelop::IProject* project, const QString& name, CMakeFolderItem* parent, const QString& _outputName, const KDevelop::Path&)
    : KDevelop::ProjectLibraryTargetItem( project, name, parent), outputName(_outputName)
{}

CMakeFolderItem::~CMakeFolderItem() {}

DescriptorAttatched::~DescriptorAttatched() {}

DUChainAttatched::~DUChainAttatched() {}

CompilationDataAttached::~CompilationDataAttached() {}

QUrl CMakeExecutableTargetItem::installedUrl() const {
    return QUrl();
}
void CompilationDataAttached::setIncludeDirectories(const QStringList& l)
{
    m_includeList = l;
    m_includeList.removeDuplicates();
}
