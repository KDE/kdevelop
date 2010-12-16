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
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <cmakecondition.h>
#include <cmakeparserutils.h>
#include <tests/autotestshell.h>
#include <astfactory.h>
#include <cmakeprojectdata.h>

QTEST_KDEMAIN_CORE(CMakeProjectVisitorTest)

using namespace KDevelop;

#undef TRUE //krazy:exclude=captruefalse
#undef FALSE //krazy:exclude=captruefalse

CMakeProjectVisitorTest::CMakeProjectVisitorTest()
 : CMakeProjectVisitor( QString(), 0)
{
    AutoTestShell::init();
    KDevelop::Core::initialize(0, KDevelop::Core::NoUi);
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
    QTest::newRow("varsinvar") << "${${a}${b}a" << (QStringList() << "a" << "b");
    QTest::newRow("varsinvar") << "${a${b}a}${a${b}a}" << (QStringList() << "a${b}a" << "b" << "a${b}a" << "b");
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
                             
    results.clear();
    results << StringPair("VAR", "FALSE");
    QTest::newRow("weirdIf") <<
                            "set(VAR FALSE)\n"
                            "if(VAR)\n"
                            "set(VAR a)\n"
                            "endif()\n"
                            << cacheValues << results;
                            
    results.clear();
    results << StringPair("GOOD", "TRUE");
    QTest::newRow("twoconditions") <<
                            "set(GOOD FALSE)\n"
                            "set(aaa ca)\n"
                            "set(aab co)\n"
                            "set(b a)\n"
//                             "message(STATUS \"${aaa}${aab} should be caco\")\n"
//                             "message(STATUS \"${a${b}a}${a${b}b} should be caco\")\n"
                            "if(\"${a${b}a}${a${b}b}\" STREQUAL caco )\n"
                            "  set(GOOD TRUE)\n"
                            "endif()\n"
                            << cacheValues << results;
                           
    results.clear();                        
    results << StringPair("str", "babababababa");
    QTest::newRow("replace") <<
                            "set(str tatatttatatttata)\n"
                            "string(REGEX REPLACE \"t+\" \"b\" str ${str})\n"
                            << cacheValues << results;
                            
    results.clear();
    results << StringPair("str", "potatoe\"\npotatoe");
    QTest::newRow("scaping") <<
                            "set(str potatoe\\\"\\npotatoe)\n"
                            << cacheValues << results;
    results.clear();
    results << StringPair("str", "123");
    QTest::newRow("regex") <<
                            "STRING(REGEX REPLACE \"QT_LIBINFIX *= *([^\\n]*)\" \"\\\\1\" str \"QT_LIBINFIX = 123\")\n"
                            << cacheValues << results;
                            
    //This test should probably be linux-only
    results.clear();
    results << StringPair("good", "TRUE");
    QTest::newRow("ifexists") <<
                            "set(good FALSE)\n"
                            "if(EXISTS Makefile)\n" //Relative
                            "   set(good TRUE)\n"
                            "endif()\n"
                            
                            "if(EXISTS /proc)\n"    //Absolute
                            "   set(good TRUE)\n"
                            "else()\n"
                            "   set(good FALSE)\n"
                            "endif()\n"
                            
                            "if(EXISTS /pppppppp)\n" //Doesn't exist absolute
                            "   set(good FALSE)\n"
                            "else()\n"
                            "   set(good TRUE)\n"
                            "endif()\n"
                            
                            "if(EXISTS MAAAAA)\n" //Doesn't exist relative
                            "   set(good FALSE)\n"
                            "else()\n"
                            "   set(good TRUE)\n"
                            "endif()\n"
                            << cacheValues << results;
    
    //This test should probably be linux-only
    results.clear();
    results << StringPair("output", "/lib");
    QTest::newRow("get_filename_component") <<
                            "get_filename_component(output /usr/lib/libdl.so PATH)\n"
                            << cacheValues << results;
                            
    results.clear();
    QTest::newRow("unfinished function") <<
                            "function(test)\n"
                            << cacheValues << results;
    results.clear();

    
    results << StringPair("args", "one;two;three;four");
    results << StringPair("args2", "one;two;\"three;four\"");
    QTest::newRow("separate arguments") <<
                            "SET(args \"one two three four\")\n"
                            "SET(args2 \"one two \\\"three four\\\"\")\n"
                            "SEPARATE_ARGUMENTS(args)\n"
                            "SEPARATE_ARGUMENTS(args2)\n"
                            << cacheValues << results;
    
    results.clear();
    QTest::newRow("break") <<
                            "while(1)\n"
                            "break()\n"
                            "endwhile(1)\n"
                            << cacheValues << results;
    
    results.clear();
    QTest::newRow("break1") <<
                            "while(1)\n"
                            "if(TRUE)\n"
                                "break()\n"
                            "endif(TRUE)\n"
                            "endwhile(1)\n"
                            << cacheValues << results;
    
    cacheValues << StringPair("VAR", "OFF");
    results.clear();
    results << StringPair("val", "TRUE");
    QTest::newRow("break1") <<
                            "option(VAR \"something\" ON)\n"
                            "if(NOT VAR)\n"
                                "set(val TRUE)\n"
                            "endif(NOT VAR)\n"
                            << cacheValues << results;
}

