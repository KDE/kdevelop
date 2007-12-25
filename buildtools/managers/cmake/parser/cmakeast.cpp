/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
 *
 * Some parts of this code are based on CMake
 * Copyright 2002 Kitware, Inc. Insight Consortium <kitware@kitware.com>
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

#include <QRegExp>
#include <QString>

#include <KDebug>

#include "astfactory.h"
#include "cmakelistsparser.h"

void CMakeAst::writeBack(QString& ) const
{
}

CMAKE_REGISTER_AST( CustomCommandAst, add_custom_command )
CMAKE_REGISTER_AST( CustomTargetAst, add_custom_target )
CMAKE_REGISTER_AST( AddDefinitionsAst, add_definitions )
CMAKE_REGISTER_AST( AddDependenciesAst, add_dependencies )
CMAKE_REGISTER_AST( AddExecutableAst, add_executable )
CMAKE_REGISTER_AST( AddLibraryAst, add_library )
CMAKE_REGISTER_AST( AddSubdirectoryAst, add_subdirectory )
CMAKE_REGISTER_AST( AddTestAst,  add_test )
CMAKE_REGISTER_AST( AuxSourceDirectoryAst, aux_source_directory )
CMAKE_REGISTER_AST( BuildCommandAst, build_command )
CMAKE_REGISTER_AST( BuildNameAst, build_name )
CMAKE_REGISTER_AST( CMakeMinimumRequiredAst, cmake_minimum_required )
CMAKE_REGISTER_AST( ConfigureFileAst, configure_file )
CMAKE_REGISTER_AST( IncludeAst, include )
CMAKE_REGISTER_AST( IfAst, if )
CMAKE_REGISTER_AST( IncludeDirectoriesAst, include_directories )
CMAKE_REGISTER_AST( SetAst, set )
CMAKE_REGISTER_AST( ProjectAst, project )
CMAKE_REGISTER_AST( MacroAst, macro )
CMAKE_REGISTER_AST( ExecProgramAst, exec_program )
CMAKE_REGISTER_AST( MarkAsAdvancedAst, mark_as_advanced )
CMAKE_REGISTER_AST( FindPackageAst, find_package )
CMAKE_REGISTER_AST( FindProgramAst, find_program )
CMAKE_REGISTER_AST( FindPathAst, find_path )
CMAKE_REGISTER_AST( FindFileAst, find_file )
CMAKE_REGISTER_AST( FindLibraryAst, find_library )
CMAKE_REGISTER_AST( FileAst, file )
CMAKE_REGISTER_AST( MessageAst, message )
CMAKE_REGISTER_AST( MathAst, math )
CMAKE_REGISTER_AST( GetFilenameComponentAst, get_filename_component )
CMAKE_REGISTER_AST( ListAst, list )
CMAKE_REGISTER_AST( OptionAst, option )
CMAKE_REGISTER_AST( StringAst, string )
CMAKE_REGISTER_AST( SubdirsAst, subdirs )
CMAKE_REGISTER_AST( GetCMakePropertyAst, get_cmake_property )
CMAKE_REGISTER_AST( ForeachAst, foreach )
CMAKE_REGISTER_AST( ExecuteProcessAst, execute_process )
CMAKE_REGISTER_AST( IncludeRegularExpressionAst, include_regular_expression )
CMAKE_REGISTER_AST( LinkDirectoriesAst, link_directories )
CMAKE_REGISTER_AST( TryRunAst, try_run )
CMAKE_REGISTER_AST( UseMangledMesaAst, use_mangled_mesa )
CMAKE_REGISTER_AST( UtilitySourceAst, utility_source )
CMAKE_REGISTER_AST( VariableRequiresAst, variable_requires )
CMAKE_REGISTER_AST( WhileAst, while)

enum Stage {NAMES, PATHS, PATH_SUFFIXES};

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

void CustomCommandAst::writeBack( QString& /*buffer */ ) const
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
        QString copy = it->value;
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

void CustomTargetAst::writeBack( QString& ) const
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
    if(func.arguments.count()>1)
    {
        CMakeFunctionArgument arg2 = func.arguments[1];
        if ( arg2.value.toUpper() == QLatin1String( "ALL" ) )
            m_buildAlways = true;
        else
            m_buildAlways = false;
    }

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
    it = func.arguments.begin();
    if(m_buildAlways)
        it+=2;
    else
        ++it;
    QString currCmd;
    for ( ; it != itEnd; ++it )
    {
        QString arg = it->value;
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
            currCmd.clear();
            act = ParsingCommand;
        }
        else
        {
            switch( act )
            {
            case ParsingCommand:
                if(m_commandArgs.contains(currCmd))
                    m_commandArgs[currCmd].append(arg);
                else
                {
                    currCmd=arg;
                    m_commandArgs.insert(arg, QStringList());
                }
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

    return true;
}

/* Add Definitions AST */

AddDefinitionsAst::AddDefinitionsAst()
{

}

AddDefinitionsAst::~AddDefinitionsAst()
{
}

void AddDefinitionsAst::writeBack( QString& ) const
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

void AddDependenciesAst::writeBack( QString& ) const
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
        m_dependencies << it->value;
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

void AddExecutableAst::writeBack( QString& ) const
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
    m_executable = it->value;
    ++it;
    for ( ; it != itEnd; ++it )
    {
        if ( it->value == "WIN32" )
            m_isWin32 = true;
        else if ( it->value == "MACOSX_BUNDLE" )
            m_isOsXBundle = true;
        else if ( it->value == "EXCLUDE_FROM_ALL" )
            m_excludeFromAll = true;
        else
            m_sourceLists.append( it->value );
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

void AddLibraryAst::writeBack( QString& ) const
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
    m_libraryName = it->value;
    ++it;
    while ( it != itEnd )
    {
        if ( it->value == "STATIC" && !libTypeSet )
        {
            m_isStatic = true;
            libTypeSet = true;
            ++it;
        }
        else if ( it->value == "SHARED" && !libTypeSet )
        {
            m_isShared = true;
            libTypeSet = true;
            ++it;
        }
        else if ( it->value == "MODULE" && !libTypeSet )
        {
            m_isModule = true;
            libTypeSet = true;
            ++it;
        }
        else if ( it->value == "EXCLUDE_FROM_ALL" )
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
            m_sourceLists.append( it->value );
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

void AddSubdirectoryAst::writeBack( QString& ) const
{
}

bool AddSubdirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "add_subdirectory" )
        return false;
    if ( func.arguments.size() < 1 )
        return false;

    m_sourceDir = func.arguments[0].value;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    it = ++func.arguments.begin();
    for ( ; it != itEnd; ++it )
    {
        if ( it->value == "EXCLUDE_FROM_ALL" )
            m_excludeFromAll = true;
        else if ( m_binaryDir.isEmpty() )
            m_binaryDir = it->value;
        else
            return false; //invalid num of args
    }

    return true;
}

AddTestAst::AddTestAst()
{
}

AddTestAst::~AddTestAst()
{
}

void AddTestAst::writeBack( QString& ) const
{
}

bool AddTestAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "add_test" )
        return false;
    if ( func.arguments.size() < 2 )
        return false;

    m_testName = func.arguments[0].value;
    m_exeName = func.arguments[1].value;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    it = func.arguments.begin() + 2;
    for ( ; it != itEnd; ++it )
        m_testArgs << it->value;

    return true;
}

