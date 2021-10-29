/*
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_FILTERINGSTRATEGY_H
#define KDEVPLATFORM_TEST_FILTERINGSTRATEGY_H

#include <QTest>

namespace KDevelop
{

class TestFilteringStrategy : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testNoFilterStrategy_data();
    void testNoFilterStrategy();
    void testCompilerFilterStrategy_data();
    void testCompilerFilterStrategy();
    void testCompilerFilterstrategyMultipleKeywords_data();
    void testCompilerFilterstrategyMultipleKeywords();
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

#endif // KDEVPLATFORM_TEST_FILTERINGSTRATEGY_H
