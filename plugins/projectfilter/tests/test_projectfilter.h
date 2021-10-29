/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTPROJECTFILTER_H
#define TESTPROJECTFILTER_H

#include <QObject>

class TestProjectFilter : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void match();
    void match_data();

    void bench();
    void bench_data();
};

#endif // TESTPROJECTFILTER_H
