/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_CPPCHECK_JOB_H
#define TEST_CPPCHECK_JOB_H

#include <QObject>

class TestCppcheckJob : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testJob();
};

#endif
