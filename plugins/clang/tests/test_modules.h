/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTMODULES_H
#define TESTMODULES_H

#include <QObject>

class TestModules : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testStdVersionRank();
    void testIsCpp20OrLater();
    void testIsCppModuleUnit();
    void testRemoveHarmfulModuleArguments();
    void testParseP1689Json();
    void testClangScanDeps();
    void testScanAndPrecompileModules();
    void testCancellation();
    void testModuleGraphBasic();
    void testModuleGraphTopologicalSort();
    void testModuleGraphCycleDetection();
    void testModuleGraphMultiTargetSort();
    void testModuleRepository();
    void testRepositoryCachedLookup();
    void testPrecompileModulesCycleDetection();
    void testProjectModuleDiscovery();
    void testModuleProblemsReporting();
    void testModulePrecompileJobThreadWeaver();
    void testDependencyScanner();
};

#endif // TESTMODULES_H
