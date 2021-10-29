/*
    SPDX-FileCopyrightText: 2016 Peje Nilsson <peje66@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_CPPCHECK_PARSER_H
#define TEST_CPPCHECK_PARSER_H

#include <QObject>

class TestCppcheckParser : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testParser();
};

#endif
