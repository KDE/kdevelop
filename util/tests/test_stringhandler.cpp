/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
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

#include "test_stringhandler.h"

#include "kdevstringhandler.h"

#include <QtTest>

QTEST_MAIN(TestStringHandler);

using namespace KDevelop;

Q_DECLARE_METATYPE(HtmlToPlainTextMode);

void TestStringHandler::testHtmlToPlainText()
{
    QFETCH(QString, html);
    QFETCH(HtmlToPlainTextMode, mode);
    QFETCH(QString, expectedPlainText);

    QString plainText = htmlToPlainText(html, mode);
    QCOMPARE(plainText, expectedPlainText);
}

void TestStringHandler::testHtmlToPlainText_data()
{
    QTest::addColumn<QString>("html");
    QTest::addColumn<HtmlToPlainTextMode>("mode");
    QTest::addColumn<QString>("expectedPlainText");

    QTest::newRow("simple-fast")
        << "<p>bar() </p><dl><dt class=\"param-name-index-0\">a</dt><dd class=\"param-descr-index-0\"> foo</dd></dl>"
        << KDevelop::FastMode << "bar() a foo";
    QTest::newRow("simple-complete")
        << "<p>bar() </p><dl><dt class=\"param-name-index-0\">a</dt><dd class=\"param-descr-index-0\"> foo</dd></dl>"
        << KDevelop::CompleteMode << "bar() \na\nfoo";
}

#include "test_stringhandler.moc"
