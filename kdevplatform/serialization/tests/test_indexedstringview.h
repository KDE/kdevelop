/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_TEST_INDEXED_STRING_VIEW_H
#define KDEVPLATFORM_TEST_INDEXED_STRING_VIEW_H

#include "itemrepositorytestbase.h"

class TestIndexedStringView : public ItemRepositoryTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void testUrl_data();
    void testUrl();

    void test_data();
    void test();

    void testCString();
};

#endif // KDEVPLATFORM_TEST_INDEXED_STRING_VIEW_H
