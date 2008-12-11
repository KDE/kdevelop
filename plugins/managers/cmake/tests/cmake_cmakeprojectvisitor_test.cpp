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
#include <language/duchain/indexedstring.h>

QTEST_KDEMAIN_CORE(CMakeProjectVisitorTest)

using namespace KDevelop;

#undef TRUE
#undef FALSE

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
    QTest::newRow("varinvar") << "${${a}${b}a" << (QStringList() << "a" << "b");
}

void CMakeProjectVisitorTest::testVariables()
{
    QFETCH(QString, input);
    QFETCH(QStringList, result);
    
    QStringList name;
    QList<CMakeProjectVisitor::IntPair> variables =CMakeProjectVisitor::parseArgument(input);
    
//     qDebug() << "kakakaka" << result << variables;
    QCOMPARE(result.count(), variables.count());
    if(!variables.isEmpty())
        QCOMPARE(1, variables.last().level);
    
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
            "#message(STATUS \"ooooo- ${aaa} ${bbb} ${ccc} ${ddd}\")\n"
            "find_path(eee stdio.h /usr/include)\n"
            "find_library(fff stdio.h /usr/include)\n"
            "find_program(ggg gcc /usr/gcc)\n" << cacheValues << results;
            
    cacheValues.clear();
    results.clear();
    results << StringPair("FOOBAR", "ORT Basket Is Strange ABORT");
    results << StringPair("RES", "Ok");
    results << StringPair("BARFOO", "ORT Is Basket Strange? ABORT");
    results << StringPair("BARFOO_MATCH", "Basket Is");
    results << StringPair("BARFOO_MATCHALL", "Basket Is;Basket Is;Basket Is;Basket Is");
    QTest::newRow("string") << 
            "set(FOOBAR \"ORT Basket Is Strange ABORT\")\n"
            "if( FOOBAR MATCHES \"^ORT Bas\")\n"
            "  set(RES Ok)\n"
            "else( FOOBAR MATCHES \"^ORT Bas\")\n"
            "  set(RES Wrong)\n"
            "endif( FOOBAR MATCHES \"^ORT Bas\")\n"
            "string( REGEX REPLACE \"Basket ([a-zA-Z]*) ([a-zA-Z]*)\" \"\\\\1 Basket \\\\2?\" BARFOO ${FOOBAR})\n"
            "string( REGEX MATCH \"Basket Is\" BARFOO_MATCH ${FOOBAR} ${RES} ${FOOBAR})\n"
            "string( REGEX MATCHALL \"Basket Is\" BARFOO_MATCHALL ${FOOBAR} \"${FOOBAR}${RES}${FOOBAR}\" ${FOOBAR})\n"
            << cacheValues << results;
            
    
    cacheValues.clear();
    results.clear();
    results << StringPair("kkk", "abcdef");
    QTest::newRow("abc") << "set(a abc)\n"
                            "set(b def)\n"
                            "SET(kkk \"${a}${b}\")\n" << cacheValues << results;
    
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
    results << StringPair("b", "k");
    QTest::newRow("envCC") <<   "set(a $ENV{PATH})\n"
                                "if(DEFINED a)\n"
                                "   set(b k)\n"
                                "endif(DEFINED a)\n"<< cacheValues << results;
    
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
    results << StringPair("res", "222aaaa333");
    QTest::newRow("invar_concatenation") << "set(abc aaaa)\n"
                                            "set(kk b)\n"
                                            "set(res \"222${a${kk}c}333\")\n" << cacheValues << results;
    
                                        
    cacheValues.clear();
    results.clear();
    results << StringPair("res", "oooaaaa");
    QTest::newRow("composing") <<   "set(tt aaaa)\n"
                                    "set(a t)\n"
                                    "set(b t)\n"
                                    "set(res ooo${${a}${b}})\"\n" << cacheValues << results;
    cacheValues.clear();
    results.clear();
    results << StringPair("res", "aaaaaa");
    QTest::newRow("composing1") <<  "set(tt aaaaa)\n"
                                    "set(a t)\n"
                                    "set(res \"${${a}${a}}a\")\n" << cacheValues << results;
                                    
    cacheValues.clear();
    results.clear();
    results << StringPair("_deps", "");
    QTest::newRow("mad") << "set(_deps tamare)\n"
                            "aux_source_directory(/tmp _deps)\n"//any unimplemented method
                            "set(_deps ${_deps})\n"
                            "set(${${a}${b}a 33)\n" << cacheValues << results;
                            
    cacheValues.clear();
    results.clear();
    results << StringPair("res", "1");
    QTest::newRow("parenthesed condition") <<
                            "set(ONE TRUE)\n"
                            "set(ZERO FALSE)\n"
                            "if(( ONE AND ZERO ) OR ( ZERO OR ONE ))\n"
                               "set(res 1)\n"
                            "endif(( ONE AND ZERO ) OR ( ZERO OR ONE ))\n"
                             << cacheValues << results;
                             
                             cacheValues.clear();
    results.clear();
    results << StringPair("res", "1");
    results << StringPair("end", "1");
    QTest::newRow("full conditional") <<
                            "set(ONE TRUE)\n"
                            "set(ZERO FALSE)\n"
                            "if(ONE)\n"
                               "set(res 1)\n"
                               "set(res 1)\n"
                            "else(ONE)\n"
                               "set(res 0)\n"
                               "set(res 0)\n"
                            "endif(ONE)\n"
                            "set(end 1)\n"
                             << cacheValues << results;
                             
    results.clear();
    results << StringPair("res", "1");
    results << StringPair("end", "1");
    QTest::newRow("full conditional.false") <<
                            "set(ONE TRUE)\n"
                            "set(ZERO FALSE)\n"
                            "if(ZERO)\n"
                               "set(res 0)\n"
                               "set(res 0)\n"
                            "else(ZERO)\n"
                               "set(res 1)\n"
                               "set(res 1)\n"
                            "endif(ZERO)\n"
                            "set(end 1)\n"
                             << cacheValues << results;
                             
    results.clear();
    QTest::newRow("no_endif") <<
                            "set(ZERO FALSE)\n"
                            "if(ZERO)\n"
                               "set(res 0)\n"
                               "set(res 0)\n"
                             << cacheValues << results;
                             
    results.clear();
    QTest::newRow("no_endwhile") <<
                            "set(VAR TRUE)\n"
                            "while(VAR)\n"
                               "set(res 0)\n"
                               "set(res 0)\n"
                               "set(VAR FALSE)\n"
                             << cacheValues << results;
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

#include "cmake_cmakeprojectvisitor_test.moc"
