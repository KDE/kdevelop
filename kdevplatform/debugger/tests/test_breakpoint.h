/*
 *  SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef KDEVPLATFORM_TEST_BREAKPOINT_H
#define KDEVPLATFORM_TEST_BREAKPOINT_H

#include <QObject>

/**
 * Tests KDevelop::Breakpoint
 */
class TestBreakpoint : public QObject
{
    Q_OBJECT
public:
    explicit TestBreakpoint(QObject* parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testKindSetDataExpression_data();
    void testKindSetDataExpression();
    void testSetDataExpression_data();
    void testSetDataExpression();
    void testSetDataUrlAndLine_data();
    void testSetDataUrlAndLine();
};

#endif // KDEVPLATFORM_TEST_BREAKPOINT_H
