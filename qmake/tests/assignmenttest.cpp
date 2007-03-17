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

#include "assignmenttest.h"
#include "qmakedriver.h"
#include "qmakeast.h"

#include "testmacros.h"

// VARIABLE += " value "
// VARIABLE += " value ( " -> Only 1 parenthesis
// VARIABLE = value1=value++

QTEST_MAIN( AssignmentTest )

AssignmentTest::AssignmentTest( QObject* parent )
    : QObject( parent ), ast(0)
{}

AssignmentTest::~AssignmentTest()
{}

BEGINTESTFUNCIMPL( AssignmentTest, simpleParsed, 1)
    QMake::AssignmentAST* assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements().first() );
TESTASSIGNMENT( assignment, "VAR", " = ", 1, "VALUE" )
ENDTESTFUNCIMPL

DATAFUNCIMPL(AssignmentTest, simpleParsed, "VAR = VALUE\n")

BEGINTESTFUNCIMPL( AssignmentTest, assignInValue, 1)
    QMake::AssignmentAST* assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements().first() );
TESTASSIGNMENT( assignment, "VARIABLE", " = ", 1, "value1=value++" )
ENDTESTFUNCIMPL

DATAFUNCIMPL(AssignmentTest, assignInValue, "VARIABLE = value1=value++\n")

BEGINTESTFUNCIMPL( AssignmentTest, commentCont, 1)
    QMake::AssignmentAST* assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements().first() );
    qDebug() << assignment->values().join("");
TESTASSIGNMENT( assignment, "VARIABLE", " = ", 4, "foobar\\#somecomment\nnextval")
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, commentCont, "VARIABLE = foobar\\#somecomment\nnextval\n" )

void AssignmentTest::init()
{
    ast = new QMake::ProjectAST();
    QVERIFY( ast != 0 );
}

void AssignmentTest::cleanup()
{
    delete ast;
    ast = 0;
    QVERIFY( ast == 0 );
}

#include "assignmenttest.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
