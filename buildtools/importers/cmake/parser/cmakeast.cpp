/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * Some parts of this code are based on CMake
 * Copyright 2002 Kitware, Inc. Insight Consortium
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

#include "cmakeast.h"

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include "astfactory.h"
#include "cmakelistsparser.h"

void CMakeAst::writeBack(QString& buffer)
{

}


CMAKE_REGISTER_AST( CustomCommandAst, add_custom_command )
CMAKE_REGISTER_AST( CustomTargetAst, add_custom_target )
CMAKE_REGISTER_AST( AddDefinitionsAst, add_definitions )
CMAKE_REGISTER_AST( AddDependenciesAst, add_dependencies )
CMAKE_REGISTER_AST( AddExecutableAst, add_executable )
CMAKE_REGISTER_AST( AddLibraryAst, add_library )

CustomCommandAst::CustomCommandAst()
{
    m_forTarget = false;
    m_buildStage = PostBuild;
    m_isVerbatim = false;
    m_append = false;
}

CustomCommandAst::~CustomCommandAst()
{
}

void CustomCommandAst::writeBack( QString& /*buffer */ )
{
}

bool CustomCommandAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != QLatin1String( "add_custom_command" )  )
        return false;

    if (func.arguments.size() < 4)
        return false;

    enum tdoing {
        doing_source,
        doing_command,
        doing_target,
        doing_depends,
        doing_main_dependency,
        doing_output,
        doing_outputs,
        doing_comment,
        doing_working_directory,
        doing_nothing
    };

    tdoing doing = doing_nothing;
    QString currentLine;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    for ( it = func.arguments.begin(); it != itEnd; ++it)
    {
        QString copy = ( *it ).value;
        if(copy == "SOURCE")
            doing = doing_source;
        else if(copy == "COMMAND")
        {
            doing = doing_command;
            // Save the current command before starting the next command.
            if ( !currentLine.isEmpty() )
            {
                m_commands.append( currentLine );
                currentLine.clear();
            }
        }
        else if(copy == "PRE_BUILD")
            m_buildStage = PreBuild;
        else if(copy == "PRE_LINK")
            m_buildStage = PreLink;
        else if(copy == "POST_BUILD")
            m_buildStage = PostBuild;
        else if(copy == "VERBATIM")
            m_isVerbatim = true;
        else if(copy == "APPEND")
            m_append = true;
        else if(copy == "TARGET")
            doing = doing_target;
        else if(copy == "ARGS")
            // Ignore this old keyword.
            ;
        else if (copy == "DEPENDS")
            doing = doing_depends;
        else if (copy == "OUTPUTS")
            doing = doing_outputs;
        else if (copy == "OUTPUT")
            doing = doing_output;
        else if (copy == "WORKING_DIRECTORY")
            doing = doing_working_directory;
        else if (copy == "MAIN_DEPENDENCY")
            doing = doing_main_dependency;
        else if (copy == "COMMENT")
            doing = doing_comment;
        else
        {
            switch (doing)
            {
            case doing_working_directory:
                m_workingDir = copy;
                break;
            case doing_source:
                m_source = copy;
                break;
            case doing_main_dependency:
                m_mainDep = copy;
                break;
            case doing_command:
                m_commands.append( copy );
                break;
            case doing_target:
                m_target = copy;
                break;
            case doing_depends:
                m_otherDeps.append( copy );
                break;
            case doing_outputs:
            case doing_output:
                m_outputs.append(copy);
                break;
            case doing_comment:
                m_comment = copy;
                break;
            default:
                return false;
            }
        }
    }

    // Store the last command line finished.
    if ( !currentLine.isEmpty() )
    {
        m_commands.append( currentLine );
        currentLine.clear();
    }

    // At this point we could complain about the lack of arguments.  For
    // the moment, let's say that COMMAND, TARGET are always required.
    if ( m_outputs.isEmpty() && m_target.isEmpty() )
        return false;

    if ( m_source.isEmpty() && !m_target.isEmpty() && !m_outputs.isEmpty())
        return false;

    if ( m_append && m_outputs.isEmpty() )
        return false;

    return true;
}

CustomTargetAst::CustomTargetAst()
{
    m_buildAlways = false;
    m_isVerbatim = false;
}

CustomTargetAst::~CustomTargetAst()
{
}

void CustomTargetAst::writeBack( QString& )
{
}