AuxSourceDirectoryAst::AuxSourceDirectoryAst()
{
}

AuxSourceDirectoryAst::~AuxSourceDirectoryAst()
{
}

void AuxSourceDirectoryAst::writeBack( QString& ) const
{
}

bool AuxSourceDirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "aux_source_directory" )
        return false;
    if ( func.arguments.size() != 2 )
        return false;

    m_dirName = func.arguments[0].value;
    m_variableName = func.arguments[1].value;

    return true;
}

BuildCommandAst::BuildCommandAst()
{
}

BuildCommandAst::~BuildCommandAst()
{
}

void BuildCommandAst::writeBack( QString& ) const
{
}

bool BuildCommandAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "build_command" )
        return false;
    if ( func.arguments.size() < 2 )
        return false;

    m_variableName = func.arguments[0].value;
    m_makeCommand = func.arguments[1].value;

    return true;

}

BuildNameAst::BuildNameAst()
{
}

BuildNameAst::~BuildNameAst()
{
}

void BuildNameAst::writeBack( QString& ) const
{
}

bool BuildNameAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "build_name" )
        return false;
    if ( func.arguments.size() != 1 )
        return false;

    m_buildName = func.arguments[0].value;

    return true;
}

CMakeMinimumRequiredAst::CMakeMinimumRequiredAst()
{
}

CMakeMinimumRequiredAst::~CMakeMinimumRequiredAst()
{
}

void CMakeMinimumRequiredAst::writeBack( QString& ) const
{
}

bool CMakeMinimumRequiredAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "cmake_minimum_required" )
        return false;
    if ( func.arguments.size() < 2 || func.arguments.first().value.toUpper()!="VERSION")
        return false;

    QRegExp rx("([0-9]*).([0-9]*).([0-9]*)");
    rx.indexIn(func.arguments[1].value);
    foreach(QString s, rx.capturedTexts())
    {
        bool correct;
        m_version.append(s.toInt(&correct));
        if(!correct)
            return false;
    }

    if(func.arguments.count()==3)
    {
        if(func.arguments[2].value.toUpper()=="FATAL_ERROR")
            m_wrongVersionIsFatal = true;
        else
            return false;
    }
    else if(func.arguments.count()>3)
        return false;
    return true;
}

ConfigureFileAst::ConfigureFileAst()
{
}

ConfigureFileAst::~ConfigureFileAst()
{
}

void ConfigureFileAst::writeBack( QString& ) const
{
}

bool ConfigureFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "configure_file" )
        return false;

    if ( func.arguments.size() < 2 )
        return false;

    m_inputFile = func.arguments[0].value;
    m_outputFile = func.arguments[1].value;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    it = func.arguments.begin() + 2;
    for ( ; it != itEnd; ++it )
    {
        CMakeFunctionArgument arg = ( *it );
        if ( arg.value == "COPYONLY" )
            m_copyOnly = true;
        else if ( arg.value == "ESCAPE_QUOTES" )
            m_escapeQuotes = true;
        else if ( arg.value == "@ONLY" )
            m_atsOnly = true;
        else if ( arg.value == "IMMEDIATE" )
            m_immediate = true;
    }

    return true;

}

CreateTestSourcelistAst::CreateTestSourcelistAst()
{
}

CreateTestSourcelistAst::~CreateTestSourcelistAst()
{
}

void CreateTestSourcelistAst::writeBack( QString& ) const
{
}

bool CreateTestSourcelistAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "create_test_sourcelist" )
        return false;

    if ( func.arguments.count() < 3 )
        return false;

    return true;
}

EnableLanguageAst::EnableLanguageAst()
{
}

EnableLanguageAst::~EnableLanguageAst()
{
}

void EnableLanguageAst::writeBack( QString& ) const
{
}

bool EnableLanguageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "enable_language" )
        return false;

    if ( func.arguments.count() != 1 )
        return false;

    m_language = func.arguments[0].value;
    return true;
}

EnableTestingAst::EnableTestingAst()
{
}

EnableTestingAst::~EnableTestingAst()
{
}

void EnableTestingAst::writeBack( QString& ) const
{
}

bool EnableTestingAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "enable_testing" )
        return false;

    if ( !func.arguments.isEmpty() )
        return false;

    return true;
}

ExecProgramAst::ExecProgramAst()
{
}

ExecProgramAst::~ExecProgramAst()
{
}

void ExecProgramAst::writeBack( QString& ) const
{
}

bool ExecProgramAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower() != "exec_program" || func.arguments.count()<2)
        return false;
    m_executableName = func.arguments[0].value;
    bool args=false;

    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin(), itEnd=func.arguments.end();
    for(; it!=itEnd; ++it) {
        if(it->value.toUpper()=="OUTPUT_VARIABLE") {
            ++it;
            if(it!=itEnd)
                m_outputVariable = it->value;
            else
                return false;
        } else if(it->value.toUpper()=="RETURN_VALUE") {
            ++it;
            if(it!=itEnd)
                m_returnValue = it->value;
            else
                return false;
        } else if(it->value.toUpper()=="ARGS") {
            args=true;
        } else if(args) {
            m_arguments += it->value;
        } else
            m_workingDirectory = it->value;
    }
    return true;
}

ExecuteProcessAst::ExecuteProcessAst()
    : m_timeout(0.f), m_isOutputQuiet(false), m_isErrorQuiet(false),
                m_isOutputStrip(false), m_isErrorStrip(false)
{
}

ExecuteProcessAst::~ExecuteProcessAst()
{
}

void ExecuteProcessAst::writeBack( QString& ) const
{
}

