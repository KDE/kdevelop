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

#include "functionscopetest.h"
#include "ast.h"
#include "qmakedriver.h"
#include "testhelpers.h"

QTEST_MAIN(FunctionScopeTest)

FunctionScopeTest::FunctionScopeTest(QObject* parent)
    : QObject(parent)
{
}

FunctionScopeTest::~FunctionScopeTest()
{
}

void FunctionScopeTest::init()
{
    ast = new QMake::ProjectAST();
    QVERIFY(ast != nullptr);
}

void FunctionScopeTest::cleanup()
{
    delete ast;
    ast = nullptr;
    QVERIFY(ast == nullptr);
}

BEGINTESTFUNCIMPL(FunctionScopeTest, execBasicFunc, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "foobar")
    QVERIFY(scope->body == nullptr);
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, execBasicFunc, "foobar()\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, execSimpleFunc, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "foobar")
    QVERIFY(scope->body == nullptr);

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, execSimpleFunc, "foobar( arg1, arg2 )\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, argWithEqual, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "foobar")
    QVERIFY(scope->body == nullptr);

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, argWithEqual, "foobar( arg1 = arg2 )\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, evalQMakeSyntax, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "eval")
    QVERIFY(scope->body == nullptr);

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, evalQMakeSyntax, "eval($${subdir}.depends = $$basename($${subdir})/$(MAKEFILE) )\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, simpleVarArg, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "eval")
    QVERIFY(scope->body == nullptr);

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, simpleVarArg, "eval($${subdir}, $$SOMEVAR, $(SHELLVAR))\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, partlyQuotedArg, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "eval")
    QVERIFY(scope->body == nullptr);

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, partlyQuotedArg, "eval(\"SOMELITERALSTRNIG\" SOMETHINGELSE)\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, slashArg, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "eval")
    QVERIFY(scope->body == nullptr);

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, slashArg, "eval(SOMELITERALSTRNIG/$$SOMETHINGELSE)\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, nestedFunccalls, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "eval")
    QVERIFY(scope->body == nullptr);

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, nestedFunccalls, "eval(val, $$contains(QT_PROJECT, $$foobar(some)))\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, oneStatementScope, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "contains")

    QList<QMake::StatementAST*> teststmts;
    auto tst = new QMake::AssignmentAST(scope->body);
    auto val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val1");
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);
    teststmts.append(tst);
    TESTSCOPEBODY(scope, teststmts, 1)
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, oneStatementScope, "contains(SOMETHINGELSE, foobar) : VARIABLE = val1 val2\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, oneStatementSubScope, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "contains")

    QList<QMake::StatementAST*> teststmts;
    auto call = new QMake::FunctionCallAST(scope->body);
    auto val = new QMake::ValueAST(call);
    val->value = QStringLiteral("contains");
    call->identifier = val;
    val = new QMake::ValueAST(call);
    val->value = QStringLiteral("foobar");
    call->args.insert(0, val);
    auto body = new QMake::ScopeBodyAST(call);
    auto tst = new QMake::AssignmentAST(body);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val1");
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);

    body->statements.insert(0, tst);
    call->body = body;
    teststmts.append(call);
    TESTSCOPEBODY(scope, teststmts, 1)
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, oneStatementSubScope,
             "contains(SOMETHINGELSE, foobar) : contains( foobar ) : VARIABLE = val1 val2\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, multiLineScope, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "contains")

    QList<QMake::StatementAST*> teststmts;
    auto tst = new QMake::AssignmentAST(scope->body);
    auto val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val1");
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);
    teststmts.append(tst);
    tst = new QMake::AssignmentAST(scope->body);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE2");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);
    teststmts.append(tst);
    TESTSCOPEBODY(scope, teststmts, 2)
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, multiLineScope,
             "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\nVARIABLE2 = val2\n}\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, multiLineScopeFuncCall, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "contains")

    QList<QMake::StatementAST*> teststmts;
    auto tst = new QMake::AssignmentAST(scope->body);
    auto val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val1");
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);
    teststmts.append(tst);
    QMake::FunctionCallAST* call;
    call = new QMake::FunctionCallAST(scope->body);
    val = new QMake::ValueAST(call);
    val->value = QStringLiteral("func2");
    call->identifier = val;
    QMake::ScopeBodyAST* body;
    body = new QMake::ScopeBodyAST(call);
    tst = new QMake::AssignmentAST(body);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE2");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);
    body->statements.insert(0, tst);
    call->body = body;
    teststmts.append(call);
    TESTSCOPEBODY(scope, teststmts, 2)
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, multiLineScopeFuncCall,
             "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\n  func2() {\n    VARIABLE2 = val2\n}\n}\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, notFunc, 1)
    auto* scope = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(scope, "!contains")

    QList<QMake::StatementAST*> teststmts;
    auto tst = new QMake::AssignmentAST(scope->body);
    auto val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val1");
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);
    teststmts.append(tst);
    TESTSCOPEBODY(scope, teststmts, 1)
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, notFunc, "!contains(SOMETHINGELSE, foobar) : VARIABLE = val1 val2\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, orOperator, 1)
    auto* orop = dynamic_cast<QMake::OrAST*>(ast->statements.first());
    QStringList funcs;
    funcs << QStringLiteral("!contains")
          << QStringLiteral("contains");
    TESTOROP(orop, funcs)

    QList<QMake::StatementAST*> teststmts;
    auto tst = new QMake::AssignmentAST(orop->body);
    auto val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("VARIABLE");
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("=");
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val1");
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = QStringLiteral("val2");
    tst->values.append(val);
    teststmts.append(tst);
    TESTSCOPEBODY(orop, teststmts, 1)
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, orOperator,
             "!contains(SOMETHINGELSE, foobar) | contains(OTHER,foo) : VARIABLE = val1 val2\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, spaceBeforeBrace, 1)
    auto* fn = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(fn, "func")
    QStringList testlist;
    testlist << QStringLiteral("some ");

ENDTESTFUNCIMPL
DATAFUNCIMPL(FunctionScopeTest, spaceBeforeBrace, "func ( some )\n")

BEGINTESTFAILFUNCIMPL(FunctionScopeTest, missingParenthesis, "No closing parenthesis for function call")
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, missingParenthesis, "eval(SOMETHINGELSE\n")

BEGINTESTFUNCIMPL(FunctionScopeTest, missingStatement, 1)
    auto* fn = dynamic_cast<QMake::FunctionCallAST*>(ast->statements.first());
    TESTFUNCNAME(fn, "eval")
    QStringList testlist;
    testlist << QStringLiteral("SOMETHINGELSE ");

ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, missingStatement, "eval(SOMETHINGELSE):\n")

BEGINTESTFAILFUNCIMPL(FunctionScopeTest, missingBrace, "No closing brace for function scope")
ENDTESTFUNCIMPL

DATAFUNCIMPL(FunctionScopeTest, missingBrace, "eval(SOMETHINGELSE){\n")
