/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef FUNCTIONSCOPETEST_H
#define FUNCTIONSCOPETEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QtCore/QList>


namespace QMake
{
    class StatementAST;
    class ProjectAST;
    class FunctionArgAST;
}

class FunctionScopeTest : public QObject
{
        Q_OBJECT
    public:
        FunctionScopeTest( QObject* parent = 0 );
        ~FunctionScopeTest();
    private slots:
        void init();
        void cleanup();
        void execSimpleFunc();
        void execSimpleFunc_data();
        void execBasicFunc();
        void execBasicFunc_data();
        void evalQMakeSyntax();
        void evalQMakeSyntax_data();
        void simpleVarArg();
        void simpleVarArg_data();
        void argWithEqual();
        void argWithEqual_data();
        void partlyQuotedArg();
        void partlyQuotedArg_data();
        void slashArg();
        void slashArg_data();
        void oneStmtScope();
        void oneStmtScope_data();
        void nestedFunccalls();
        void nestedFunccalls_data();
        void multiLineScope();
        void multiLineScope_data();
        void multiLineScopeFuncCall();
        void multiLineScopeFuncCall_data();
        void oneStmtSubScope();
        void oneStmtSubScope_data();

        void missingStmt();
        void missingStmt_data();
        void missingBrace();
        void missingBrace_data();
        void missingParenthesis();
        void missingParenthesis_data();
    private:
        QMake::ProjectAST* ast;

};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
