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

#include <qtest_kde.h>

QTEST_KDEMAIN(KDevelop::FilteringStrategyTest, NoGUI)

namespace KDevelop
{
void FilteringStrategyTest::testNoFilterstrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<bool>("expected");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << false;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << false;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << false;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << false;
    QTest::newRow("compiler-action-line")
    << buildCompilerActionLine() << false;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << false;
}

void FilteringStrategyTest::testNoFilterstrategy()
{
    QFETCH(QString, line);
    QFETCH(bool, expected);
    NoFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.isValid() == expected);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.isValid() == expected);
}

void FilteringStrategyTest::testCompilerFilterstrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<bool>("expectedError");
    QTest::addColumn<bool>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << false << false;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << false << false;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << false << false;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << true << false;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << false << true;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << false << false;
}

void FilteringStrategyTest::testCompilerFilterstrategy()
{
    QFETCH(QString, line);
    QFETCH(bool, expectedError);
    QFETCH(bool, expectedAction);
    KUrl projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.isValid() == expectedError);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.isValid() == expectedAction);
}

void FilteringStrategyTest::testScriptErrorFilterstrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<bool>("expectedError");
    QTest::addColumn<bool>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << false << false;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << true << false;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << false << false;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << true << false;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << false << false;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << false << false;
}

void FilteringStrategyTest::testScriptErrorFilterstrategy()
{
    QFETCH(QString, line);
    QFETCH(bool, expectedError);
    QFETCH(bool, expectedAction);
    ScriptErrorFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.isValid() == expectedError);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.isValid() == expectedAction);
}

void FilteringStrategyTest::testStaticAnalysisFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<bool>("expectedError");
    QTest::addColumn<bool>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << false << false;
    QTest::newRow("cppcheck-error-line")
    << buildCppCheckErrorLine() << true << false;
    QTest::newRow("compiler-line")
    << buildCompilerLine() << false << false;
    QTest::newRow("compiler-error-line")
    << buildCompilerErrorLine() << false << false;
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << false << false;
    QTest::newRow("python-error-line")
    << buildPythonErrorLine() << false << false;
}

void FilteringStrategyTest::testStaticAnalysisFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(bool, expectedError);
    QFETCH(bool, expectedAction);
    StaticAnalysisFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QVERIFY(item1.isValid() == expectedError);
    item1 = testee.actionInLine(line);
    QVERIFY(item1.isValid() == expectedAction);
}


}


