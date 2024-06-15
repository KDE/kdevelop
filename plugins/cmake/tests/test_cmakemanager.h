/*
    SPDX-FileCopyrightText: 2010 Esben Mose Hansen <kde@mosehansen.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEST_CMAKEMANAGER_H
#define TEST_CMAKEMANAGER_H

#include <QTest>

namespace KDevelop {
class IProject;
}

/**
 *  Test of the cmake manager.
 *   + Test that CMakeCache.txt is found, loaded and parsed
 * @author Esben Mose Hansen <kde@mosehansen.dk>
 **/
class TestCMakeManager : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testWithBuildDirProject();
    void testIncludePaths();
    void testRelativePaths();
    void testTargetIncludeDirectories();
    void testTargetIncludePaths();
    void testDefines();
    void testCustomTargetSources();
    void testConditionsInSubdirectoryBasedOnRootVariables();
    void testQt6App();
    void testKF6App();
    void testEnumerateTargets();
    void testFaultyTarget();
    void testParenthesesInTestArguments();
    void testReload();
    void testExecutableOutputPath();
};

#endif // TEST_CMAKEMANAGER_H
