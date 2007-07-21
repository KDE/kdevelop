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

#include "qmakeast.h"

namespace QMake
{
OrAST::OrAST( AST* parent )
        : ScopeAST( parent ), m_lScope( 0 ), m_rScope( 0 )
{}

OrAST::~OrAST()
{
    delete m_lScope;
    m_lScope = 0;
    delete m_rScope;
    m_rScope = 0;
}

ScopeAST* OrAST::leftScope() const
{
    return m_lScope;
}

ScopeAST* OrAST::rightScope() const
{
    return m_rScope;
}

void OrAST::setLeftScope( ScopeAST* call )
{
    m_lScope = call;
}

void OrAST::setRightScope( ScopeAST* call )
{
    m_rScope = call;
}

}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
