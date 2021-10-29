/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTCOMPILERPROVIDER_H
#define TESTCOMPILERPROVIDER_H

#include <QObject>

class TestCompilerProvider : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testRegisterCompiler();
    void testCompilerIncludesAndDefines();
    void testStorageBackwardsCompatible();
    void testCompilerIncludesAndDefinesForProject();
    void testStorageNewSystem();
};

#endif
