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

#include "cmakeconditiontest.h"
#include "cmakecondition.h"
#include <QStringList>

QTEST_MAIN( CMakeConditionTest )

CMakeConditionTest::CMakeConditionTest()
{
    m_vars.insert("TRUE", QStringList("TRUE"));
    m_vars.insert("FALSE", QStringList("FALSE"));
    
    m_vars.insert("EMPTY", QStringList());
    m_vars.insert("ZERO", QStringList("0"));
    m_vars.insert("ONE", QStringList("1"));
    m_vars.insert("EXP", QStringList("-llala -lexpression"));
    m_vars.insert("UNFORTUNATE-NOTFOUND", QStringList("TRUE"));
    
    m_vars.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList("./"));
    m_vars.insert("MYTRUE", QStringList("NOT FALSE"));
    
    m_vars.insert("UNFOUNDVAR", QStringList("UNFOUNDVAR-NOTFOUND"));
    m_vars.insert("BLEP2", QStringList("TRUE"));
    
    Macro m;
    m.name = "testmacro";
    m.isFunction=false;
    m_macros.insert("testmacro", m);
}

CMakeConditionTest::~CMakeConditionTest()
{}

void CMakeConditionTest::testGoodParse()
{
    QFETCH( QStringList, expression );
    QFETCH( bool, result );
    
    CMakeProjectVisitor v(QString(), 0);
    v.setVariableMap( &m_vars );
    v.setMacroMap( &m_macros );
    v.setCacheValues( &m_cache );
    
    CMakeCondition cond(&v);
    QCOMPARE( cond.condition(expression), result );
}

