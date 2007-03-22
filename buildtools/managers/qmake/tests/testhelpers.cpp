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
            QVERIFY( ( scope->functionCall() && testscope->functionCall() )
                    || ( scope->scopeName() == testscope->scopeName() ) );
            if( scope->functionCall() && testscope->functionCall() )
            {
                TESTFUNCNAME( scope, testscope->functionCall()->functionName() )
                TESTFUNCARGS( scope->functionCall(), testscope->functionCall()->arguments() )
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
