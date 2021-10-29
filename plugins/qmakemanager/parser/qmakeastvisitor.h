/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QMAKEASTVISITOR_H
#define QMAKEASTVISITOR_H

#include "parser_export.h"

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
    using parser_fun_t = void (ASTVisitor::*)(AST*);
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

