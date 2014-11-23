/*
    This file is part of KDevelop
    Copyright 2012 Milian Wolff <mail@milianw.de>
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "filteringstrategytest.h"
#include "testlinebuilderfunctions.h"

#include <outputview/outputfilteringstrategies.h>
#include <outputview/filtereditem.h>
#include <language/interfaces/quickopenfilter.h>

QTEST_GUILESS_MAIN(KDevelop::FilteringStrategyTest)

namespace QTest {

template<>
inline char* toString(const KDevelop::FilteredItem::FilteredOutputItemType& type)
{
    switch (type) {
        case KDevelop::FilteredItem::ActionItem:
            return qstrdup("ActionItem");
        case KDevelop::FilteredItem::CustomItem:
            return qstrdup("CustomItem");
        case KDevelop::FilteredItem::ErrorItem:
            return qstrdup("ErrorItem");
        case KDevelop::FilteredItem::InformationItem:
            return qstrdup("InformationItem");
        case KDevelop::FilteredItem::InvalidItem:
            return qstrdup("InvalidItem");
        case KDevelop::FilteredItem::StandardItem:
            return qstrdup("StandardItem");
        case KDevelop::FilteredItem::WarningItem:
            return qstrdup("WarningItem");
    }
    return qstrdup("unknown");
}

}

namespace KDevelop
{

void FilteringStrategyTest::testNoFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expected");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::InvalidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::InvalidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::InvalidItem;
    QTest::newRow("compiler-action-line")
    << buildCompilerActionLine() << FilteredItem::InvalidItem;
    QTest::newRow("compiler-information-line")
    << buildCompilerInformationLine() << FilteredItem::InvalidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::InvalidItem;
}

void FilteringStrategyTest::testNoFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expected);
    NoFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expected);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expected);
}

void FilteringStrategyTest::testCompilerFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-information-line")
    << buildCompilerInformationLine() << FilteredItem::InformationItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-information-line2")
    << buildInfileIncludedFromFirstLine() << FilteredItem::InformationItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-information-line3")
    << buildInfileIncludedFromSecondLine() << FilteredItem::InformationItem << FilteredItem::InvalidItem;
    QTest::newRow("cmake-error-line1")
    << "CMake Error at CMakeLists.txt:2 (cmake_minimum_required):" << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("cmake-error-multiline1")
    << "CMake Error: Error in cmake code at" << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("cmake-error-multiline2")
    << buildCmakeConfigureMultiLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("cmake-warning-line")
    << "CMake Warning (dev) in CMakeLists.txt:" << FilteredItem::WarningItem << FilteredItem::InvalidItem;
    QTest::newRow("cmake-automoc-error")
    << "AUTOMOC: error: /foo/bar.cpp The file includes the moc file \"moc_bar1.cpp\"" << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("cmake-automoc4-error")
    << "automoc4: The file \"/foo/bar.cpp\" includes the moc file \"bar1.moc\"" << FilteredItem::InformationItem << FilteredItem::InvalidItem;
    QTest::newRow("cmake-autogen-error")
    << "AUTOGEN: error: /foo/bar.cpp The file includes the moc file \"moc_bar1.cpp\"" << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("linker-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::InvalidItem << FilteredItem::ActionItem;
    QTest::newRow("linker-error-line")
    << buildLinkerErrorLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
}

void FilteringStrategyTest::testCompilerFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    QUrl projecturl = QUrl::fromLocalFile( projectPath() );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void FilteringStrategyTest::testCompilerFilterstrategyMultipleKeywords_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");

    QTest::newRow("warning-containing-error-word")
    << "RingBuffer.cpp:64:6: warning: unused parameter ‘errorItem’ [-Wunused-parameter]"
    << FilteredItem::WarningItem << FilteredItem::InvalidItem;
    QTest::newRow("error-containing-info-word")
    << "NodeSet.hpp:89:27: error: ‘Info’ was not declared in this scope"
    << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("warning-in-filename-containing-error-word")
    << "ErrorHandling.cpp:100:56: warning: unused parameter ‘item’ [-Wunused-parameter]"
    << FilteredItem::WarningItem << FilteredItem::InvalidItem;
    QTest::newRow("error-in-filename-containing-warning-word")
    << "WarningHandling.cpp:100:56: error: ‘Item’ was not declared in this scope"
    << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
}

void FilteringStrategyTest::testCompilerFilterstrategyMultipleKeywords()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    QUrl projecturl = QUrl::fromLocalFile( projectPath() );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void FilteringStrategyTest::testCompilerFilterStrategyShortenedText_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("expectedShortenedText");

    QTest::newRow("c++-compile")
    << "g++ -c main.cpp -o main.o" << "compiling main.cpp (g++)";
    QTest::newRow("clang++-link")
    << "clang++ -c main.cpp -o main.o" << "compiling main.cpp (clang++)";
    // see bug: https://bugs.kde.org/show_bug.cgi?id=240017
    QTest::newRow("mpicc-link")
    << "/usr/bin/mpicc -c main.cpp -o main.o" << "compiling main.cpp (mpicc)";

    QTest::newRow("c++-link")
    << "/usr/bin/g++ main.cpp -o main" << "linking main (g++)";
    QTest::newRow("clang++-link")
    << "/usr/bin/clang++ main.cpp -o a.out" << "linking a.out (clang++)";
    QTest::newRow("mpicc-link")
    << "mpicc main.cpp -o main" << "linking main (mpicc)";
}

void FilteringStrategyTest::testCompilerFilterStrategyShortenedText()
{
    QFETCH(QString, line);
    QFETCH(QString, expectedShortenedText);

    QUrl projecturl = QUrl::fromLocalFile( projectPath() );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item = testee.actionInLine(line);
    QCOMPARE(item.shortenedText, expectedShortenedText);
}

void FilteringStrategyTest::testScriptErrorFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
}

void FilteringStrategyTest::testScriptErrorFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    ScriptErrorFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void FilteringStrategyTest::testNativeAppErrorFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("lineNo");
    QTest::addColumn<int>("column");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("itemtype");

    // TODO: qt-connect-* and friends shouldn't be error items but warnings items instead
    // this needs refactoring in outputfilteringstrategies, though...
    QTest::newRow("qt-connect-nosuch-slot")
        << "QObject::connect: No such slot Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-connect-nosuch-signal")
        << "QObject::connect: No such signal Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-connect-parentheses-slot")
        << "QObject::connect: Parentheses expected, slot Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-connect-parentheses-signal")
        << "QObject::connect: Parentheses expected, signal Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-assert")
        << "ASSERT: \"errors().isEmpty()\" in file /tmp/foo/bar.cpp, line 49"
        << "/tmp/foo/bar.cpp"
        << 48 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qttest-assert")
        << "QFATAL : FooTest::testBar() ASSERT: \"index.isValid()\" in file /foo/bar.cpp, line 32"
        << "/foo/bar.cpp"
        << 31 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qttest-loc")
        << "   Loc: [/foo/bar.cpp(33)]"
        << "/foo/bar.cpp"
        << 32 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qttest-loc-nocatch")
        << "   Loc: [Unknown file(0)]"
        << ""
        << -1 << -1 << FilteredItem::InvalidItem;
}

void FilteringStrategyTest::testNativeAppErrorFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(QString, file);
    QFETCH(int, lineNo);
    QFETCH(int, column);
    QFETCH(FilteredItem::FilteredOutputItemType, itemtype);
    NativeAppErrorFilterStrategy testee;
    FilteredItem item = testee.errorInLine(line);
    QCOMPARE(item.url.path(), file);
    QCOMPARE(item.lineNo , lineNo);
    QCOMPARE(item.columnNo , column);
    QCOMPARE(item.type , itemtype);
}

void FilteringStrategyTest::testStaticAnalysisFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("krazy2-error-line")
    << buildKrazyErrorLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("krazy2-error-line-two-colons")
    << buildKrazyErrorLine2() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("krazy2-error-line-error-description")
    << buildKrazyErrorLine3() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("krazy2-error-line-wo-line-info")
    << buildKrazyErrorLineNoLineInfo() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
}

void FilteringStrategyTest::testStaticAnalysisFilterStrategy()
{
    // Test that url's are extracted correctly as well
    QString referencePath = projectPath() + "main.cpp";

    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    StaticAnalysisFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QString extractedPath = item1.url.toLocalFile();
    QVERIFY((item1.type != FilteredItem::ErrorItem) || ( extractedPath == referencePath));
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void FilteringStrategyTest::testCompilerFilterstrategyUrlFromAction_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("expectedLastDir");
    QString basepath = projectPath();

    QTest::newRow("cmake-line1")
    << "[ 25%] Building CXX object /path/to/one/CMakeFiles/file.o" << QString( basepath + "/path/to/one" );
    QTest::newRow("cmake-line2")
    << "[ 26%] Building CXX object /path/to/two/CMakeFiles/file.o" << QString( basepath + "/path/to/two");
    QTest::newRow("cmake-line3")
    << "[ 26%] Building CXX object /path/to/three/CMakeFiles/file.o" << QString( basepath + "/path/to/three");
    QTest::newRow("cmake-line4")
    << "[ 26%] Building CXX object /path/to/four/CMakeFiles/file.o" << QString( basepath + "/path/to/four");
    QTest::newRow("cmake-line5")
    << "[ 26%] Building CXX object /path/to/two/CMakeFiles/file.o" << QString( basepath + "/path/to/two");
    QTest::newRow("cd-line6")
    << QString("make[4]: Entering directory '" + basepath + "/path/to/one/'") << QString( basepath + "/path/to/one");
    QTest::newRow("waf-cd")
    << QString("Waf: Entering directory `" + basepath + "/path/to/two/'") << QString( basepath + "/path/to/two");
    QTest::newRow("cmake-line7")
    << QString("[ 50%] Building CXX object CMakeFiles/testdeque.dir/RingBuffer.cpp.o") << QString( basepath);
}

void FilteringStrategyTest::testCompilerFilterstrategyUrlFromAction()
{
    QFETCH(QString, line);
    QFETCH(QString, expectedLastDir);
    QUrl projecturl = QUrl::fromLocalFile( projectPath() );
    static CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.actionInLine(line);
    QCOMPARE(testee.getCurrentDirs().last(), expectedLastDir);
}

void FilteringStrategyTest::benchMarkCompilerFilterAction()
{
    QString projecturl = projectPath();
    QStringList outputlines;
    const int numLines(10000);
    int j(0), k(0), l(0), m(0);
    do {
        ++j; ++k; ++l;
        QString tmp;
        if(m % 2 == 0) {
            tmp = QString( "[ 26%] Building CXX object /this/is/the/path/to/the/files/%1/%2/%3/CMakeFiles/file.o").arg( j ).arg( k ).arg( l );
        } else {
            tmp = QString( "make[4]: Entering directory '" + projecturl + "/this/is/the/path/to/the/files/%1/%2/%3/").arg( j ).arg( k ).arg( l );
        }
        outputlines << tmp;
        if(j % 6 == 0) {
            j = 0; ++m;
        }
        if(k % 9 == 0) {
            k = 0; ++m;
        }
        if(l % 13 == 0) {
            l = 0; ++m;
        }
    }
    while(outputlines.size() < numLines ); // gives us numLines (-ish)

    QElapsedTimer totalTime;
    totalTime.start();

    static CompilerFilterStrategy testee(QUrl::fromLocalFile(projecturl));
    FilteredItem item1("dummyline", FilteredItem::InvalidItem);
    QBENCHMARK {
        for(int i = 0; i < outputlines.size(); ++i) {
            item1 = testee.actionInLine(outputlines.at(i));
        }
    }

    const qint64 elapsed = totalTime.elapsed();

    qDebug() << "ms elapsed to add directories: " << elapsed;
    qDebug() << "total number of directories: " << outputlines.count();
    const double avgDirectoryInsertion = double(elapsed) / outputlines.count();
    qDebug() << "average ms spend pr. dir: " << avgDirectoryInsertion;

    QVERIFY(avgDirectoryInsertion < 2);
}

void FilteringStrategyTest::testExtractionOfLineAndColumn_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("lineNr");
    QTest::addColumn<int>("column");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("itemtype");

    QTest::newRow("gcc-with-col")
        << "/path/to/file.cpp:123:45: fatal error: ..."
        << "/path/to/file.cpp" << 122 << 44 << FilteredItem::ErrorItem;
    QTest::newRow("gcc-no-col")
        << "/path/to/file.cpp:123: error ..."
        << "/path/to/file.cpp" << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("fortcom")
        << "fortcom: Error: Ogive8.f90, line 123: ..."
        << QString(projectPath() + "/Ogive8.f90") << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("fortcomError")
        << "fortcom: Error: ./Ogive8.f90, line 123: ..."
        << QString(projectPath() + "/Ogive8.f90") << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("fortcomWarning")
        << "fortcom: Warning: /path/Ogive8.f90, line 123: ..."
        << "/path/Ogive8.f90" << 122 << 0 << FilteredItem::WarningItem;
    QTest::newRow("fortcomInfo")
        << "fortcom: Info: Ogive8.f90, line 123: ..."
        << QString(projectPath() + "/Ogive8.f90") << 122 << 0 << FilteredItem::InformationItem;
    QTest::newRow("libtool")
        << "libtool: link: warning: ..."
        << "" << -1 << 0  << FilteredItem::WarningItem;
    QTest::newRow("gfortranError1")
        << "/path/to/file.f90:123.456:Error: ...."
        << "/path/to/file.f90" << 122 << 455  << FilteredItem::ErrorItem;
    QTest::newRow("gfortranError2")
        << "/path/flib.f90:3567.22:"
        << "/path/flib.f90" << 3566 << 21 << FilteredItem::ErrorItem;
}

void FilteringStrategyTest::testExtractionOfLineAndColumn()
{
    QFETCH(QString, line);
    QFETCH(QString, file);
    QFETCH(int, lineNr);
    QFETCH(int, column);
    QFETCH(FilteredItem::FilteredOutputItemType, itemtype);
    QUrl projecturl = QUrl::fromLocalFile( projectPath() );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type , itemtype);
    QCOMPARE(item1.url.path(), file);
    QCOMPARE(item1.lineNo , lineNr);
    QCOMPARE(item1.columnNo , column);
}




}


