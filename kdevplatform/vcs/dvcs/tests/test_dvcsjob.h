/*
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_TEST_DVCSJOB_H
#define KDEVPLATFORM_TEST_DVCSJOB_H

#include <QObject>

class TestDVcsJob: public QObject
{
    Q_OBJECT

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testJob();
};

#endif