bool ExecuteProcessAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower() != "execute_process" || func.arguments.count()<2)
        return false;
    
    enum Action {
        None,
        Cmd,
        WorkDir,
        Timeout,
        ResultVar,
        OutputVar,
        ErrorVar,
        InputFile,
        OutputFile,
        ErrorFile
    };
    Action act=None;
    foreach(CMakeFunctionArgument a, func.arguments) {
        QString val=a.value.toLower();
        Action actAnt=act;
        if(val=="command") {
            m_commands.append(QStringList());
            act=Cmd;
        } else if(val=="working_directory")
            act=WorkDir;
        else if(val=="timeout")
            act=Timeout;
        else if(val=="result_variable")
            act=ResultVar;
        else if(val=="output_variable")
            act=OutputVar;
        else if(val=="error_variable")
            act=ErrorVar;
        else if(val=="input_file")
            act=InputFile;
        else if(val=="output_file")
            act=OutputFile;
        else if(val=="error_file")
            act=ErrorFile;
        else if(val=="output_quiet")
        {
            m_isOutputQuiet=true;
            act=None;
        }
        else if(val=="error_quiet")
        {
            m_isErrorQuiet=true;
            act=None;
        }
        else if(val=="output_strip_trailing_whitespace")
        {
            m_isOutputStrip=true;
            act=None;
        }
        else if(val=="error_strip_trailing_whitespace")
        {
            m_isErrorStrip=true;
            act=None;
        }
        if(act!=actAnt)
            val.clear();
        
        switch(act) {
            case None:
                break;
            case Cmd:
                if(!val.isEmpty())
                    m_commands.last().append(a.value);
                break;
            case WorkDir:
                m_workingDirectory=a.value;
                break;
            case Timeout:
                m_timeout=val.toFloat();
                break;
            case ResultVar:
                m_resultVariable=a.value;
                break;
            case OutputVar:
                m_outputVariable=a.value;
                break;
            case ErrorVar:
                m_errorVariable=a.value;
                break;
            case InputFile:
                m_inputFile=a.value;
                break;
            case OutputFile:
                m_outputFile=a.value;
                break;
            case ErrorFile:
                m_errorFile=a.value;
                break;
        }
    }
    return true;
}

ExportLibraryDepsAst::ExportLibraryDepsAst()
{
}

ExportLibraryDepsAst::~ExportLibraryDepsAst()
{
}

void ExportLibraryDepsAst::writeBack( QString& ) const
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

void FileAst::writeBack( QString& ) const
{
}

bool FileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "file" )
        return false;

    if ( func.arguments.count()<2 )
        return false;

    QString type = func.arguments[0].value.toUpper();
    int min_args=-1;
    if(type=="WRITE") {
       m_type = WRITE;
       min_args=3;
    } else if(type=="APPEND") {
        m_type = APPEND;
        min_args=3;
    } else if(type=="READ") {
        m_type = READ;
        min_args=3;
    } else if(type=="GLOB") {
        m_type = GLOB;
        min_args=3;
    } else if(type=="GLOB_RECURSE") {
        m_type = GLOB_RECURSE;
        min_args = 3;
    } else if(type=="REMOVE") {
        m_type = REMOVE;
        min_args=2;
    } else if(type=="REMOVE_RECURSE") {
        m_type = REMOVE_RECURSE;
        min_args=2;
    } else if(type=="MAKE_DIRECTORY") {
        m_type = MAKE_DIRECTORY;
        min_args=2;
    } else if(type=="RELATIVE_PATH") {
        m_type = RELATIVE_PATH;
        min_args=4;
    } else if(type=="TO_CMAKE_PATH") {
        m_type = TO_CMAKE_PATH;
        min_args=3;
    } else if(type=="TO_NATIVE_PATH") {
        m_type = TO_NATIVE_PATH;
        min_args=3;
    }

    if(func.arguments.count()<min_args)
        return false;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd;
    switch(m_type) {
        case WRITE:
        case APPEND:
            m_path = func.arguments[1].value;
            m_message = func.arguments[2].value;
            break;
        case READ:
            m_path=func.arguments[1].value;
            m_variable = func.arguments[2].value;
            break;
        case GLOB:
        case GLOB_RECURSE:
            m_variable = func.arguments[1].value;
            it=func.arguments.constBegin()+2;
            itEnd=func.arguments.constEnd();
            
            for(; it!=itEnd; ++it) {
                if(it->value=="RELATIVE") {
                    it++;
                    if(it==itEnd)
                        return false;
                    else
                        m_path = it->value;
                } else
                    m_globbingExpressions << it->value;
            }
            break;
        case REMOVE:
        case REMOVE_RECURSE:
        case MAKE_DIRECTORY:
            it=func.arguments.constBegin()+1;
            itEnd=func.arguments.constEnd();
            
            for(; it!=itEnd; ++it)
                m_directories << it->value;
            break;
        case RELATIVE_PATH:
            m_variable = func.arguments[1].value;
            m_directory = func.arguments[2].value;
            m_path = func.arguments[3].value;
            break;
        case TO_CMAKE_PATH:
        case TO_NATIVE_PATH:
            m_path = func.arguments[1].value;
            m_variable = func.arguments[2].value;
            break;
    }
    return true;
}

FindFileAst::FindFileAst()
    : m_noDefaultPath(false)
{
}

FindFileAst::~FindFileAst()
{
}

void FindFileAst::writeBack( QString& ) const
{
}

bool FindFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_path" || func.arguments.count()<3)
        return false;
    
    //FIXME: FIND_PATH(KDE4_DATA_DIR cmake/modules/FindKDE4Internal.cmake ${_data_DIR})
    bool definedNames=false;
    m_variableName = func.arguments[0].value;
    Stage s = NAMES;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+1, itEnd=func.arguments.end();
    if(it->value=="NAMES") {
        ++it;
        definedNames = true;
    } else {
        m_filenames=QStringList(it->value);
        it++;
        s=PATHS;
        definedNames = false;
    }

    for(; it!=itEnd; ++it) {
        if(it->value=="NO_DEFAULT_PATH")
            m_noDefaultPath = true;
        else if(it->value=="NO_CMAKE_ENVIRONMENT_PATH")
            m_noCmakeEnvironmentPath = true;
        else if(it->value=="NO_CMAKE_PATH")
            m_noSystemEnvironmentPath = true;
        else if(it->value=="DOC") {
            ++it;
            if(it==itEnd)
                return false;
            m_documentation = it->value;
        } else if(it->value=="PATHS")
            s=PATHS;
            else if(it->value=="PATH_SUFFIXES")
                s=PATH_SUFFIXES;
            else switch(s) {
                case NAMES:
                    m_filenames << it->value;
                    if(!definedNames)
                        s=PATHS;
                    break;
                case PATHS:
                    m_path << it->value;
                    break;
                case PATH_SUFFIXES:
                    m_pathSuffixes << it->value;
                    break;
            }
    }
    return !m_filenames.isEmpty() && !m_path.isEmpty();
}


