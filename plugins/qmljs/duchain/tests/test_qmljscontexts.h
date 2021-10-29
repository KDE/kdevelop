/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTCONTEXTS_H
#define TESTCONTEXTS_H

#include <QObject>

class TestContexts : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testFunctionContext();
    void testFunctionContext_data();

    void testQMLContext();
};

#endif // TESTCONTEXTS_H
