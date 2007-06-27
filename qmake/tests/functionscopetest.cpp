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
#include "qmakeast.h"
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
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->scopeBody() == 0 );
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execBasicFunc, "foobar()\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, execSimpleFunc, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << " arg1" << " arg2 ";
    TESTFUNCARGS( scope, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execSimpleFunc, "foobar( arg1, arg2 )\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, argWithEqual, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << " arg1 = arg2 ";
    TESTFUNCARGS( scope, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, argWithEqual, "foobar( arg1 = arg2 )\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, evalQMakeSyntax, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "$${subdir}.depends = $$basename($${subdir})/$(MAKEFILE) ";
    TESTFUNCARGS( scope, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, evalQMakeSyntax,
              "eval($${subdir}.depends = $$basename($${subdir})/$(MAKEFILE) )\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, simpleVarArg, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "$${subdir}" << " $$SOMEVAR" << " $(SHELLVAR)";
    TESTFUNCARGS( scope, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, simpleVarArg,
              "eval($${subdir}, $$SOMEVAR, $(SHELLVAR))\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, partlyQuotedArg, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "\"SOMELITERALSTRNIG\" SOMETHINGELSE";
    TESTFUNCARGS( scope, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, partlyQuotedArg,
              "eval(\"SOMELITERALSTRNIG\" SOMETHINGELSE)\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, slashArg, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist <<"SOMELITERALSTRNIG/$$SOMETHINGELSE";
    TESTFUNCARGS( scope, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, slashArg,
              "eval(SOMELITERALSTRNIG/$$SOMETHINGELSE)\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, nestedFunccalls, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "val" << " $$contains(QT_PROJECT, $$foobar(some))";
    TESTFUNCARGS( scope, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, nestedFunccalls,
              "eval(val, $$contains(QT_PROJECT, $$foobar(some)))\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, oneStmtScope, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" <<  " foobar";
    TESTFUNCARGS( scope, testlist )
    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE" );
    tst->setOp( " = " );
    tst->addValue("val1");
    tst->addValue(" ");
    tst->addValue("val2");
    tst->setLineEnding( "\n" );
    teststmts.append( tst );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, oneStmtScope,
              "contains(SOMETHINGELSE, foobar) : VARIABLE = val1 val2\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, oneStmtSubScope, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist <<  "SOMETHINGELSE" << " foobar";
    TESTFUNCARGS( scope, testlist )
    QList<QMake::StatementAST*> teststmts;
    QMake::FunctionCallAST* call = new QMake::FunctionCallAST();
    call->setFunctionName( "contains" );
    call->setBegin( "(" );
    call->setEnd( ")" );
    call->insertArgument( 0, " foobar " );
    call->setLineEnding("\n");
    QMake::ScopeBodyAST* body = new QMake::ScopeBodyAST();
    body->setBegin( "{" );
    body->setEnd( "}" );
    QMake::AssignmentAST* tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE" );
    tst->setOp( " = " );
    tst->addValue("val1");
    tst->addValue(" ");
    tst->addValue("val2");
    tst->setLineEnding( "\n" );

    body->insertStatement(0, tst );
    call->setScopeBody( body );
    teststmts.append( call );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, oneStmtSubScope,
              "contains(SOMETHINGELSE, foobar) : contains( foobar ) : VARIABLE = val1 val2\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, multiLineScope, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" << " foobar";
    TESTFUNCARGS( scope, testlist )
    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE" );
    tst->setOp( " = " );
    tst->addValue("val1");
    tst->addValue(" ");
    tst->addValue("val2");
    tst->setLineEnding( "\n" );
    teststmts.append( tst );
    tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE2" );
    tst->setOp( " = " );
    tst->addValue("val2");
    tst->setLineEnding( "\n" );
    teststmts.append( tst );
    TESTSCOPEBODY( scope, teststmts, 2 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, multiLineScope,
              "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\nVARIABLE2 = val2\n}\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, multiLineScopeFuncCall, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" << " foobar";
    TESTFUNCARGS( scope, testlist )
    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE" );
    tst->setOp( " = " );
    tst->addValue("val1");
    tst->addValue(" ");
    tst->addValue("val2");
    tst->setLineEnding( "\n" );
    teststmts.append( tst );
    QMake::FunctionCallAST* call;
    call = new QMake::FunctionCallAST( );
    call->setFunctionName( "func2" );
    call->setBegin( "(" );
    call->setEnd( ")" );
    call->setLineEnding("\n");
    QMake::ScopeBodyAST* body;
    body = new QMake::ScopeBodyAST();
    body->setBegin("{");
    body->setEnd( "}" );
    tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE2" );
    tst->setOp( " = " );
    tst->addValue("val2");
    tst->setLineEnding( "\n" );
    body->insertStatement(0, tst );
    call->setScopeBody( body );
    teststmts.append( call );
    TESTSCOPEBODY( scope, teststmts, 2 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, multiLineScopeFuncCall,
              "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\n  func2() {\n    VARIABLE2 = val2\n}\n}\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, notFunc, 1 )
    QMake::FunctionCallAST* scope = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "!contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" <<  " foobar";
    TESTFUNCARGS( scope, testlist )
    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE" );
    tst->setOp( " = " );
    tst->addValue("val1");
    tst->addValue(" ");
    tst->addValue("val2");
    tst->setLineEnding( "\n" );
    teststmts.append( tst );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, notFunc,
              "!contains(SOMETHINGELSE, foobar) : VARIABLE = val1 val2\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, orOp, 1 )
    QMake::OrAST* orop = dynamic_cast<QMake::OrAST*>( ast->statements().first() );
    TESTOROP( orop, "!contains", "contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" <<  " foobar";
    TESTFUNCARGS( leftfunccall, testlist )
    testlist.clear();
    testlist << "OTHER" <<  "foo";
    TESTFUNCARGS( rightfunccall, testlist )
    QList<QMake::StatementAST*> teststmts;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST();
    tst->setVariable( "VARIABLE" );
    tst->setOp( " = " );
    tst->addValue("val1");
    tst->addValue(" ");
    tst->addValue("val2");
    tst->setLineEnding( "\n" );
    teststmts.append( tst );
    TESTSCOPEBODY( orop, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, orOp,
              "!contains(SOMETHINGELSE, foobar) | contains(OTHER,foo) : VARIABLE = val1 val2\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, spaceBeforeBrace, 1 )
    QMake::FunctionCallAST* fn = dynamic_cast<QMake::FunctionCallAST*>( ast->statements().first() );
    TESTFUNCNAME( fn, "func" )
    QStringList testlist;
    testlist << " some " ;
    TESTFUNCARGS( fn, testlist )
ENDTESTFUNCIMPL
DATAFUNCIMPL( FunctionScopeTest, spaceBeforeBrace, "func ( some )\n")

BEGINTESTFAILFUNCIMPL( FunctionScopeTest, missingParenthesis, "No closing parenthesis for function call" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, missingParenthesis,
              "eval(SOMETHINGELSE\n" )


BEGINTESTFAILFUNCIMPL( FunctionScopeTest, missingStmt, "No statement on one line function scope" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, missingStmt,
              "eval(SOMETHINGELSE):\n" )

BEGINTESTFAILFUNCIMPL( FunctionScopeTest, missingBrace, "No closing brace for function scope" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, missingBrace,
              "eval(SOMETHINGELSE){\n" )

#include "functionscopetest.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
