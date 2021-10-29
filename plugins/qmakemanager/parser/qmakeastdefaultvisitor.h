/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QMAKEASTDEFAULTVISITOR_H
#define QMAKEASTDEFAULTVISITOR_H

#include "qmakeastvisitor.h"
#include "parser_export.h"

namespace QMake
{
    class KDEVQMAKEPARSER_EXPORT ASTDefaultVisitor : public ASTVisitor
    {
    public:
        ~ASTDefaultVisitor() override;
        void visitProject( ProjectAST* node ) override;
        void visitAssignment( AssignmentAST* node ) override;
        void visitValue( ValueAST* node ) override;
        void visitFunctionCall( FunctionCallAST* node ) override;
        void visitScopeBody( ScopeBodyAST* node ) override;
        void visitOr( OrAST* node ) override;
        void visitSimpleScope( SimpleScopeAST* node ) override;
    };
}

#endif

