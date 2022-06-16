/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEDEBUGVISITOR_H
#define QMAKEDEBUGVISITOR_H

#include "qmakedefaultvisitor.h"
#include "qmakeparser.h"

#include <QTextStream>

namespace QMake
{

class parser;

class DebugVisitor:  public DefaultVisitor
{

public:
    explicit DebugVisitor(QMake::Parser* parser);
    void visitArgumentList( ArgumentListAst *node ) override;
    void visitFunctionArguments( FunctionArgumentsAst *node ) override;
    void visitOrOperator( OrOperatorAst *node ) override;
    void visitItem( ItemAst *node ) override;
    void visitScope( ScopeAst *node ) override;
    void visitOp( OpAst *node ) override;
    void visitProject( ProjectAst *node ) override;
    void visitScopeBody( ScopeBodyAst *node ) override;
    void visitStatement( StatementAst *node ) override;
    void visitValue( ValueAst *node ) override;
    void visitValueList( ValueListAst *node ) override;
    void visitVariableAssignment( VariableAssignmentAst *node ) override;
    QString getTokenInfo(qint64 idx);
    QString getIndent();
private:
    QTextStream m_out;
    QMake::Parser* m_parser;
    int indent;
};

} // end of namespace QMake

#endif

