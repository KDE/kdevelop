/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2009 Aleix Pol <aleixpol@kde.org>
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

#include <QString>

#include <KDebug>

#include "astfactory.h"
#include "cmakelistsparser.h"
#include "cmakeparserutils.h"

#define CMAKE_REGISTER_AST( klassName, astId ) namespace {                 \
        CMakeAst* Create##klassName() { return new klassName; }            \
        bool b_##astId = AstFactory::self()->registerAst( QLatin1String( #astId ), Create##klassName ); }
        
QDebug operator<<(QDebug dbg, const CMakeFunctionDesc &func)
{
    dbg.nospace() << func.name << "(" << func.arguments << ")";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const CMakeFunctionArgument &arg)
{
    dbg.nospace() << arg.value;

    return dbg.space();
}

CMAKE_REGISTER_AST( AddDefinitionsAst, add_definitions )
CMAKE_REGISTER_AST( AddDependenciesAst, add_dependencies )
CMAKE_REGISTER_AST( AddExecutableAst, add_executable )
CMAKE_REGISTER_AST( AddLibraryAst, add_library )
CMAKE_REGISTER_AST( AddSubdirectoryAst, add_subdirectory )
CMAKE_REGISTER_AST( AddTestAst,  add_test )
CMAKE_REGISTER_AST( AuxSourceDirectoryAst, aux_source_directory )
CMAKE_REGISTER_AST( BreakAst, break)
CMAKE_REGISTER_AST( BuildCommandAst, build_command )
CMAKE_REGISTER_AST( BuildNameAst, build_name )
CMAKE_REGISTER_AST( CMakeMinimumRequiredAst, cmake_minimum_required )
CMAKE_REGISTER_AST( CMakePolicyAst, cmake_policy)
CMAKE_REGISTER_AST( ConfigureFileAst, configure_file )
CMAKE_REGISTER_AST( CreateTestSourcelistAst, create_test_sourcelist )
CMAKE_REGISTER_AST( CustomCommandAst, add_custom_command )
CMAKE_REGISTER_AST( CustomTargetAst, add_custom_target )
CMAKE_REGISTER_AST( EnableLanguageAst, enable_language )
CMAKE_REGISTER_AST( EnableTestingAst, enable_testing )
CMAKE_REGISTER_AST( ExecProgramAst, exec_program )
CMAKE_REGISTER_AST( ExecuteProcessAst, execute_process )
CMAKE_REGISTER_AST( ExportAst, export)
CMAKE_REGISTER_AST( ExportLibraryDepsAst, export_library_dependencies)
CMAKE_REGISTER_AST( FileAst, file )
CMAKE_REGISTER_AST( FindFileAst, find_file )
CMAKE_REGISTER_AST( FindLibraryAst, find_library )
CMAKE_REGISTER_AST( FindPackageAst, find_package )
CMAKE_REGISTER_AST( FindPathAst, find_path )
CMAKE_REGISTER_AST( FindProgramAst, find_program )
CMAKE_REGISTER_AST( ForeachAst, foreach )
CMAKE_REGISTER_AST( FunctionAst, function )
CMAKE_REGISTER_AST( GetCMakePropertyAst, get_cmake_property )
CMAKE_REGISTER_AST( GetDirPropertyAst, get_directory_property )
CMAKE_REGISTER_AST( GetPropertyAst, get_property )
CMAKE_REGISTER_AST( GetSourceFilePropAst, get_source_file_property )
CMAKE_REGISTER_AST( GetTargetPropAst, get_target_property )
CMAKE_REGISTER_AST( GetTestPropAst, get_test_property )
CMAKE_REGISTER_AST( GetFilenameComponentAst, get_filename_component )
CMAKE_REGISTER_AST( IfAst, if )
CMAKE_REGISTER_AST( IncludeAst, include )
CMAKE_REGISTER_AST( IncludeDirectoriesAst, include_directories )
CMAKE_REGISTER_AST( IncludeRegularExpressionAst, include_regular_expression )
CMAKE_REGISTER_AST( InstallFilesAst, install_files )
CMAKE_REGISTER_AST( InstallProgramsAst, install_programs )
CMAKE_REGISTER_AST( InstallTargetsAst, install_targets )
CMAKE_REGISTER_AST( LinkDirectoriesAst, link_directories )
CMAKE_REGISTER_AST( LinkLibrariesAst, link_libraries )
CMAKE_REGISTER_AST( ListAst, list )
CMAKE_REGISTER_AST( LoadCacheAst, load_cache )
CMAKE_REGISTER_AST( LoadCommandAst, load_command )
CMAKE_REGISTER_AST( MacroAst, macro )
CMAKE_REGISTER_AST( MarkAsAdvancedAst, mark_as_advanced )
CMAKE_REGISTER_AST( MakeDirectoryAst, make_directory )
CMAKE_REGISTER_AST( MathAst, math )
CMAKE_REGISTER_AST( MessageAst, message )
CMAKE_REGISTER_AST( OptionAst, option )
CMAKE_REGISTER_AST( OutputRequiredFilesAst, output_required_files )
CMAKE_REGISTER_AST( ProjectAst, project )
CMAKE_REGISTER_AST( RemoveAst, remove )
CMAKE_REGISTER_AST( ReturnAst, return )
CMAKE_REGISTER_AST( RemoveDefinitionsAst, remove_definitions )
CMAKE_REGISTER_AST( SetAst, set )
CMAKE_REGISTER_AST( SetDirectoryPropsAst, set_directory_properties )
CMAKE_REGISTER_AST( SetSourceFilesPropsAst, set_source_files_properties )
CMAKE_REGISTER_AST( SetTargetPropsAst, set_target_properties )
CMAKE_REGISTER_AST( SetTestsPropsAst, set_tests_properties )
CMAKE_REGISTER_AST( SetPropertyAst, set_property )
CMAKE_REGISTER_AST( SourceGroupAst, source_group )
CMAKE_REGISTER_AST( SeparateArgumentsAst, separate_arguments )
CMAKE_REGISTER_AST( SiteNameAst, site_name )
CMAKE_REGISTER_AST( StringAst, string )
CMAKE_REGISTER_AST( SubdirsAst, subdirs )
CMAKE_REGISTER_AST( SubdirDependsAst, subdir_depends )
CMAKE_REGISTER_AST( TargetLinkLibrariesAst, target_link_libraries)
CMAKE_REGISTER_AST( TryCompileAst, try_compile )
CMAKE_REGISTER_AST( TryRunAst, try_run )
CMAKE_REGISTER_AST( UseMangledMesaAst, use_mangled_mesa )
CMAKE_REGISTER_AST( UtilitySourceAst, utility_source )
CMAKE_REGISTER_AST( UnsetAst, unset )
CMAKE_REGISTER_AST( VariableRequiresAst, variable_requires )
CMAKE_REGISTER_AST( WhileAst, while)
CMAKE_REGISTER_AST( WriteFileAst, write_file)

#undef CMAKE_REGISTER_AST

enum Stage {NAMES, PATHS, PATH_SUFFIXES, HINTS};

CustomCommandAst::CustomCommandAst()
{
    m_isForTarget = false; //only used here? :S
    m_buildStage = PostBuild;
    m_isVerbatim = false;
    m_append = false;
}

CustomCommandAst::~CustomCommandAst()
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
                m_workingDirectory = copy;
                break;
            case doing_source:
                m_source = copy;
                break;
            case doing_main_dependency:
                m_mainDependency = copy;
                break;
            case doing_command:
                m_commands.append( copy );
                break;
            case doing_target:
                m_targetName = copy;
                break;
            case doing_depends:
                m_otherDependencies.append( copy );
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
    if ( m_outputs.isEmpty() && m_targetName.isEmpty() )
        return false;

    if ( m_source.isEmpty() && !m_targetName.isEmpty() && !m_outputs.isEmpty())
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

bool CustomTargetAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != QLatin1String( "add_custom_target" ) )
        return false;

    //make sure we have at least one argument
    if ( func.arguments.size() < 1 )
        return false;

    //check and make sure the target name isn't something silly
    CMakeFunctionArgument arg = func.arguments.front();
    if ( arg.value=="ALL" )
        return false;
    else
        m_target = arg.value;

    //check if we're part of the special "all" target
    if(func.arguments.count()>1)
    {
        CMakeFunctionArgument arg2 = func.arguments[1];
        m_buildAlways = arg2.value == "ALL";
    }

    //what are we doing?
    enum Action {
        ParsingCommand,
        ParsingDep,
        ParsingWorkingDir,
        ParsingComment,
        ParsingVerbatim,
        ParsingSources
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
        else if ( arg == "SOURCES" )
            act = ParsingSources;
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
            case ParsingSources:
                m_sourceLists.append( arg );
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

bool AddDefinitionsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "add_definitions" )
        return false;

    if ( func.arguments.isEmpty() )
        return false;

    foreach( const CMakeFunctionArgument& arg, func.arguments )
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

