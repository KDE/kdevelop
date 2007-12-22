/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "ast.h"

namespace QMake
{
FunctionCallAST::FunctionCallAST( AST* parent )
        : ScopeAST( parent )
{}


FunctionCallAST::~FunctionCallAST()
{
    qDeleteAll( m_args );
    m_args.clear();
}

QList<ValueAST*> FunctionCallAST::arguments() const
{
    return m_args;
}

void FunctionCallAST::addArgument( ValueAST* arg )
{
    m_args.append( arg );
}

void FunctionCallAST::insertArgument( int i, ValueAST* arg )
{
    m_args.insert( i, arg );
}

ValueAST* FunctionCallAST::functionName() const
{
    return identifier();
}

void FunctionCallAST::setFunctionName( ValueAST* name )
{
    setIdentifier(name);
}

void FunctionCallAST::removeArgument( int i )
{
    m_args.removeAt( i );
}

AST::Type FunctionCallAST::type() const
{
    return AST::FunctionCall;
}

}

