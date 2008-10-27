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
#include "cmakelistsparser.h"
#include <QString>
#include <qtest_kde.h>

QTEST_KDEMAIN_CORE(CMakeProjectVisitorTest)

using namespace KDevelop;

CMakeProjectVisitorTest::CMakeProjectVisitorTest()
 : CMakeProjectVisitor( QString(), new TopDUContext(IndexedString("test"), SimpleRange(0,0,0,0)))
{
    m_fakeContext = new TopDUContext(IndexedString("test"), SimpleRange(0,0,0,0));
}

void CMakeProjectVisitorTest::testVariables_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("result");
    
    QTest::newRow("a variable") << "${MY_VAR}" << QStringList("MY_VAR");
    QTest::newRow("env var") << "$ENV{MY_VAR}" << QStringList("MY_VAR");
    QTest::newRow("Contains a variable") << "${MY_VAR}/lol" << QStringList("MY_VAR");
    QTest::newRow("Contains a variable") << "${yipiee}#include <${it}>\n" << (QStringList("yipiee") << "it");
    QTest::newRow("Contains a variable") << "${a}${b}\n" << (QStringList("a") << "b");
    QTest::newRow("mess") << "{}{}{}}}}{{{{}${a}\n" << QStringList("a");
    QTest::newRow("Nothing") << "aaaa${aaaa" << QStringList();
    QTest::newRow("varinvar") << "${${${a}}}" << (QStringList() << "${${a}}" << "${a}" << "a");
}

void CMakeProjectVisitorTest::testVariables()
{
    QFETCH(QString, input);
    QFETCH(QStringList, result);
    
    QStringList name;
    QList<CMakeProjectVisitor::IntPair> variables =CMakeProjectVisitor::parseArgument(input);
    
//     qDebug() << "kakakaka" << result << variables;
    QCOMPARE(result.count(), variables.count());
    
    typedef QPair<int,int> IntPair;
    foreach(const CMakeProjectVisitor::IntPair& v, variables)
    {
        QString name=input.mid(v.first+1, v.second-v.first-1);
        if(!result.contains(name))
            qDebug() << "not a var:" << name;
        QVERIFY(result.contains(name));
    }
}

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
    results << StringPair("aaa", "cmd");
    results << StringPair("bbb", "cmd");
    results << StringPair("ccc", "cmd");
    results << StringPair("ddd", "script");
    results << StringPair("eee", "cmd");
    results << StringPair("fff", "cmd");
    results << StringPair("ggg", "cmd");
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
            
    
    cacheValues.clear();
    results.clear();
    results << StringPair("kkk", "abcdef");
    QTest::newRow("abc") << "set(a abc)\nset(b def)\nSET(kkk \"${a}${b}\")\n" << cacheValues << results;
    
    cacheValues.clear();
    results.clear();
    results << StringPair("kkk", "abcdef");
    QTest::newRow("defabc") << "set(a abc)\nset(b def)\nSET(kkk \"${kkk}${a}\")\nSET(kkk \"${kkk}${b}\")\n" << cacheValues << results;
    
    cacheValues.clear();
    results.clear();
    results << StringPair("_INCLUDE_FILES", "#include <a>\n"
                                            "#include <b>\n"
                                            "#include <c>\n");
    QTest::newRow("foreach") <<
            "set(_HEADER a b c)\n"
            "FOREACH (it ${_HEADER})\n"
            "    SET(_INCLUDE_FILES \"${_INCLUDE_FILES}#include <${it}>\n\")\n"
            "ENDFOREACH (it)\n" << cacheValues << results;
            
    cacheValues.clear();
    results.clear();
    results << StringPair("b", "abc");
    results << StringPair("c", "def");
    QTest::newRow("semicolons1") << "set(a abc;def)\n" 
                               "LIST(GET a 1 c)\nLIST(GET a 0 b)\n" << cacheValues << results;
   
    cacheValues.clear();
    results.clear();
    results << StringPair("a", "potatoe");
    results << StringPair("b", "def");
    QTest::newRow("varinvar") << "set(a potatoe)\n"
                                    "set(potatoe \"abc\")\n"
                                    "set(abc \"def\")\n"
                                    "set(b \"${${${a}}}\")\n)" << cacheValues << results;
    
    cacheValues.clear();
    results.clear();
    results << StringPair("a", "potatoe\n");
    QTest::newRow("envCC") <<   "set(a \"potatoe\\n\")"
                                "IF($ENV{CC} MATCHES \".+\")\n"
                                "  MESSAGE(STATUS \"we!\")\n"
                                "ENDIF($ENV{CC} MATCHES \".+\")\n" << cacheValues << results;
    
    cacheValues.clear();
    results.clear();
    results << StringPair("res", "${caca}");
    QTest::newRow("strange_var") <<   "set(caca aaaa)\n"
                                "set(v1 \"{ca\")\n"
                                "set(v2 \"ca}\")\n"
                                "set(res \"$${v1}${v2}\")\"\n" << cacheValues << results;
                                
    cacheValues.clear();
    results.clear();
    results << StringPair("res", "222aaaa333");
    QTest::newRow("concatenation") <<   "set(tt aaaa)\n"
                                "set(res \"222${tt}333\")\"\n" << cacheValues << results;
                                
    cacheValues.clear();
    results.clear();
    results << StringPair("res", "oooaaaa");
    QTest::newRow("composing") <<   "set(tt aaaa)\n"
                                    "set(a t)\n"
                                    "set(b t)\n"
                                    "set(res ooo${${a}${b}})\"\n" << cacheValues << results;
                                    
    cacheValues.clear();
    results.clear();
    results << StringPair("res", "oooaaaa");
    QTest::newRow("mad") << "set(ARGN tamare)"
                            "GET_SOURCE_FILE_PROPERTY(_deps ${_file} OBJECT_DEPENDS)\n"
                            "set(_deps ${_deps} ${ARGN})\n" << cacheValues << results;
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
        CMakeFunctionArgument arg;
        arg.value=vp.first;
        
        QCOMPARE(vm.value(vp.first).join(QString(";")), vp.second);
    }
}

