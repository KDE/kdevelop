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

QTEST_MAIN( FunctionScopeTest )

FunctionScopeTest::FunctionScopeTest( QObject* parent )
    : QObject( parent ), ast(0)
{}

FunctionScopeTest::~FunctionScopeTest()
{}

void FunctionScopeTest::init()
{
    ast = new QMake::ProjectAST();
    QVERIFY( ast != 0 );
}

void FunctionScopeTest::cleanup()
{
    delete ast;
    ast = 0;
    QVERIFY( ast == 0 );
}

BEGINTESTFUNCIMPL( FunctionScopeTest, execBasicFunc, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->body == 0 );
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execBasicFunc, "foobar()\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, execSimpleFunc, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->body == 0 );

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execSimpleFunc, "foobar( arg1, arg2 )\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, argWithEqual, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->body == 0 );

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, argWithEqual, "foobar( arg1 = arg2 )\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, evalQMakeSyntax, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->body == 0 );

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, evalQMakeSyntax,
              "eval($${subdir}.depends = $$basename($${subdir})/$(MAKEFILE) )\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, simpleVarArg, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->body == 0 );

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, simpleVarArg,
              "eval($${subdir}, $$SOMEVAR, $(SHELLVAR))\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, partlyQuotedArg, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->body == 0 );

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, partlyQuotedArg,
              "eval(\"SOMELITERALSTRNIG\" SOMETHINGELSE)\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, slashArg, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->body == 0 );

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, slashArg,
              "eval(SOMELITERALSTRNIG/$$SOMETHINGELSE)\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, nestedFunccalls, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->body == 0 );

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, nestedFunccalls,
              "eval(val, $$contains(QT_PROJECT, $$foobar(some)))\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, oneStatementScope, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "contains" )

    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(tst);
    val->value = "VARIABLE";
    tst->identifier = val;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op = val;
    val = new QMake::ValueAST(tst);
    val->value = "val1";
    tst->values.append( val );
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);
    teststmts.append( tst );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, oneStatementScope,
              "contains(SOMETHINGELSE, foobar) : VARIABLE = val1 val2\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, oneStatementSubScope, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "contains" )

    QList<QMake::StatementAST*> teststmts;
    QMake::FunctionCallAST* call = new QMake::FunctionCallAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(call);
    val->value = "contains";
    call->identifier = val;
    val = new QMake::ValueAST(call);
    val->value = "foobar";
    call->args.insert( 0, val );
    QMake::ScopeBodyAST* body = new QMake::ScopeBodyAST(call);
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(body);
    val = new QMake::ValueAST(tst);
    val->value = "VARIABLE";
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "val1";
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);

    body->statements.insert(0, tst );
    call->body =  body ;
    teststmts.append( call );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, oneStatementSubScope,
              "contains(SOMETHINGELSE, foobar) : contains( foobar ) : VARIABLE = val1 val2\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, multiLineScope, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "contains" )

    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(tst);
    val->value = "VARIABLE";
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "val1";
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);
    teststmts.append( tst );
    tst = new QMake::AssignmentAST(scope->body);
    val = new QMake::ValueAST(tst);
    val->value = "VARIABLE2";
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);
    teststmts.append( tst );
    TESTSCOPEBODY( scope, teststmts, 2 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, multiLineScope,
              "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\nVARIABLE2 = val2\n}\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, multiLineScopeFuncCall, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "contains" )

    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(tst);
    val->value = "VARIABLE";
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "val1";
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);
    teststmts.append( tst );
    QMake::FunctionCallAST* call;
    call = new QMake::FunctionCallAST(scope->body);
    val = new QMake::ValueAST(call);
    val->value = "func2";
    call->identifier = val;
    QMake::ScopeBodyAST* body;
    body = new QMake::ScopeBodyAST(call);
    tst = new QMake::AssignmentAST(body);
    val = new QMake::ValueAST(tst);
    val->value = "VARIABLE2";
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);
    body->statements.insert(0, tst );
    call->body =  body ;
    teststmts.append( call );
    TESTSCOPEBODY( scope, teststmts, 2 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, multiLineScopeFuncCall,
              "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\n  func2() {\n    VARIABLE2 = val2\n}\n}\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, notFunc, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( scope, "!contains" )

    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(tst);
    val->value = "VARIABLE";
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "val1";
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);
    teststmts.append( tst );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, notFunc,
              "!contains(SOMETHINGELSE, foobar) : VARIABLE = val1 val2\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, orOperator, 1 )
    QMake::OrAST* orop = dynamic_cast<QMake::OrAST*>( ast->statements.first() );
    QStringList funcs;
    funcs << "!contains" << "contains";
    TESTOROP( orop, funcs )

    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(orop->body);
    QMake::ValueAST* val = new QMake::ValueAST(tst);
    val->value = "VARIABLE";
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "=";
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "val1";
    tst->values.append(val);
    val = new QMake::ValueAST(tst);
    val->value = "val2";
    tst->values.append(val);
    teststmts.append( tst );
    TESTSCOPEBODY( orop, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, orOperator,
              "!contains(SOMETHINGELSE, foobar) | contains(OTHER,foo) : VARIABLE = val1 val2\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, spaceBeforeBrace, 1 )
    QMake::FunctionCallAST* fn = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( fn, "func" )
    QStringList testlist;
    testlist << "some " ;

ENDTESTFUNCIMPL
DATAFUNCIMPL( FunctionScopeTest, spaceBeforeBrace, "func ( some )\n")

BEGINTESTFAILFUNCIMPL( FunctionScopeTest, missingParenthesis, "No closing parenthesis for function call" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, missingParenthesis,
              "eval(SOMETHINGELSE\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, missingStatement, 1 )
    QMake::FunctionCallAST* fn = dynamic_cast<QMake::FunctionCallAST*>( ast->statements.first() );
    TESTFUNCNAME( fn, "eval" )
    QStringList testlist;
    testlist << "SOMETHINGELSE " ;

ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, missingStatement,
              "eval(SOMETHINGELSE):\n" )

BEGINTESTFAILFUNCIMPL( FunctionScopeTest, missingBrace, "No closing brace for function scope" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, missingBrace,
              "eval(SOMETHINGELSE){\n" )