bool AddDependenciesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "add_dependencies" )
        return false;

    if ( func.arguments.size() < 2 )
        return false;

    QList<CMakeFunctionArgument> args = func.arguments;
    m_target = args.front().value;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = args.constEnd();
    it = args.constBegin() + 1; //skip the first argument since it's the target
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
    m_isImported = false;
}

AddExecutableAst::~AddExecutableAst()
{
}

bool AddExecutableAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "add_executable" )
        return false;

    if ( func.arguments.size() < 2 )
        return false;
    
    QList<CMakeFunctionArgument> args = func.arguments;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = args.constEnd();
    it = args.constBegin();
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
        else if ( it->value == "IMPORTED")
            m_isImported = true;
        else
            m_sourceLists.append( it->value );
    }

    if(!m_isImported && m_sourceLists.isEmpty())
        return false;

    return true;

}

QMap<QString, AddLibraryAst::LibraryType> AddLibraryAst::s_typeForName;
AddLibraryAst::AddLibraryAst()
{
    if(s_typeForName.isEmpty()) {
        s_typeForName.insert("STATIC", Static);
        s_typeForName.insert("SHARED", Shared);
        s_typeForName.insert("MODULE", Module);
        s_typeForName.insert("UNKNOWN", Unknown);
    }
    
    m_type = Static;
    m_isImported = false;
    m_excludeFromAll = false;
}

AddLibraryAst::~AddLibraryAst()
{
}

bool AddLibraryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "add_library" || func.arguments.isEmpty() )
        return false;

    bool libTypeSet = false;
    QList<CMakeFunctionArgument> args = func.arguments;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = args.constEnd();
    it = args.constBegin();
    m_libraryName = it->value;
    ++it;
    for(; it != itEnd;)
    {
        if ( !libTypeSet && s_typeForName.contains(it->value) )
        {
            m_type = s_typeForName.value(it->value);
            libTypeSet = true;
            ++it;
        }
        else if ( it->value == "IMPORTED")
        {
            m_isImported = true;
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

    if ( !m_isImported )
    {
        while ( it != itEnd )
        {
                m_sourceLists.append( it->value );
                ++it;
        }

        if ( m_sourceLists.isEmpty() )
            return false;
    }
    return true;

}
AddSubdirectoryAst::AddSubdirectoryAst()
{
}

AddSubdirectoryAst::~AddSubdirectoryAst()
{
}

bool AddSubdirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "add_subdirectory" )
        return false;
    if ( func.arguments.size() < 1 || func.arguments[0].value.isEmpty())
        return false;

    m_sourceDir = func.arguments[0].value;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.constEnd();
    it = ++func.arguments.constBegin();
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

bool AddTestAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "add_test" )
        return false;
    if ( func.arguments.size() < 2 )
        return false;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.constEnd();
    it = func.arguments.constBegin();
    enum {Name, Command, Arg, Unsupported} state = Unsupported;
    if (it->value != "NAME")
    {
        m_testName = (it++)->value;
        m_exeName = (it++)->value;
        state = Arg;
    }
    for ( ; it != itEnd; ++it )
    {
        if ( it->value == "NAME" ) state = Name;
        else if ( it->value == "COMMAND" ) state = Command;
        else if ( it->value == "CONFIGURATIONS" || it->value == "WORKING_DIRECTORY" ) state = Unsupported;
        else switch (state) {
            case Name:
                m_testName = it->value;
                break;
            case Command:
                m_exeName = it->value;
                state = Arg;
                break;
            case Arg:
                m_testArgs << it->value;
                break;
            default:
                break;
        }
    }

    return !m_exeName.isEmpty();
}

AuxSourceDirectoryAst::AuxSourceDirectoryAst()
{
}

AuxSourceDirectoryAst::~AuxSourceDirectoryAst()
{
}

bool AuxSourceDirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "aux_source_directory" )
        return false;
    if ( func.arguments.size() != 2 )
        return false;

    m_dirName = func.arguments[0].value;
    m_variableName = func.arguments[1].value;

    addOutputArgument(func.arguments[1].value);
    return true;
}

BuildCommandAst::BuildCommandAst()
{
}

BuildCommandAst::~BuildCommandAst()
{
}

bool BuildCommandAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "build_command" )
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

bool BuildNameAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "build_name" )
        return false;
    if ( func.arguments.size() != 1 )
        return false;

    m_buildName = func.arguments[0].value;

    addOutputArgument(func.arguments[0].value);
    return true;
}

CMakeMinimumRequiredAst::CMakeMinimumRequiredAst()
    : m_wrongVersionIsFatal(false)
{
}

CMakeMinimumRequiredAst::~CMakeMinimumRequiredAst()
{
}

bool CMakeMinimumRequiredAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "cmake_minimum_required" )
        return false;
    if ( func.arguments.size() < 2 || func.arguments.count() > 3 || func.arguments.first().value != "VERSION")
        return false;

    bool correct = false;
    m_version = CMakeParserUtils::parseVersion(func.arguments[1].value, &correct);
    if (!correct)
        return false;

    if(func.arguments.count()==3)
    {
        if(func.arguments[2].value=="FATAL_ERROR")
            m_wrongVersionIsFatal = true;
        else
            return false;
    }
    return true;
}

ConfigureFileAst::ConfigureFileAst()
{
}

ConfigureFileAst::~ConfigureFileAst()
{
}


bool ConfigureFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "configure_file" )
        return false;

    if ( func.arguments.size() < 2 )
        return false;

    m_inputFile = func.arguments[0].value;
    m_outputFile = func.arguments[1].value;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.constEnd();
    it = func.arguments.constBegin() + 2;
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


bool CreateTestSourcelistAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "create_test_sourcelist" )
        return false;

    if ( func.arguments.count() < 3 )
        return false;

    addOutputArgument(func.arguments[0]);
    m_name=func.arguments[0].value;
    m_driverName=func.arguments[1].value;
    
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.constEnd();
    it = func.arguments.constBegin() + 2;
    enum State { Tests, ExtraInclude, Function};
    State s=Tests;

    for(; it!=itEnd; ++it)
    {
        if(it->value=="EXTRA_INCLUDE") s=ExtraInclude;
        else if(it->value=="FUNCTION") s=Function;
        else switch(s) {
            case Tests:
                m_tests.append(it->value);
                break;
            case ExtraInclude:
                m_extraIncludes.append(it->value);
                s=Tests;
                break;
            case Function:
                m_function.append(it->value);
                s=Tests;
                break;
        }
    }
    return !m_tests.isEmpty();
}

EnableLanguageAst::EnableLanguageAst()
{
}

EnableLanguageAst::~EnableLanguageAst()
{
}


bool EnableLanguageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "enable_language" )
        return false;

    if ( func.arguments.isEmpty() || func.arguments.count() != 1 )
        return false;

    if ( func.arguments[0].value.isEmpty() )
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


bool EnableTestingAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return func.name.toLower() == "enable_testing" && func.arguments.isEmpty();
}

ExecProgramAst::ExecProgramAst()
{
}

ExecProgramAst::~ExecProgramAst()
{
}


bool ExecProgramAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name != "exec_program" || func.arguments.count() == 0 )
        return false;

    m_executableName = func.arguments[0].value;
    bool args=false;

    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin(), itEnd=func.arguments.constEnd();
    // don't re-read the first element
    it++;
    for(; it!=itEnd; ++it) {
        if(it->value=="OUTPUT_VARIABLE") {
            ++it;
            if(it!=itEnd)
            {
                addOutputArgument(*it);
                m_outputVariable = it->value;
            }
            else
                return false;
        } else if(it->value=="RETURN_VALUE") {
            ++it;
            if(it!=itEnd)
            {
                addOutputArgument(*it);
                m_returnValue = it->value;
            }
            else
                return false;
        }
        else if(it->value=="ARGS")
        {
            args=true;
        }
        else if(args)
        {
            m_arguments += it->value;
        }
        else
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


bool ExecuteProcessAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name != "execute_process" || func.arguments.count()<2)
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
    foreach(const CMakeFunctionArgument &a, func.arguments) {
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
                addOutputArgument(a);
                m_resultVariable=a.value;
                break;
            case OutputVar:
                addOutputArgument(a);
                m_outputVariable=a.value;
                break;
            case ErrorVar:
                addOutputArgument(a);
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
    return !m_commands.isEmpty();
}

ExportLibraryDepsAst::ExportLibraryDepsAst()
    : m_append(false)
{
}

