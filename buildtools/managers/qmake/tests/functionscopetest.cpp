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
    QVERIFY( scope->functionCall() != 0 );
    QVERIFY( scope->scopeName().isEmpty() );
    QVERIFY( scope->scopeBody() == 0 );
    QMake::FunctionCallAST* funccall = scope->functionCall();
    QVERIFY( funccall->functionName() == "foobar" );
    QVERIFY( funccall->arguments().isEmpty() );
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execBasicFunc, "foobar()\n")


BEGINTESTFUNCIMPL( FunctionScopeTest, execSimpleFunc, 1 )
    QMake::ScopeAST* scope = dynamic_cast<QMake::ScopeAST*>( ast->statements().first() );
    QVERIFY( scope->functionCall() != 0 );
    QVERIFY( scope->scopeName().isEmpty() );
    QVERIFY( scope->scopeBody() == 0 );
    QMake::FunctionCallAST* funccall = scope->functionCall();
    QVERIFY( funccall->functionName() == "foobar" );
    QVERIFY( funccall->arguments().count() == 2 );
    QMake::FunctionArgAST* fnarg = funccall->arguments().first();
    QVERIFY( fnarg != 0 );
    QMake::SimpleFunctionArgAST* simple;
    simple = dynamic_cast<QMake::SimpleFunctionArgAST*>( fnarg );
    QVERIFY( simple != 0);
    QVERIFY( simple->value() == "arg1" );
    fnarg = funccall->arguments().at(1);
    QVERIFY( fnarg != 0 );
    simple = dynamic_cast<QMake::SimpleFunctionArgAST*>(fnarg);
    QVERIFY( simple != 0);
    QVERIFY( simple->value() == "arg2 " );
ENDTESTFUNCIMPL

DATAFUNCIMPL( FunctionScopeTest, execSimpleFunc, "foobar( arg1, arg2 )\n")


#include "functionscopetest.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
