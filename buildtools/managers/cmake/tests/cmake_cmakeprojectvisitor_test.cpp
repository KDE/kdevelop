/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol Gonzalez <aleixpol@gmail.com>
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

#include "cmake_cmakeprojectvisitor_test.h"
#include "cmakeast.h"
#include "cmakeprojectvisitor.h"

#include <identifier.h>
#include <declaration.h>
#include <duchainlock.h>
#include <duchain.h>
#include <simplerange.h>
#include <language/duchain/dumpchain.h>

QTEST_MAIN( CMakeProjectVisitorTest )

using namespace KDevelop;

CMakeProjectVisitorTest::CMakeProjectVisitorTest()
{
    m_fakeContext = new TopDUContext(HashedString("test"), SimpleRange(0,0,0,0));
}

/*void CMakeProjectVisitorTest::testVariables_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("containsVariable");
    QTest::addColumn<QString>("result");
    
    QTest::newRow("A variable alone") << "${MY_VAR}" << true << "MY_VAR";
    QTest::newRow("Contains a variable") << "${MY_VAR}/lol" << true << "MY_VAR";
    QTest::newRow("Nothing") << "aaaa${aaaa" << false << "";
    
}

void CMakeProjectVisitorTest::testVariables()
{
    QFETCH(QString, input);
    QFETCH(bool, containsVariable);
    QFETCH(QString, result);
    
    QHash<QString, QStringList> vars;
    vars.insert("MY_VAR", QStringList("val"));

    QCOMPARE(CMakeProjectVisitor::resolveVariables(QStringList(input), &vars), result);
}*/

typedef QPair<QString, QString> StringPair;
Q_DECLARE_METATYPE(QList<StringPair>)

void CMakeProjectVisitorTest::testRun_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<StringPair> >("cache");
    QTest::addColumn<QList<StringPair> >("results");

    QList<StringPair> cacheValues;
    cacheValues << StringPair("aaa", "cmd");
    cacheValues << StringPair("bbb", "cmd");
    cacheValues << StringPair("ccc", "cmd");
    cacheValues << StringPair("ddd", "cmd");
    cacheValues << StringPair("eee", "cmd");
    cacheValues << StringPair("fff", "cmd");
    cacheValues << StringPair("ggg", "cmd");

    QList<StringPair> results;
    results << StringPair("${aaa}", "cmd");
    results << StringPair("${bbb}", "cmd");
    results << StringPair("${ccc}", "cmd");
    results << StringPair("${ddd}", "script");
    results << StringPair("${eee}", "cmd");
    results << StringPair("${fff}", "cmd");
    results << StringPair("${ggg}", "cmd");
    QTest::newRow("cache") <<
            "project(simpletest)\n"
            "cmake_minimum_required(VERSION 2.6)\n"
            "find_file(aaa stdio.h /usr/include)\n"
            "set(bbb script CACHE STRING HELLO)\n"
            "set(ccc script CACHE STRING HELLO FORCE)\n"
            "set(ddd script)\n"
            "find_path(eee stdio.h /usr/include)\n"
            "find_library(fff stdio.h /usr/include)\n"
            "find_program(ggg gcc /usr/gcc)\n"
            "#message(STATUS \"ooooo- ${aaa} ${bbb} ${ccc} ${ddd}\")\n" << cacheValues << results;
}

void CMakeProjectVisitorTest::testRun()
{
    QFETCH(QString, input);
    QFETCH(QList<StringPair>, cache);
    QFETCH(QList<StringPair>, results);
    
    QFile file("cmake_visitor_test");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    
    QTextStream out(&file);
    out << input;
    file.close();
    CMakeFileContent code=CMakeListsParser::readCMakeFile(file.fileName());
    file.remove();
    QVERIFY(code.count() != 0);
    
    MacroMap mm;
    VariableMap vm;
    CacheValues val;
    foreach(const StringPair& v, cache)
        val[v.first]=v.second;
    
    CMakeProjectVisitor v(file.fileName(), m_fakeContext);
    v.setVariableMap(&vm);
    v.setMacroMap(&mm);
    v.setCacheValues( &val );
    v.walk(code, 0);

    foreach(const StringPair& vp, results)
    {
//         qDebug() << vp << v.resolveVariable(vp.first).join("");
        QCOMPARE(v.resolveVariable(vp.first).join(""), vp.second);
    }

}

#include "cmake_cmakeprojectvisitor_test.moc"