bool CustomTargetAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != QLatin1String( "add_custom_target" ) )
        return false;

    //make sure we have at least one argument
    if ( func.arguments.size() < 1 )
        return false;

    //check and make sure the target name isn't something silly
    CMakeFunctionArgument arg = func.arguments.front();
    if ( arg.value.toLower() == QLatin1String( "all" ) )
        return false;
    else
        m_target = arg.value;

    //check if we're part of the special "all" target
    CMakeFunctionArgument arg2 = func.arguments[1];
    if ( arg2.value == QLatin1String( "ALL" ) )
        m_buildAlways = true;
    else
        m_buildAlways = false;

    //what are we doing?
    enum Action {
        ParsingCommand,
        ParsingDep,
        ParsingWorkingDir,
        ParsingComment,
        ParsingVerbatim
    };

    //command should be first
    QString currentLine;
    Action act = ParsingCommand;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    it = func.arguments.begin() + 2; //advance the iterator two places
    for ( ; it != itEnd; ++it )
    {
        QString arg = ( *it ).value;
        if ( arg == "DEPENDS" )
            act = ParsingDep;
        else if ( arg == "WORKING_DIRECTORY" )
            act = ParsingWorkingDir;
        else if ( arg == "VERBATIM" )
        {
            m_isVerbatim = true;
            act = ParsingVerbatim;
        }
        else if ( arg == "COMMENT" )
            act = ParsingComment;
        else if ( arg == "COMMAND" )
        {
            act = ParsingCommand;
            if ( !currentLine.isEmpty() )
            {
                m_commands.append( currentLine );
                currentLine.clear();
            }
        }
        else
        {
            switch( act )
            {
            case ParsingCommand:
                m_commands.append( arg );
                break;
            case ParsingDep:
                m_dependencies.append( arg );
                break;
            case ParsingWorkingDir:
                m_workingDir = arg;
                break;
            case ParsingComment:
                m_comment += arg;
                break;
            default:
                return false;
            }
        }
    }

    //check for bogus characters in the target name
    if ( m_target.indexOf( QRegExp( "(#|<|>)" ) ) != -1 )
        return false;

    if ( !currentLine.isEmpty() )
    {
        m_commands.append( currentLine );
        currentLine.clear();
    }

    return true;
}

/* Add Definitions AST */

AddDefinitionsAst::AddDefinitionsAst()
{

}

AddDefinitionsAst::~AddDefinitionsAst()
{
}

void AddDefinitionsAst::writeBack( QString& )
{
}

bool AddDefinitionsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "add_definitions" )
        return false;

    if ( func.arguments.isEmpty() )
        return false;

    foreach( CMakeFunctionArgument arg, func.arguments )
    {
        m_definitions << arg.value;
    }

    return true;
}

/* Add Dependencies AST */

AddDependenciesAst::AddDependenciesAst()
{
}

AddDependenciesAst::~AddDependenciesAst()
{
}

void AddDependenciesAst::writeBack( QString& )
{
}

bool AddDependenciesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "add_dependencies" )
        return false;

    if ( func.arguments.size() < 2 )
        return false;

    QList<CMakeFunctionArgument> args = func.arguments;
    m_target = args.front().value;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = args.end();
    it = args.begin() + 1; //skip the first argument since it's the target
    for ( ; it != itEnd; ++it )
    {
        m_dependencies << ( *it ).value;
    }

    return true;
}

AddExecutableAst::AddExecutableAst()
{
    m_isWin32 = false;
    m_isOsXBundle = false;
    m_excludeFromAll = false;
}

AddExecutableAst::~AddExecutableAst()
{
}

void AddExecutableAst::writeBack( QString& )
{
}

bool AddExecutableAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "add_executable" )
        return false;

    if ( func.arguments.size() < 2 )
        return false;

    QList<CMakeFunctionArgument> args = func.arguments;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = args.end();
    it = args.begin();
    m_executable = ( *it ).value;
    ++it;
    for ( ; it != itEnd; ++it )
    {
        if ( ( *it ).value == "WIN32" )
            m_isWin32 = true;
        else if ( ( *it ).value == "MACOSX_BUNDLE" )
            m_isOsXBundle = true;
        else if ( ( *it ).value == "EXCLUDE_FROM_ALL" )
            m_excludeFromAll = true;
        else
            m_sourceLists.append( ( *it ).value );
    }

    if ( m_sourceLists.isEmpty() )
        return false;

    return true;

}

AddLibraryAst::AddLibraryAst()
{
    m_isShared = false;
    m_isStatic = true;
    m_isModule = false;
    m_excludeFromAll = false;
}

