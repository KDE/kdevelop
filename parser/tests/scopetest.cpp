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
#include "ast.h"
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
    QMake::SimpleScopeAST* scope = dynamic_cast<QMake::SimpleScopeAST*>( ast->statements.first() );
TESTSCOPENAME( scope, "foobar" )
    QList<QMake::StatementAST*> testlist;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(tst);
    val->value =  "VARIABLE" ;
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value =  "=" ;
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "FOO";
    tst->values.append(val);
    testlist.append( tst );
TESTSCOPEBODY( scope, testlist, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, basicScope, "foobar : VARIABLE = FOO\n")

BEGINTESTFUNCIMPL( ScopeTest, basicScopeBrace, 1 )
    QMake::SimpleScopeAST* scope = dynamic_cast<QMake::SimpleScopeAST*>( ast->statements.first() );
TESTSCOPENAME( scope, "foobar" )
    QList<QMake::StatementAST*> testlist;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(tst);
    val->value =  "VARIABLE" ;
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value =  "=" ;
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "FOO";
    tst->values.append(val);
    testlist.append( tst );
TESTSCOPEBODY( scope, testlist, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, basicScopeBrace, "foobar {\n  VARIABLE = FOO\n}\n")

BEGINTESTFUNCIMPL( ScopeTest, nestedScope, 1 )
    QMake::SimpleScopeAST* scope = dynamic_cast<QMake::SimpleScopeAST*>( ast->statements.first() );
TESTSCOPENAME( scope, "foobar" )
    QList<QMake::StatementAST*> testlist;
    QMake::SimpleScopeAST* simple = new QMake::SimpleScopeAST(scope->body);
    QMake::ValueAST* val = new QMake::ValueAST(simple);
    val->value =  "barfoo" ;
    simple->identifier = val;
    QMake::ScopeBodyAST* body = new QMake::ScopeBodyAST(simple);
    QList<QMake::StatementAST*> sublist;
    QMake::AssignmentAST* tst = new QMake::AssignmentAST(body);
    val = new QMake::ValueAST(tst);
    val->value =  "VARIABLE" ;
    tst->identifier =  val ;
    val = new QMake::ValueAST(tst);
    val->value =  "=" ;
    tst->op =  val ;
    val = new QMake::ValueAST(tst);
    val->value = "FOO";
    tst->values.append(val);
    body->statements.append( tst );
    simple->body = body;
    testlist.append( simple );

TESTSCOPEBODY( scope, testlist, 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, nestedScope, "foobar :barfoo : VARIABLE = FOO\n")

BEGINTESTFUNCIMPL( ScopeTest, missingStatement, 1 )
    QMake::SimpleScopeAST* scope = dynamic_cast<QMake::SimpleScopeAST*>( ast->statements.first() );
TESTSCOPENAME( scope, "eval" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, missingStatement,
              "eval :\n" )



BEGINTESTFAILFUNCIMPL( ScopeTest, missingColon, "No colon" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( ScopeTest, missingColon,
              "eval \n" )

void ScopeTest::strangeScopeNames()
{
    QMake::Driver d;
    d.setContent( "linux-gcc++-* {\n  VARIABLE = FOO\n}\n" );
    bool ret = d.parse( &ast );
    QVERIFY( ret );
}
