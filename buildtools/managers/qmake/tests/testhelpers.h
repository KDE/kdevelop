/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#ifndef QMAKETESTMACROS_H
#define QMAKETESTMACROS_H

namespace QMake{
    class StatementAST;
}

template <typename T> class QList;

#define FUNCDEF( funcname )\
        void  funcname();\
        void  funcname##_data();

#define DATAFUNCIMPL( classname, funcname, data ) \
void classname::funcname##_data()\
{\
    QTest::addColumn<QString>( "project" );\
    QTest::addColumn<QString>( "output" );\
    QTest::newRow( "row1" ) << data << data; \
}

#define BEGINTESTFUNCIMPL( classname, funcname, astcount ) \
void classname::funcname()\
{\
    QFETCH( QString, project );\
    QFETCH( QString, output );\
    QMake::Driver d; \
    d.setContent( project ); \
    bool ret = d.parse( ast );\
    QVERIFY( ret );\
    QVERIFY( ast->statements().count() == astcount );\
    QString result;\
    ast->writeToString(result);\
    QVERIFY( result == output );

#define BEGINTESTFAILFUNCIMPL( classname, funcname, comment ) \
void classname::funcname()\
{\
    QFETCH( QString, project );\
    QFETCH( QString, output );\
    QMake::Driver d; \
    d.setContent( project ); \
    bool ret = d.parse( ast );\
    QEXPECT_FAIL( "", comment, Continue );\
    QVERIFY( ret );


#define ENDTESTFUNCIMPL }

#define TESTASSIGNMENT( ast, var, opval, valcount, joinvalues ) \
    QVERIFY( ast != 0 );\
    QVERIFY( ast->variable() == var );\
    QVERIFY( ast->op() == opval );\
    QVERIFY( ast->values().count() == valcount );\
    QVERIFY( ast->values().join("") == joinvalues );

#define TESTFUNCNAME( scopeast, funcname ) \
    QVERIFY( scopeast ); \
    QVERIFY( scopeast->functionName() == funcname );

#define TESTSCOPENAME( scopeast, scopename ) \
    QVERIFY( scopeast ); \
    QVERIFY( scopeast->scopeName() == scopename );

#define TESTOROP( scopeast, funcname1, funcname2 ) \
    QVERIFY( scopeast->leftCall() != 0 ); \
    QVERIFY( scopeast->rightCall() != 0 ); \
    QMake::FunctionCallAST* leftfunccall = scopeast->leftCall(); \
    QMake::FunctionCallAST* rightfunccall = scopeast->rightCall(); \
    QVERIFY( leftfunccall->functionName() == funcname1 ); \
    QVERIFY( rightfunccall->functionName() == funcname2 );

#define TESTFUNCARGS( funccall, arglist ) \
    QVERIFY( funccall != 0 ); \
    QVERIFY( funccall->arguments() == arglist );

#define TESTSCOPEBODY( scope, teststmts, stmtcount ) \
    QVERIFY( scope->scopeBody() != 0 ); \
    QVERIFY( scope->scopeBody()->statements().count() == stmtcount ); \
    matchScopeBodies(scope->scopeBody()->statements(), teststmts);


void matchScopeBodies( QList<QMake::StatementAST*>,
                             QList<QMake::StatementAST*> );

#endif
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

