/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    qDebug() << "Beginning Test Function"; \
    QFETCH( QString, project );\
    QMake::Driver d; \
    d.setContent( project ); \
    bool ret = d.parse( &ast );\
    QVERIFY( ret );\
    QVERIFY( ast->statements.count() == astcount );

#define BEGINTESTFAILFUNCIMPL( classname, funcname, comment ) \
void classname::funcname()\
{\
    qDebug() << "Beginning Test FAIL Function"; \
    QFETCH( QString, project );\
    QMake::Driver d; \
    d.setContent( project ); \
    bool ret = d.parse( &ast );\
    QEXPECT_FAIL( "", comment, Continue );\
    QVERIFY( ret );


#define ENDTESTFUNCIMPL }

#define TESTASSIGNMENT( ast, var, opval, valcount ) \
    QVERIFY( ast != nullptr );\
    QVERIFY( ast->identifier->value == var );\
    QVERIFY( ast->op->value == opval );\
    QVERIFY( ast->values.count() == valcount );

#define TESTFUNCNAME( scopeast, funcname ) \
    QVERIFY( scopeast ); \
    if( QMake::SimpleScopeAST* simpleast = dynamic_cast<QMake::SimpleScopeAST*>( scopeast ) ) \
    { \
        QVERIFY( simpleast->identifier->value == funcname );\
    } else if( QMake::FunctionCallAST* funast = dynamic_cast<QMake::FunctionCallAST*>( scopeast ) ) \
    { \
        QVERIFY( funast->identifier->value == funcname );\
    }

#define TESTSCOPENAME( scopeast, scopename ) \
    QVERIFY( scopeast ); \
    QVERIFY( scopeast->identifier->value == scopename );

#define TESTOROP( scopeast, funclist ) \
    for( int i = 0; i < funclist.size(); i++) \
    {\
        QVERIFY( i < scopeast->scopes.count() );\
        if( QMake::SimpleScopeAST* simpleast = dynamic_cast<QMake::SimpleScopeAST*>( scopeast->scopes.at(i) ) ) \
        { \
            QVERIFY( simpleast->identifier->value == funclist.at(i) );\
        } else if( QMake::FunctionCallAST* funast = dynamic_cast<QMake::FunctionCallAST*>( scopeast->scopes.at(i) ) ) \
        { \
            QVERIFY( funast->identifier->value == funclist.at(i) );\
        } \
    }

#define TESTSCOPEBODY(scope, teststmts, stmtcount)                                                                     \
    QVERIFY(scope->body != nullptr);                                                                                   \
    QVERIFY(scope->body->ifStatements.count() == stmtcount);                                                           \
    matchScopeBodies(scope->body->ifStatements, teststmts);                                                            \
    qDeleteAll(teststmts);

#define TESTSCOPEAST( scope, testscope ) \
    QVERIFY( scope ); \
    QVERIFY( testscope ); \
    QMake::SimpleScopeAST* simple = dynamic_cast<QMake::SimpleScopeAST*>( scope ); \
    QMake::SimpleScopeAST* testsimple = dynamic_cast<QMake::SimpleScopeAST*>( scope ); \
    QMake::FunctionCallAST* fun = dynamic_cast<QMake::FunctionCallAST*>( scope ); \
    QMake::FunctionCallAST* testfun = dynamic_cast<QMake::FunctionCallAST*>( scope ); \
    QVERIFY( ( simple && testsimple ) || ( fun && testfun ) ); \
    if( simple ) \
    { \
        QVERIFY( simple->identifier->value == testsimple->identifier->value ); \
    } else \
    { \
        QVERIFY( fun->identifier->value == testfun->identifier->value ); \
    }

#define TESTOROPAST(orop,testorop) \
    for(int i = 0; i < orop->scopes.count(); i++ ) \
    { \
        QVERIFY( i < testorop->scopes.count() ); \
        TESTSCOPEAST( orop->scopes.at(i), testorop->scopes.at(i) ) \
    }

void matchScopeBodies(const QList<QMake::StatementAST*>&,
                             const QList<QMake::StatementAST*>& );

#endif