void CMakeProjectVisitorTest::testRun()
{
    QFETCH(QString, input);
    QFETCH(QList<StringPair>, cache);
    QFETCH(QList<StringPair>, results);
    
    KDevelop::ReferencedTopDUContext fakeContext=
                    new TopDUContext(IndexedString("test"), RangeInRevision(0,0,0,0));
    DUChain::self()->addDocumentChain(fakeContext);
    
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
    
    vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList("./"));
    
    CMakeProjectVisitor v(file.fileName(), fakeContext);
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
    {
        KDevelop::DUChainWriteLocker lock(DUChain::lock());
        DUChain::self()->removeDocumentChain(fakeContext);
    }
}

void CMakeProjectVisitorTest::testFinder_data()
{
    QTest::addColumn<QString>("module");
    
    QTest::newRow("Qt4") << "Qt4";
    QTest::newRow("KDE4") << "KDE4";
//     QTest::newRow("Eigen2") << "Eigen2";
//     QTest::newRow("Exiv2") << "Exiv2";
//     QTest::newRow("QtGstreamer") << "QtGstreamer"; //commented because it might not be installed, but works
}

void CMakeProjectVisitorTest::testFinder_init()
{
    QPair<VariableMap, QStringList> initials=CMakeParserUtils::initialVariables();
    modulePath += initials.first.value("CMAKE_MODULE_PATH");
    modulePath += CMAKE_INSTALLED_MODULES;
//     modulePath += QStringList(CMAKE_TESTS_PROJECTS_DIR "/modules"); //Not used yet

    initialVariables=initials.first;
    buildstrap=initials.second;
}

void CMakeProjectVisitorTest::testFinder()
{
    QFETCH(QString, module);
    testFinder_init();
    
    KDevelop::ReferencedTopDUContext fakeContext=
                    new TopDUContext(IndexedString("test"), RangeInRevision(0,0,0,0));
    DUChain::self()->addDocumentChain(fakeContext);
    
    QFile file("cmake_visitor_test");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    
    QTextStream out(&file);
    out << QString("find_package(%1 REQUIRED)\n").arg(module);
    file.close();
    CMakeFileContent code=CMakeListsParser::readCMakeFile(file.fileName());
    file.remove();
    QVERIFY(code.count() != 0);
    
    CMakeProjectData data;
    data.vm=initialVariables;
    data.vm.insert("CMAKE_BINARY_DIR", QStringList("./"));
    data.vm.insert("CMAKE_MODULE_PATH", modulePath);
    
    foreach(const QString& script, buildstrap)
    {
        QString scriptfile=CMakeProjectVisitor::findFile(script, modulePath, QStringList());
        fakeContext=CMakeParserUtils::includeScript(scriptfile, fakeContext, &data, "./");
    }
    
    data.vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList("./"));
    CMakeProjectVisitor v(file.fileName(), fakeContext);
    v.setVariableMap(&data.vm);
    v.setMacroMap(&data.mm);
    v.setCacheValues( &data.cache );
    v.walk(code, 0);
    
    QString foundvar=QString("%1_FOUND").arg(module.toUpper());
    bool found=CMakeCondition(&v).condition(QStringList(foundvar));
    if(!found)
        qDebug() << "result: " << data.vm.value(foundvar);
    
    QVERIFY(found);
    
    {
        KDevelop::DUChainWriteLocker lock(DUChain::lock());
        DUChain::self()->removeDocumentChain(fakeContext);
    }
}

void CMakeProjectVisitorTest::testForeachLines()
{
    CMakeFunctionDesc foreachDesc, messageDesc, endForeachDesc;
    foreachDesc.name = "foreach";
    foreachDesc.addArguments(QStringList() << "i" << "RANGE" << "10" << "1");
    messageDesc.name = "message"; // or anything
    messageDesc.addArguments(QStringList() << "STATUS" << "pyon");
    endForeachDesc.name = "endforeach";

    CMakeFileContent content;
    content << messageDesc << foreachDesc << messageDesc << endForeachDesc << messageDesc;

    ForeachAst* ast = static_cast<ForeachAst*>(AstFactory::self()->createAst("foreach"));
    ast->setContent(content, 1);
    ast->parseFunctionInfo(foreachDesc);

    VariableMap vm;
    CMakeProjectVisitor v("somefile", ReferencedTopDUContext());
    v.setVariableMap(&vm);

    QCOMPARE(v.visit(ast), 3);
    delete ast;
}


#include "cmake_cmakeprojectvisitor_test.moc"
