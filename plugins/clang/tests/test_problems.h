/*
    SPDX-FileCopyrightText: Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_PROBLEMS_H
#define TEST_PROBLEMS_H

#include <QObject>

#include <language/duchain/problem.h>

class TestProblems : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testNoProblems();
    void testBasicProblems();
    void testBasicRangeSupport();
    void testChildDiagnostics();
    void testRanges_data();
    void testRanges();
    void testProblemsForIncludedFiles();

    void testFixits();
    void testFixits_data();
    void testTodoProblems();
    void testTodoProblems_data();

    void testMissingInclude();
    void testMissingInclude_data();

    void testSeverity();
    void testSeverity_data();
};

#endif // TEST_PROBLEMS_H
