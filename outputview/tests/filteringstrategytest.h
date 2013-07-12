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
    void testNoFilterstrategy_data();
    void testNoFilterstrategy();
    void testCompilerFilterstrategy_data();
    void testCompilerFilterstrategy();
    void testCompilerFilterstrategyMultipleKeywords_data();
    void testCompilerFilterstrategyMultipleKeywords();
    void testCompilerFilterStrategyShortenedText();
    void testCompilerFilterStrategyShortenedText_data();
    void testCompilerFilterstrategyUrlFromAction_data();
    void testCompilerFilterstrategyUrlFromAction();
    void testScriptErrorFilterstrategy_data();
    void testScriptErrorFilterstrategy();
    void testStaticAnalysisFilterStrategy_data();
    void testStaticAnalysisFilterStrategy();
    void benchMarkCompilerFilterAction();
    void testExtractionOfLineAndCulmn_data();
    void testExtractionOfLineAndCulmn();

};

}

#endif // KDEVPLATFORM_FILTERINGSTRATEGYTEST_H