AddLibraryAst::~AddLibraryAst()
{
}

void AddLibraryAst::writeBack( QString& )
{
}

bool AddLibraryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "add_library" )
        return false;

    if ( func.arguments.size() < 1 )
        return false;

    bool libTypeSet = false;
    QList<CMakeFunctionArgument> args = func.arguments;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = args.end();
    it = args.begin();
    m_libraryName = ( *it ).value;
    ++it;
    while ( it != itEnd )
    {
        if ( ( *it ).value == "STATIC" && !libTypeSet )
        {
            m_isStatic = true;
            libTypeSet = true;
            ++it;
        }
        else if ( ( *it ).value == "SHARED" && !libTypeSet )
        {
            m_isShared = true;
            libTypeSet = true;
            ++it;
        }
        else if ( ( *it ).value == "MODULE" && !libTypeSet )
        {
            m_isModule = true;
            libTypeSet = true;
            ++it;
        }
        else if ( ( *it ).value == "EXCLUDE_FROM_ALL" )
        {
            m_excludeFromAll = true;
            ++it;
        }
        else
            break;
    }

    if ( it == itEnd )
        return false; //there are no sources

    while ( it != itEnd )
    {
            m_sourceLists.append( ( *it ).value );
            ++it;
    }

    if ( m_sourceLists.isEmpty() )
        return false;

    return true;

}
AddSubdirectoryAst::AddSubdirectoryAst()
{
}

AddSubdirectoryAst::~AddSubdirectoryAst()
{
}

void AddSubdirectoryAst::writeBack( QString& )
{
}

bool AddSubdirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

AddTestAst::AddTestAst()
{
}

AddTestAst::~AddTestAst()
{
}

void AddTestAst::writeBack( QString& )
{
}

bool AddTestAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

AuxSourceDirectoryAst::AuxSourceDirectoryAst()
{
}

AuxSourceDirectoryAst::~AuxSourceDirectoryAst()
{
}

void AuxSourceDirectoryAst::writeBack( QString& )
{
}

bool AuxSourceDirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

BuildCommandAst::BuildCommandAst()
{
}

BuildCommandAst::~BuildCommandAst()
{
}

void BuildCommandAst::writeBack( QString& )
{
}

bool BuildCommandAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

BuildNameAst::BuildNameAst()
{
}

BuildNameAst::~BuildNameAst()
{
}

void BuildNameAst::writeBack( QString& )
{
}

bool BuildNameAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

CMakeMinimumRequiredAst::CMakeMinimumRequiredAst()
{
}

CMakeMinimumRequiredAst::~CMakeMinimumRequiredAst()
{
}

void CMakeMinimumRequiredAst::writeBack( QString& )
{
}

bool CMakeMinimumRequiredAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

ConfigureFileAst::ConfigureFileAst()
{
}

ConfigureFileAst::~ConfigureFileAst()
{
}

void ConfigureFileAst::writeBack( QString& )
{
}

bool ConfigureFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

CreateTestSourcelistAst::CreateTestSourcelistAst()
{
}

CreateTestSourcelistAst::~CreateTestSourcelistAst()
{
}

void CreateTestSourcelistAst::writeBack( QString& )
{
}

bool CreateTestSourcelistAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

EnableLanguageAst::EnableLanguageAst()
{
}

EnableLanguageAst::~EnableLanguageAst()
{
}

void EnableLanguageAst::writeBack( QString& )
{
}

bool EnableLanguageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

EnableTestingAst::EnableTestingAst()
{
}

EnableTestingAst::~EnableTestingAst()
{
}

void EnableTestingAst::writeBack( QString& )
{
}

bool EnableTestingAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

ExecProgramAst::ExecProgramAst()
{
}

ExecProgramAst::~ExecProgramAst()
{
}

void ExecProgramAst::writeBack( QString& )
{
}

bool ExecProgramAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

ExecuteProcessAst::ExecuteProcessAst()
{
}

ExecuteProcessAst::~ExecuteProcessAst()
{
}

void ExecuteProcessAst::writeBack( QString& )
{
}

bool ExecuteProcessAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

ExportLibraryDepsAst::ExportLibraryDepsAst()
{
}

ExportLibraryDepsAst::~ExportLibraryDepsAst()
{
}

void ExportLibraryDepsAst::writeBack( QString& )
{
}

bool ExportLibraryDepsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

