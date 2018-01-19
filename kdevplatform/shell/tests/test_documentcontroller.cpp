/*
    Unit tests for DocumentController.*
    Copyright 2011 Damien Flament <contact.damienflament@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "test_documentcontroller.h"

#include <QString>

#include <KTextEditor/Document>
#include <KTextEditor/Cursor>

#include <tests/autotestshell.h>
#include <shell/core.h>
#include <interfaces/idocumentcontroller.h>
#include <ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <languagecontroller.h>
#include <documentcontroller.h>
#include <tests/testcore.h>

#include <QTest>
#include <QSignalSpy>

using namespace KDevelop;

void TestDocumentController::initTestCase()
{
    AutoTestShell::init({{}}); // do not load plugins at all
    TestCore::initialize();
    Core::self()->languageController()->backgroundParser()->disableProcessing();
    m_subject = Core::self()->documentController();
}

void TestDocumentController::init()
{
    Core::self()->documentControllerInternal()->initialize();

    // create temp files
    m_file1.setFileTemplate(m_tempDir.path() + "/tmp_XXXXXX.txt");
    m_file2.setFileTemplate(m_tempDir.path() + "/tmp_XXXXXX.txt");
    if(!m_file1.open() || !m_file2.open()) {
        QFAIL("Can't create temp files");
    }

    // pre-conditions
    QVERIFY(m_subject->openDocuments().empty());
    QVERIFY(m_subject->documentForUrl(QUrl()) == nullptr);
    QVERIFY(m_subject->activeDocument() == nullptr);
}

void TestDocumentController::cleanup()
{
    // ensure there are not opened documents for next test
    foreach(IDocument* document, m_subject->openDocuments()) {
        document->close(IDocument::Discard);
    }

    Core::self()->documentControllerInternal()->cleanup();
}

void TestDocumentController::cleanupTestCase()
{
    TestCore::shutdown();
    m_tempDir.remove();
}

void TestDocumentController::testOpeningNewDocumentFromText()
{
    qRegisterMetaType<KDevelop::IDocument*>("KDevelop::IDocument*");
    QSignalSpy createdSpy(m_subject, SIGNAL(textDocumentCreated(KDevelop::IDocument*)));
    QVERIFY(createdSpy.isValid());
    QSignalSpy openedSpy(m_subject, SIGNAL(documentOpened(KDevelop::IDocument*)));
    QVERIFY(openedSpy.isValid());

    IDocument* document = m_subject->openDocumentFromText(QLatin1String(""));
    QVERIFY(document != nullptr);

    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(openedSpy.count(), 1);

    QVERIFY(!m_subject->openDocuments().empty());
    QVERIFY(m_subject->documentForUrl(document->url()) == document);
    QVERIFY(m_subject->activeDocument() == document);
}

void TestDocumentController::testOpeningDocumentFromUrl()
{
    QUrl url = QUrl::fromLocalFile(m_file1.fileName());
    IDocument* document = m_subject->openDocument(url);
    QVERIFY(document != nullptr);
}

void TestDocumentController::testSaveSomeDocuments()
{
    // create documents
    QTemporaryDir dir;
    IDocument *document1 = m_subject->openDocument(createFile(dir, QStringLiteral("foo")));
    IDocument *document2 = m_subject->openDocument(createFile(dir, QStringLiteral("bar")));
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Clean);

    // edit both documents
    document1->textDocument()->insertText(KTextEditor::Cursor(), QStringLiteral("some text"));
    document2->textDocument()->insertText(KTextEditor::Cursor(), QStringLiteral("some text"));
    QCOMPARE(document1->state(), IDocument::Modified);
    QCOMPARE(document2->state(), IDocument::Modified);

    // save one document (Silent == don't ask user)
    m_subject->saveSomeDocuments(QList<IDocument*>() << document1, IDocument::Silent);
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Modified);
}

void TestDocumentController::testSaveAllDocuments()
{
    // create documents
    QTemporaryDir dir;
    IDocument *document1 = m_subject->openDocument(createFile(dir, QStringLiteral("foo")));
    IDocument *document2 = m_subject->openDocument(createFile(dir, QStringLiteral("bar")));
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Clean);

    // edit documents
    document1->textDocument()->insertText(KTextEditor::Cursor(), QStringLiteral("some text"));
    document2->textDocument()->insertText(KTextEditor::Cursor(), QStringLiteral("some text"));
    QCOMPARE(document1->state(), IDocument::Modified);
    QCOMPARE(document2->state(), IDocument::Modified);

    // save documents
    m_subject->saveAllDocuments(IDocument::Silent);
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Clean);
}


void TestDocumentController::testCloseAllDocuments()
{
    // create documents
    m_subject->openDocumentFromText(QLatin1String(""));
    m_subject->openDocumentFromText(QLatin1String(""));
    QVERIFY(!m_subject->openDocuments().empty());

    m_subject->closeAllDocuments();
    QVERIFY(m_subject->openDocuments().empty());
}



QUrl TestDocumentController::createFile(const QTemporaryDir& dir, const QString& filename)
{
    QFile file(dir.path() + filename);
    bool success = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if(!success)
    {
        QWARN(QString("Failed to create file: " + dir.path() + filename).toLatin1().data());
        return QUrl();
    }
    file.close();
    return QUrl::fromLocalFile(dir.path() + filename);
}

void TestDocumentController::testEmptyUrl()
{
    const auto first = DocumentController::nextEmptyDocumentUrl();
    QVERIFY(DocumentController::isEmptyDocumentUrl(first));
    QCOMPARE(DocumentController::nextEmptyDocumentUrl(), first);

    auto doc = m_subject->openDocumentFromText(QString());
    QCOMPARE(doc->url(), first);

    const auto second = DocumentController::nextEmptyDocumentUrl();
    QVERIFY(first != second);
    QVERIFY(DocumentController::isEmptyDocumentUrl(second));

    QVERIFY(!DocumentController::isEmptyDocumentUrl(QUrl()));
    QVERIFY(!DocumentController::isEmptyDocumentUrl(QUrl(QStringLiteral("http://foo.org"))));
    QVERIFY(!DocumentController::isEmptyDocumentUrl(QUrl(QStringLiteral("http://foo.org/test"))));
    QVERIFY(!DocumentController::isEmptyDocumentUrl(QUrl::fromLocalFile(QStringLiteral("/"))));
    QVERIFY(!DocumentController::isEmptyDocumentUrl(QUrl::fromLocalFile(QStringLiteral("/test"))));
}

QTEST_MAIN(TestDocumentController);
