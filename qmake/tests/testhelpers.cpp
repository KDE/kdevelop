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
#include "qmakeast.h"

bool matchScopeBodies( QList<QMake::StatementAST*> realbody,
                             QList<QMake::StatementAST*> testbody )
{
    if( realbody.count() != testbody.count() )
        return false;
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
            if( ( scope->scopeBody() && !testscope->scopeBody() )
                    || ( !scope->scopeBody() && testscope->scopeBody() ) )
                return false;
            if( scope->scopeBody() && testscope->scopeBody() )
            {
                QList<QMake::StatementAST*> bodylist;
                QList<QMake::StatementAST*> testbodylist;
                bodylist = scope->scopeBody()->statements();
                testbodylist = testscope->scopeBody()->statements();
                if( !matchScopeBodies( bodylist, testbodylist ) )
                    return false;
            }
        }
        assign = dynamic_cast<QMake::AssignmentAST*>(ast);
        testassign = dynamic_cast<QMake::AssignmentAST*>( testbody.at( i ) );
        if( assign && testassign )
        {
            if( assign->variable() != testassign->variable()
                    || assign->op() != testassign->op()
                    || assign->values() != testassign->values() )
                return false;
        }
        i++;
    }
    return true;
}
