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

#include "qmakeastdefaultvisitor.h"
#include "ast.h"

namespace QMake
{

ASTDefaultVisitor::~ASTDefaultVisitor()
{
}

void ASTDefaultVisitor::visitProject( ProjectAST* node )
{
    foreach( StatementAST* stmt, node->statements )
    {
        visitNode(stmt);
    }
}

void ASTDefaultVisitor::visitAssignment( AssignmentAST* node )
{
    visitNode(node->identifier);
    visitNode(node->op);
    foreach( ValueAST* value, node->values )
    {
        visitNode(value);
    }
}


void ASTDefaultVisitor::visitValue( ValueAST* )
{
}

void ASTDefaultVisitor::visitFunctionCall( FunctionCallAST* node )
{
    visitNode(node->identifier);
    foreach(ValueAST* value, node->args)
    {
        visitNode(value);
    }
    visitNode(node->body);
}

void ASTDefaultVisitor::visitScopeBody( ScopeBodyAST* node )
{
    foreach( StatementAST* stmt, node->statements )
    {
        visitNode(stmt);
    }
}

void ASTDefaultVisitor::visitOr( OrAST* node )
{
    foreach( ScopeAST* scope, node->scopes )
    {
        visitNode(scope);
    }
    visitNode(node->body);
}

void ASTDefaultVisitor::visitSimpleScope( SimpleScopeAST* node )
{
    visitNode(node->identifier);
    visitNode(node->body);
}

}

