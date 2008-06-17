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

#ifndef QMAKE_CONTEXTBUILDER_H
#define QMAKE_CONTEXTBUILDER_H

#include <language/duchain/abstractcontextbuilder.h>
#include "parser/qmakeastvisitor.h"
#include "parser/ast.h"

#include "qmakeduchainexport.h"

class KUrl;

namespace KDevelop
{
class DUContext;
}

class QMakeEditorIntegrator;

typedef KDevelop::AbstractContextBuilder<QMake::AST, QMake::AST> ContextBuilderBase;

class KDEVQMAKEDUCHAIN_EXPORT ContextBuilder : public QMake::ASTVisitor, public ContextBuilderBase
{
public:
    ContextBuilder();
    ~ContextBuilder();
protected:
    QMakeEditorIntegrator* editor() const;
    virtual void startVisiting( QMake::AST* node );
    virtual void setContextOnNode( QMake::AST* node, KDevelop::DUContext* ctx );
    virtual KDevelop::DUContext* contextFromNode( QMake::AST* node );
    virtual KTextEditor::Range editorFindRange( QMake::AST* fromRange, QMake::AST* toRange );
    virtual KDevelop::QualifiedIdentifier identifierForNode( QMake::AST* );
};

#endif
