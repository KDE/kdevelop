/* KDevelop CMake Support
 *
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
#include "cmakeprojectvisitor.h"
#include "cmakeast.h"

#include <kdebug.h>
#include <QHash>

CMakeProjectVisitor::CMakeProjectVisitor(QHash<QString, QStringList> *vars) : m_vars(vars)
{
}

void CMakeProjectVisitor::notImplemented() const {
    kDebug(9032) << "not implemented!" << endl;
}

void CMakeProjectVisitor::visit(const CMakeAst *ast)
{
    QList<CMakeAst*> children = ast->children();
    QList<CMakeAst*>::const_iterator it = children.begin();
    for(; it!=children.end(); it++) {
        kDebug(9032) << "Navigating ast: " << *it << endl;
        if(*it)
            (*it)->accept(this);
        else
            kWarning(9032) << "Oops!!! found a null object in the AST!" << endl;
    }
}

void CMakeProjectVisitor::visit(const ProjectAst *project)
{
    m_projectName = project->projectName();
}

void CMakeProjectVisitor::visit(const AddSubdirectoryAst *subd)
{
    m_subdirectories += subd->sourceDir();
}

void CMakeProjectVisitor::visit(const AddExecutableAst *exec)
{
    m_filesPerTarget.insert(exec->executable(), exec->sourceLists());
    kDebug(9032) << "exec: " << exec->executable() << endl;
}

void CMakeProjectVisitor::visit(const AddLibraryAst *lib)
{
    m_filesPerTarget.insert(lib->libraryName(), lib->sourceLists());
    kDebug(9032) << "lib: " << lib->libraryName() << endl;
}

void CMakeProjectVisitor::visit(const SetAst *set)
{
    m_vars->remove(set->variableName());
    m_vars->insert(set->variableName(), set->values());
    kDebug(9032) << "set: " << set->variableName() << "=" << set->values() << endl;
}

void CMakeProjectVisitor::visit(const IncludeDirectoriesAst * dirs)
{
    IncludeDirectoriesAst::IncludeType t = dirs->includeType();

    if(t==IncludeDirectoriesAst::DEFAULT && m_vars->value("CMAKE_INCLUDE_DIRECTORIES_BEFORE")==QStringList("ON"))
        t = IncludeDirectoriesAst::BEFORE;
    else
        t = IncludeDirectoriesAst::AFTER;
    
    if(t==IncludeDirectoriesAst::AFTER)
        m_includeDirectories += dirs->includedDirectories();
    else
        m_includeDirectories = dirs->includedDirectories() + m_includeDirectories;
}

