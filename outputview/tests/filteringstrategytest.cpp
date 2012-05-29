/*
    This file is part of KDevelop
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
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
    << buildCppCheckInformationLine() << FilteredItem::NotAValidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-action-line")
    << buildCompilerActionLine() << FilteredItem::NotAValidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::NotAValidItem;
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
    << buildCppCheckInformationLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::ErrorItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::NotAValidItem << FilteredItem::ActionItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
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

void FilteringStrategyTest::testScriptErrorFilterstrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::ErrorItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::ErrorItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
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
    << buildCppCheckInformationLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << FilteredItem::ErrorItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << FilteredItem::NotAValidItem << FilteredItem::NotAValidItem;
}

void FilteringStrategyTest::testStaticAnalysisFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    StaticAnalysisFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
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
    << "[ 25%] Building CXX object /path/to/one/CMakeFiles/file.o" << QString( basepath + "path/to/one" );
    QTest::newRow("cmake-line2")
    << "[ 26%] Building CXX object /path/to/two/CMakeFiles/file.o" << QString( basepath + "path/to/two");
    QTest::newRow("cmake-line3")
    << "[ 26%] Building CXX object /path/to/three/CMakeFiles/file.o" << QString( basepath + "path/to/three");
    QTest::newRow("cmake-line4")
    << "[ 26%] Building CXX object /path/to/four/CMakeFiles/file.o" << QString( basepath + "path/to/four");
    QTest::newRow("cmake-line5")
    << "[ 26%] Building CXX object /path/to/two/CMakeFiles/file.o" << QString( basepath + "path/to/two");
    QTest::newRow("cd-line6")
    << QString("make[4]: Entering directory '" + basepath + "path/to/one/'") << QString( basepath + "path/to/one/");
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
    FilteredItem item1("dummyline", FilteredItem::NotAValidItem);
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




}