ExportLibraryDepsAst::~ExportLibraryDepsAst()
{
}


bool ExportLibraryDepsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name != "export_library_dependencies" || func.arguments.isEmpty())
        return false;
    m_file=func.arguments[0].value;
    if(func.arguments.count()>=2)
    {
        if(func.arguments[1].value=="APPEND")
        {
            m_append=true;
        }
        if(func.arguments.count()>(1+m_append))
            return false;
    }
    return true;
}

FileAst::FileAst()
    : m_isFollowingSymlinks(false), m_newlineConsume(false), m_noHexConversion(false)
{
}

FileAst::~FileAst()
{
}


bool FileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "file" || func.arguments.count()<2)
        return false;

    QString type = func.arguments.first().value;
    int min_args=-1;
    if(type=="WRITE") {
       m_type = Write;
       min_args=3;
    } else if(type=="APPEND") {
        m_type = Append;
        min_args=3;
    } else if(type=="READ") {
        m_type = Read;
        min_args=3;
    } else if(type=="GLOB") {
        m_type = Glob;
        min_args=3;
    } else if(type=="GLOB_RECURSE") {
        m_type = GlobRecurse;
        min_args = 3;
    } else if(type=="REMOVE") {
        m_type = Remove;
        min_args=2;
    } else if(type=="REMOVE_RECURSE") {
        m_type = RemoveRecurse;
        min_args=2;
    } else if(type=="MAKE_DIRECTORY") {
        m_type = MakeDirectory;
        min_args=2;
    } else if(type=="RELATIVE_PATH") {
        m_type = RelativePath;
        min_args=4;
    } else if(type=="TO_CMAKE_PATH") {
        m_type = ToCmakePath;
        min_args=3;
    } else if(type=="TO_NATIVE_PATH") {
        m_type = ToNativePath;
        min_args=3;
    } else if(type=="STRINGS") {
        m_type = Strings;
        min_args=3;
    } else if(type=="DOWNLOADS") {
        m_type = Download;
        min_args=3;
    } else
        return false;

    if(func.arguments.count()<min_args)
        return false;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd;
    switch(m_type) {
        case Write:
        case Append:
            m_path = func.arguments[1].value;
            m_message = func.arguments[2].value;
            break;
        case Read:
            m_path=func.arguments[1].value;
            m_variable = func.arguments[2].value;
            addOutputArgument(func.arguments[2]);
            break;
        case Glob:
            addOutputArgument(func.arguments[1]);
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
        case GlobRecurse:
            addOutputArgument(func.arguments[1]);
            m_variable = func.arguments[1].value;
            m_isFollowingSymlinks = false;
            it=func.arguments.constBegin()+2;
            itEnd=func.arguments.constEnd();
            
            for(; it!=itEnd; ++it) {
                if(it->value=="RELATIVE") {
                    it++;
                    if(it==itEnd)
                        return false;
                    else
                        m_path = it->value;
                } else if(it->value=="FOLLOW_SYMLINKS") {
                    m_isFollowingSymlinks = true;
                } else
                    m_globbingExpressions << it->value;
            }
            break;
        case Remove:
        case RemoveRecurse:
        case MakeDirectory:
            it=func.arguments.constBegin()+1;
            itEnd=func.arguments.constEnd();
            
            for(; it!=itEnd; ++it)
                m_directories << it->value;
            break;
        case RelativePath:
            addOutputArgument(func.arguments[1]);
            m_variable = func.arguments[1].value;
            m_directory = func.arguments[2].value;
            m_path = func.arguments[3].value;
            break;
        case ToCmakePath:
        case ToNativePath:
            addOutputArgument(func.arguments[2]);
            m_path = func.arguments[1].value;
            m_variable = func.arguments[2].value;
            break;
        case Download:
            m_url=func.arguments[1].value;
            m_path=func.arguments[2].value;
            it=func.arguments.constBegin()+3;
            itEnd=func.arguments.constEnd();
            for(; it!=itEnd; ++it)
            {
                if(it->value=="STATUS")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_variable = it->value;
                }
                else if(it->value=="LOG")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_message = it->value;
                }
            }
            break;
        case Strings:
            m_path=func.arguments[1].value;
            m_variable=func.arguments[2].value;
            it=func.arguments.constBegin()+3;
            itEnd=func.arguments.constEnd();
            for(; it!=itEnd; ++it)
            {
                bool corr;
                if(it->value=="LIMIT_COUNT")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_limitCount = it->value.toInt(&corr);
                    if(!corr) return false;
                }
                else if(it->value=="LIMIT_INPUT")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_limitInput = it->value.toInt(&corr);
                    if(!corr) return false;
                }
                else if(it->value=="LIMIT_OUTPUT")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_limitOutput = it->value.toInt(&corr);
                    if(!corr) return false;
                }
                else if(it->value=="LENGTH_MINIMUM")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_lengthMinimum = it->value.toInt(&corr);
                    if(!corr) return false;
                }
                else if(it->value=="LENGTH_MAXIMUM")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_lengthMaximum = it->value.toInt(&corr);
                    if(!corr) return false;
                }
                else if(it->value=="REGEX")
                {
                    ++it;
                    if(it==itEnd) break;
                    m_regex = it->value;
                }
                else if(it->value=="NEWLINE_CONSUME")
                {
                    m_newlineConsume=true;
                }
                else if(it->value=="NO_HEX_CONVERSION")
                {
                    m_noHexConversion=true;
                }
            }
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


bool FindFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_file" || func.arguments.count()<3)
        return false;
    
    bool definedNames=false;
    addOutputArgument(func.arguments.first());
    m_variableName = func.arguments.first().value;
    Stage s;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1, itEnd=func.arguments.constEnd();
    if(it->value=="NAMES") {
        definedNames = true;
        s=NAMES;
    } else {
        m_filenames=QStringList(it->value);
        s=PATHS;
        definedNames = false;
    }
    ++it;

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
        else if(it->value=="HINTS")
            s=HINTS;
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
            case HINTS:
                m_hints << it->value;
                break;
        }
    }
    return !m_filenames.isEmpty();
}


MacroCallAst::MacroCallAst()
{
}

MacroCallAst::~MacroCallAst()
{
}


bool MacroCallAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.isEmpty())
        return false;
    m_name = func.name.toLower();

    foreach(const CMakeFunctionArgument& fa, func.arguments)
    {
        m_arguments += fa.value;
    }
    return true;
}

FindLibraryAst::FindLibraryAst()
    : m_noDefaultPath(false), m_cmakeFindRootPath(false)
{
}

FindLibraryAst::~FindLibraryAst()
{
}


bool FindLibraryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_library" || func.arguments.count()<2)
        return false;
    
    bool definedNames=false;
    addOutputArgument(func.arguments[0]);
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
        else if(it->value=="NO_CMAKE_FIND_ROOT_PATH")
            m_cmakeFindRootPath = true;
        else if(it->value=="DOC") {
            ++it;
            if(it==itEnd)
                return false;
            m_documentation = it->value;
        } else if(it->value=="PATHS")
            s=PATHS;
        else if(it->value=="PATH_SUFFIXES")
            s=PATH_SUFFIXES;
        else if(it->value=="HINTS")
            s=HINTS;
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
            case HINTS:
                m_hints << it->value;
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


bool FindPackageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    m_isQuiet=false;
    m_noModule=false;
    m_isRequired=false;
    if ( func.name != "find_package" )
        return false;

    if ( func.arguments.isEmpty() )
        return false;

    m_name = func.arguments[0].value;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1;
    QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.constEnd();

    bool ret=true;
    enum State { None, Components, Paths };
    State s=None;
    for(; it!=itEnd; ++it)
    {
        if(it->value.isEmpty())
        {}
        else if(it->value[0].isNumber()) m_version=it->value;
        else if(it->value=="QUIET") m_isQuiet=true;
        else if(it->value=="NO_MODULE") m_noModule=true;
        else if(it->value=="REQUIRED") { m_isRequired=true; s=Components; }
        else if(it->value=="COMPONENTS") s=Components;
        else if(it->value=="PATHS") s=Paths;
        else if(s==Components)
            m_components.append(it->value);
        else if(s==Paths)
            m_paths.append(it->value);
        else
            ret=false;
    }
    return ret;
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


bool FindPathAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_path" || func.arguments.count()<2)
        return false;
    
    //FIXME: FIND_PATH(KDE4_DATA_DIR cmake/modules/FindKDE4Internal.cmake ${_data_DIR})
    bool definedNames=false;
    addOutputArgument(func.arguments[0]);
    m_variableName = func.arguments[0].value;
    Stage s = NAMES;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1, itEnd=func.arguments.constEnd();
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
            m_noCmakePath = true;
        else if(it->value=="NO_CMAKE_SYSTEM_PATH")
            m_noCmakeSystemPath = true;
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
        else if(it->value=="HINTS")
            s=HINTS;
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
            case HINTS:
                m_hints << it->value;
                break;
        }
    }
    return !m_filenames.isEmpty();
}

