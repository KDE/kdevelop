/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testhelpers.h"

#include <QList>
#include <QTest>
#include "ast.h"

void matchScopeBodies(const QList<QMake::StatementAST*>& realbody, const QList<QMake::StatementAST*>& testbody)
{
    QCOMPARE(realbody.count(), testbody.count());
    int i = 0;
    QMake::AssignmentAST* assign;
    QMake::ScopeAST* scope;
    QMake::AssignmentAST* testassign;
    QMake::ScopeAST* testscope;

    for (QMake::StatementAST* ast : realbody) {
        scope = dynamic_cast<QMake::ScopeAST*>(ast);
        testscope = dynamic_cast<QMake::ScopeAST*>(testbody.at(i));

        if (scope && testscope) {
            auto* call = dynamic_cast<QMake::FunctionCallAST*>(scope);
            auto* testcall = dynamic_cast<QMake::FunctionCallAST*>(testscope);
            auto* simple = dynamic_cast<QMake::SimpleScopeAST*>(scope);
            auto* testsimple = dynamic_cast<QMake::SimpleScopeAST*>(testscope);
            auto* orop = dynamic_cast<QMake::OrAST*>(scope);
            auto* testorop = dynamic_cast<QMake::OrAST*>(testscope);
            QVERIFY((call && testcall) || (simple && testsimple) || (orop && testorop));
            if (call && testcall) {
                TESTFUNCNAME(call, testcall->identifier->value)
            } else if (simple && testsimple) {

            } else if (orop && testorop) {
                TESTOROPAST(orop, testorop)
            }
            QCOMPARE(static_cast<bool>(scope->body), static_cast<bool>(testscope->body));
            if (scope->body && testscope->body) {
                matchScopeBodies(scope->body->ifStatements, testscope->body->ifStatements);
                matchScopeBodies(scope->body->elseStatements, testscope->body->elseStatements);
            }
        }
        assign = dynamic_cast<QMake::AssignmentAST*>(ast);
        testassign = dynamic_cast<QMake::AssignmentAST*>(testbody.at(i));
        if (assign && testassign) {
            TESTASSIGNMENT(assign, testassign->identifier->value, testassign->op->value, testassign->values.count())
        }
        i++;
    }
}
