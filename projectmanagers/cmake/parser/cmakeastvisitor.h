
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

class SetTestsPropsAst;
class SetDirectoryPropsAst;
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
class FunctionAst;
class GetCMakePropertyAst;
class GetDirPropertyAst;
class GetFilenameComponentAst;
class GetPropertyAst;
class GetSourceFilePropAst;
class GetTargetPropAst;
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
class RemoveDefinitionsAst;
class SetAst;
class SetTargetPropsAst;
class SeparateArgumentsAst;
class SetPropertyAst;
class StringAst;
class SubdirsAst;
class TryCompileAst;
class TargetLinkLibrariesAst;
class UnsetAst;
class WhileAst;

class CMakeAstVisitor {
    public: //maybe protected?
        virtual ~CMakeAstVisitor() {}

        virtual int visit( const AddDefinitionsAst * ) = 0;
        virtual int visit( const AddExecutableAst * ) = 0;
        virtual int visit( const AddLibraryAst * ) = 0;
        virtual int visit( const AddSubdirectoryAst * ) = 0;
        virtual int visit( const AddTestAst * ) = 0;
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
        virtual int visit( const FunctionAst * ) = 0;
        virtual int visit( const GetCMakePropertyAst * ) = 0;
        virtual int visit( const GetDirPropertyAst * ) = 0;
        virtual int visit( const GetFilenameComponentAst * ) = 0;
        virtual int visit( const GetPropertyAst * ) = 0;
        virtual int visit( const GetTargetPropAst * ) = 0;
        virtual int visit( const GetSourceFilePropAst * )=0;
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
        virtual int visit( const RemoveDefinitionsAst * ) = 0;
        virtual int visit( const SetAst * ) = 0;
        virtual int visit( const SetTargetPropsAst * ) = 0;
        virtual int visit( const SetDirectoryPropsAst * ) = 0;
        virtual int visit( const SetPropertyAst * ) = 0;
        virtual int visit( const SetTestsPropsAst * ) = 0;
        virtual int visit( const StringAst * ) = 0;
        virtual int visit( const SubdirsAst * ) = 0;
        virtual int visit( const TryCompileAst * ) = 0;
        virtual int visit( const TargetLinkLibrariesAst * ) = 0;
        virtual int visit( const SeparateArgumentsAst * ) = 0;
        virtual int visit( const UnsetAst * ) = 0;
        virtual int visit( const WhileAst * ) = 0;

        virtual int visit( const CMakeAst * ) = 0;
};

#endif
