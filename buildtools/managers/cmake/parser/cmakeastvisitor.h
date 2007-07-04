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

#ifndef CMAKEVISITOR_H
#define CMAKEVISITOR_H

class CustomCommandAst;
class CustomTargetAst;
class AddDefinitionsAst;
class AddDependenciesAst;
class AddExecutableAst;
class AddLibraryAst;
class AddSubdirectoryAst;
class AddTestAst;
class AuxSourceDirectoryAst;
class BuildCommandAst;
class BuildNameAst;
class CMakeMinimumRequiredAst;
class ConfigureFileAst;
class IncludeAst;
class SetAst;
class ProjectAst;
class MacroAst;
class IncludeDirectoriesAst;
class MacroCallAst;
class FindPackageAst;
class FindProgramAst;
class FindPathAst;
class CMakeAst;

class CMakeAstVisitor {
    public: //maybe protected?
        virtual ~CMakeAstVisitor() {}
        
        virtual void visit( const CustomCommandAst * ) = 0;
        virtual void visit( const CustomTargetAst * ) = 0;
        virtual void visit( const AddDefinitionsAst * ) = 0;
        virtual void visit( const AddDependenciesAst * ) = 0;
        virtual void visit( const AddExecutableAst * ) = 0;
        virtual void visit( const AddLibraryAst * ) = 0;
        virtual void visit( const AddSubdirectoryAst * ) = 0;
        virtual void visit( const AddTestAst * ) = 0;
        virtual void visit( const AuxSourceDirectoryAst * ) = 0;
        virtual void visit( const BuildCommandAst * ) = 0;
        virtual void visit( const BuildNameAst * ) = 0;
        virtual void visit( const CMakeMinimumRequiredAst * ) = 0;
        virtual void visit( const ConfigureFileAst * ) = 0;
        virtual void visit( const IncludeAst * ) = 0;
        virtual void visit( const SetAst * ) = 0;
        virtual void visit( const ProjectAst * ) = 0;
        virtual void visit( const IncludeDirectoriesAst * ) = 0;
        virtual void visit( const MacroCallAst * ) = 0;
        virtual void visit( const FindPackageAst * ) = 0;
        virtual void visit( const FindProgramAst * ) = 0;
        virtual void visit( const FindPathAst * ) = 0;
        virtual void visit( MacroAst * ) = 0;
        virtual void visit( const CMakeAst * ) = 0;
};

#endif