MacroCallAst::MacroCallAst()
{
}

MacroCallAst::~MacroCallAst()
{
}

void MacroCallAst::writeBack( QString& ) const
{
}

bool MacroCallAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.isEmpty())
        return false;
    m_name = func.name.toLower();

    foreach(CMakeFunctionArgument fa, func.arguments)
    {
        m_arguments += fa.value;
    }
    return true;
}

FindLibraryAst::FindLibraryAst()
    : m_noDefaultPath(false)
{
}

FindLibraryAst::~FindLibraryAst()
{
}

void FindLibraryAst::writeBack( QString& ) const
{
}

bool FindLibraryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_library" || func.arguments.count()<2)
        return false;
    
    bool definedNames=false;
    m_variableName = func.arguments[0].value;
    Stage s = NAMES;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1, itEnd=func.arguments.constEnd();
    if(it->value=="NAMES") {
        ++it;
        definedNames = true;
    } else {
        m_filenames=QStringList(it->value);
        ++it;
        s=PATHS;
        definedNames = false;
    }

    for(; it!=itEnd; ++it) {
        if(it->value=="NO_DEFAULT_PATH")
            m_noDefaultPath = true;
        else if(it->value=="NO_CMAKE_ENVIRONMENT_PATH")
            m_noCmakeEnvironmentPath = true;
        else if(it->value=="NO_CMAKE_PATH")
            m_noSystemEnvironmentPath = true;
        else if(it->value=="DOC") {
            ++it;
            if(it==itEnd)
                return false;
            m_documentation = it->value;
        } else if(it->value=="PATHS")
            s=PATHS;
        else if(it->value=="PATH_SUFFIXES")
            s=PATH_SUFFIXES;
        else switch(s) {
            case NAMES:
                m_filenames << it->value;
                if(!definedNames)
                    s=PATHS;
                break;
            case PATHS:
                m_path << it->value;
                break;
            case PATH_SUFFIXES:
                m_pathSuffixes << it->value;
                break;
        }
    }
    return !m_filenames.isEmpty();
}

FindPackageAst::FindPackageAst()
{
}

FindPackageAst::~FindPackageAst()
{
}

void FindPackageAst::writeBack( QString& ) const
{
}

bool FindPackageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    m_isQuiet=false;
    m_noModule=false;
    m_isRequired=false;
    if ( func.name.toLower() != "find_package" )
        return false;

    if ( func.arguments.isEmpty() )
        return false;

    m_name = func.arguments[0].value;

    foreach( CMakeFunctionArgument arg, func.arguments ) {
        if(arg.value[0].isNumber()) {
            bool correctmin, correctmaj;
            QStringList version = func.arguments[1].value.split('.');
            m_minorVersion = version[0].toInt(&correctmin);
            m_majorVersion = version[1].toInt(&correctmaj);
            if(!correctmin || !correctmaj)
                return false;
        } else if(arg.value=="QUIET")
            m_isQuiet=true;
        else if(arg.value=="NO_MODULE")
            m_noModule=true;
        else if(arg.value=="REQUIRED")
            m_isRequired=true;
    }
    return true;
}

FindPathAst::FindPathAst()
    : m_noDefaultPath(false)
    , m_noCmakeEnvironmentPath(false)
    , m_noCmakePath(false)
    , m_noSystemEnvironmentPath(false)
    , m_noCmakeSystemPath(false)
{
}

FindPathAst::~FindPathAst()
{
}

void FindPathAst::writeBack( QString& ) const
{
}

bool FindPathAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_path" || func.arguments.count()<3)
        return false;
    
    //FIXME: FIND_PATH(KDE4_DATA_DIR cmake/modules/FindKDE4Internal.cmake ${_data_DIR})
    bool definedNames=false;
    m_variableName = func.arguments[0].value;
    Stage s = NAMES;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+1, itEnd=func.arguments.end();
    if(it->value=="NAMES") {
        ++it;
        definedNames = true;
    } else {
        m_filenames=QStringList(it->value);
        it++;
        s=PATHS;
        definedNames = false;
    }

    for(; it!=itEnd; ++it) {
        if(it->value=="NO_DEFAULT_PATH")
            m_noDefaultPath = true;
        else if(it->value=="NO_CMAKE_ENVIRONMENT_PATH")
            m_noCmakeEnvironmentPath = true;
        else if(it->value=="NO_CMAKE_PATH")
            m_noSystemEnvironmentPath = true;
        else if(it->value=="DOC") {
            ++it;
            if(it==itEnd)
                return false;
            m_documentation = it->value;
        } else if(it->value=="PATHS")
            s=PATHS;
        else if(it->value=="PATH_SUFFIXES")
            s=PATH_SUFFIXES;
        else switch(s) {
            case NAMES:
                m_filenames << it->value;
                if(!definedNames)
                    s=PATHS;
                break;
            case PATHS:
                m_path << it->value;
                break;
            case PATH_SUFFIXES:
                m_pathSuffixes << it->value;
                break;
        }
    }
    return !m_filenames.isEmpty() && !m_path.isEmpty();
}

FindProgramAst::FindProgramAst()
{
        m_noDefaultPath = m_noCmakeEnvironmentPath = m_noCmakePath =
        m_noSystemEnvironmentPath = m_noSystemEnvironmentPath = false;
}

FindProgramAst::~FindProgramAst()
{
}

void FindProgramAst::writeBack( QString& ) const
{
}

bool FindProgramAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_program" || func.arguments.count()<2)
        return false;
    
    m_variableName = func.arguments[0].value;
    Stage s = NAMES;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+1, itEnd=func.arguments.end();
    if(it->value=="NAMES")
        ++it;
    else {
        m_filenames=QStringList(it->value);
        s=PATHS;
    }

    for(; it!=itEnd; ++it) {
        if(it->value=="NO_DEFAULT_PATH")
            m_noDefaultPath = true;
        else if(it->value=="NO_CMAKE_ENVIRONMENT_PATH")
            m_noCmakeEnvironmentPath = true;
        else if(it->value=="NO_CMAKE_PATH")
            m_noCmakePath=true;
        else if(it->value=="NO_SYSTEM_ENVIRONMENT_PATH")
            m_noSystemEnvironmentPath = true;
        else if(it->value=="NO_SYSTEM_ENVIRONMENT_PATH")
            m_noSystemEnvironmentPath = true;
        else if(it->value=="DOC") {
            ++it;
            if(it==itEnd)
                return false;
            m_documentation = it->value;
        } else if(it->value=="PATHS")
            s=PATHS;
        else if(it->value=="PATH_SUFFIXES")
            s=PATH_SUFFIXES;
        else switch(s) {
            case NAMES:
                m_filenames << it->value;
                break;
            case PATHS:
                m_path << it->value;
                break;
            case PATH_SUFFIXES:
                m_pathSuffixes << it->value;
                break;
        }
    }
    return !m_filenames.isEmpty();
}

