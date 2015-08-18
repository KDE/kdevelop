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

#ifndef QMAKEASTDEFAULTVISITOR_H
#define QMAKEASTDEFAULTVISITOR_H

#include "qmakeastvisitor.h"
#include "parser_export.h"

namespace QMake
{
    class KDEVQMAKEPARSER_EXPORT ASTDefaultVisitor : public ASTVisitor
    {
    public:
        virtual ~ASTDefaultVisitor();
        virtual void visitProject( ProjectAST* node ) override;
        virtual void visitAssignment( AssignmentAST* node ) override;
        virtual void visitValue( ValueAST* node ) override;
        virtual void visitFunctionCall( FunctionCallAST* node ) override;
        virtual void visitScopeBody( ScopeBodyAST* node ) override;
        virtual void visitOr( OrAST* node ) override;
        virtual void visitSimpleScope( SimpleScopeAST* node ) override;
    };
}

#endif

