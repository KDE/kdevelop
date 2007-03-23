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

#include "scopetest.h"
#include "qmakeast.h"
#include "qmakedriver.h"
#include "testhelpers.h"

QTEST_MAIN( ScopeTest )

ScopeTest::ScopeTest( QObject* parent )
    : QObject( parent ), ast(0)
{}

ScopeTest::~ScopeTest()
{}

void ScopeTest::init()
{
    ast = new QMake::ProjectAST();
    QVERIFY( ast != 0 );
}

void ScopeTest::cleanup()
{
    delete ast;
    ast = 0;
    QVERIFY( ast == 0 );
}

BEGINTESTFUNCIMPL( ScopeTest, basicScope, 1 )
    QMake::SimpleScopeAST* scope = dynamic_cast<QMake::SimpleScopeAST*>( ast->statements().first() );
TESTSCOPENAME( scope, "foobar" )
    QList<QMake::StatementAST*> testlist;
    testlist.append( new QMake::AssignmentAST( "VARIABLE", " = ", QStringList() << "FOO", "\n" ) );
TESTSCOPEBODY( scope, testlist, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, basicScope, "foobar : VARIABLE = FOO\n")

BEGINTESTFUNCIMPL( ScopeTest, basicScopeBrace, 1 )
    QMake::SimpleScopeAST* scope = dynamic_cast<QMake::SimpleScopeAST*>( ast->statements().first() );
TESTSCOPENAME( scope, "foobar" )
    QList<QMake::StatementAST*> testlist;
    testlist.append( new QMake::AssignmentAST( "VARIABLE", " = ", QStringList() << "FOO", "\n" ) );
TESTSCOPEBODY( scope, testlist, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, basicScopeBrace, "foobar {\n  VARIABLE = FOO\n}\n")

BEGINTESTFUNCIMPL( ScopeTest, nestedScope, 1 )
    QMake::SimpleScopeAST* scope = dynamic_cast<QMake::SimpleScopeAST*>( ast->statements().first() );
TESTSCOPENAME( scope, "foobar" )
    QList<QMake::StatementAST*> testlist;
    QMake::SimpleScopeAST* simple = new QMake::SimpleScopeAST( "barfoo" );
    QList<QMake::StatementAST*> sublist;
    sublist.append( new QMake::AssignmentAST( "VARIABLE", " = ", QStringList() << "FOO", "\n" ) );
    QMake::ScopeBodyAST* body = new QMake::ScopeBodyAST(" : ", sublist );
    simple->setScopeBody( body );
    testlist.append( simple );

TESTSCOPEBODY( scope, testlist, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, nestedScope, "foobar :barfoo : VARIABLE = FOO\n")



BEGINTESTFAILFUNCIMPL( ScopeTest, missingStatement, "No statement after colon" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, missingStatement,
              "eval :\n" )

BEGINTESTFAILFUNCIMPL( ScopeTest, missingColon, "No colon" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, missingColon,
              "eval \n" )

#include "scopetest.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
