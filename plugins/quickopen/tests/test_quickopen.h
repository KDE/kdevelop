/*
    SPDX-FileCopyrightText: Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_TEST_QUICKOPEN_H
#define KDEVPLATFORM_PLUGIN_TEST_QUICKOPEN_H

#include "quickopentestbase.h"

class TestQuickOpen
    : public QuickOpenTestBase
{
    Q_OBJECT
public:
    explicit TestQuickOpen(QObject* parent = nullptr);
private Q_SLOTS:
    void testProjectFileSwap();
    void testSorting();
    void testSorting_data();
    void testStableSort();
    void testAbbreviations();
    void testAbbreviations_data();
    void testDuchainFilter();
    void testDuchainFilter_data();

    void testProjectFileFilter();
};

#endif // KDEVPLATFORM_PLUGIN_TEST_QUICKOPEN_H
