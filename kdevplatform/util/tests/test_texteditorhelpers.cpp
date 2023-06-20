/*
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_texteditorhelpers.h"

#include "texteditorhelpers.h"

#include <QTest>
#include <QStandardPaths>

QTEST_MAIN(TestKTextEditorHelpers)

using namespace KDevelop;

void TestKTextEditorHelpers::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestKTextEditorHelpers::testExtractCursor()
{
    QFETCH(QString, input);
    QFETCH(KTextEditor::Cursor, expectedCursor);
    QFETCH(QString, expectedPath);

    int pathLen;
    const auto cursor = KTextEditorHelpers::extractCursor(input, &pathLen);
    QCOMPARE(cursor, expectedCursor);
    QCOMPARE(input.mid(0, pathLen), expectedPath);
}

void TestKTextEditorHelpers::testExtractCursor_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<KTextEditor::Cursor>("expectedCursor");
    QTest::addColumn<QString>("expectedPath");

    // valid input
    QTest::newRow("file")
        << QStringLiteral("widget.cpp")
        << KTextEditor::Cursor::invalid()
        << QStringLiteral("widget.cpp");
    QTest::newRow("file:line")
        << QStringLiteral("widget.cpp:12")
        << KTextEditor::Cursor(11, 0)
        << QStringLiteral("widget.cpp");
    QTest::newRow("file:line:column")
        << QStringLiteral("widget.cpp:12:5")
        << KTextEditor::Cursor(11, 4)
        << QStringLiteral("widget.cpp");
    QTest::newRow("file:line")
        << QStringLiteral("widget.cpp#12")
        << KTextEditor::Cursor(11, 0)
        << QStringLiteral("widget.cpp");
    QTest::newRow("file:line")
        << QStringLiteral("widget.cpp#L12")
        << KTextEditor::Cursor(11, 0)
        << QStringLiteral("widget.cpp");
    QTest::newRow("file:line")
        << QStringLiteral("widget.cpp#n12")
        << KTextEditor::Cursor(11, 0)
        << QStringLiteral("widget.cpp");
    // partially invalid input
    QTest::newRow("file:")
        << QStringLiteral("widget.cpp:")
        << KTextEditor::Cursor::invalid()
        << QStringLiteral("widget.cpp:");
    QTest::newRow("file:")
        << QStringLiteral("widget.cpp#")
        << KTextEditor::Cursor::invalid()
        << QStringLiteral("widget.cpp#");
    QTest::newRow("file:")
        << QStringLiteral("widget.cpp#L")
        << KTextEditor::Cursor::invalid()
        << QStringLiteral("widget.cpp#L");
    QTest::newRow("file:")
        << QStringLiteral("widget.cpp#n")
        << KTextEditor::Cursor::invalid()
        << QStringLiteral("widget.cpp#n");
}

#include "moc_test_texteditorhelpers.cpp"
