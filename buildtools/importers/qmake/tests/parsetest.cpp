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

#include "parsetest.h"
#include "qmakeparser.h"

QTEST_MAIN( ParseTest )

ParseTest::ParseTest( QObject* parent )
        : QObject( parent )
{}

ParseTest::~ParseTest()
{}

void ParseTest::successSimpleProject()
{
    QFETCH( QString, project );
    QMake::ProjectAST* a;
    int ret = QMake::Parser::parseString( project, &a );
    QVERIFY( ret == 0 );
}

void ParseTest::successSimpleProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "VAR = VALUE\n"
        "func1(arg1)\n";
}

void ParseTest::failSimpleProject()
{
    QFETCH( QString, project );
    QMake::ProjectAST* a;
    int ret = QMake::Parser::parseString( project, &a );
    QVERIFY( ret != 0 );
}

void ParseTest::failSimpleProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "VAR  ";
}

void ParseTest::successFullProject()
{
    QFETCH( QString, project );
    QMake::ProjectAST* a;
    int ret = QMake::Parser::parseString( project, &a );
    QVERIFY( ret == 0 );
}

void ParseTest::successFullProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "#Comment\n"
        "VARIABLE1 = Value1 Value2\n"
        "VARIABLE2= Value1 Value2\n"
        "VARIABLE3 =Value1 Value2\n"
        "VARIABLE4=Value1 Value2\n"
        "VARIABLE = Value1 Value2 #some comment\n"
        "VARIABLE = $$Value1 $(Value2) $${Value3} #some comment\n"
        "VARIABLE = $$Value1 $(Value2) $${Value3} \\\nValue4\n"
        "message( foo, bar, $$foobar( foo, $$FOOBAR ), $${FOOBAR}, $(SHELL) ) : FO=0\n"
        "message( foo, bar, $$foobar( foo, $$FOOBAR ), $${FOOBAR}, $(SHELL) ) {  \n"
        "FOO = bar\n"
        "}\n"
        "do()";
}

void ParseTest::failFullProject()
{
    QFETCH( QString, project );
    QMake::ProjectAST* a;
    int ret = QMake::Parser::parseString( project, &a );
    QVERIFY( ret != 0);
}

void ParseTest::failFullProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "#Comment\n"
        "VARIABLE1 = Value1 Value2\n"
        "VARIABLE2= Value1 Value2\n"
        "VARIABLE3 =Value1 Value2\n"
        "VARIABLE4=Value1 Value2\n"
        "VARIABLE4=Value1 Value2 \\\n"
        "  Value3 Value4\n"
        "fo()\n{\n"
        "VARIABLE = Value1 Value2 \\#some comment\n"
        "\n";
}

#include "parsetest.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

