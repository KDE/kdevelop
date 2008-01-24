
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

#ifndef CMAKEASTVISITOR_H
#define CMAKEASTVISITOR_H

class AddDefinitionsAst;
class AddDependenciesAst;
class AddExecutableAst;
class AddLibraryAst;
class AddSubdirectoryAst;
class AddTestAst;
class AuxSourceDirectoryAst;
class BuildCommandAst;
class BuildNameAst;
class CMakeAst;
class CMakeMinimumRequiredAst;
class ConfigureFileAst;
class CustomCommandAst;
class CustomTargetAst;
class ExecProgramAst;
class ExecuteProcessAst;
class FileAst;
class FindFileAst;
class FindLibraryAst;
class FindPackageAst;
class FindPathAst;
class FindProgramAst;
class ForeachAst;
class GetCMakePropertyAst;
class GetFilenameComponentAst;
class IfAst;
class IncludeAst;
class IncludeDirectoriesAst;
class ListAst;
class MacroAst;
class MacroCallAst;
class MarkAsAdvancedAst;
class MathAst;
class MessageAst;
class OptionAst;
class ProjectAst;
class SetAst;
class StringAst;
class SubdirsAst;
class TryCompileAst;
class TargetLinkLibrariesAst;

class CMakeAstVisitor {
    public: //maybe protected?
        virtual ~CMakeAstVisitor() {}

        virtual int visit( const AddDefinitionsAst * ) = 0;
        virtual int visit( const AddDependenciesAst * ) = 0;
        virtual int visit( const AddExecutableAst * ) = 0;
        virtual int visit( const AddLibraryAst * ) = 0;
        virtual int visit( const AddSubdirectoryAst * ) = 0;
        virtual int visit( const AddTestAst * ) = 0;
        virtual int visit( const AuxSourceDirectoryAst * ) = 0;
        virtual int visit( const BuildCommandAst * ) = 0;
        virtual int visit( const BuildNameAst * ) = 0;
        virtual int visit( const CMakeMinimumRequiredAst * ) = 0;
        virtual int visit( const ConfigureFileAst * ) = 0;
        virtual int visit( const CustomCommandAst * ) = 0;
        virtual int visit( const CustomTargetAst * ) = 0;
        virtual int visit( const ExecProgramAst * ) = 0;
        virtual int visit( const ExecuteProcessAst * ) = 0;
        virtual int visit( const FileAst * ) = 0;
        virtual int visit( const FindFileAst * ) = 0;
        virtual int visit( const FindLibraryAst * ) = 0;
        virtual int visit( const FindPackageAst * ) = 0;
        virtual int visit( const FindPathAst * ) = 0;
        virtual int visit( const FindProgramAst * ) = 0;
        virtual int visit( const ForeachAst * ) = 0;
        virtual int visit( const GetCMakePropertyAst * ) = 0;
        virtual int visit( const GetFilenameComponentAst * ) = 0;
        virtual int visit( const IfAst * ) = 0;
        virtual int visit( const IncludeAst * ) = 0;
        virtual int visit( const IncludeDirectoriesAst * ) = 0;
        virtual int visit( const ListAst * ) = 0;
        virtual int visit( const MacroAst * ) = 0;
        virtual int visit( const MacroCallAst * ) = 0;
        virtual int visit( const MarkAsAdvancedAst * ) = 0;
        virtual int visit( const MathAst * ) = 0;
        virtual int visit( const MessageAst * ) = 0;
        virtual int visit( const OptionAst * ) = 0;
        virtual int visit( const ProjectAst * ) = 0;
        virtual int visit( const SetAst * ) = 0;
        virtual int visit( const StringAst * ) = 0;
        virtual int visit( const SubdirsAst * ) = 0;
        virtual int visit( const TryCompileAst * ) = 0;
        virtual int visit( const TargetLinkLibrariesAst * ) = 0;

        virtual int visit( const CMakeAst * ) = 0;
};

#endif