FindProgramAst::FindProgramAst()
{
        m_noDefaultPath = m_noCmakeEnvironmentPath = m_noCmakePath =
        m_noSystemEnvironmentPath = m_noCMakeFindRootPath = false;
}

FindProgramAst::~FindProgramAst()
{
}


bool FindProgramAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="find_program" || func.arguments.count()<2)
        return false;
    
    addOutputArgument(func.arguments[0]);
    m_variableName = func.arguments[0].value;
    Stage s = NAMES;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1, itEnd=func.arguments.constEnd();
    if(it->value=="NAMES")
        ++it;
    else {
        m_filenames=QStringList(it->value);
        s=PATHS;
        ++it;
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
        else if(it->value=="NO_CMAKE_FIND_ROOT_PATH")
            m_noCMakeFindRootPath = true;
        else if(it->value=="DOC") {
            ++it;
            if(it==itEnd)
                return false;
            m_documentation = it->value;
        } else if(it->value=="PATHS")
            s=PATHS;
        else if(it->value=="PATH_SUFFIXES")
            s=PATH_SUFFIXES;
        else if(it->value=="HINTS")
            s=HINTS;
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
            case HINTS:
                m_hints << it->value;
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


bool FltkWrapUiAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

ForeachAst::ForeachAst()
{
}

ForeachAst::~ForeachAst()
{
}


bool ForeachAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="foreach" || func.arguments.count()<1)
        return false;
    addOutputArgument(func.arguments.first());
    m_loopVar=func.arguments.first().value;
    if(func.arguments.count()>1 && func.arguments[1].value=="RANGE") {
        bool correctStart = true, correctStop = true, correctRange = true;
        m_type=Range;
        if(func.arguments.count()<3)
            return false;
        m_ranges.step = 1;
        m_ranges.start = 0;
        if( func.arguments.count() == 3 )
        {
            m_ranges.stop = func.arguments[2].value.toInt(&correctStop);
        }else
        {
            m_ranges.start = func.arguments[2].value.toInt(&correctStart);
            m_ranges.stop = func.arguments[3].value.toInt(&correctStop);
        }
        if(func.arguments.count()==5)
            m_ranges.step = func.arguments[4].value.toInt(&correctRange);
        if(!correctStart || !correctStop || !correctRange)
            return false;
    } else {
        int incr;
        if(func.arguments.count()>1 && func.arguments[1].value=="IN") {
            if(func.arguments[2].value=="LISTS") {
                m_type = InLists;
            } else if(func.arguments[2].value=="ITEMS") {
                m_type = InItems;
            } else
                return false;
            incr=3;
        } else {
            m_type=InItems;
            incr=1;
        }
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+incr, itEnd=func.arguments.constEnd();
        for(; it!=itEnd; ++it) {
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


bool GetCMakePropertyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_cmake_property" || func.arguments.count()!=2)
        return false;
    addOutputArgument(func.arguments[0]);
    m_variableName = func.arguments[0].value;
    
    QString type=func.arguments[1].value;
    if(type=="VARIABLES")
        m_type=Variables;
    else if(type=="CACHE_VARIABLES")
        m_type=CacheVariables;
    else if(type=="COMMANDS")
        m_type=Commands;
    else if(type=="MACROS")
        m_type=Macros;
    else if(type=="COMPONENTS")
        m_type=Components;
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


bool GetDirPropertyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_directory_property" || (func.arguments.count()!=2 && func.arguments.count()!=4))
        return false;
    
    addOutputArgument(func.arguments[0]);
    m_outputVariable = func.arguments[0].value;
    int next=1;
    if(func.arguments.count()==4) {
        if(func.arguments[1].value!="DIRECTORY")
            return false;
        m_directory=func.arguments[2].value;
        next=3;
    }
    m_propName=func.arguments[next].value;
    return true;
}

GetFilenameComponentAst::GetFilenameComponentAst()
{
}

GetFilenameComponentAst::~GetFilenameComponentAst()
{
}


bool GetFilenameComponentAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_filename_component" || func.arguments.count()<3)
        return false;
    addOutputArgument(func.arguments[0]);
    m_variableName = func.arguments[0].value;
    m_fileName = func.arguments[1].value;
    QString t = func.arguments[2].value;
    
    if(t=="PATH") m_type=Path;
    else if(t=="ABSOLUTE") m_type=Absolute;
    else if(t=="NAME") m_type=Name;
    else if(t=="EXT") m_type=Ext;
    else if(t=="NAME_WE") m_type=NameWe;
    else if(t=="PROGRAM") m_type=Program;
    else
        return false;
    
    if(m_type==Program) {
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


bool GetSourceFilePropAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_source_file_property" || func.arguments.count()!=3)
        return false;
    addOutputArgument(func.arguments[0]);
    m_variableName=func.arguments[0].value;
    m_filename=func.arguments[1].value;
    m_property=func.arguments[2].value;
    return true;
}

GetTargetPropAst::GetTargetPropAst()
{
}

GetTargetPropAst::~GetTargetPropAst()
{
}


bool GetTargetPropAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_target_property" || func.arguments.count()!=3)
        return false;
    addOutputArgument(func.arguments[0]);
    m_variableName=func.arguments[0].value;
    m_target=func.arguments[1].value;
    m_property=func.arguments[2].value;
    return true;
}

GetTestPropAst::GetTestPropAst()
{
}

GetTestPropAst::~GetTestPropAst()
{
}


bool GetTestPropAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_target_property" || func.arguments.count()!=3)
        return false;
    addOutputArgument(func.arguments[1]);
    m_test=func.arguments[0].value;
    m_variableName=func.arguments[1].value;
    m_property=func.arguments[2].value;
    return true;
}

IfAst::IfAst()
{
}

IfAst::~IfAst()
{
}

bool IfAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="if" && func.name.toLower()!="elseif" && func.name.toLower()!="else")
        return false;
    if(func.name.toLower()=="else" && !func.arguments.isEmpty())
        return false;

    m_kind = func.name;
    m_condition.clear();
    foreach(const CMakeFunctionArgument& fa, func.arguments)
    {
        m_condition += fa.value;
    }
    return true;
}

IncludeAst::IncludeAst()
    : m_optional(false)
{
}

IncludeAst::~IncludeAst()
{
}


bool IncludeAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "include" || (func.arguments.isEmpty() || func.arguments.size() > 4))
        return false;

    m_includeFile = func.arguments[0].value;
    
    QList<CMakeFunctionArgument>::const_iterator it, itEnd;
    it=func.arguments.constBegin() + 1;
    itEnd = func.arguments.constEnd();

    bool nextIsResult=false;
    for ( ; it != itEnd; ++it ) {
        if(nextIsResult)
        {
            m_resultVariable=it->value;
            addOutputArgument( *it );
            nextIsResult=false;
        }
        else if ( it->value == "OPTIONAL" )
            m_optional = true;
        else if( it->value == "RESULT_VARIABLE" )
            nextIsResult=true;
    }

    return !m_includeFile.isEmpty();
}

 IncludeDirectoriesAst::IncludeDirectoriesAst()
{
}

IncludeDirectoriesAst::~IncludeDirectoriesAst()
{
}


bool IncludeDirectoriesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "include_directories" || func.arguments.isEmpty() )
        return false;
    
    int i=0;
    m_includeType = Default;
    m_isSystem = false;
    
    if(func.arguments.first().value=="AFTER") {
        m_includeType = After;
        i++;
    } else if(func.arguments.first().value=="BEFORE") {
        m_includeType = Before;
        i++;
    }

    if(i<func.arguments.size() && func.arguments[i].value=="SYSTEM") {
        m_isSystem = true;
        i++;
    }
    
    if ( func.arguments.size() <= i )
        return false;

    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin() + i;
    QList<CMakeFunctionArgument>::const_iterator itEnd = func.arguments.constEnd();

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


bool IncludeExternalMsProjectAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

IncludeRegularExpressionAst::IncludeRegularExpressionAst()
{
}

IncludeRegularExpressionAst::~IncludeRegularExpressionAst()
{
}


bool IncludeRegularExpressionAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "include_regular_expression" || func.arguments.isEmpty() || func.arguments.count()>2 )
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


bool InstallAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

InstallFilesAst::InstallFilesAst()
{
}

InstallFilesAst::~InstallFilesAst()
{
}