FltkWrapUiAst::FltkWrapUiAst()
{
}

FltkWrapUiAst::~FltkWrapUiAst()
{
}

void FltkWrapUiAst::writeBack( QString& ) const
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

void ForeachAst::writeBack( QString& ) const
{
}

bool ForeachAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="foreach" || func.arguments.count()<2)
        return false;
    m_loopVar=func.arguments.first().value;
    if(func.arguments[1].value=="RANGE") {
        bool correctStart, correctStop, correctRange;
        m_range=true;
        if(func.arguments.count()<4)
            return false;
        m_ranges.step = 1;
        m_ranges.start = func.arguments[2].value.toInt(&correctStart);
        m_ranges.stop = func.arguments[3].value.toInt(&correctStop);
        if(func.arguments.count()==5)
            m_ranges.step = func.arguments[4].value.toInt(&correctRange);
        if(!correctStart || !correctStop || !correctRange)
            return false;
    } else {
        m_range=false;
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1, itEnd=func.arguments.constEnd();
        for(; it!=itEnd; ++it)
        {
            m_arguments += it->value;
        }
    }
    return true;
}

GetCMakePropertyAst::GetCMakePropertyAst()
{
}

GetCMakePropertyAst::~GetCMakePropertyAst()
{
}

void GetCMakePropertyAst::writeBack( QString& ) const
{
}

bool GetCMakePropertyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_cmake_property" || func.arguments.count()!=2)
        return false;
    m_variableName = func.arguments[0].value;
    
    QString type=func.arguments[1].value.toUpper();
    if(type=="VARIABLES")
        m_type=VARIABLES;
    else if(type=="CACHE_VARIABLES")
        m_type=CACHE_VARIABLES;
    else if(type=="COMMANDS")
        m_type=COMMANDS;
    else if(type=="MACROS")
        m_type=MACROS;
    else
        return false;
    return true;
}

GetDirPropertyAst::GetDirPropertyAst()
{
}

GetDirPropertyAst::~GetDirPropertyAst()
{
}

void GetDirPropertyAst::writeBack( QString& ) const
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

void GetFilenameComponentAst::writeBack( QString& ) const
{
}

bool GetFilenameComponentAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_filename_component" || func.arguments.count()<3)
        return false;
    m_variableName = func.arguments[0].value;
    m_fileName = func.arguments[1].value;
    QString t = func.arguments[2].value;
    
    if(t=="PATH") m_type=PATH;
    else if(t=="ABSOLUTE") m_type=ABSOLUTE;
    else if(t=="NAME") m_type=NAME;
    else if(t=="EXT") m_type=EXT;
    else if(t=="NAME_WE") m_type=NAME_WE;
    else if(t=="PROGRAM") m_type=PROGRAM;
    else
        return false;
    
    if(m_type==PROGRAM) {
        //TODO: Did not understand this option
    }
    
    m_cache = func.arguments.last().value=="CACHE";
    return true;
}

GetSourceFilePropAst::GetSourceFilePropAst()
{
}

GetSourceFilePropAst::~GetSourceFilePropAst()
{
}

void GetSourceFilePropAst::writeBack( QString& ) const
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

void GetTargetPropAst::writeBack( QString& ) const
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

void GetTestPropAst::writeBack( QString& ) const
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

void IfAst::writeBack( QString& s) const
{
    kDebug(9032)<< "if:" << s;
}

bool IfAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="if" && func.name.toLower()!="elseif" && func.name.toLower()!="else")
        return false;
    if(func.name.toLower()=="else" && !func.arguments.isEmpty())
        return false;

    m_kind = func.name;
    m_condition = QStringList();
    foreach(CMakeFunctionArgument fa, func.arguments)
    {
        m_condition += fa.value;
    }
    return true;
}

IncludeAst::IncludeAst()
{
    m_optional = false;
}

IncludeAst::~IncludeAst()
{
}

void IncludeAst::writeBack( QString& ) const
{
}

bool IncludeAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "include" )
        return false;
    if ( func.arguments.size() < 1 || func.arguments.size() > 2 )
        return false;

    m_includeFile = func.arguments[0].value;
    if ( func.arguments.size() == 2 && func.arguments[1].value == "OPTIONAL" )
        m_optional = true;

    return true;
}

 IncludeDirectoriesAst::IncludeDirectoriesAst()
{
}

IncludeDirectoriesAst::~IncludeDirectoriesAst()
{
}

void IncludeDirectoriesAst::writeBack( QString& ) const
{
}

bool IncludeDirectoriesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "include_directories" || func.arguments.isEmpty() )
        return false;
    
    int i=0;
    m_includeType = DEFAULT;
    m_isSystem = false;
    
    if(func.arguments[i].value=="AFTER") {
        if(m_includeType!=DEFAULT)
            return false;
        m_includeType = AFTER;
        i++;
    }
    
    if(func.arguments[i].value=="BEFORE") {
        if(m_includeType!=DEFAULT)
            return false;
        m_includeType = BEFORE;
        i++;
    }

    if(func.arguments[i].value=="SYSTEM") {
        m_isSystem = true;
        i++;
    }
    
    if ( func.arguments.size() <= i )
        return false;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd;
    it=func.arguments.begin() + i;
    itEnd = func.arguments.end();

    for ( ; it != itEnd; ++it )
        m_includedDirectories.append(it->value);

    return true;
}

IncludeExternalMsProjectAst::IncludeExternalMsProjectAst()
{
}

IncludeExternalMsProjectAst::~IncludeExternalMsProjectAst()
{
}

void IncludeExternalMsProjectAst::writeBack( QString& ) const
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

void IncludeRegularExpressionAst::writeBack( QString& ) const
{
}

bool IncludeRegularExpressionAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "include_regular_expression" || func.arguments.isEmpty() || func.arguments.count()>2 )
        return false;
    m_match=func.arguments[0].value;
    if(func.arguments.count()==2)
        m_complain=func.arguments[1].value;
    return true;
}

InstallAst::InstallAst()
{
}

InstallAst::~InstallAst()
{
}

void InstallAst::writeBack( QString& ) const
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

void InstallFilesAst::writeBack( QString& ) const
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

void InstallProgramsAst::writeBack( QString& ) const
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

void InstallTargetsAst::writeBack( QString& ) const
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

void LinkDirectoriesAst::writeBack( QString& ) const
{
}

