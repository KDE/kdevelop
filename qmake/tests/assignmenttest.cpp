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

QTEST_MAIN( AssignmentTest )

AssignmentTest::AssignmentTest( QObject* parent )
        : QObject( parent )
{}

AssignmentTest::~AssignmentTest()
{}

void AssignmentTest::simpleParsed()
{
    QFETCH( QString, project );
    QFETCH( QString, output );
    QMake::ProjectAST* a = new QMake::ProjectAST();
    int ret = QMake::Driver::parseString( project, a );
    QVERIFY( ret == 0 );
    QVERIFY( a->statements().count() == 1 );
    QString result;
    a->writeToString(result);
    QVERIFY( result == output );
    QMake::AssignmentAST* assignment = dynamic_cast<QMake::AssignmentAST*>( a->statements().first() );
    QVERIFY( assignment != 0 );
    QVERIFY( assignment->variable() == "VAR" );
    QVERIFY( assignment->op() == " = " );
    QVERIFY( assignment->values().count() == 1 );
    QVERIFY( assignment->values().first() == "VALUE" );
}

void AssignmentTest::simpleParsed_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::addColumn<QString>( "output" );
    QTest::newRow( "row1" ) << "VAR = VALUE\n" << "VAR = VALUE\n";
}

#include "assignmenttest.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
