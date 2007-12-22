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
SimpleScopeAST::SimpleScopeAST( AST* parent )
        : ScopeAST( parent )
{}

SimpleScopeAST::~SimpleScopeAST()
{}

ValueAST* SimpleScopeAST::scopeName() const
{
    return identifier();
}

void SimpleScopeAST::setScopeName( ValueAST* name )
{
    setIdentifier(name);
}

AST::Type SimpleScopeAST::type() const
{
    return AST::SimpleScope;
}

}