bool LinkDirectoriesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "include_regular_expression" || func.arguments.isEmpty() )
        return false;
    
    foreach(CMakeFunctionArgument arg, func.arguments)
        m_directories.append(arg.value);
    return true;
}

LinkLibrariesAst::LinkLibrariesAst()
{
}

LinkLibrariesAst::~LinkLibrariesAst()
{
}

void LinkLibrariesAst::writeBack( QString& ) const
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

void ListAst::writeBack( QString& ) const
{
}

bool ListAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "list" )
        return false;
    if ( func.arguments.size() < 2)
        return false;

    QString argName = func.arguments[0].value;
    if(argName=="LENGTH")
        m_type = LENGTH;
    else if(argName=="GET")
        m_type=GET;
    else if(argName=="APPEND")
        m_type = APPEND;
    else if(argName=="INSERT")
        m_type = INSERT;
    else if(argName=="REMOVE_ITEM")
        m_type = REMOVE_ITEM;
    else if(argName=="REMOVE_AT")
        m_type = REMOVE_AT;
    else if(argName=="SORT")
        m_type = SORT;
    else if(argName=="REVERSE")
        m_type = REVERSE;
    else
        return false;

    m_list = func.arguments[1].value;
    switch(m_type)
    {
        case LENGTH:
            if(func.arguments.count()!=3)
                return false;
            m_output = func.arguments[2].value;
            break;
        case GET: {
            if(func.arguments.count()<3)
                return false;
            
            bool correct;
            m_output = func.arguments.last().value;
            m_index.append(func.arguments[2].value.toInt(&correct));
            if(!correct)
                return false;
            int i=0;
            foreach(CMakeFunctionArgument arg, func.arguments)
            {
                if(i>2 && i<func.arguments.count())
                    m_elements.append(arg.value);
                i++;
            }
        } break;
        case APPEND: {
            if(func.arguments.count()<3)
                return false;
            int i=0;
            foreach(CMakeFunctionArgument arg, func.arguments)
            {
                if(i>1)
                    m_elements.append(arg.value);
                i++;
            }
        } break;
        case INSERT: {
            bool correct;
            if(func.arguments.count()<4)
                return false;
            int i=0;
            m_index.append(func.arguments[1].value.toInt(&correct));
            if(!correct)
                return false;
            foreach(CMakeFunctionArgument arg, func.arguments)
            {
                if(i>2)
                    m_elements.append(arg.value);
                i++;
            }
        } break;
        case REMOVE_ITEM: {
            if(func.arguments.count()<4)
                return false;
            int i=0;
            foreach(CMakeFunctionArgument arg, func.arguments)
            {
                if(i>2)
                    m_elements.append(arg.value);
                i++;
            }
        } break;
        case REMOVE_AT: {
            if(func.arguments.count()<3)
                return false;
            int i=0;
            foreach(CMakeFunctionArgument arg, func.arguments)
            {
                if(i>1)
                {
                    bool correct;
                    m_index.append(arg.value.toInt(&correct));
                    kDebug(9042) << "???" << arg.value;
                    Q_ASSERT(correct);
                }
                i++;
            }
        } break;
        case SORT:
        case REVERSE:
            if(func.arguments.count()>2)
                return false;
            break;
    }
    return true;
}

LoadCacheAst::LoadCacheAst()
{
}

LoadCacheAst::~LoadCacheAst()
{
}

void LoadCacheAst::writeBack( QString& ) const
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

void LoadCommandAst::writeBack( QString& ) const
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

void MacroAst::writeBack( QString& ) const
{
}

bool MacroAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "macro" || func.arguments.isEmpty())
        return false;

    m_macroName = func.arguments[0].value.toLower();
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    for ( it = func.arguments.begin() + 1; it != itEnd; ++it )
        m_knownArgs.append( it->value );

    return true;
}

MakeDirectoryAst::MakeDirectoryAst()
{
}

MakeDirectoryAst::~MakeDirectoryAst()
{
}

void MakeDirectoryAst::writeBack( QString& ) const
{
}

bool MakeDirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "make_directory" || func.arguments.isEmpty() || func.arguments.size()>1)
        return false;
    m_directory=func.arguments.first().value;
    return true;
}

MarkAsAdvancedAst::MarkAsAdvancedAst()
{
}

MarkAsAdvancedAst::~MarkAsAdvancedAst()
{
}

void MarkAsAdvancedAst::writeBack( QString& ) const
{
}


bool MarkAsAdvancedAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "mark_as_advanced" || func.arguments.isEmpty() )
        return false;

    m_isClear = func.arguments[0].value == "CLEAR";
    m_isForce = func.arguments[0].value == "FORCE";

    if ( (m_isClear || m_isForce) && func.arguments.size() < 2 )
        return false;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd;
    it=func.arguments.begin();
    itEnd = func.arguments.end();

    if(m_isClear || m_isForce)
	it++;

    for ( ; it != itEnd; ++it )
    {
        m_advancedVars.append(it->value);
    }

    return true;
}

MathAst::MathAst()
{
}

MathAst::~MathAst()
{
}

void MathAst::writeBack( QString& ) const
{
}

bool MathAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="math" || func.arguments.count()!=3 || func.arguments.first().value!="EXPR")
        return false;
    
    m_outputVariable = func.arguments[1].value;
    m_expression = func.arguments.last().value;
    return true;
}

MessageAst::MessageAst()
{
}

MessageAst::~MessageAst()
{
}

void MessageAst::writeBack( QString& ) const
{
}

bool MessageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="message" || func.arguments.isEmpty())
        return false;
    if(func.arguments.count()>1) {
        QString type=func.arguments.first().value;
        if(type=="SEND_ERROR")
            m_type=SEND_ERROR;
        else if(type=="STATUS")
            m_type=STATUS;
        else if(type=="FATAL_ERROR")
            m_type=FATAL_ERROR;
    }
    
    m_message.append(func.arguments.last().value); //Maybe should do a foreach
    return true;
}

OptionAst::OptionAst()
{
}

OptionAst::~OptionAst()
{
}

void OptionAst::writeBack( QString& ) const
{
}

bool OptionAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="option" || (func.arguments.count()<2 && func.arguments.count()>3))
        return false;
    m_variableName = func.arguments[0].value;
    m_description = func.arguments[1].value;
    m_defaultValue = "OFF";
    if(func.arguments.count() ==3)
        m_defaultValue = func.arguments[2].value;
    return true;
}

OutputRequiredFilesAst::OutputRequiredFilesAst()
{
}

OutputRequiredFilesAst::~OutputRequiredFilesAst()
{
}

void OutputRequiredFilesAst::writeBack( QString& ) const
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

void ProjectAst::writeBack( QString& ) const
{
}

