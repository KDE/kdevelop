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

#include "qmakeastvisitor.h"
#include "ast.h"

namespace QMake
{

ASTVisitor::parser_fun_t ASTVisitor::_S_parser_table[] =  {
        reinterpret_cast<parser_fun_t>(&ASTVisitor::visitProject),
        reinterpret_cast<parser_fun_t>(&ASTVisitor::visitScopeBody),
        reinterpret_cast<parser_fun_t>(&ASTVisitor::visitAssignment),
        reinterpret_cast<parser_fun_t>(&ASTVisitor::visitFunctionCall),
        reinterpret_cast<parser_fun_t>(&ASTVisitor::visitSimpleScope),
        reinterpret_cast<parser_fun_t>(&ASTVisitor::visitOr),
        reinterpret_cast<parser_fun_t>(&ASTVisitor::visitValue),
    }; // _S_parser_table[]

ASTVisitor::~ASTVisitor()
{
}

void ASTVisitor::visitNode( AST* node )
{
    if( node )
    {
        (this->*_S_parser_table[node->type()])(node);
    }
}

void ASTVisitor::visitProject( ProjectAST* )
{
}

void ASTVisitor::visitAssignment( AssignmentAST* )
{
}

void ASTVisitor::visitValue( ValueAST* )
{
}

void ASTVisitor::visitFunctionCall( FunctionCallAST* )
{
}

void ASTVisitor::visitScopeBody( ScopeBodyAST* )
{
}

void ASTVisitor::visitOr( OrAST* )
{
}

void ASTVisitor::visitSimpleScope( SimpleScopeAST* )
{
}


}

