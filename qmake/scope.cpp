/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "scope.h"

Scope::Scope()
    : m_parent(0)
{
}

Scope::~Scope()
{
    m_ast = 0;
    qDeleteAll( m_subScopes );
    m_subScopes.clear();
}

void Scope::addSubScope( Scope* s )
{
    m_subScopes.append( s );
}

void Scope::setParent( Scope* s )
{
    m_parent = s;
}

Scope* Scope::parent() const
{
    return m_parent;
}

void Scope::addVariable(const QString& var, QMake::AssignmentAST* node )
{
    m_variables[var] = node;
}

void Scope::setAst( QMake::ScopeBodyAST* a )
{
    m_ast = a;
}

QMake::ScopeBodyAST* Scope::ast() const
{
    return m_ast;
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
