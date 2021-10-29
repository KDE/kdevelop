/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REPLACEMENTSPARSERTESTER_H
#define REPLACEMENTSPARSERTESTER_H

#include <QObject>

class ReplacementsParserTester : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void doTest();
};

#endif // REPLACEMENTSPARSERTESTER_H
