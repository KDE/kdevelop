/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEST_CUSTOMMAKE_H
#define TEST_CUSTOMMAKE_H

#include <QObject>

class TestCustomMake : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testIncludeDirectories();
    void testFrameworkDirectories();
    void testDefines();
};

#endif // TEST_CUSTOMMAKE_H