FileAst::FileAst()
{
}

FileAst::~FileAst()
{
}

void FileAst::writeBack( QString& )
{
}

bool FileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

FindFileAst::FindFileAst()
{
}

FindFileAst::~FindFileAst()
{
}

void FindFileAst::writeBack( QString& )
{
}

bool FindFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

FindLibraryAst::FindLibraryAst()
{
}

FindLibraryAst::~FindLibraryAst()
{
}

void FindLibraryAst::writeBack( QString& )
{
}

bool FindLibraryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

FindPackageAst::FindPackageAst()
{
}

FindPackageAst::~FindPackageAst()
{
}

void FindPackageAst::writeBack( QString& )
{
}

bool FindPackageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

FindPathAst::FindPathAst()
{
}

FindPathAst::~FindPathAst()
{
}

void FindPathAst::writeBack( QString& )
{
}

bool FindPathAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

FindProgramAst::FindProgramAst()
{
}

FindProgramAst::~FindProgramAst()
{
}

void FindProgramAst::writeBack( QString& )
{
}

bool FindProgramAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

FltkWrapUiAst::FltkWrapUiAst()
{
}

FltkWrapUiAst::~FltkWrapUiAst()
{
}

void FltkWrapUiAst::writeBack( QString& )
{
}

bool FltkWrapUiAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

ForeachAst::ForeachAst()
{
}

ForeachAst::~ForeachAst()
{
}

void ForeachAst::writeBack( QString& )
{
}

bool ForeachAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

GetCMakePropertyAst::GetCMakePropertyAst()
{
}

GetCMakePropertyAst::~GetCMakePropertyAst()
{
}

void GetCMakePropertyAst::writeBack( QString& )
{
}

bool GetCMakePropertyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

GetDirPropertyAst::GetDirPropertyAst()
{
}

GetDirPropertyAst::~GetDirPropertyAst()
{
}

void GetDirPropertyAst::writeBack( QString& )
{
}

bool GetDirPropertyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

GetFilenameComponentAst::GetFilenameComponentAst()
{
}

GetFilenameComponentAst::~GetFilenameComponentAst()
{
}

void GetFilenameComponentAst::writeBack( QString& )
{
}

bool GetFilenameComponentAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

GetSourceFilePropAst::GetSourceFilePropAst()
{
}

GetSourceFilePropAst::~GetSourceFilePropAst()
{
}

void GetSourceFilePropAst::writeBack( QString& )
{
}

bool GetSourceFilePropAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

GetTargetPropAst::GetTargetPropAst()
{
}

GetTargetPropAst::~GetTargetPropAst()
{
}

void GetTargetPropAst::writeBack( QString& )
{
}

bool GetTargetPropAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

GetTestPropAst::GetTestPropAst()
{
}

GetTestPropAst::~GetTestPropAst()
{
}

void GetTestPropAst::writeBack( QString& )
{
}

bool GetTestPropAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

IfAst::IfAst()
{
}

IfAst::~IfAst()
{
}

void IfAst::writeBack( QString& )
{
}

bool IfAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

IncludeAst::IncludeAst()
{
}

IncludeAst::~IncludeAst()
{
}

void IncludeAst::writeBack( QString& )
{
}

bool IncludeAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

IncludeDirectoriesAst::IncludeDirectoriesAst()
{
}

IncludeDirectoriesAst::~IncludeDirectoriesAst()
{
}

void IncludeDirectoriesAst::writeBack( QString& )
{
}

bool IncludeDirectoriesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

IncludeExternalMsProjectAst::IncludeExternalMsProjectAst()
{
}

IncludeExternalMsProjectAst::~IncludeExternalMsProjectAst()
{
}

void IncludeExternalMsProjectAst::writeBack( QString& )
{
}

bool IncludeExternalMsProjectAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

IncludeRegularExpressionAst::IncludeRegularExpressionAst()
{
}

IncludeRegularExpressionAst::~IncludeRegularExpressionAst()
{
}

void IncludeRegularExpressionAst::writeBack( QString& )
{
}

bool IncludeRegularExpressionAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

InstallAst::InstallAst()
{
}

InstallAst::~InstallAst()
{
}

void InstallAst::writeBack( QString& )
{
}

bool InstallAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

InstallFilesAst::InstallFilesAst()
{
}

InstallFilesAst::~InstallFilesAst()
{
}

void InstallFilesAst::writeBack( QString& )
{
}

bool InstallFilesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

InstallProgramsAst::InstallProgramsAst()
{
}

