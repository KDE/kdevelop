/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEV_TESTMICOMMAND_H
#define KDEV_TESTMICOMMAND_H

#include <QObject>

class TestMICommand : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testUserCommand();
    void testMICommandHandler();
    void testQObjectCommandHandler();
};

#endif
