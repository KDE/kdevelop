/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTCLANGUTILS_H
#define TESTCLANGUTILS_H

#include <QObject>

class TestClangUtils : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testGetScope();
    void testGetScope_data();
    void testTemplateArgumentTypes();
    void testTemplateArgumentTypes_data();
    void testGetRawContents();
    void testGetRawContents_data();
    void testRangeForIncludePathSpec();
    void testGetCursorSignature();
    void testGetCursorSignature_data();
};

#endif // TESTCLANGUTILS_H
