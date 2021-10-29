/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTSTRINGHANDLER_H
#define TESTSTRINGHANDLER_H

#include <QObject>

class TestStringHandler : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testHtmlToPlainText();
    void testHtmlToPlainText_data();

    void testFindAsciiIdentifierLength();
    void testFindAsciiIdentifierLength_data();
    void testFindAsciiIdentifierLengthNoMatch();
    void testFindAsciiIdentifierLengthNoMatch_data();

    void testMatchUnbracedAsciiVariable();
    void testMatchUnbracedAsciiVariable_data();
    void testUnmatchedAsciiVariable();
    void testUnmatchedAsciiVariable_data();
    void testMatchBracedAsciiVariable();
    void testMatchBracedAsciiVariable_data();

    void testStripAnsiSequences();
    void testStripAnsiSequences_data();

    void testNormalizeLineEndings();
    void testNormalizeLineEndings_data();
};

#endif // TESTSTRINGHANDLER_H
