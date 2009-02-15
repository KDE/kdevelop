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

#ifndef QMAKEASTVISITOR_H
#define QMAKEASTVISITOR_H

#include "parserexport.h"

namespace QMake
{

class AST;
class ProjectAST;
class AssignmentAST;
class FunctionCallAST;
class SimpleScopeAST;
class OrAST;
class ScopeBodyAST;
class ValueAST;

class KDEVQMAKEPARSER_EXPORT ASTVisitor
{
    typedef void (ASTVisitor::*parser_fun_t)(AST*);
    static parser_fun_t _S_parser_table[];
    public:
        virtual ~ASTVisitor();
        virtual void visitNode( AST* node );
        virtual void visitProject( ProjectAST* node );
        virtual void visitAssignment( AssignmentAST* node );
        virtual void visitValue( ValueAST* node );
        virtual void visitFunctionCall( FunctionCallAST* node );
        virtual void visitScopeBody( ScopeBodyAST* node );
        virtual void visitOr( OrAST* node );
        virtual void visitSimpleScope( SimpleScopeAST* node );
};

}

#endif

