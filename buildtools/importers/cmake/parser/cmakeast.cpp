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


CMAKE_BEGIN_AST_CLASS( SetAst )
CMAKE_END_AST_CLASS( SetAst )
CMAKE_REGISTER_AST( SetAst, set )

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

void CustomTargetAst::writeBack( const QString& )
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

AddDefinitionsAst::AddDefinitionsAst()
{

}

AddDefinitionsAst::~AddDefinitionsAst()
{
}

void AddDefinitionsAst::writeBack( const QString& )
{
}

bool AddDefinitionsAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "add_definitions" )
        return false;

    foreach( CMakeFunctionArgument arg, func.arguments )
    {
        m_definitions << arg.value;
    }

    return true;
}

SetAst::SetAst()
{
}

SetAst::~SetAst()
{
}

void SetAst::writeBack( const QString& buffer )
{
}

bool SetAst::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    if ( func.name.toLower() != "set" )
        return false;

    return true;
}
