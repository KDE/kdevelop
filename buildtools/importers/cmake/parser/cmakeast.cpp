/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include <QtCore/QString>
#include "astfactory.h"
#include "cmakelistsparser.h"

void CMakeAst::writeBack(QString& buffer)
{

}


CMAKE_REGISTER_AST( CustomCommandAst, add_custom_command )

CMAKE_BEGIN_AST_CLASS( CustomTargetAst )
CMAKE_END_AST_CLASS( CustomTargetAst )
CMAKE_REGISTER_AST( CustomTargetAst, add_custom_target )

CMAKE_BEGIN_AST_CLASS( AddDefinitionsAst )
CMAKE_END_AST_CLASS( AddDefinitionsAst )
CMAKE_REGISTER_AST( AddDefinitionsAst, add_definitions )


CMAKE_BEGIN_AST_CLASS( SetAst )
CMAKE_END_AST_CLASS( SetAst )
CMAKE_REGISTER_AST( SetAst, set )

void CustomCommandAst::writeBack( QString& /*buffer */ )
{
}

bool CustomCommandAst::parseFunctionInfo( const CMakeFunctionDesc& )
{
    return false;
}

void CustomTargetAst::writeBack( const QString& )
{
}

bool CustomTargetAst::parseFunctionInfo( const CMakeFunctionDesc& )
{
    return false;
}

void AddDefinitionsAst::writeBack( const QString& )
{
}

bool AddDefinitionsAst::parseFunctionInfo( const CMakeFunctionDesc& )
{
   return false;
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
