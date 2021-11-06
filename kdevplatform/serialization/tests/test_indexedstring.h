/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTINDEXEDSTRING_H
#define TESTINDEXEDSTRING_H

#include "itemrepositorytestbase.h"

class TestIndexedString
    : public ItemRepositoryTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void testUrl_data();
    void testUrl();

    void test();
    void test_data();

    void testMoveAssignment();
    void testSwap();
    void testSwap_data();

    void testCString();
};

#endif // TESTINDEXEDSTRING_H
