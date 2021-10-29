/*
    SPDX-FileCopyrightText: 2015 Artur Puzio <cytadela88@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTENVIRONMENT_H
#define TESTENVIRONMENT_H

#include <QObject>

class TestEnvironment : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testExpandVariables_data();
    void testExpandVariables();
};

#endif // TESTENVIRONMENT_H
