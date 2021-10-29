/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_HIGHLIGHTING_H
#define KDEVPLATFORM_TEST_HIGHLIGHTING_H

#include <QObject>

class TestHighlighting
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // for valgrind
    void testInitialization();
};

#endif // KDEVPLATFORM_TEST_HIGHLIGHTING_H