bool ProjectAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "project" )
        return false;

    if ( func.arguments.size() < 1 )
        return false;

    m_projectName = func.arguments[0].value;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    it = func.arguments.begin();
    ++it;
    for ( ; it != itEnd; ++it )
    {
        CMakeFunctionArgument arg = ( *it );
        if ( arg.value == "CXX" )
            m_useCpp = true;
        else if ( arg.value == "C" )
            m_useC = true;
        else if ( arg.value == "Java" )
            m_useJava = true;
        else
            return false;
    }

    return true;
}

QtWrapCppAst::QtWrapCppAst()
{
}

QtWrapCppAst::~QtWrapCppAst()
{
}

void QtWrapCppAst::writeBack( QString& ) const
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

void QtWrapUiAst::writeBack( QString& ) const
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

void RemoveAst::writeBack( QString& ) const
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

void RemoveDefinitionsAst::writeBack( QString& ) const
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

void SeparateArgumentsAst::writeBack( QString& ) const
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

void SetAst::writeBack( QString& ) const
{
}

bool SetAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "set" )
        return false;

    if ( func.arguments.size() < 1 )
        return false;

    m_variableName = func.arguments[0].value;

    int argSize = func.arguments.size();

    //look for the FORCE argument. Thanks to the CMake folks for letting
    //me read their code
    m_forceStoring = ( argSize > 4 && func.arguments.last().value == "FORCE" );

    m_storeInCache = ( argSize > 3 && func.arguments[argSize - 3 - ( m_forceStoring ? 1 : 0 )].value == "CACHE" );

    int numCacheArgs = ( m_storeInCache ? 3 : 0 );
    int numForceArgs = ( m_forceStoring ? 1 : 0 );
    if ( argSize > 1 + numCacheArgs + numForceArgs )
    {
        QList<CMakeFunctionArgument> args = func.arguments;
        QList<CMakeFunctionArgument>::const_iterator it, itEnd;
        it = args.begin() + 1;
        itEnd = args.end() - numCacheArgs - numForceArgs;
        for ( ; it != itEnd; ++it )
            m_values.append( it->value );
    }

    //catch some simple things. if CACHE is the last or next to last arg or if
    //FORCE was used without CACHE, then there's a problem.
    if ( func.arguments.last().value == "CACHE" ||
         ( argSize > 1 && func.arguments[argSize - 2].value == "CACHE" ) ||
         m_forceStoring && !m_storeInCache )
    {
        return false;
    }

    return true;
}

SetDirectoryPropsAst::SetDirectoryPropsAst()
{
}

SetDirectoryPropsAst::~SetDirectoryPropsAst()
{
}

void SetDirectoryPropsAst::writeBack( QString& ) const
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

void SetSourceFilesPropsAst::writeBack( QString& ) const
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

void SetTargetPropsAst::writeBack( QString& ) const
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

void SetTestsPropsAst::writeBack( QString& ) const
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

void SiteNameAst::writeBack( QString& ) const
{
}

bool SiteNameAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="site_name" || func.arguments.count()!=1)
        return false;
    m_variableName = func.arguments.first().value;
    return true;
}

SourceGroupAst::SourceGroupAst()
{
}

SourceGroupAst::~SourceGroupAst()
{
}

void SourceGroupAst::writeBack( QString& ) const
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

void StringAst::writeBack( QString& ) const
{
}

bool StringAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="string" || func.arguments.count()<3)
        return false;
    QString stringType=func.arguments[0].value.toUpper();
    if(stringType=="REGEX")
    {
        if(func.arguments.count()<5)
            return false;
        m_type=REGEX;
        QString regexType = func.arguments[1].value.toUpper();

        int outpos=3;
        if(regexType=="MATCH") m_cmdType=MATCH;
        else if(regexType=="MATCHALL") m_cmdType=MATCHALL;
        else if(regexType=="REPLACE") {
            m_cmdType=REGEX_REPLACE; 
            m_replace=func.arguments[3].value;
            outpos=4;
        }

        m_regex = func.arguments[2].value;
        m_outputVariable = func.arguments[outpos].value;
        
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+outpos+1;
        QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
        for(; it!=itEnd; ++it)
        {
            m_input += it->value;
        }
    }
    else if(stringType=="REPLACE")
    {
        m_type=REPLACE;
        m_regex = func.arguments[1].value;
        m_replace=func.arguments[2].value;
        m_outputVariable = func.arguments[3].value;
        
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+4;
        QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
        for(; it!=itEnd; ++it)
        {
            m_input += it->value;
        }
    }
    else if(stringType=="COMPARE")
    {
        if(func.arguments.count()!=5)
            return false;
        m_type=COMPARE;
        QString argumentType=func.arguments[1].value.toUpper();
        if(argumentType=="EQUAL") m_cmdType=EQUAL;
        else if(argumentType=="NOTEQUAL") m_cmdType=NOTEQUAL;
        else if(argumentType=="LESS") m_cmdType=LESS;
        else if(argumentType=="GREATER") m_cmdType=GREATER;
        
        m_input.append(func.arguments[2].value);
        m_input.append(func.arguments[3].value);
        m_outputVariable = func.arguments[4].value;
    }
    else if(stringType=="ASCII")
    {
        m_type=ASCII;
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+1;
        QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
        for(; it!=itEnd; ++it)
        {
            m_input += it->value;
        }
        m_input.pop_back(); //We remove the output var
        m_outputVariable = func.arguments.last().value;
    }
    else if(stringType=="CONFIGURE")
    {
        m_type=CONFIGURE;
        m_input += func.arguments[1].value;
        m_outputVariable=func.arguments[2].value;
        
        int i=3;
        m_only = func.arguments[i].value.toUpper()=="@ONLY";
        if(m_only) i++;
        m_escapeQuotes = func.arguments[i].value.toUpper()=="ESCAPE_QUOTES";
    }
    else if(stringType=="TOUPPER")
    {
        m_type=TOUPPER;
        m_input.append(func.arguments[1].value);
        m_outputVariable = func.arguments[2].value;
    }
    else if(stringType=="TOLOWER")
    {
        m_type=TOLOWER;
        m_input.append(func.arguments[1].value);
        m_outputVariable = func.arguments[2].value;
    }
    else if(stringType=="LENGTH")
    {
        m_type=LENGTH;
        m_input.append(func.arguments[1].value);
        m_outputVariable = func.arguments[2].value;
    }
    else if(stringType=="SUBSTRING")
    {
        if(func.arguments.count()<5)
            return false;
        bool correctBegin, correctLength;
        m_type=SUBSTRING;
        m_input.append(func.arguments[1].value);
        m_begin = func.arguments[2].value.toInt(&correctBegin);
        m_length = func.arguments[3].value.toInt(&correctLength);
        m_outputVariable = func.arguments[4].value;
        if(!correctBegin || !correctLength)
            return false;
    }
    return true;
}

