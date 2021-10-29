/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_FOREGROUNDLOCK_H
#define KDEVPLATFORM_TEST_FOREGROUNDLOCK_H

#include <QObject>

namespace KDevelop {
class TestForegroundLock : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testTryLock_data();
    void testTryLock();
};
}

#endif // KDEVPLATFORM_TEST_FOREGROUNDLOCK_H