bool InstallFilesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()=="install_files" || func.arguments.count()<2)
        return false;
    m_directory=func.arguments[0].value;
    if(func.arguments.count()==2)
    {
        m_regex=func.arguments[1].value;
    }
    else
    {
        QList<CMakeFunctionArgument>::const_iterator it, itEnd=func.arguments.constEnd();
        if(func.arguments[1].value!="FILES")
        m_extension=func.arguments[1].value;
        for(it=func.arguments.constBegin()+2; it!=itEnd; ++it)
        {
            m_files.append(it->value);
        }
    }
    return !m_files.isEmpty() || !m_regex.isEmpty();
}

InstallProgramsAst::InstallProgramsAst()
{
}

InstallProgramsAst::~InstallProgramsAst()
{
}


bool InstallProgramsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()=="install_programs" ||  func.arguments.count()<2)
        return false;
    m_directory=func.arguments[0].value;
    if(func.arguments.count()==2)
    {
        m_regex=func.arguments[1].value;
    }
    else
    {
        QList<CMakeFunctionArgument>::const_iterator it, itEnd=func.arguments.constEnd();
        int firstpos=1;
        if(func.arguments[1].value!="FILES")
            firstpos++;
        for(it=func.arguments.constBegin()+firstpos; it!=itEnd; ++it)
        {
            m_files.append(it->value);
        }
    }
    return !m_files.isEmpty() || !m_regex.isEmpty();
}

InstallTargetsAst::InstallTargetsAst()
{
}

InstallTargetsAst::~InstallTargetsAst()
{
}


bool InstallTargetsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()=="install_targets" || func.arguments.count()<2)
        return false;
    m_directory=func.arguments[0].value;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd=func.arguments.constEnd();
    int firstpos=1;
    if(func.arguments[1].value=="RUNTIME_DIRECTORY")
    {
        firstpos+=2;
        if(func.arguments.count()<3)
                return false;
        m_runtimeDir=func.arguments[2].value;
    }
    for(it=func.arguments.constBegin()+firstpos; it!=itEnd; ++it)
    {
        m_targets.append(it->value);
    }
    return !m_targets.isEmpty();
}

LinkDirectoriesAst::LinkDirectoriesAst()
{
}

LinkDirectoriesAst::~LinkDirectoriesAst()
{
}


bool LinkDirectoriesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "link_directories" || func.arguments.isEmpty() )
        return false;
    
    foreach(const CMakeFunctionArgument &arg, func.arguments)
        m_directories.append(arg.value);
    return true;
}

LinkLibrariesAst::LinkLibrariesAst()
{
}

LinkLibrariesAst::~LinkLibrariesAst()
{
}


bool LinkLibrariesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "link_libraries" || func.arguments.isEmpty() )
        return false;
    
    QString lastLib;
    foreach(const CMakeFunctionArgument &arg, func.arguments)
    {
        BuildType current=None;
        if(arg.value=="debug")
            current=Debug;
        else if(arg.value=="optimized")
            current=Optimized;
        else
        {
            if(!lastLib.isEmpty())
                m_libraries.append(LibraryType(lastLib, None));
            lastLib=arg.value;
        }
        if(current!=None)
            m_libraries.append(LibraryType(lastLib, current));
    }
    return true;
}

ListAst::ListAst()
{
}

ListAst::~ListAst()
{
}


bool ListAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "list" )
        return false;
    if ( func.arguments.size() < 2)
        return false;

    QString argName = func.arguments.first().value;
    if(argName=="LENGTH")
        m_type = Length;
    else if(argName=="GET")
        m_type=Get;
    else if(argName=="APPEND")
        m_type = Append;
    else if(argName=="FIND")
        m_type = Find;
    else if(argName=="INSERT")
        m_type = Insert;
    else if(argName=="REMOVE_ITEM")
        m_type = RemoveItem;
    else if(argName=="REMOVE_AT")
        m_type = RemoveAt;
    else if(argName=="SORT")
        m_type = Sort;
    else if(argName=="REVERSE")
        m_type = Reverse;
    else if(argName=="REMOVE_DUPLICATES")
        m_type = RemoveDuplicates;
    else
        return false;

    m_list = func.arguments[1].value;
    switch(m_type)
    {
        case Length:
            if(func.arguments.count()!=3)
                return false;
            m_output = func.arguments[2].value;
            addOutputArgument(func.arguments[2]);
            break;
        case Get: {
            if(func.arguments.count()<3)
                return false;
            
            addOutputArgument(func.arguments[1]);
            addOutputArgument(func.arguments.last());
            
            m_output = func.arguments.last().value;
            
            QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+2, itEnd=func.arguments.constEnd();
            int i=2;
            bool correct;
            for(; it!=itEnd; ++it)
            {
                if(i!=func.arguments.size()-1) {
                    m_index.append(it->value.toInt(&correct));
                    if(!correct)
                        return false;
                }
                i++;
            }
        } break;
        case Append: {
            if(func.arguments.count()<3)
                return false;
            
            addOutputArgument(func.arguments[1]);
            m_output = func.arguments[1].value;
            int i=0;
            foreach(const CMakeFunctionArgument& arg, func.arguments)
            {
                if(i>1)
                    m_elements.append(arg.value);
                i++;
            }
        } break;
        case Find: {
            if(func.arguments.count()!=4)
                return false;
            m_elements.append(func.arguments[2].value);
            m_output=func.arguments[3].value;

            addOutputArgument(func.arguments[3]);
        } break;
        case Insert: {
            bool correct;
            if(func.arguments.count()<4)
                return false;
            int i=0;
            addOutputArgument(func.arguments[1]);
            m_index.append(func.arguments[2].value.toInt(&correct));
            if(!correct)
                return false;
            foreach(const CMakeFunctionArgument& arg, func.arguments)
            {
                if(i>2)
                    m_elements.append(arg.value);
                i++;
            }
        } break;
        case RemoveItem: {
            if(func.arguments.count()<3)
                return false;
            addOutputArgument(func.arguments[1]);
            m_output = func.arguments[1].value;
            
            QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+2, itEnd=func.arguments.constEnd();
            for(; it!=itEnd; ++it)
            {
                m_elements.append(it->value);
            }
        } break;
        case RemoveAt: {
            if(func.arguments.count()<3)
                return false;
            addOutputArgument(func.arguments[1]);
            m_output = func.arguments[1].value;
            int i=0;
            foreach(const CMakeFunctionArgument& arg, func.arguments)
            {
                if(i>1)
                {
                    bool correct;
                    m_index.append(arg.value.toInt(&correct));
                    kDebug(9042) << "???" << arg.value;
                    if(!correct) return false;
                }
                i++;
            }
        } break;
        case Sort:
        case Reverse:
        case RemoveDuplicates:
            addOutputArgument(func.arguments[1]);
            m_output = func.arguments[1].value;
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


bool LoadCacheAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "load_cache" || func.arguments.count()<4)
        return false;
    m_cachePath=func.arguments[0].value;
    if(func.arguments[1].value=="READ_WITH_PREFIX")
    {
        QString prefix;
        QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.constEnd();
        for ( it = func.arguments.constBegin() + 2; it != itEnd; ++it )
        {
            if(prefix.isEmpty())
            {
                prefix=it->value;
            }
            else
            {
                m_prefixes=PrefixEntry(prefix, it->value);
                prefix.clear();
            }
        }
        return prefix.isEmpty();
    }
    else
    {
        bool exclude=false;
        QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.constEnd();
        for ( it = func.arguments.constBegin() + 2; it != itEnd; ++it )
        {
            if(it->value=="EXCLUDE")
                exclude=true;
            else if(it->value=="INCLUDE_INTERNALS")
                exclude=false;
            else
            {
                if(exclude)
                    m_exclude.append(it->value);
                else
                    m_includeInternals.append(it->value);
            }
        }
        return true;
    }
    return false;
}

LoadCommandAst::LoadCommandAst()
{
}

LoadCommandAst::~LoadCommandAst()
{
}


bool LoadCommandAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "load_command" || func.arguments.count()<4)
        return false;
    m_cmdName=func.arguments[0].value;
    
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.constEnd();
    for ( it = func.arguments.constBegin() + 1; it != itEnd; ++it )
        m_location.append( it->value );
    return !m_location.isEmpty();
}

MacroAst::MacroAst()
{
}

MacroAst::~MacroAst()
{
}


bool MacroAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "macro" || func.arguments.isEmpty())
        return false;

    m_macroName = func.arguments.first().value.toLower();
    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    for ( it = func.arguments.begin() + 1; it != itEnd; ++it )
        m_knownArgs.append( it->value );

    return !m_macroName.isEmpty();
}

FunctionAst::FunctionAst()
{
}

