/*
    This file is part of KDevelop
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
#ifndef KDEVPLATFORM_FILTERINGSTRATEGYTEST_H
#define KDEVPLATFORM_FILTERINGSTRATEGYTEST_H

#include <QtTest/QtTest>

namespace KDevelop
{

class FilteringStrategyTest : public QObject
{
    Q_OBJECT
private slots:
    void testNoFilterStrategy_data();
    void testNoFilterStrategy();
    void testCompilerFilterStrategy_data();
    void testCompilerFilterStrategy();
    void testCompilerFilterstrategyMultipleKeywords_data();
    void testCompilerFilterstrategyMultipleKeywords();
    void testCompilerFilterStrategyShortenedText();
    void testCompilerFilterStrategyShortenedText_data();
    void testCompilerFilterstrategyUrlFromAction_data();
    void testCompilerFilterstrategyUrlFromAction();
    void testScriptErrorFilterStrategy_data();
    void testScriptErrorFilterStrategy();
    void testNativeAppErrorFilterStrategy_data();
    void testNativeAppErrorFilterStrategy();
    void testStaticAnalysisFilterStrategy_data();
    void testStaticAnalysisFilterStrategy();
    void testExtractionOfLineAndColumn_data();
    void testExtractionOfLineAndColumn();

    void benchMarkCompilerFilterAction();
};

}

#endif // KDEVPLATFORM_FILTERINGSTRATEGYTEST_H
