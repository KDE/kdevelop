/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_CTESTFINDSUITES_H
#define TEST_CTESTFINDSUITES_H

#include <QTest>

class TestCTestFindSuites : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();

    void initTestCase();
    void cleanupTestCase();

    void testCTestSuite();
    void testQtTestCases();
};

#endif
