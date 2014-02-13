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

#include <qtest_kde.h>

QTEST_KDEMAIN(KDevelop::FilteringStrategyTest, NoGUI)

namespace KDevelop
{
void FilteringStrategyTest::testNoFilterstrategy_data()
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

void FilteringStrategyTest::testNoFilterstrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expected);
    NoFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.type == expected);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.type == expected);
}

void FilteringStrategyTest::testCompilerFilterstrategy_data()
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
    QTest::newRow("linker-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::InvalidItem << FilteredItem::ActionItem;
    QTest::newRow("linker-error-line")
    << buildLinkerErrorLine() << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
}

void FilteringStrategyTest::testCompilerFilterstrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    KUrl projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.type == expectedError);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.type == expectedAction);
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
    KUrl projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.type == expectedError);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.type == expectedAction);
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

    KUrl projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item = testee.actionInLine(line);
    QCOMPARE(item.shortenedText, expectedShortenedText);
}

void FilteringStrategyTest::testScriptErrorFilterstrategy_data()
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

void FilteringStrategyTest::testScriptErrorFilterstrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    ScriptErrorFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.type == expectedError);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.type == expectedAction);
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
    QString referencePath( PROJECTS_SOURCE_DIR"/onefileproject/main.cpp" ); 

    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    StaticAnalysisFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QString extractedPath = item1.url.toLocalFile();
    QVERIFY((item1.type != FilteredItem::ErrorItem) || ( extractedPath == referencePath));
    QVERIFY(item1.type == expectedError);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.type == expectedAction);
}

void FilteringStrategyTest::testCompilerFilterstrategyUrlFromAction_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("expectedLastDir");
    QString basepath( PROJECTS_SOURCE_DIR"/onefileproject/" );


    QTest::newRow("cmake-line1")
    << "[ 25%] Building CXX object /path/to/one/CMakeFiles/file.o" << QString( basepath + "path/to/one/" );
    QTest::newRow("cmake-line2")
    << "[ 26%] Building CXX object /path/to/two/CMakeFiles/file.o" << QString( basepath + "path/to/two/");
    QTest::newRow("cmake-line3")
    << "[ 26%] Building CXX object /path/to/three/CMakeFiles/file.o" << QString( basepath + "path/to/three/");
    QTest::newRow("cmake-line4")
    << "[ 26%] Building CXX object /path/to/four/CMakeFiles/file.o" << QString( basepath + "path/to/four/");
    QTest::newRow("cmake-line5")
    << "[ 26%] Building CXX object /path/to/two/CMakeFiles/file.o" << QString( basepath + "path/to/two/");
    QTest::newRow("cd-line6")
    << QString("make[4]: Entering directory '" + basepath + "path/to/one/'") << QString( basepath + "path/to/one/");
    QTest::newRow("waf-cd")
    << QString("Waf: Entering directory `" + basepath + "path/to/two/'") << QString( basepath + "path/to/two/");
    QTest::newRow("cmake-line7")
    << QString("[ 50%] Building CXX object CMakeFiles/testdeque.dir/RingBuffer.cpp.o") << QString( basepath);
}

void FilteringStrategyTest::testCompilerFilterstrategyUrlFromAction()
{
    QFETCH(QString, line);
    QFETCH(QString, expectedLastDir);
    KUrl projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );
    static CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.actionInLine(line);
    QCOMPARE(testee.getCurrentDirs().last(), expectedLastDir);
}

void FilteringStrategyTest::benchMarkCompilerFilterAction()
{
    QString projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );
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

    static CompilerFilterStrategy testee(projecturl);
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

void FilteringStrategyTest::testExtractionOfLineAndCulmn_data()
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
        << "./Ogive8.f90" << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("fortcomError")
        << "fortcom: Error: ./Ogive8.f90, line 123: ..."
        << "././Ogive8.f90" << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("fortcomWarning")
        << "fortcom: Warning: /path/Ogive8.f90, line 123: ..."
        << "/path/Ogive8.f90" << 122 << 0 << FilteredItem::WarningItem;
    QTest::newRow("fortcomInfo")
        << "fortcom: Info: Ogive8.f90, line 123: ..."
        << "./Ogive8.f90" << 122 << 0 << FilteredItem::InformationItem;
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

void FilteringStrategyTest::testExtractionOfLineAndCulmn()
{
    QFETCH(QString, line);
    QFETCH(QString, file);
    QFETCH(int, lineNr);
    QFETCH(int, column);
    QFETCH(FilteredItem::FilteredOutputItemType, itemtype);
    KUrl projecturl( "./" );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type , itemtype);
    QCOMPARE(item1.url.path(), file);
    QCOMPARE(item1.lineNo , lineNr);
    QCOMPARE(item1.columnNo , column);
}




}