InstallProgramsAst::~InstallProgramsAst()
{
}

void InstallProgramsAst::writeBack( QString& )
{
}

bool InstallProgramsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

InstallTargetsAst::InstallTargetsAst()
{
}

InstallTargetsAst::~InstallTargetsAst()
{
}

void InstallTargetsAst::writeBack( QString& )
{
}

bool InstallTargetsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

LinkDirectoriesAst::LinkDirectoriesAst()
{
}

LinkDirectoriesAst::~LinkDirectoriesAst()
{
}

void LinkDirectoriesAst::writeBack( QString& )
{
}

bool LinkDirectoriesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

LinkLibrariesAst::LinkLibrariesAst()
{
}

LinkLibrariesAst::~LinkLibrariesAst()
{
}

void LinkLibrariesAst::writeBack( QString& )
{
}

bool LinkLibrariesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

ListAst::ListAst()
{
}

ListAst::~ListAst()
{
}

void ListAst::writeBack( QString& )
{
}

bool ListAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

LoadCacheAst::LoadCacheAst()
{
}

LoadCacheAst::~LoadCacheAst()
{
}

void LoadCacheAst::writeBack( QString& )
{
}

bool LoadCacheAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

LoadCommandAst::LoadCommandAst()
{
}

LoadCommandAst::~LoadCommandAst()
{
}

void LoadCommandAst::writeBack( QString& )
{
}

bool LoadCommandAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

MacroAst::MacroAst()
{
}

MacroAst::~MacroAst()
{
}

void MacroAst::writeBack( QString& )
{
}

bool MacroAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

MakeDirectoryAst::MakeDirectoryAst()
{
}

MakeDirectoryAst::~MakeDirectoryAst()
{
}

void MakeDirectoryAst::writeBack( QString& )
{
}

bool MakeDirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

MarkAsAdvancedAst::MarkAsAdvancedAst()
{
}

MarkAsAdvancedAst::~MarkAsAdvancedAst()
{
}

void MarkAsAdvancedAst::writeBack( QString& )
{
}

bool MarkAsAdvancedAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

MathAst::MathAst()
{
}

MathAst::~MathAst()
{
}

void MathAst::writeBack( QString& )
{
}

bool MathAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

MessageAst::MessageAst()
{
}

MessageAst::~MessageAst()
{
}

void MessageAst::writeBack( QString& )
{
}

bool MessageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

OptionAst::OptionAst()
{
}

OptionAst::~OptionAst()
{
}

void OptionAst::writeBack( QString& )
{
}

bool OptionAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

OutputRequiredFilesAst::OutputRequiredFilesAst()
{
}

OutputRequiredFilesAst::~OutputRequiredFilesAst()
{
}

void OutputRequiredFilesAst::writeBack( QString& )
{
}

bool OutputRequiredFilesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

ProjectAst::ProjectAst()
{
}

ProjectAst::~ProjectAst()
{
}

void ProjectAst::writeBack( QString& )
{
}

bool ProjectAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

QtWrapCppAst::QtWrapCppAst()
{
}

QtWrapCppAst::~QtWrapCppAst()
{
}

void QtWrapCppAst::writeBack( QString& )
{
}

bool QtWrapCppAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

QtWrapUiAst::QtWrapUiAst()
{
}

QtWrapUiAst::~QtWrapUiAst()
{
}

void QtWrapUiAst::writeBack( QString& )
{
}

bool QtWrapUiAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

RemoveAst::RemoveAst()
{
}

RemoveAst::~RemoveAst()
{
}

void RemoveAst::writeBack( QString& )
{
}

bool RemoveAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

RemoveDefinitionsAst::RemoveDefinitionsAst()
{
}

RemoveDefinitionsAst::~RemoveDefinitionsAst()
{
}

void RemoveDefinitionsAst::writeBack( QString& )
{
}

bool RemoveDefinitionsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SeparateArgumentsAst::SeparateArgumentsAst()
{
}

SeparateArgumentsAst::~SeparateArgumentsAst()
{
}

void SeparateArgumentsAst::writeBack( QString& )
{
}

bool SeparateArgumentsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SetAst::SetAst()
{
}

SetAst::~SetAst()
{
}

void SetAst::writeBack( QString& )
{
}

bool SetAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SetDirectoryPropsAst::SetDirectoryPropsAst()
{
}

SetDirectoryPropsAst::~SetDirectoryPropsAst()
{
}