SubdirDependsAst::SubdirDependsAst()
{
}

SubdirDependsAst::~SubdirDependsAst()
{
}

void SubdirDependsAst::writeBack( QString& ) const
{
}

bool SubdirDependsAst::parseFunctionInfo( const CMakeFunctionDesc& func)
{
    //NOTE: Should not do anything :)
    return true;
}

SubdirsAst::SubdirsAst() : m_preorder(false)
{
}

SubdirsAst::~SubdirsAst()
{
}

void SubdirsAst::writeBack( QString& ) const
{
}

bool SubdirsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "subdirs" )
        return false;

    if ( func.arguments.isEmpty() )
        return false;
    
    bool excludeFromAll=false;
    foreach(CMakeFunctionArgument arg, func.arguments)
    {
        if(arg.value.toLower()=="exclude_from_all")
            excludeFromAll=true;
        else if(arg.value.toLower()=="preorder")
            m_preorder=true;
        else {
            if(excludeFromAll)
                m_exluceFromAll.append(arg.value);
            else
                m_directories.append(arg.value);
        }
    }
    return true;
}

TargetLinkLibrariesAst::TargetLinkLibrariesAst()
{
}

TargetLinkLibrariesAst::~TargetLinkLibrariesAst()
{
}

void TargetLinkLibrariesAst::writeBack( QString& ) const
{
}

bool TargetLinkLibrariesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "target_link_libraries" )
        return false;

    //we don't do variable expansion when parsing like CMake does, so we
    //need to have at least two arguments for target_link_libraries
    if ( func.arguments.size() < 2 )
        return false;


    QList<CMakeFunctionArgument>::const_iterator it, itEnd;
    it = func.arguments.begin() + 1;
    itEnd = func.arguments.end();

    for ( ; it != itEnd; ++it )
    {
        CMakeFunctionArgument arg = ( *it );
        if ( arg.value == "debug" )
        {
            ++it;
            if ( it == itEnd )
                return false;
            else
                m_debugLibs.append( it->value );
        }
        else if ( arg.value == "optimized" )
        {
            ++it;
            if ( it == itEnd )
                return false;
            else
                m_optimizedLibs.append( it->value );
        }
        else
            m_otherLibs.append( arg.value );
    }

    return true;
}

TryCompileAst::TryCompileAst()
{
}

TryCompileAst::~TryCompileAst()
{
}

void TryCompileAst::writeBack( QString& ) const
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

void TryRunAst::writeBack( QString& ) const
{
}

bool TryRunAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="try_run" || func.arguments.count()<4)
        return false;
    
    enum Actions { None, CMakeFlags, CompileDefs, OutputVariable, Args };
    Actions act;
    unsigned int i=0;
    foreach(CMakeFunctionArgument arg, func.arguments) {
        QString val=arg.value.toLower();
        if(i<4)
            act=None;

        if(i==0)
            m_runResultVar=arg.value;
        else if(i==1)
            m_compileResultVar=arg.value;
        else if(i==2)
            m_binDir=arg.value;
        else if(i==3)
            m_srcFile=arg.value;
        else if(val=="cmake_flags")
            act=CMakeFlags;
        else if(val=="compile_definitions")
            act=CompileDefs;
        else if(val=="output_variable")
            act=OutputVariable;
        else if(val=="args")
            act=Args;
        else switch(act) {
            case None:
                return false;
            case CMakeFlags:
                m_cmakeFlags.append(arg.value);
                break;
            case CompileDefs:
                m_compileDefs.append(arg.value);
            case OutputVariable:
                m_outputVar=arg.value;
                break;
            case Args:
                m_args.append(arg.value);
                break;
        }
        i++;
    }
    return true;
}

UseMangledMesaAst::UseMangledMesaAst()
{
}

UseMangledMesaAst::~UseMangledMesaAst()
{
}

void UseMangledMesaAst::writeBack( QString& ) const
{
}

bool UseMangledMesaAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="path_to_mesa" || func.arguments.count()!=2)
        return false;
    m_pathToMesa=func.arguments[0].value;
    m_outputDir=func.arguments[1].value;
    return true;
}

UtilitySourceAst::UtilitySourceAst()
{
}

UtilitySourceAst::~UtilitySourceAst()
{
}

void UtilitySourceAst::writeBack( QString& ) const
{
}

bool UtilitySourceAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="utility_source" || func.arguments.count()<3)
        return false;
    unsigned int i=0;
    foreach(CMakeFunctionArgument arg, func.arguments) {
        if(i==0)
            m_cacheEntry=arg.value;
        else if(i==1)
            m_executableName=arg.value;
        else if(i==2)
            m_pathToSource=arg.value;
        else
            m_fileList.append(arg.value);
    }
    return true;
}

VariableRequiresAst::VariableRequiresAst()
{
}

VariableRequiresAst::~VariableRequiresAst()
{
}

void VariableRequiresAst::writeBack( QString& ) const
{
}

bool VariableRequiresAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="variable_requires" || func.arguments.count()<2)
        return false;
    unsigned int i=0;
    foreach(CMakeFunctionArgument arg, func.arguments) {
        if(i==0)
            m_testVariable=arg.value;
        else if(i==1)
            m_resultVariable=arg.value;
        else
            m_requiredVariables.append(arg.value);
        i++;
    }
    return true;
}

VtkMakeInstantiatorAst::VtkMakeInstantiatorAst()
{
}

VtkMakeInstantiatorAst::~VtkMakeInstantiatorAst()
{
}

void VtkMakeInstantiatorAst::writeBack( QString& ) const
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

void VtkWrapJavaAst::writeBack( QString& ) const
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

void VtkWrapPythonAst::writeBack( QString& ) const
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

void VtkWrapTclAst::writeBack( QString& ) const
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

void WhileAst::writeBack( QString& ) const
{
}

bool WhileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="while" || func.arguments.isEmpty())
        return false;
    foreach(CMakeFunctionArgument arg, func.arguments) {
        m_condition.append(arg.value);
    }
    return true;
}

WriteFileAst::WriteFileAst()
{
}

WriteFileAst::~WriteFileAst()
{
}

void WriteFileAst::writeBack( QString& ) const
{
}

bool WriteFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

CustomInvokationAst::CustomInvokationAst()
{
}

CustomInvokationAst::~CustomInvokationAst()
{
}

void CustomInvokationAst::writeBack( QString& ) const
{
}

bool CustomInvokationAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    m_function = func;
    m_arguments = func.arguments;
    return true;
}


