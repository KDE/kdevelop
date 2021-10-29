/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_CLANGTIDYPARSER_H
#define TEST_CLANGTIDYPARSER_H

#include <QObject>

class TestClangTidyParser : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testParser();
};

#endif // TEST_CLANGTIDYPARSER_H
