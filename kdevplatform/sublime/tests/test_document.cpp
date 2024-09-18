/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_document.h"

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/urldocument.h>
#include <sublime/view.h>
#include <tests/testhelpermacros.h>

#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QTest>
#include <QTextEdit>
#include <QUrl>

using namespace Sublime;

namespace {
QUrl makeUrl(const QString& fileName)
{
    constexpr auto nonexistentDirPath =
#ifdef Q_OS_WIN
        "C:"
#endif
        "/non/existent/path/to/a-dir/";

    return QUrl::fromLocalFile(nonexistentDirPath + fileName);
}

void testUrlDocument(const UrlDocument& doc, const QUrl& url)
{
    QCOMPARE(doc.url(), url);
    QCOMPARE(doc.title(Document::TitleType::Normal), url.fileName());

    QCOMPARE(doc.statusIcon(), QIcon{});
    QCOMPARE(doc.icon(), doc.defaultIcon());

    QCOMPARE(doc.mimeType(), QMimeDatabase().mimeTypeForUrl(url));
    QCOMPARE(doc.defaultIcon(), QIcon::fromTheme(doc.mimeType().iconName()));
}

class EditableUrlDocument : public UrlDocument
{
    Q_OBJECT
public:
    using UrlDocument::setUrl;
    using UrlDocument::UrlDocument;
};
} // unnamed namespace

void TestDocument::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestDocument::testUrlDocument_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("c") << "source.c";
    QTest::newRow("cpp") << "test.cpp";
    QTest::newRow("python") << "a.py";
    QTest::newRow("php") << "d.php";
    QTest::newRow("Objective-C") << "obj.m";
    QTest::newRow("CMakeLists") << "CMakeLists.txt";
    QTest::newRow("cmake") << "module.cmake";
    QTest::newRow("diff") << "my.diff";
    QTest::newRow("htm") << "page.htm";
    QTest::newRow("java") << "to.java";
    QTest::newRow("txt") << "bar.txt";
    QTest::newRow("no-extension") << "x";
}

void TestDocument::testUrlDocument()
{
    QFETCH(const QString, fileName);
    const auto url = makeUrl(fileName);

    Controller controller;
    EditableUrlDocument doc(&controller, url);

    ::testUrlDocument(doc, url);
    RETURN_IF_TEST_FAILED();

    const auto plainTextUrl = makeUrl("x.txt");
    doc.setUrl(plainTextUrl);
    ::testUrlDocument(doc, plainTextUrl);
    RETURN_IF_TEST_FAILED();

    doc.setUrl(url);
    ::testUrlDocument(doc, url);
    RETURN_IF_TEST_FAILED();
}

void TestDocument::viewDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(QStringLiteral("tool"), &controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool")));

    View *view = doc->createView();
    view->widget();
    QCOMPARE(doc->views().count(), 1);

    delete view;
    QCOMPARE(doc->views().count(), 0);
}

QTEST_MAIN(TestDocument)

#include "test_document.moc"
#include "moc_test_document.cpp"
