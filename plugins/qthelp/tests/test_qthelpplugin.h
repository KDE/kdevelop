/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEST_QTHELPPLUGIN_H
#define TEST_QTHELPPLUGIN_H

#include <tests/testcore.h>

class QtHelpPlugin;

class TestQtHelpPlugin : public QObject
{
    Q_OBJECT
public:
    TestQtHelpPlugin();
private:
    KDevelop::TestCore* m_testCore;
    QtHelpPlugin *m_plugin;
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

    void cleanup();
    void cleanupTestCase();
};

#endif // TEST_QTHELPPLUGIN_H
