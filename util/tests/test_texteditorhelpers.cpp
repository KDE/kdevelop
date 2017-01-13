/*
 * Copyright 2016 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "test_texteditorhelpers.h"

#include "texteditorhelpers.h"

#include <QtTest>

QTEST_MAIN(TestKTextEditorHelpers);

using namespace KDevelop;

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
