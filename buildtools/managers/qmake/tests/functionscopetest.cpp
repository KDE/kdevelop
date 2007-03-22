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

#define TESTFUNCNAME( scopeast, funcname ) \
    QVERIFY( scopeast->functionCall() != 0 ); \
    QVERIFY( scopeast->scopeName().isEmpty() ); \
    QMake::FunctionCallAST* funccall = scopeast->functionCall(); \
    QVERIFY( funccall->functionName() == funcname );

#define TESTFUNCARGS( funccall, arglist ) \
    QVERIFY( funccall != 0 ); \
    QVERIFY( funccall->arguments() == arglist );

#define TESTSCOPEBODY( scope, teststmts, stmtcount ) \
    QVERIFY( scope->scopeBody() != 0 ); \
    QVERIFY( scope->scopeBody()->statements().count() == stmtcount ); \
    QVERIFY( matchScopeBodies(scope->scopeBody()->statements(), teststmts) );

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
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->scopeBody() == 0 );
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execBasicFunc, "foobar()\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, execSimpleFunc, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << " arg1" << " arg2 ";
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execSimpleFunc, "foobar( arg1, arg2 )\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, argWithEqual, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "foobar" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << " arg1 = arg2 ";
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, argWithEqual, "foobar( arg1 = arg2 )\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, evalQMakeSyntax, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "$${subdir}.depends = $$basename($${subdir})/$(MAKEFILE) ";
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, evalQMakeSyntax,
              "eval($${subdir}.depends = $$basename($${subdir})/$(MAKEFILE) )\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, simpleVarArg, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "$${subdir}" << " $$SOMEVAR" << " $(SHELLVAR)";
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, simpleVarArg,
              "eval($${subdir}, $$SOMEVAR, $(SHELLVAR))\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, partlyQuotedArg, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "\"SOMELITERALSTRNIG\" SOMETHINGELSE";
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, partlyQuotedArg,
              "eval(\"SOMELITERALSTRNIG\" SOMETHINGELSE)\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, slashArg, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist <<"SOMELITERALSTRNIG/$$SOMETHINGELSE";
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, slashArg,
              "eval(SOMELITERALSTRNIG/$$SOMETHINGELSE)\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, nestedFunccalls, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "eval" )
    QVERIFY( scope->scopeBody() == 0 );
    QStringList testlist;
    testlist << "val" << " $$contains(QT_PROJECT, $$foobar(some))";
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, nestedFunccalls,
              "eval(val, $$contains(QT_PROJECT, $$foobar(some)))\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, oneStmtScope, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" <<  " foobar";
    TESTFUNCARGS( funccall, testlist )
    QList<QMake::StatementAST*> teststmts;
    teststmts.append( new QMake::AssignmentAST( "VARIABLE", " = ", QStringList() << "val1" << " " << "val2", "\n" ) );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, oneStmtScope,
              "contains(SOMETHINGELSE, foobar) : VARIABLE = val1 val2\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, oneStmtSubScope, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist <<  "SOMETHINGELSE" << " foobar";
    TESTFUNCARGS( funccall, testlist )
    QList<QMake::StatementAST*> teststmts;
    QMake::FunctionCallAST* call;
    call = new QMake::FunctionCallAST( "contains", "(", QStringList(), ")" );
    QMake::ScopeAST* subscope;
    subscope = new QMake::ScopeAST( call, "\n" );
    QMake::ScopeBodyAST* body;
    body = new QMake::ScopeBodyAST( "{", QList<QMake::StatementAST*>(), "}" );
    body->insertStatement(0, new QMake::AssignmentAST( "VARIABLE", " = ", QStringList() << "val1" << " " << "val2", "\n" ) );
    subscope->setScopeBody( body );
    teststmts.append( subscope );
    TESTSCOPEBODY( scope, teststmts, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, oneStmtSubScope,
              "contains(SOMETHINGELSE, foobar) : contains( foobar ) : VARIABLE = val1 val2\n" )

BEGINTESTFUNCIMPL( FunctionScopeTest, multiLineScope, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" << " foobar";
    TESTFUNCARGS( funccall, testlist )
    QList<QMake::StatementAST*> teststmts;
    teststmts.append( new QMake::AssignmentAST( "VARIABLE", " = ", QStringList() << "val1" << " " << "val2", "\n" ) );
    teststmts.append( new QMake::AssignmentAST( "VARIABLE2", " = ", QStringList() << "val2", "\n" ) );
    TESTSCOPEBODY( scope, teststmts, 2 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, multiLineScope,
              "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\nVARIABLE2 = val2\n}\n" )


BEGINTESTFUNCIMPL( FunctionScopeTest, multiLineScopeFuncCall, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    TESTFUNCNAME( scope, "contains" )
    QStringList testlist;
    testlist << "SOMETHINGELSE" << " foobar";
    TESTFUNCARGS( funccall, testlist )
    QList<QMake::StatementAST*> teststmts;
    teststmts.append( new QMake::AssignmentAST( "VARIABLE", " = ", QStringList() << "val1" << " " << "val2", "\n" ) );
    QMake::FunctionCallAST* call;
    call = new QMake::FunctionCallAST( "func2", "(", QStringList(), ")" );
    QMake::ScopeAST* subscope;
    subscope = new QMake::ScopeAST( call, "\n" );
    QMake::ScopeBodyAST* body;
    body = new QMake::ScopeBodyAST( "{", QList<QMake::StatementAST*>(), "}" );
    body->insertStatement(0, new QMake::AssignmentAST( "VARIABLE2", " = ", QStringList() << "val2", "\n" ) );
    subscope->setScopeBody( body );
    teststmts.append( subscope );
    TESTSCOPEBODY( scope, teststmts, 2 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, multiLineScopeFuncCall,
              "contains(SOMETHINGELSE, foobar) {\n  VARIABLE = val1 val2\n  func2() {\n    VARIABLE2 = val2\n}\n}\n" )

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