FunctionAst::~FunctionAst()
{
}


bool FunctionAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "function" || func.arguments.isEmpty())
        return false;

    m_name = func.arguments.first().value.toLower();
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


bool MakeDirectoryAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "make_directory" || func.arguments.isEmpty() || func.arguments.size()>1)
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



bool MarkAsAdvancedAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "mark_as_advanced" || func.arguments.isEmpty() )
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


bool MathAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="math" || func.arguments.count()!=3 || func.arguments.first().value!="EXPR")
        return false;
    
    addOutputArgument(func.arguments[1]);
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


bool MessageAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="message" || func.arguments.isEmpty())
        return false;
    if(func.arguments.count()>1) {
        QString type=func.arguments.first().value;
        if(type=="SEND_ERROR")
            m_type=SendError;
        else if(type=="STATUS")
            m_type=Status;
        else if(type=="FATAL_ERROR")
            m_type=FatalError;
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


bool OptionAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="option" || func.arguments.count()<2 || func.arguments.count()>3)
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


bool OutputRequiredFilesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="output_required_files" || func.arguments.count()!=2)
        return false;
    m_srcFile=func.arguments[0].value;
    m_outputFile=func.arguments[1].value;
    return true;
}

ProjectAst::ProjectAst()
{
}

ProjectAst::~ProjectAst()
{
}


bool ProjectAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "project" )
        return false;

    if ( func.arguments.size() < 1 )
        return false;

    m_projectName = func.arguments[0].value;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    for ( it = func.arguments.begin()+1; it != itEnd; ++it )
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


bool QtWrapCppAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

QtWrapUiAst::QtWrapUiAst()
{
}

QtWrapUiAst::~QtWrapUiAst()
{
}


bool QtWrapUiAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

RemoveAst::RemoveAst()
{
}

RemoveAst::~RemoveAst()
{
}


bool RemoveAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "remove" || func.arguments.isEmpty())
        return false;

    m_variableName = func.arguments[0].value;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    for ( it = func.arguments.begin()+1; it != itEnd; ++it )
    {
        m_values.append(it->value);
    }
    return !m_values.isEmpty();
}

RemoveDefinitionsAst::RemoveDefinitionsAst()
{
}

RemoveDefinitionsAst::~RemoveDefinitionsAst()
{
}


bool RemoveDefinitionsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "remove_definitions")
        return false;

    QList<CMakeFunctionArgument>::const_iterator it, itEnd = func.arguments.end();
    for ( it = func.arguments.begin(); it != itEnd; ++it )
    {
        m_definitions.append(it->value);
    }
    return !m_definitions.isEmpty();
}

SeparateArgumentsAst::SeparateArgumentsAst()
{
}

SeparateArgumentsAst::~SeparateArgumentsAst()
{
}

/// @todo Parse the UNIX_COMMAND and WINDOWS_COMMAND keywords
/// introduced in CMake 2.8.
bool SeparateArgumentsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "separate_arguments" || func.arguments.count()!=1)
        return false;

    addOutputArgument(func.arguments.first());
    m_variableName=func.arguments.first().value;
    return true;
}

SetAst::SetAst()
    : m_parentScope(false)
{
}

SetAst::~SetAst()
{
}


bool SetAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "set" || func.arguments.isEmpty() )
        return false;

    m_variableName = func.arguments.first().value;
    addOutputArgument(func.arguments.first());

    int argSize = func.arguments.size();

    //look for the FORCE argument. Thanks to the CMake folks for letting
    //me read their code
    m_forceStoring = ( argSize > 4 && func.arguments.last().value == "FORCE" );
    m_parentScope  = ( argSize > 2 && func.arguments.last().value == "PARENT_SCOPE" );
    m_storeInCache = ( argSize > 3 &&
        func.arguments[argSize - 3 - ( m_forceStoring || m_parentScope ? 1 : 0 )].value == "CACHE" );

    int numCacheArgs = ( m_storeInCache ? 3 : 0 );
    int numForceArgs = ( m_forceStoring ? 1 : 0 );
    int numParentScope = ( m_parentScope ? 1 : 0 );
    if ( argSize > 1 + numCacheArgs + numForceArgs + numParentScope )
    {
        QList<CMakeFunctionArgument> args = func.arguments;
        QList<CMakeFunctionArgument>::const_iterator it, itEnd;
        it = args.constBegin() + 1;
        itEnd = args.constEnd() - numCacheArgs - numForceArgs - numParentScope;
        for ( ; it != itEnd; ++it )
            m_values.append( it->value );
    }

    //catch some simple things. if CACHE is the last or next to last arg or if
    //FORCE was used without CACHE, then there's a problem.
    if ( func.arguments.last().value == "CACHE" ||
         ( argSize > 1 && func.arguments[argSize - 2].value == "CACHE" ) ||
         (m_forceStoring && !m_storeInCache) )
    {
        return false;
    }
    if((m_storeInCache || m_forceStoring) && m_parentScope)
        return false;
    if(func.arguments.last().value=="FORCE" && !m_forceStoring)
        return false;

    return true;
}

SetDirectoryPropsAst::SetDirectoryPropsAst()
{
}

SetDirectoryPropsAst::~SetDirectoryPropsAst()
{
}


bool SetDirectoryPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="set_directory_properties" || func.arguments.count()<3)
        return false;
    if(func.arguments.first().value!="PROPERTIES")
        return false;
    
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+1;
    QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
    
    for(; it!=itEnd; ++it)
    {
        QString prop=it->value;
        ++it;
        
        if(it==itEnd)
            return false;
        
        m_properties.append(PropPair(prop, it->value));
    }
    return !m_properties.isEmpty();
}

SetSourceFilesPropsAst::SetSourceFilesPropsAst()
{
}

SetSourceFilesPropsAst::~SetSourceFilesPropsAst()
{
}


bool SetSourceFilesPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="set_source_files_properties" || func.arguments.count()<4)
        return false;
    bool props=false;
    
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin();
    QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
    QString prop;
    for(; it!=itEnd; ++it)
    {
        if(it->value=="PROPERTIES")
        {
            props=true;
            continue;
        }
        if(!props)
        {
            m_files.append(it->value);
        }
        else
        {
            if(prop.isEmpty())
                prop=it->value;
            else
            {
                m_properties.append(PropPair(prop, it->value));
                prop.clear();
            }
        }
    }
    return prop.isEmpty();
}

SetTargetPropsAst::SetTargetPropsAst()
{
}

SetTargetPropsAst::~SetTargetPropsAst()
{
}


bool SetTargetPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="set_target_properties" || func.arguments.count()<4)
        return false;
    bool props=false;
    
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin();
    QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
    QString prop;
    for(; it!=itEnd; ++it)
    {
        if(it->value=="PROPERTIES")
        {
            props=true;
            continue;
        }
        if(!props)
        {
            m_targets.append(it->value);
        }
        else
        {
            if(prop.isEmpty())
                prop=it->value;
            else
            {
                m_properties.append(PropPair(prop, it->value));
                prop.clear();
            }
        }
    }
    return prop.isEmpty();
}

SetTestsPropsAst::SetTestsPropsAst()
{
}

SetTestsPropsAst::~SetTestsPropsAst()
{
}


bool SetTestsPropsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="set_tests_properties" || func.arguments.count()<4)
        return false;
    bool props=false;
    
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin();
    QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
    QString prop;
    for(; it!=itEnd; ++it)
    {
        if(it->value=="PROPERTIES")
        {
            props=true;
            continue;
        }
        if(!props)
        {
            m_tests.append(it->value);
        }
        else
        {
            if(prop.isEmpty())
                prop=it->value;
            else
            {
                m_properties.append(PropPair(prop, it->value));
                prop.clear();
            }
        }
    }
    return prop.isEmpty();
}

SiteNameAst::SiteNameAst()
{
}

SiteNameAst::~SiteNameAst()
{
}


bool SiteNameAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="site_name" || func.arguments.count()!=1)
        return false;
    addOutputArgument(func.arguments.first());
    m_variableName = func.arguments.first().value;
    return true;
}

SourceGroupAst::SourceGroupAst()
{
}

SourceGroupAst::~SourceGroupAst()
{
}


bool SourceGroupAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="source_group" || func.arguments.count()>1)
        return false;
    
    m_name=func.arguments[0].value;
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+1;
    QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
    enum Param { None, Regex, Files };
    Param current=None;
    for(; it!=itEnd; ++it)
    {
        if(it->value=="REGULAR_EXPRESSION")
            current=Regex;
        else if(it->value=="FILES")
            current=Files;
        else switch(current)
        {
            case Regex:
                m_regex=it->value;
            case Files:
                m_files.append(it->value);
                break;
            case None:
                return false;
        }
    }
    return !m_regex.isEmpty() || !m_files.isEmpty();
}

