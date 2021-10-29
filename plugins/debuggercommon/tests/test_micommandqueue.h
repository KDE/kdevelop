/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEV_TESTMICOMMANDQUEUE_H
#define KDEV_TESTMICOMMANDQUEUE_H

#include <QObject>

class TestMICommandQueue : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testConstructor();
    void testDestructor();
    void addAndTake_data();
    void addAndTake();
    void clearQueue();
};

#endif
