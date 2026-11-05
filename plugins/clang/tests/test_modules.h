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
    void testGccModmapResolutionAndPrecompilation();
    void testParseModulesEndToEnd();
};

#endif // TESTMODULES_H