void CMakeConditionTest::testGoodParse_data()
{
    QTest::addColumn<QStringList>( "expression" );
    QTest::addColumn<bool>( "result" );
    
    QTest::newRow( "one" ) << QString("1").split(" ") << true;
    QTest::newRow( "undefinedvar" ) << QStringList("IAMUNDEFINED") << false;
    QTest::newRow( "variable check" ) << QStringList("ONE") << true;
    QTest::newRow( "false variable check" ) << QStringList("ZERO") << false;
    QTest::newRow( "not" ) << QString("NOT;ZERO").split(";") << true;
    QTest::newRow( "not1" ) << QString("NOT;ONE").split(";") << false;
    QTest::newRow( "and" ) << QString("ONE;AND;ONE").split(";") << true;
    QTest::newRow( "false+and" ) << QString("ZERO;AND;ONE").split(";") << false;
    QTest::newRow( "and+false" ) << QString("ONE;AND;ZERO").split(";") << false;
    QTest::newRow( "not+and" ) << QString("NOT;ZERO;AND;ONE").split(";") << true;
    QTest::newRow( "not+and+command" ) << QString("NOT;ZERO;AND;COMMAND;testmacro").split(";") << true;
    QTest::newRow( "not+and+command" ) << QString("COMMAND;add_library").split(";") << true;
#ifdef Q_OS_WIN
    QTest::newRow( "not+and+exists" ) << QString("NOT;ZERO;AND;EXISTS;" + qApp->applicationDirPath() + "/cmakeconditiontest.exe").split(";") << true;
#else
    QTest::newRow( "not+and+exists" ) << QString("NOT;ZERO;AND;EXISTS;" + qApp->applicationDirPath() + "/cmakeconditiontest").split(";") << true;
#endif
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
    QTest::newRow( "empty+strequal" ) << QString("NOT;;STREQUAL;").split(";") << false;
    QTest::newRow( "weirdmatch" ) << QString("EXP MATCHES expression").split(" ") << true;
    QTest::newRow( "isabsolute+true" ) << QString("IS_ABSOLUTE /foo/bar").split(" ") << true;
    QTest::newRow( "isabsolute+false" ) << QString("IS_ABSOLUTE ../bar").split(" ") << false;
    QTest::newRow( "version_less" ) << QString("1.1 VERSION_LESS 1.3.1").split(" ") << true;
    QTest::newRow( "version_equal" ) << QString("1.3.1 VERSION_EQUAL 1.3.1").split(" ") << true;
    QTest::newRow( "version_greater" ) << QString("1.4 VERSION_GREATER 1.3.1").split(" ") << true;
    QTest::newRow( "version_greater" ) << QString("4.6.80 VERSION_GREATER 4.6").split(" ") << true;
    QTest::newRow( "detect_number" ) << QString("BLEP2").split(" ") << true;

    //parentheses: 2.6.3
    QTest::newRow( "parenthese0" ) << QString("ONE AND ( NOT ZERO OR ZERO )").split(" ") << true;
    QTest::newRow( "parenthese01" ) << QString("ZERO AND ( ZERO OR ZERO )").split(" ") << false;
    QTest::newRow( "parenthese1" ) << QString("( ONE AND NOT ZERO ) OR ZERO").split(" ") << true;
    QTest::newRow( "parenthese2" ) << QString("( ZERO AND NOT ZERO ) OR ZERO").split(" ") << false;
    QTest::newRow( "parenthese3" ) << QString("( ZERO AND ZERO ) OR ONE").split(" ") << true;
    QTest::newRow( "parenthese4" ) << QString("( ZERO AND ZERO ) OR ZERO").split(" ") << false;
    QTest::newRow( "parenthese5" ) << QString("( ONE AND ZERO ) OR ( ZERO OR ONE )").split(" ") << true;
    
    QTest::newRow( "case" ) << QString("NOT settings.kcfgc STREQUAL GENERATE_MOC AND NOT settings.kcfgc STREQUAL USE_RELATIVE_PATH").split(" ") << true;

    // Constants
    QTest::newRow( "false constant 1" ) << QStringList("") << false;
    QTest::newRow( "false constant 2" ) << QStringList("/a/path/to/somewhere") << false;
    QTest::newRow( "false constant 3" ) << QStringList("0") << false;
    QTest::newRow( "false constant 4" ) << QStringList("1 ") << false;
    QTest::newRow( "false constant 5" ) << QStringList("OFF") << false;
    QTest::newRow( "false constant 6" ) << QStringList("NO") << false;
    QTest::newRow( "false constant 7" ) << QStringList("FALSE") << false;
    QTest::newRow( "false constant 8" ) << QStringList("N") << false;
    QTest::newRow( "false constant 9" ) << QStringList("xxxx-NOTFOUND") << false;
    QTest::newRow( "false constant 10" ) << QStringList("faLsE") << false;
    QTest::newRow( "false constant 11" ) << QStringList("-0") << false;
    QTest::newRow( "false constant 12" ) << QStringList("UNFORTUNATE-NOTFOUND") << false;

    QTest::newRow( "true constant 1" ) << QStringList(" 10") << true;
    QTest::newRow( "true constant 2" ) << QStringList("10") << true;
    QTest::newRow( "true constant 3" ) << QStringList("1") << true;
    QTest::newRow( "true constant 4" ) << QStringList("ON") << true;
    QTest::newRow( "true constant 5" ) << QStringList("YeS") << true;
    QTest::newRow( "true constant 6" ) << QStringList("tRUe") << true;
    QTest::newRow( "true constant 7" ) << QStringList("Y") << true;
    QTest::newRow( "true constant 8" ) << QStringList("-2") << true;
    
    QTest::newRow( "notfound variable" ) << QStringList("UNFOUNDVAR") << false;
    
    QTest::newRow( "<empty> OR NOT <empty>" ) << QString(" OR NOT ").split(" ") << true;
}

void CMakeConditionTest::testBadParse()
{
    QFETCH( QStringList, expression );
    
    CMakeProjectVisitor v(QString(), 0);
    v.setVariableMap( &m_vars );
    v.setMacroMap( &m_macros );
    v.setCacheValues( &m_cache );
    
    CMakeCondition cond(&v);
    QCOMPARE( cond.condition(expression), false );
}

void CMakeConditionTest::testBadParse_data()
{
    QTest::addColumn<QStringList>( "expression" );
    QTest::newRow( "missing operator" ) << QString("MATCHES STUFF").split(" ");
    QTest::newRow( "OR NOT" ) << QString("OR NOT").split(" ");
}
