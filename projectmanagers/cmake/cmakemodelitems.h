/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2008 Aleix Pol <aleixpol@gmail.com>
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

#ifndef CMAKEMODELITEMS_H
#define CMAKEMODELITEMS_H

#include <QHash>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <language/duchain/topducontext.h>
#include <parser/cmakelistsparser.h>
#include <parser/cmaketypes.h>

namespace KDevelop {
    class IProject;
    class TopDUContext;
    class Declaration;
}
class CMakeFolderItem;

class KDEVCMAKECOMMON_EXPORT DescriptorAttatched
{
    public:
        // Required, and must be non-inline, for dynamic_cast to work
        virtual ~DescriptorAttatched();
        void setDescriptor(const CMakeFunctionDesc & desc) { m_desc=desc; }
        CMakeFunctionDesc descriptor() const { return m_desc; }
    private:
        CMakeFunctionDesc m_desc;
};

class KDEVCMAKECOMMON_EXPORT DUChainAttatched
{
    public:
        // Required, and must be non-inline, for dynamic_cast to work
        virtual ~DUChainAttatched();
        DUChainAttatched(KDevelop::IndexedDeclaration _decl) : decl(_decl) {}
        KDevelop::IndexedDeclaration declaration() const { return decl; }
    private:
        KDevelop::IndexedDeclaration decl;
};

class KDEVCMAKECOMMON_EXPORT CompilationDataAttached
{
    public:
        // Required, and must be non-inline, for dynamic_cast to work
        virtual ~CompilationDataAttached();
        void setIncludeDirectories(const QStringList &l);
        QStringList includeDirectories(KDevelop::ProjectBaseItem* placeInHierarchy) const;

        CMakeDefinitions definitions(CMakeFolderItem* parent) const;
        void setDefinitions(const CMakeDefinitions& defs) { m_defines=defs; }
        void defineVariables(const QStringList& vars);

    private:
        CMakeDefinitions m_defines;
        QStringList m_includeList;
};

/**
 * The project model item for CMake folders.
 *
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */

class KDEVCMAKECOMMON_EXPORT CMakeFolderItem
    : public KDevelop::ProjectBuildFolderItem
    , public DescriptorAttatched, public CompilationDataAttached
{
    public:
        CMakeFolderItem( KDevelop::IProject* project, const KUrl& folder, const QString& build, CMakeFolderItem* item);
        // Required, and must be non-inline, for dynamic_cast to work
        virtual ~CMakeFolderItem();
        
        void setTopDUContext(KDevelop::ReferencedTopDUContext ctx) { m_topcontext=ctx; }
        KDevelop::ReferencedTopDUContext topDUContext() const { return m_topcontext;}
        
        /** The upper level parent is not the real parent anymore (in some 
        corner cases) we have the former parent here that has the information*/
        CMakeFolderItem* formerParent() const { return m_formerParent; }
        
        void setFormerParent(CMakeFolderItem* parent) { m_formerParent=parent; }
        
        QString buildDir() const { return m_buildDir; }
        void setBuildDir(const QString& bd) { m_buildDir = bd; }
        
        KDevelop::ProjectTargetItem* targetNamed(Target::Type type, const QString& targetName) const;
        KDevelop::ProjectFolderItem* folderNamed(const QString& name) const;
    private:
        KDevelop::ReferencedTopDUContext m_topcontext;
        CMakeFolderItem* m_formerParent;
        QString m_buildDir;
};

class KDEVCMAKECOMMON_EXPORT CMakeExecutableTargetItem 
    : public KDevelop::ProjectExecutableTargetItem
    , public DUChainAttatched, public DescriptorAttatched, public CompilationDataAttached
{
    public:
        CMakeExecutableTargetItem(KDevelop::IProject* project, const QString &name,
                                  CMakeFolderItem *parent, KDevelop::IndexedDeclaration c,
                                  const QString& _outputName, const KUrl& basepath);
        
        virtual KUrl builtUrl() const;
        virtual KUrl installedUrl() const;
        
    private:
        QString outputName;
        KUrl path;
};

class KDEVCMAKECOMMON_EXPORT CMakeLibraryTargetItem
    : public KDevelop::ProjectLibraryTargetItem
    , public DUChainAttatched, public DescriptorAttatched, public CompilationDataAttached
{
    public:
        CMakeLibraryTargetItem(KDevelop::IProject* project, const QString &name,
                               CMakeFolderItem *parent, KDevelop::IndexedDeclaration c,
                               const QString& _outputName, const KUrl& /*basepath*/);
            
    private:
        QString outputName;
};

class KDEVCMAKECOMMON_EXPORT CMakeCustomTargetItem
    : public KDevelop::ProjectTargetItem, public DUChainAttatched, public DescriptorAttatched
{
    public:
        CMakeCustomTargetItem(KDevelop::IProject* project, const QString &name,
                               CMakeFolderItem *parent, KDevelop::IndexedDeclaration c, const QString& _outputName)
            : KDevelop::ProjectTargetItem( project, name, parent), DUChainAttatched(c), outputName(_outputName) {}
            
    private:
        QString outputName;
};

/*
class CMakeFileItem : public KDevelop::ProjectFileItem, public DUChainAttatched
{
    public:
        CMakeFileItem( KDevelop::IProject* project, const KUrl& file, QStandardItem *parent, KDevelop::Declaration *c)
            : ProjectFileItem( project, file, parent), DUChainAttatched(c) {}
};
*/

#endif
