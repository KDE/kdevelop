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

#include "testhelpers.h"

#include <QtCore/QList>
#include <QtTest/QtTest>
#include "qmakeast.h"

void matchScopeBodies( QList<QMake::StatementAST*> realbody,
                             QList<QMake::StatementAST*> testbody )
{
    QVERIFY( realbody.count() == testbody.count() );
    int i = 0;
    QMake::AssignmentAST* assign;
    QMake::ScopeAST* scope;
    QMake::AssignmentAST* testassign;
    QMake::ScopeAST* testscope;

    foreach( QMake::StatementAST* ast, realbody )
    {
        scope = dynamic_cast<QMake::ScopeAST*>(ast);
        testscope = dynamic_cast<QMake::ScopeAST*>( testbody.at( i ) );

        if( scope && testscope )
        {
            QMake::FunctionCallAST* call = dynamic_cast<QMake::FunctionCallAST*>(scope);
            QMake::FunctionCallAST* testcall = dynamic_cast<QMake::FunctionCallAST*>(testscope);
            QMake::SimpleScopeAST* simple = dynamic_cast<QMake::SimpleScopeAST*>(scope);
            QMake::SimpleScopeAST* testsimple = dynamic_cast<QMake::SimpleScopeAST*>(testscope);
            QMake::OrAST* orop = dynamic_cast<QMake::OrAST*>(scope);
            QMake::OrAST* testorop = dynamic_cast<QMake::OrAST*>(testscope);
            QVERIFY( ( call && testcall )
                || ( simple && testsimple )
                || ( orop && testorop ) );
            if( call && testcall )
            {
                TESTFUNCNAME( call, testcall->functionName() )
                TESTFUNCARGS( call, testcall->arguments() )
            }else if( simple && testsimple )
            {

            }else if( orop && testorop )
            {
                TESTFUNCNAME( orop->leftCall(),
                                testorop->leftCall()->functionName() )
                TESTFUNCNAME( orop->rightCall(),
                                testorop->rightCall()->functionName() )
                TESTFUNCARGS( orop->leftCall(),
                                testorop->leftCall()->arguments() )
                TESTFUNCARGS( orop->rightCall(),
                                testorop->rightCall()->arguments() )
            }
            QVERIFY( ( scope->scopeBody() && testscope->scopeBody() )
                        || ( !scope->scopeBody() && !testscope->scopeBody() ) );
            if( scope->scopeBody() && testscope->scopeBody() )
            {
                QList<QMake::StatementAST*> bodylist;
                QList<QMake::StatementAST*> testbodylist;
                bodylist = scope->scopeBody()->statements();
                testbodylist = testscope->scopeBody()->statements();
                matchScopeBodies( bodylist, testbodylist );
            }
        }
        assign = dynamic_cast<QMake::AssignmentAST*>(ast);
        testassign = dynamic_cast<QMake::AssignmentAST*>( testbody.at( i ) );
        if( assign && testassign )
        {
            TESTASSIGNMENT( assign, testassign->variable(), testassign->op(),
                            testassign->values().count(), testassign->values().join("") )
        }
        i++;
    }
}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
