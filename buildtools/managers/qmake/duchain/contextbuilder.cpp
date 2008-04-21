/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "contextbuilder.h"
#include "qmakeeditorintegrator.h"

ContextBuilder::ContextBuilder()
    : QMake::ASTVisitor(), KDevelop::BaseContextBuilder<QMake::AST>( new QMakeEditorIntegrator() )
{
}

ContextBuilder::~ContextBuilder()
{
}

void ContextBuilder::supportBuild( QMake::AST* node )
{
}

void ContextBuilder::setContextOnNode( QMake::AST* node, KDevelop::DUContext* ctx )
{
    node->context = ctx;
}

KDevelop::DUContext* ContextBuilder::contextFromNode( QMake::AST* node )
{
    return node->context;
}

KTextEditor::Range ContextBuilder::editorFindRange( QMake::AST* fromRange, QMake::AST* toRange )
{
    QMakeEditorIntegrator* ed = editor<QMakeEditorIntegrator>();
    return ed->findRange(fromRange, toRange);
}

const KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode( QMake::AST* node )
{
    QMake::ValueAST* val = static_cast<QMake::ValueAST*>( node );
    setIdentifier( val->value );
    return qualifiedIdentifier();
}

