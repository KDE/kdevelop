/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_DUCHAINSHUTDOWN_H
#define KDEVPLATFORM_TEST_DUCHAINSHUTDOWN_H

#include <QObject>

namespace KDevelop {
class TestCore;
}

class TestDUChainShutdown
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void runTest();

private:
    KDevelop::TestCore* m_core;
};

#endif // TST_DUCHAINSHUTDOWN_H