StringAst::StringAst()
{
}

StringAst::~StringAst()
{
}


bool StringAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="string" || func.arguments.count()<2)
        return false;
    QString stringType=func.arguments[0].value;
    if(stringType=="REGEX")
    {
        if(func.arguments.count()<5)
            return false;
        m_type=Regex;
        QString regexType = func.arguments[1].value;

        int outpos=3;
        if(regexType=="MATCH") m_cmdType=Match;
        else if(regexType=="MATCHALL") m_cmdType=MatchAll;
        else if(regexType=="REPLACE") {
            m_cmdType=RegexReplace; 
            m_replace=func.arguments[3].value;
            outpos=4;
        }

        m_regex = func.arguments[2].value;
        addOutputArgument(func.arguments[outpos]);
        m_outputVariable = func.arguments[outpos].value;
        
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+outpos+1;
        QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
        for(; it!=itEnd; ++it)
        {
            m_input += it->value;
        }
        kDebug(9042) << "performing REGEX" << regexType << " : " << m_regex << " > "<< m_outputVariable << "result: ";
    }
    else if(stringType=="REPLACE")
    {
        if(func.arguments.count()<4)
            return false;
        
        m_type=Replace;
        m_regex = func.arguments[1].value;
        m_replace=func.arguments[2].value;
        m_outputVariable = func.arguments[3].value;
        addOutputArgument(func.arguments[3]);
        
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+4, itEnd=func.arguments.end();
        for(; it!=itEnd; ++it)
        {
            m_input += it->value;
        }
    }
    else if(stringType=="COMPARE")
    {
        if(func.arguments.count()!=5)
            return false;
        m_type=Compare;
        QString argumentType=func.arguments[1].value;
        if(argumentType=="EQUAL") m_cmdType=Equal;
        else if(argumentType=="NOTEQUAL") m_cmdType=NotEqual;
        else if(argumentType=="LESS") m_cmdType=Less;
        else if(argumentType=="GREATER") m_cmdType=Greater;
        
        m_input.append(func.arguments[2].value);
        m_input.append(func.arguments[3].value);
        m_outputVariable = func.arguments[4].value;
        addOutputArgument(func.arguments[4]);
    }
    else if(stringType=="ASCII")
    {
        m_type=Ascii;
        QList<CMakeFunctionArgument>::const_iterator it=func.arguments.begin()+1;
        QList<CMakeFunctionArgument>::const_iterator itEnd=func.arguments.end();
        for(; it!=itEnd; ++it)
        {
            m_input += it->value;
        }
        m_input.pop_back(); //We remove the output var
        m_outputVariable = func.arguments.last().value;
        addOutputArgument(func.arguments.last());
    }
    else if(stringType=="CONFIGURE")
    {
        m_type=Configure;
        if(func.arguments.isEmpty())
            return false;
        m_input += func.arguments[1].value;
        m_outputVariable=func.arguments[2].value;
        addOutputArgument(func.arguments[2]);
        
        int i=3;
        if(func.arguments.count()>i) m_only = func.arguments[i].value=="@ONLY";
        if(m_only) i++;
        if(func.arguments.count()>i) m_escapeQuotes = func.arguments[i].value=="ESCAPE_QUOTES";
    }
    else if(stringType=="TOUPPER" && func.arguments.count()==3)
    {
        m_type=ToUpper;
        m_input.append(func.arguments[1].value);
        m_outputVariable = func.arguments[2].value;
        addOutputArgument(func.arguments[2]);
    }
    else if(stringType=="TOLOWER" && func.arguments.count()==3)
    {
        m_type=ToLower;
        m_input.append(func.arguments[1].value);
        m_outputVariable = func.arguments[2].value;
        addOutputArgument(func.arguments[2]);
    }
    else if(stringType=="LENGTH" && func.arguments.count()==3)
    {
        m_type=Length;
        m_input.append(func.arguments[1].value);
        m_outputVariable = func.arguments[2].value;
        addOutputArgument(func.arguments[2]);
    }
    else if(stringType=="SUBSTRING")
    {
        if(func.arguments.count()<5)
            return false;
        bool correctBegin, correctLength;
        m_type=Substring;
        m_input.append(func.arguments[1].value);
        m_begin = func.arguments[2].value.toInt(&correctBegin);
        m_length = func.arguments[3].value.toInt(&correctLength);
        m_outputVariable = func.arguments[4].value;
        addOutputArgument(func.arguments[4]);
        if(!correctBegin || !correctLength)
            return false;
    }
    else if(stringType=="STRIP")
    {
        m_type=Strip;
        if(func.arguments.count()!=3)
            return false;
        m_string = func.arguments[1].value;
        m_outputVariable = func.arguments.last().value;
        addOutputArgument(func.arguments.last());
    }
    else if(stringType=="RANDOM")
    {
        if(func.arguments.count()>6 || func.arguments.count()<2)
            return false;
        m_type=Random;
        enum State { Alphabet, Length, None };
        State s = None;
        m_length=5;
        m_string="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        bool first=true;
        
        foreach(const CMakeFunctionArgument& arg, func.arguments)
        {
            if(first) { first=false; continue; }
            
            bool correct=true;
            switch(s) {
                case Alphabet:
                    m_regex=arg.value;
                    s=None;
                    continue;
                case Length:
                    m_length = arg.value.toInt(&correct);
                    if(!correct) return false;
                    s=None;
                    continue;
                case None:
                    break;
            }
            
            if(arg.value=="LENGTH")
                s = Length;
            else if(arg.value=="ALPHABET")
                s = Alphabet;
            else
            {
                s=None;
                if(!m_outputVariable.isEmpty())
                {
                    return false;
                }
                m_outputVariable = arg.value;
                addOutputArgument(arg);
            }
        }
    }
    else
        return false;
    return true;
}

SubdirDependsAst::SubdirDependsAst()
{
}

SubdirDependsAst::~SubdirDependsAst()
{
}


bool SubdirDependsAst::parseFunctionInfo( const CMakeFunctionDesc& func)
{
    if ( func.name != "subdir_depends" || func.arguments.isEmpty())
        return false;

    return true;
}

SubdirsAst::SubdirsAst() : m_preorder(false)
{
}

SubdirsAst::~SubdirsAst()
{
}


bool SubdirsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "subdirs" )
        return false;

    if ( func.arguments.isEmpty() )
        return false;
    
    bool excludeFromAll=false;
    foreach(const CMakeFunctionArgument& arg, func.arguments)
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


bool TargetLinkLibrariesAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "target_link_libraries" )
        return false;

    //we don't do variable expansion when parsing like CMake does, so we
    //need to have at least two arguments for target_link_libraries
    if ( func.arguments.size() < 2 )
        return false;

    m_target = func.arguments[0].value;

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


bool TryCompileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name != "try_compile" || func.arguments.size() < 3)
        return false;
    m_resultName=func.arguments[0].value;
    m_binDir=func.arguments[1].value;
    m_source=func.arguments[2].value;
    
    enum Param { None, CMakeFlags, CompileDefinitions, OutputVariable, CopyFile };
    
    Param current=None;
    QList<CMakeFunctionArgument>::const_iterator it, itEnd;
    it = func.arguments.begin() + 3;
    itEnd = func.arguments.end();

    //FIXME: Should look for errors
    for ( ; it != itEnd; ++it )
    {
        if(it->value=="CMAKE_FLAGS") current=CMakeFlags;
        else if(it->value=="COMPILE_DEFINITIONS") current=CompileDefinitions;
        else if(it->value=="OUTPUT_VARIABLE") current=OutputVariable;
        else if(it->value=="COPY_FILE") current=OutputVariable;
        else switch(current)
        {
            case None:
                if(m_projectName.isEmpty())
                    m_projectName=it->value;
                else
                    m_targetName=it->value;
                current=None;
                break;
            case CMakeFlags:
                m_cmakeFlags.append(it->value);
                break;
            case CompileDefinitions:
                m_compileDefinitions.append(it->value);
                break;
            case OutputVariable:
                m_outputName=it->value;
                addOutputArgument(*it);
                current=None;
                break;
            case CopyFile:
                m_copyFile=it->value;
                current=None;
                break;
        }
    }
    
    return true;
}

TryRunAst::TryRunAst()
{
}

TryRunAst::~TryRunAst()
{
}


