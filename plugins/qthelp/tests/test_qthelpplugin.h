/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEST_QTHELPPLUGIN_H
#define TEST_QTHELPPLUGIN_H

#include <tests/testcore.h>

class TestQtHelpPlugin : public QObject
{
    Q_OBJECT
private:
    KDevelop::TestCore* m_testCore = nullptr;

private Q_SLOTS:
    void initTestCase();
    void init();

    void testDefaultValue();
    void testUnsetQtHelpDoc();
    void testAddOneValidProvider();
    void testAddTwoDifferentValidProvider();
    void testAddInvalidProvider();
    void testAddTwiceSameProvider();
    void testRemoveOneProvider();

    void testDeclarationLookup_data();
    void testDeclarationLookup();

    void cleanupTestCase();
};

#endif // TEST_QTHELPPLUGIN_H