void CMakeProjectVisitorTest::init()
{
    m_someCMakeLists = "foo_file_tmp_";
}

// helper
CMakeProjectVisitor* CMakeProjectVisitorTest::setupVisitor()
{
    MacroMap* mm = new MacroMap;
    VariableMap* vm = new VariableMap;
    CacheValues* val = new CacheValues;
    CMakeProjectVisitor* vtor = new CMakeProjectVisitor(m_someCMakeLists, m_fakeContext);
    vtor->setVariableMap(vm);
    vtor->setMacroMap(mm);
    vtor->setCacheValues(val);
    return vtor;
}

// helper
CMakeFileContent CMakeProjectVisitorTest::setupFileContent(const QString& content)
{
    QFile file(m_someCMakeLists);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    Q_ASSERT(file.isOpen());
    QTextStream out(&file);
    out << content;
    file.close();
    CMakeFileContent code = CMakeListsParser::readCMakeFile(m_someCMakeLists);
    file.remove();
    Q_ASSERT(code.count()!=0);
    return code;
}

void CMakeProjectVisitorTest::assertTestFound(CMakeProjectVisitor* visitor, 
        const QString& name, const QString& exe, const QStringList& arguments)
{
    QVERIFY(visitor->tests().contains(name));
    QCOMPARE(exe, visitor->testExecutable(name));
    QCOMPARE(arguments, visitor->testArguments(name));
}


// command
void CMakeProjectVisitorTest::addTest_single()
{
    QString content("add_test(foo foo.exe)\n");

    CMakeFileContent code = setupFileContent(content);
    CMakeProjectVisitor* vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(1, vtor->tests().count());
    assertTestFound(vtor, "foo", "foo.exe");
}

