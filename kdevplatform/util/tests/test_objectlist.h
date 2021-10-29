/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTOBJECTLIST_H
#define TESTOBJECTLIST_H

#include <QObject>

class TestObjectList : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testBasicInterface();
    void testDeleteAll();
    void testBehaviorOnDestruction();
};

#endif // TESTOBJECTLIST_H
