/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASSIGNMENTTEST_H
#define ASSIGNMENTTEST_H

#include <QObject>
#include <QTest>

namespace QMake
{
    class StatementAST;
    class ProjectAST;
}

class AssignmentTest : public QObject
{
        Q_OBJECT
    public:
        explicit AssignmentTest( QObject* parent = nullptr );
        ~AssignmentTest() override;
    private Q_SLOTS:
        void init();
        void cleanup();
        void simpleParsed();
        void simpleParsed_data();
        void assignInValue();
        void assignInValue_data();
        void commentCont();
        void commentCont_data();
        void quotedBrace();
        void quotedBrace_data();
        void quotedVal();
        void quotedVal_data();
        void quotedValEscapeQuote();
        void quotedValEscapeQuote_data();
        void dotVariable();
        void dotVariable_data();
        void underscoreVariable();
        void underscoreVariable_data();
        void dashVariable();
        void dashVariable_data();
        void opNoSpace();
        void opNoSpace_data();
        void opPlusEq();
        void opPlusEq_data();
        void opTildeEq();
        void opTildeEq_data();
        void opMinusEq();
        void opMinusEq_data();
        void opStarEq();
        void opStarEq_data();
        void opEqual();
        void opEqual_data();
        void otherVarSimple();
        void otherVarSimple_data();
        void otherVarBrace();
        void otherVarBrace_data();
        void otherVarBracket();
        void otherVarBracket_data();
        void shellVar();
        void shellVar_data();
        void commandExec();
        void commandExec_data();
        void bracketVarQuote();
        void bracketVarQuote_data();
        void pathValue();
        void pathValue_data();
        void varNameDollar();
        void varNameDollar_data();
        void commentInMultiline();
        void commentInMultiline_data();
        void commandExecQMakeVar();
        void commandExecQMakeVar_data();
        void varComment();
        void varComment_data();

        void noDashEndVar();
        void noDashEndVar_data();
        void contAfterComment();
        void contAfterComment_data();
    private:
        QMake::ProjectAST* ast = nullptr;
};

#endif