bool TryRunAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="try_run" || func.arguments.count()<4)
        return false;
    
    enum Actions { None, CMakeFlags, CompileDefs, OutputVariable, Args };
    Actions act = None;
    unsigned int i=0;
    foreach(const CMakeFunctionArgument& arg, func.arguments) {
        QString val=arg.value.toLower();
        if(i<4)
            act=None;

        if(i==0)
            m_runResultVar=arg.value;
        else if(i==1)
        {
            addOutputArgument(arg);
            m_compileResultVar=arg.value;
        }
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


bool UtilitySourceAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="utility_source" || func.arguments.count()<3)
        return false;
    unsigned int i=0;
    foreach(const CMakeFunctionArgument& arg, func.arguments) {
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

UnsetAst::UnsetAst()
{
}

UnsetAst::~UnsetAst()
{
}


bool UnsetAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="unset" || func.arguments.count()<1 || func.arguments.count()>2)
        return false;
    m_variableName = func.arguments.first().value;
    addOutputArgument(func.arguments.first());
    m_cache = func.arguments.count()==2 && func.arguments.last().value=="CACHE";
    m_env = m_variableName.startsWith("ENV{");
    if(m_env) {
        m_variableName = m_variableName.mid(4, -2);
    }
    return func.arguments.count()==1 || (m_cache && !m_env);
}

VariableRequiresAst::VariableRequiresAst()
{
}

VariableRequiresAst::~VariableRequiresAst()
{
}


bool VariableRequiresAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="variable_requires" || func.arguments.count()<2)
        return false;
    unsigned int i=0;
    foreach(const CMakeFunctionArgument& arg, func.arguments) {
        if(i==0)
            m_testVariable=arg.value;
        else if(i==1)
        {
            addOutputArgument(arg);
            m_resultVariable=arg.value;
        }
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


bool VtkMakeInstantiatorAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

VtkWrapJavaAst::VtkWrapJavaAst()
{
}

VtkWrapJavaAst::~VtkWrapJavaAst()
{
}


bool VtkWrapJavaAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

VtkWrapPythonAst::VtkWrapPythonAst()
{
}

VtkWrapPythonAst::~VtkWrapPythonAst()
{
}


bool VtkWrapPythonAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

VtkWrapTclAst::VtkWrapTclAst()
{
}

VtkWrapTclAst::~VtkWrapTclAst()
{
}


bool VtkWrapTclAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    Q_UNUSED(func);
    return false;
}

WhileAst::WhileAst()
{
}

WhileAst::~WhileAst()
{
}


bool WhileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="while" || func.arguments.isEmpty())
        return false;
    foreach(const CMakeFunctionArgument& arg, func.arguments)
    {
        m_condition.append(arg.value);
    }
    return true;
}

WriteFileAst::WriteFileAst()
    : m_append(false)
{
}

WriteFileAst::~WriteFileAst()
{
}


bool WriteFileAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="write_file" || func.arguments.count()<2)
        return false;
    m_filename=func.arguments[0].value;
    m_message=func.arguments[1].value;
    
    if(func.arguments.count()>2)
    {
        if(func.arguments[2].value=="APPEND")
        {
            m_append=true;
        }
            if(func.arguments.count()>(2+m_append))
                return false;
    }
    return true;
}

CustomInvokationAst::CustomInvokationAst()
{
}

CustomInvokationAst::~CustomInvokationAst()
{
}


bool CustomInvokationAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    m_function = func;
    m_arguments = func.arguments;
    return true;
}

BreakAst::BreakAst()
{
}

BreakAst::~BreakAst()
{
}


bool BreakAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return func.name.toLower()=="break";
}

CMakePolicyAst::CMakePolicyAst()
{
}

CMakePolicyAst::~CMakePolicyAst()
{
}


bool CMakePolicyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="cmake_policy" || func.arguments.isEmpty())
        return false;
    const QString &first=func.arguments[0].value;
    
    if(first=="VERSION")
    {
        bool ok = false;
        m_version = CMakeParserUtils::parseVersion(func.arguments[1].value, &ok);
        return ok;
    }
    else if(first=="SET" && func.arguments.count()==3)
    {
        QRegExp rx("CMP([1-9]*)");
        rx.indexIn(func.arguments[1].value);

        QStringList cmpValue=rx.capturedTexts();
        cmpValue.erase(cmpValue.begin());
        if(cmpValue.count()==1)
        {
            m_policyNum=cmpValue[0].toInt();
        }
        else
            return false;
        
        if(func.arguments[2].value=="OLD")
            m_isNew=false;
        else if(func.arguments[2].value=="NEW")
            m_isNew=true;
        else
            return false;
        return true;
    } else if(first=="PUSH") {
        m_action=Push;
        return func.arguments.count()==1;
    } else if(first=="POP") {
        m_action=Pop;
        return func.arguments.count()==1;
    }
    return false;
}

ExportAst::ExportAst()
{
}

ExportAst::~ExportAst()
{
}


/// @todo Implement EXPORT(PACKAGE name), introduced in CMake 2.8
bool ExportAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="export" || func.arguments.count() < 2 || func.arguments[0].value!="TARGETS")
        return false;
    
    enum Option { TARGETS, NAMESPACE, FILE };
    Option opt=TARGETS;
    foreach(const CMakeFunctionArgument& arg, func.arguments)
    {
        if(arg.value=="TARGETS")
        {
            if(opt!=TARGETS)
                return false;
        }
        else if(arg.value=="NAMESPACE")
        {
            opt=NAMESPACE;
        }
        else if(arg.value=="FILE")
        {
            opt=FILE;
        }
        else if(arg.value=="APPEND")
        {
            m_append=true;
        }
        else
        {
            switch(opt)
            {
                case TARGETS:
                    m_targets.append(arg.value);
                    break;
                case NAMESPACE:
                    m_targetNamespace=arg.value;
                    break;
                case FILE:
                    m_filename=arg.value;
                    break;
            }
            opt=TARGETS;
        }
    }
    return !m_filename.isEmpty();
}

ReturnAst::ReturnAst()
{
}

ReturnAst::~ReturnAst()
{
}


bool ReturnAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return func.arguments.isEmpty() && func.name.toLower()=="return";
}

SetPropertyAst::SetPropertyAst()
{
}

SetPropertyAst::~SetPropertyAst()
{
}


bool SetPropertyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="set_property" || func.arguments.count() < 4)
        return false;
    
    QString propName=func.arguments.first().value;
    if(propName=="GLOBAL") m_type=GlobalProperty;
    else if(propName=="DIRECTORY") m_type=DirectoryProperty;
    else if(propName=="TARGET") m_type=TargetProperty;
    else if(propName=="SOURCE") m_type=SourceProperty;
    else if(propName=="TEST") m_type=TestProperty;
    else
        return false;
    
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin()+1, itEnd=func.arguments.constEnd();
    for(; it!=itEnd && it->value!="PROPERTY" && it->value!="APPEND"; ++it)
    {
        m_args.append(it->value);
    }
    m_append=it!=itEnd && it->value=="APPEND";
    
    if(m_append)
        ++it;
    if(it!=itEnd)
        ++it; //PROPERTY
    else
        return false;
    
    m_name=it->value;
    ++it;
    for(; it!=itEnd && it->value!="PROPERTY" && it->value!="APPEND"; ++it)
    {
        m_values.append(it->value);
    }
    
    return !m_name.isEmpty();
}



GetPropertyAst::GetPropertyAst()
{}

GetPropertyAst::~GetPropertyAst()
{}

bool GetPropertyAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if(func.name.toLower()!="get_property" || func.arguments.count() < 4 || func.arguments.count() > 6)
        return false;
    
    QList<CMakeFunctionArgument>::const_iterator it=func.arguments.constBegin(), itEnd=func.arguments.constEnd();
    m_outputVariable=it->value;
    addOutputArgument(*it);
    ++it;
    
    PropertyType t;
    QString propName=it->value;
    if(propName=="GLOBAL") t=GlobalProperty;
    else if(propName=="DIRECTORY") t=DirectoryProperty;
    else if(propName=="TARGET") t=TargetProperty;
    else if(propName=="SOURCE") t=SourceProperty;
    else if(propName=="TEST") t=TestProperty;
    else if(propName=="VARIABLE") t=VariableProperty;
    else
        return false;
    m_type=t;
    
    ++it;
    if(it->value!="PROPERTY") {
        m_typeName=it->value;
        ++it;
    }
    if(it->value!="PROPERTY") return false;
    ++it;
    
    m_name=it->value;
    ++it;
    m_behaviour=None;
    if(it!=itEnd) {
        QString ee=it->value;
        if(ee=="SET") m_behaviour=Set;
        else if(ee=="DEFINED") m_behaviour=Defined;
        else if(ee=="BRIEF_DOCS") m_behaviour=BriefDocs;
        else if(ee=="FULL_DOCS") m_behaviour=FullDocs;
    }
    
    return !m_name.isEmpty();
}
