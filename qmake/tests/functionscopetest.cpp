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
#include "testmacros.h"

QTEST_MAIN( FunctionScopeTest )

#define TESTFUNCNAME( scopeast, funcname ) \
    QVERIFY( scopeast->functionCall() != 0 ); \
    QVERIFY( scopeast->scopeName().isEmpty() ); \
    QMake::FunctionCallAST* funccall = scopeast->functionCall(); \
    QVERIFY( funccall->functionName() == funcname );

#define TESTFUNCARGS( funccall, arglist ) \
    QVERIFY( funccall != 0 ); \
    QVERIFY( matchArguments( funccall->arguments(), arglist ) );

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

bool FunctionScopeTest::matchArguments( QList<QMake::FunctionArgAST*> realargs,
                                        QList<QMake::FunctionArgAST*> testargs )
{
    if( realargs.count() != testargs.count() )
        return false;
    int i = 0;
    QMake::FunctionCallAST* call;
    QMake::SimpleFunctionArgAST* simple;
    QMake::FunctionCallAST* testcall;
    QMake::SimpleFunctionArgAST* testsimple;

    foreach( QMake::FunctionArgAST* ast, realargs )
    {
        call = dynamic_cast<QMake::FunctionCallAST*>(ast);
        testcall = dynamic_cast<QMake::FunctionCallAST*>( testargs.at( i ) );
        if( call && testcall )
        {
        }
        simple = dynamic_cast<QMake::SimpleFunctionArgAST*>(ast);
        testsimple = dynamic_cast<QMake::SimpleFunctionArgAST*>( testargs.at( i ) );
        if( simple && testsimple )
        {
            if( simple->value() != testsimple->value() )
                return false;
        }
        i++;
    }
    return true;
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

    QList<QMake::FunctionArgAST*> testlist;
    testlist.append( new QMake::SimpleFunctionArgAST("arg1") );
    testlist.append( new QMake::SimpleFunctionArgAST("arg2 ") );
    TESTFUNCARGS( funccall, testlist )
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execSimpleFunc, "foobar( arg1, arg2 )\n")


#include "functionscopetest.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
