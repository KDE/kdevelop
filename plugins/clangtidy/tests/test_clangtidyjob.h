/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_CLANGTIDYJOB_H
#define TEST_CLANGTIDYJOB_H

#include <QObject>

class TestClangTidyJob : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testJob();
};

#endif
