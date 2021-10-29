/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FUNCTIONSCOPETEST_H
#define FUNCTIONSCOPETEST_H

#include <QObject>
#include <QTest>

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
        explicit FunctionScopeTest( QObject* parent = nullptr );
        ~FunctionScopeTest() override;
    private Q_SLOTS:
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
        void oneStatementScope();
        void oneStatementScope_data();
        void nestedFunccalls();
        void nestedFunccalls_data();
        void multiLineScope();
        void multiLineScope_data();
        void multiLineScopeFuncCall();
        void multiLineScopeFuncCall_data();
        void oneStatementSubScope();
        void oneStatementSubScope_data();
        void notFunc();
        void notFunc_data();
        void orOperator();
        void orOperator_data();
        void spaceBeforeBrace();
        void spaceBeforeBrace_data();

        void missingStatement();
        void missingStatement_data();
        void missingBrace();
        void missingBrace_data();
        void missingParenthesis();
        void missingParenthesis_data();
    private:
        QMake::ProjectAST* ast = nullptr;

};

#endif