// command
void CMakeProjectVisitorTest::addTest_identicalAddExecutable()
{
    QString content(
        "add_executable(foo foo.cpp)\n"
        "add_test(foo foo.exe)\n");

    CMakeFileContent code = setupFileContent(content);
    CMakeProjectVisitor* vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(1, vtor->tests().count());
    assertTestFound(vtor, "foo", "foo.exe");
    QCOMPARE(1, vtor->targets().count());
    QVERIFY(vtor->targets().contains("foo"));

    content = QString(
        "add_test(foo foo.exe)\n"
        "add_executable(foo foo.cpp)\n");

    code = setupFileContent(content);
    vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(1, vtor->tests().count());
    assertTestFound(vtor, "foo", "foo.exe");
    QCOMPARE(1, vtor->targets().count());
    QVERIFY(vtor->targets().contains("foo"));
}

// command
void CMakeProjectVisitorTest::addTest_multiple()
{
    QString content(
        "add_test(foo foo.exe)\n"
        "add_test(zoo zoo.exe)\n"
        "add_test(moo moo.exe)\n");

    CMakeFileContent code = setupFileContent(content);
    CMakeProjectVisitor* vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(3, vtor->tests().count());
    assertTestFound(vtor, "foo", "foo.exe");
    assertTestFound(vtor, "zoo", "zoo.exe");
    assertTestFound(vtor, "moo", "moo.exe");
}

#define TDD_TODO QSKIP("Not implemented yet", SkipSingle)

// command
void CMakeProjectVisitorTest::addTest_flags()
{
    QString content(
        "add_test(foo foo.exe --test123 bar)\n");

    CMakeFileContent code = setupFileContent(content);
    CMakeProjectVisitor* vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(1, vtor->tests().count());
    assertTestFound(vtor, "foo", "foo.exe", QStringList() << "--test123" << "bar");
}

// command
void CMakeProjectVisitorTest::addTest_properties()
{
    TDD_TODO;

    QString content(
        "add_test(foo foo.exe)\n"
        "set_tests_properties(foo PROPERTIES WILL_FAIL)\n");

    content = QString(
        "add_test(foo foo.exe)\n"
        "set_tests_properties(foo PROPERTIES WILL_FAIL FAIL_REGULAR_EXPRESSION)\n");

    content = QString(
        "add_test(foo foo.exe)\n"
        "add_test(bar bar.exe)\n"
        "set_tests_properties(foo bar PROPERTIESFAIL_REGULAR_EXPRESSION)\n");

}

// command
void CMakeProjectVisitorTest::addTest_url()
{
    QString content(
        "add_test(foo /path/to/foo)\n");

    CMakeFileContent code = setupFileContent(content);
    CMakeProjectVisitor* vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(1, vtor->tests().count());
    assertTestFound(vtor, "foo", "/path/to/foo");

    content = QString(
        "add_test(foo file://dir/to/bar)\n");

    code = setupFileContent(content);
    vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(1, vtor->tests(). count());
    assertTestFound(vtor, "foo", "file://dir/to/bar");
}

// command
void CMakeProjectVisitorTest::addTest_kde4AddUnitTest()
{
    TDD_TODO;

    QString content(
        "find_package(KDE4 REQUIRED)\n"
        "kde4_add_unit_test(foo foo.cpp)\n");
}

// command
void CMakeProjectVisitorTest::addTest_variableUsage()
{
    QString content(
        "set(foo_var /foo/bar)\n"
        "add_test(foo ${foo_var})\n");

    CMakeFileContent code = setupFileContent(content);
    CMakeProjectVisitor* vtor = setupVisitor();

    vtor->walk(code, 0);

    QCOMPARE(1, vtor->tests().count());
    assertTestFound(vtor, "foo", "/foo/bar");
}

#include "cmake_cmakeprojectvisitor_test.moc"
