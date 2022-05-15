// SPDX-FileCopyrightText: 2022 Gleb Popov <arrowd@FreeBSD.org>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef KDEVPLATFORM_PLUGIN_TEST_CRAFTRUNTIME_H
#define KDEVPLATFORM_PLUGIN_TEST_CRAFTRUNTIME_H

#include <QObject>

class CraftRuntimeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase_data();
    void testFindCraftRoot();
    void testGetenv();
    void testStartProcess();
    void testStartProcessEnv();
};

#endif // KDEVPLATFORM_PLUGIN_TEST_CRAFTRUNTIME_H
