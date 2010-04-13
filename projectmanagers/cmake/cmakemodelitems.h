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
#include "cmakelistsparser.h"
#include "cmaketypes.h"
#include "cmakeast.h"
#include <interfaces/iproject.h>
#include <language/duchain/topducontext.h>

namespace KDevelop {
    class IProject;
    class TopDUContext;
    class Declaration;
}

class DescriptorAttatched
{
    public:
        void setDescriptor(const CMakeFunctionDesc & desc) { m_desc=desc; }
        CMakeFunctionDesc descriptor() const { return m_desc; }
    private:
        CMakeFunctionDesc m_desc;
};

class DUChainAttatched
{
    public:
        DUChainAttatched(KDevelop::IndexedDeclaration _decl) : decl(_decl) {}
        KDevelop::IndexedDeclaration declaration() const { return decl; }
    private:
        KDevelop::IndexedDeclaration decl;
};

/**
 * The project model item for CMake folders.
 *
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */

class CMakeFolderItem : public KDevelop::ProjectBuildFolderItem, public DescriptorAttatched
{
    public:
        CMakeFolderItem( KDevelop::IProject *project, const QString &name, const QString& builddir, CMakeFolderItem* parent);
        virtual ~CMakeFolderItem() {}
        void setIncludeDirectories(const QStringList &l) { m_includeList=l; }
        QStringList includeDirectories() const;
        CMakeDefinitions definitions() const { return m_defines; }
        void setDefinitions(const CMakeDefinitions& defs) { m_defines=defs; }
        
        void setTopDUContext(KDevelop::ReferencedTopDUContext ctx) { m_topcontext=ctx; }
        KDevelop::ReferencedTopDUContext topDUContext() const { return m_topcontext;}
        
        /** The upper level parent is not the real parent anymore (in some 
        corner cases) we have the former parent here that has the information*/
        CMakeFolderItem* formerParent() const { return m_formerParent; }
        
        void setFormerParent(CMakeFolderItem* parent) { m_formerParent=parent; }
        
        QString buildDir() const { return m_buildDir; }
    private:
        KDevelop::ReferencedTopDUContext m_topcontext;
        QStringList m_includeList;
        CMakeDefinitions m_defines;
        CMakeFolderItem* m_formerParent;
        QString m_buildDir;
};

class CMakeExecutableTargetItem 
    : public KDevelop::ProjectExecutableTargetItem, public DUChainAttatched, public DescriptorAttatched
{
    public:
        CMakeExecutableTargetItem(KDevelop::IProject* project, const QString &name,
                                  CMakeFolderItem *parent, KDevelop::IndexedDeclaration c,
                                  const QString& _outputName, const KUrl& basepath)
            : KDevelop::ProjectExecutableTargetItem( project, name, parent)
            , DUChainAttatched(c)
            , outputName(_outputName)
            , path(basepath)
        {}
        
        virtual KUrl builtUrl() const;
        virtual KUrl installedUrl() const { return KUrl(); }
        
    private:
        QString outputName;
        KUrl path;
};

class CMakeLibraryTargetItem
    : public KDevelop::ProjectLibraryTargetItem, public DUChainAttatched, public DescriptorAttatched
{
    public:
        CMakeLibraryTargetItem(KDevelop::IProject* project, const QString &name,
                               CMakeFolderItem *parent, KDevelop::IndexedDeclaration c,
                               const QString& _outputName, const KUrl& /*basepath*/)
            : KDevelop::ProjectLibraryTargetItem( project, name, parent), DUChainAttatched(c), outputName(_outputName) {}
            
    private:
        QString outputName;
};

class CMakeCustomTargetItem
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
