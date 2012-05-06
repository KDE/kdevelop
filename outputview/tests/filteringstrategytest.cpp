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
#include <outputview/outputfilteringstrategies.h>
#include <outputview/filtereditem.h>

#include <qtest_kde.h>

QTEST_KDEMAIN(KDevelop::FilteringStrategyTest, NoGUI)

namespace KDevelop
{

void FilteringStrategyTest::testNoFilterstrategy()
{
    NoFilterStrategy testee;
    /// Test CPP check output
    QString outputline("[/home/mvo/mortenv_mvo-desktop_4097/mortenv_mvo-desktop_4097/fifthCustomBuild/main.cpp:90]: (error) Memory leak: str");
    FilteredItem item1(outputline);
    //(information) Cppcheck cannot find all the include files. Cpppcheck can check the code without the include files found. But the results will probably be more accurate if all the include files are found. Please check your project's include directories and add all of them as include directories for Cppcheck. To see what files Cppcheck cannot find use --check-config.
    QVERIFY(testee.isErrorInLine(outputline, item1) == false);
    QVERIFY(testee.isActionInLine(outputline, item1) == false);

    outputline = "(information) Cppcheck cannot find all the include files. Cpppcheck can check the code without the include files found. But the results will probably be more accurate if all the include files are found. Please check your project's include directories and add all of them as include directories for Cppcheck. To see what files Cppcheck cannot find use --check-config.";
    FilteredItem item2(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item2) == false);
    QVERIFY(testee.isActionInLine(outputline, item2) == false);

    /// Test with compiler output
    outputline = "/home/mvo/mortenv_mvo-desktop_4097/mortenv_mvo-desktop_4097/fifthCustomBuild/>make"; 
    FilteredItem item3(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item3) == false);
    QVERIFY(testee.isActionInLine(outputline, item3) == false);

    outputline = "linking testCustombuild (g++)"; 
    FilteredItem item4(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item4) == false);
    QVERIFY(testee.isActionInLine(outputline, item4) == false);

    outputline = "main.cpp:2:23: fatal error: someClass.h: No such file or directory"; 
    FilteredItem item5(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item5) == false);
    QVERIFY(testee.isActionInLine(outputline, item5) == false);

    /// Test with script error output;

}

void FilteringStrategyTest::testCompilerFilterstrategy()
{
    /// Use existing directory with one file
    KUrl projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );

    CompilerFilterStrategy testee(projecturl);

    /// Test CPP check output
    QString outputline("[");
    outputline.append(projecturl.path());
    outputline.append("main.cpp:26]: (error) Memory leak: str");

    FilteredItem item1(outputline);

    QVERIFY(testee.isErrorInLine(outputline, item1) == false);
    QVERIFY(testee.isActionInLine(outputline, item1) == false);

    outputline = "(information) Cppcheck cannot find all the include files. Cpppcheck can check the code without the include files found. But the results will probably be more accurate if all the include files are found. Please check your project's include directories and add all of them as include directories for Cppcheck. To see what files Cppcheck cannot find use --check-config.";
    FilteredItem item2(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item2) == false);
    QVERIFY(testee.isActionInLine(outputline, item2) == false);

    /// Test with compiler output
    outputline.clear();
    outputline.append(projecturl.path());
    outputline.append(">make"); 
    FilteredItem item3(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item3) == false);
    QVERIFY(testee.isActionInLine(outputline, item3) == false);

    outputline = "linking testCustombuild (g++)"; 
    FilteredItem item4(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item4) == false);
    QVERIFY(testee.isActionInLine(outputline, item4) == true);
    QVERIFY(item4.isActivatable == false);

    outputline.clear();
    outputline.append(projecturl.path());
    outputline.append("main.cpp:5:5: error: ‘RingBuffer’ was not declared in this scope");
    FilteredItem item5(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item5) == true);
    QVERIFY(testee.isActionInLine(outputline, item5) == false);    
    QVERIFY(item5.isActivatable == true);

    /// Test with script error output;

}

void FilteringStrategyTest::testScriptErrorFilterstrategy()
{
    ScriptErrorFilterStrategy testee;

    /// Test CPP check output
    QString outputline("[/home/mvo/mortenv_mvo-desktop_4097/mortenv_mvo-desktop_4097/fifthCustomBuild/main.cpp:90]: (error) Memory leak: str");
    FilteredItem item1(outputline);

    QVERIFY(testee.isErrorInLine(outputline, item1) == true);
    QVERIFY(testee.isActionInLine(outputline, item1) == false);

    outputline = "(information) Cppcheck cannot find all the include files. Cpppcheck can check the code without the include files found. But the results will probably be more accurate if all the include files are found. Please check your project's include directories and add all of them as include directories for Cppcheck. To see what files Cppcheck cannot find use --check-config.";
    FilteredItem item2(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item2) == false);
    QVERIFY(testee.isActionInLine(outputline, item2) == false);

    /// Test with compiler output
    outputline = "/home/mvo/mortenv_mvo-desktop_4097/mortenv_mvo-desktop_4097/fifthCustomBuild/>make"; 
    FilteredItem item3(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item3) == false);
    QVERIFY(testee.isActionInLine(outputline, item3) == false);

    outputline = "linking testCustombuild (g++)"; 
    FilteredItem item4(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item4) == false);
    QVERIFY(testee.isActionInLine(outputline, item4) == false);

    outputline = "main.cpp:2:23: fatal error: someClass.h: No such file or directory"; 
    FilteredItem item5(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item5) == false);
    QVERIFY(testee.isActionInLine(outputline, item5) == false);

    /// Test with script error output;

}

void FilteringStrategyTest::testStaticAnalysisFilterStrategy()
{
    /// Use existing directory with one file
    KUrl projecturl( PROJECTS_SOURCE_DIR"/onefileproject/" );

    StaticAnalysisFilterStrategy testee;

    /// Test CPP check output
    QString outputline("[");
    outputline.append(projecturl.path());
    outputline.append("main.cpp:26]: (error) Memory leak: str");
    FilteredItem item1(outputline);

    QVERIFY(testee.isErrorInLine(outputline, item1) == true);
    QVERIFY(testee.isActionInLine(outputline, item1) == false);
    QVERIFY(item1.url.path() == QString(PROJECTS_SOURCE_DIR"/onefileproject/main.cpp"));

    outputline = "(information) Cppcheck cannot find all the include files. Cpppcheck can check the code without the include files found. But the results will probably be more accurate if all the include files are found. Please check your project's include directories and add all of them as include directories for Cppcheck. To see what files Cppcheck cannot find use --check-config.";
    FilteredItem item2(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item2) == false);
    QVERIFY(testee.isActionInLine(outputline, item2) == false);

    /// Test with compiler output
    outputline = "/home/mvo/mortenv_mvo-desktop_4097/mortenv_mvo-desktop_4097/fifthCustomBuild/>make"; 
    FilteredItem item3(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item3) == false);
    QVERIFY(testee.isActionInLine(outputline, item3) == false);

    outputline = "linking testCustombuild (g++)"; 
    FilteredItem item4(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item4) == false);
    QVERIFY(testee.isActionInLine(outputline, item4) == false);

    outputline = "main.cpp:2:23: fatal error: someClass.h: No such file or directory"; 
    FilteredItem item5(outputline);
    QVERIFY(testee.isErrorInLine(outputline, item5) == false);
    QVERIFY(testee.isActionInLine(outputline, item5) == false);

}


}


