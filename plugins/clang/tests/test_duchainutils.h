/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTDUCHAINUTILS_H
#define TESTDUCHAINUTILS_H

#include <QObject>

class TestDUChainUtils : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getFunctionSignatureRange();
    void getFunctionSignatureRange_data();
};

#endif // TESTDUCHAINUTILS_H
