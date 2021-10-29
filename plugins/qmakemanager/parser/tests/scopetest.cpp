/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scopetest.h"
#include "ast.h"
#include "qmakedriver.h"
#include "testhelpers.h"

QTEST_GUILESS_MAIN(ScopeTest)

ScopeTest::ScopeTest(QObject* parent)
    : QObject(parent)
{
}

ScopeTest::~ScopeTest()
{
}

void ScopeTest::init()
{
    ast = new QMake::ProjectAST();
    QVERIFY(ast != nullptr);
}

void ScopeTest::cleanup()
{
    delete ast;
    ast = nullptr;
    QVERIFY(ast == nullptr);
}

BEGINTESTFUNCIMPL(ScopeTest, basicScope, 1)
    auto* scope = dynamic_cast<QMake::SimpleScopeAST*>(ast->statements.first());
    TESTSCOPENAME(scope, "foobar")
    QList<QMake::StatementAST*> testlist;
    auto tst = new QMake::AssignmentAST(scope->body);
    auto val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("FOO");
    tst->values.append(val);
    testlist.append(tst);
    TESTSCOPEBODY(scope, testlist, 1)
ENDTESTFUNCIMPL

DATAFUNCIMPL(ScopeTest, basicScope, "foobar : VARIABLE = FOO\n")

BEGINTESTFUNCIMPL(ScopeTest, basicScopeBrace, 1)
    auto* scope = dynamic_cast<QMake::SimpleScopeAST*>(ast->statements.first());
    TESTSCOPENAME(scope, "foobar")
    QList<QMake::StatementAST*> testlist;
    auto tst = new QMake::AssignmentAST(scope->body);
    auto val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("FOO");
    tst->values.append(val);
    testlist.append(tst);
    TESTSCOPEBODY(scope, testlist, 1)
ENDTESTFUNCIMPL

DATAFUNCIMPL(ScopeTest, basicScopeBrace, "foobar {\n  VARIABLE = FOO\n}\n")

BEGINTESTFUNCIMPL(ScopeTest, nestedScope, 1)
    auto* scope = dynamic_cast<QMake::SimpleScopeAST*>(ast->statements.first());
    TESTSCOPENAME(scope, "foobar")
    QList<QMake::StatementAST*> testlist;
    auto simple = new QMake::SimpleScopeAST(scope->body);
    auto val = new QMake::ValueAST(simple);
    val->value = QStringLiteral("barfoo");
    simple->identifier = val;
    auto body = new QMake::ScopeBodyAST(simple);
    auto tst = new QMake::AssignmentAST(body);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("FOO");
    tst->values.append(val);
    body->statements.append(tst);
    simple->body = body;
    testlist.append(simple);

    TESTSCOPEBODY(scope, testlist, 1)
ENDTESTFUNCIMPL

DATAFUNCIMPL(ScopeTest, nestedScope, "foobar :barfoo : VARIABLE = FOO\n")

BEGINTESTFUNCIMPL(ScopeTest, missingStatement, 1)
    auto* scope = dynamic_cast<QMake::SimpleScopeAST*>(ast->statements.first());
    TESTSCOPENAME(scope, "eval")
ENDTESTFUNCIMPL

DATAFUNCIMPL(ScopeTest, missingStatement, "eval :\n")

BEGINTESTFAILFUNCIMPL(ScopeTest, missingColon, "No colon")
ENDTESTFUNCIMPL

DATAFUNCIMPL(ScopeTest, missingColon, "eval \n")

void ScopeTest::strangeScopeNames()
{
    QMake::Driver d;
    d.setContent(QStringLiteral("linux-gcc++-* {\n  VARIABLE = FOO\n}\n"));
    bool ret = d.parse(&ast);
    QVERIFY(ret);
}
