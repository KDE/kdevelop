/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include "cmake_cmakecondition_test.h"
#include "cmakecondition.h"
#include <QStringList>

QTEST_MAIN( CMakeConditionTest )

CMakeConditionTest::CMakeConditionTest()
{
    m_vars=new VariableMap();
    m_vars->insert("TRUE", QStringList("TRUE"));
    m_vars->insert("FALSE", QStringList("FALSE"));
}

CMakeConditionTest::~CMakeConditionTest()
{
    delete m_vars;
}
        
        
void CMakeConditionTest::testGoodParse()
{
    QFETCH( QStringList, expression );
    CMakeCondition cond(m_vars);
    QVERIFY( cond.condition(expression) );
}

void CMakeConditionTest::testGoodParse_data()
{
    QStringList condition;
    
    QTest::addColumn<QStringList>( "expression" );
    QTest::newRow( "variable check" ) << QStringList("TRUE");
    QTest::newRow( "and" ) << QString("TRUE;AND;TRUE").split(";");
    QTest::newRow( "not+and" ) << QString("NOT;FALSE;AND;TRUE").split(";");
    QTest::newRow( "not+and+command" ) << QString("NOT;FALSE;AND;COMMAND;/usr/bin/ls").split(";");
    QTest::newRow( "not+and+exists" ) << QString("NOT;FALSE;AND;EXISTS;/etc/group").split(";");
}

void CMakeConditionTest::testBadParse()
{
    QFETCH( QStringList, expression );
    QVERIFY( true );
}

void CMakeConditionTest::testBadParse_data()
{//TODO
    QStringList condition;
    
    QTest::addColumn<QStringList>( "expression" );
    QTest::newRow( "variable check" ) << QStringList("FALSE");
    QTest::newRow( "ban wrong name" ) << QString("NOT;NOT").split(";");
}

#include "cmake_cmakecondition_test.moc"
