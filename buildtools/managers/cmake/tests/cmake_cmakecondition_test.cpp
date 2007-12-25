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
    
    m_vars->insert("EMPTY", QStringList());
    m_vars->insert("ZERO", QStringList("0"));
    m_vars->insert("ONE", QStringList("1"));
}

CMakeConditionTest::~CMakeConditionTest()
{
    delete m_vars;
}

void CMakeConditionTest::testGoodParse()
{
    QFETCH( QStringList, expression );
    QFETCH( bool, result );
    CMakeCondition cond(m_vars);
    QVERIFY( cond.condition(expression)==result );
}

void CMakeConditionTest::testGoodParse_data()
{
    QStringList condition;
    
    QTest::addColumn<QStringList>( "expression" );
    QTest::addColumn<bool>( "result" );
    
    QTest::newRow( "variable check" ) << QStringList("ONE") << true;
    QTest::newRow( "false variable check" ) << QStringList("ZERO") << false;
    QTest::newRow( "not" ) << QString("NOT;ZERO").split(";") << true;
    QTest::newRow( "not1" ) << QString("NOT;ONE").split(";") << false;
    QTest::newRow( "and" ) << QString("ONE;AND;ONE").split(";") << true;
    QTest::newRow( "false+and" ) << QString("ZERO;AND;ONE").split(";") << false;
    QTest::newRow( "and+false" ) << QString("ONE;AND;ZERO").split(";") << false;
    QTest::newRow( "not+and" ) << QString("NOT;ZERO;AND;ONE").split(";") << true;
    QTest::newRow( "not+and+command" ) << QString("NOT;ZERO;AND;COMMAND;/usr/bin/ls").split(";") << true;
    QTest::newRow( "not+and+exists" ) << QString("NOT;ZERO;AND;EXISTS;/etc/group").split(";") << true;
    QTest::newRow( "or" ) << QString("ONE;OR;ONE").split(";") << true;
    QTest::newRow( "false+or" ) << QString("ZERO;OR;ONE").split(";") << true;
    QTest::newRow( "false+or+false" ) << QString("ZERO;OR;ZERO").split(";") << false;
    QTest::newRow( "strequal" ) << QString("HOLA;STREQUAL;HOLA").split(";") << true;
    QTest::newRow( "not+streq" ) << QString("NOT;HOLA;STREQUAL;HOLA").split(";") << false;
    QTest::newRow( "not+or" ) << QString("NOT;ZERO;OR;ZERO").split(";") << true;
    QTest::newRow( "matches" ) << QString("-lapr-1;MATCHES;^-l").split(";") << true;
    QTest::newRow( "less" ) << QString("5;LESS;9").split(";") << true;
    QTest::newRow( "not+less" ) << QString("NOT;5;LESS;9").split(";") << false;
    QTest::newRow( "not+or+not" ) << QString("NOT;TRUE;OR;NOT;TRUE").split(";") << false;
    QTest::newRow( "empty" ) << QString("EMPTY").split(";") << false;
    QTest::newRow( "not+empty" ) << QString("NOT;EMPTY").split(";") << true;
}

#include "cmake_cmakecondition_test.moc"