void SetDirectoryPropsAst::writeBack( QString& )
{
}

bool SetDirectoryPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SetSourceFilesPropsAst::SetSourceFilesPropsAst()
{
}

SetSourceFilesPropsAst::~SetSourceFilesPropsAst()
{
}

void SetSourceFilesPropsAst::writeBack( QString& )
{
}

bool SetSourceFilesPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SetTargetPropsAst::SetTargetPropsAst()
{
}

SetTargetPropsAst::~SetTargetPropsAst()
{
}

void SetTargetPropsAst::writeBack( QString& )
{
}

bool SetTargetPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SetTestsPropsAst::SetTestsPropsAst()
{
}

SetTestsPropsAst::~SetTestsPropsAst()
{
}

void SetTestsPropsAst::writeBack( QString& )
{
}

bool SetTestsPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SiteNameAst::SiteNameAst()
{
}

SiteNameAst::~SiteNameAst()
{
}

void SiteNameAst::writeBack( QString& )
{
}

bool SiteNameAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SourceGroupAst::SourceGroupAst()
{
}

SourceGroupAst::~SourceGroupAst()
{
}

void SourceGroupAst::writeBack( QString& )
{
}

bool SourceGroupAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

StringAst::StringAst()
{
}

StringAst::~StringAst()
{
}

void StringAst::writeBack( QString& )
{
}

bool StringAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SubdirDependsAst::SubdirDependsAst()
{
}

SubdirDependsAst::~SubdirDependsAst()
{
}

void SubdirDependsAst::writeBack( QString& )
{
}

bool SubdirDependsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

SubdirsAst::SubdirsAst()
{
}

SubdirsAst::~SubdirsAst()
{
}

void SubdirsAst::writeBack( QString& )
{
}

bool SubdirsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

TargetLinkLibrariesAst::TargetLinkLibrariesAst()
{
}

TargetLinkLibrariesAst::~TargetLinkLibrariesAst()
{
}

void TargetLinkLibrariesAst::writeBack( QString& )
{
}

bool TargetLinkLibrariesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

TryCompileAst::TryCompileAst()
{
}

TryCompileAst::~TryCompileAst()
{
}

void TryCompileAst::writeBack( QString& )
{
}

bool TryCompileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

TryRunAst::TryRunAst()
{
}

TryRunAst::~TryRunAst()
{
}

void TryRunAst::writeBack( QString& )
{
}

bool TryRunAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

UseMangledMesaAst::UseMangledMesaAst()
{
}

UseMangledMesaAst::~UseMangledMesaAst()
{
}

void UseMangledMesaAst::writeBack( QString& )
{
}

bool UseMangledMesaAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

UtilitySourceAst::UtilitySourceAst()
{
}

UtilitySourceAst::~UtilitySourceAst()
{
}

void UtilitySourceAst::writeBack( QString& )
{
}

bool UtilitySourceAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

VariableRequiresAst::VariableRequiresAst()
{
}

VariableRequiresAst::~VariableRequiresAst()
{
}

void VariableRequiresAst::writeBack( QString& )
{
}

bool VariableRequiresAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

VtkMakeInstantiatorAst::VtkMakeInstantiatorAst()
{
}

VtkMakeInstantiatorAst::~VtkMakeInstantiatorAst()
{
}

void VtkMakeInstantiatorAst::writeBack( QString& )
{
}

bool VtkMakeInstantiatorAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

VtkWrapJavaAst::VtkWrapJavaAst()
{
}

VtkWrapJavaAst::~VtkWrapJavaAst()
{
}

void VtkWrapJavaAst::writeBack( QString& )
{
}

bool VtkWrapJavaAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

VtkWrapPythonAst::VtkWrapPythonAst()
{
}

VtkWrapPythonAst::~VtkWrapPythonAst()
{
}

void VtkWrapPythonAst::writeBack( QString& )
{
}

bool VtkWrapPythonAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

VtkWrapTclAst::VtkWrapTclAst()
{
}

VtkWrapTclAst::~VtkWrapTclAst()
{
}

void VtkWrapTclAst::writeBack( QString& )
{
}

bool VtkWrapTclAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

WhileAst::WhileAst()
{
}

WhileAst::~WhileAst()
{
}

void WhileAst::writeBack( QString& )
{
}

bool WhileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

WriteFileAst::WriteFileAst()
{
}

WriteFileAst::~WriteFileAst()
{
}

void WriteFileAst::writeBack( QString& )
{
}

bool WriteFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

