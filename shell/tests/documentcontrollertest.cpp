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

#include "documentcontrollertest.h"

#include <QString>

#include <KTextEditor/Document>
#include <KTextEditor/Cursor>

#include <tests/autotestshell.h>
#include <shell/core.h>
#include <interfaces/idocumentcontroller.h>
#include <KStandardDirs>
#include <ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <languagecontroller.h>
#include <documentcontroller.h>
#include <tests/testcore.h>

#include <QtTest/QTest>
#include <QSignalSpy>

using namespace KDevelop;

void DocumentControllerTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();
    Core::self()->languageController()->backgroundParser()->disableProcessing();
    m_subject = Core::self()->documentController();
}

void DocumentControllerTest::init()
{
    Core::self()->documentControllerInternal()->initialize();

    // create temp files
    m_file1.setPrefix(m_tempDir.name());
    m_file2.setPrefix(m_tempDir.name());
    m_file1.setSuffix(".txt");
    m_file2.setSuffix(".txt");
    if(!m_file1.open() || !m_file2.open()) {
        QFAIL("Can't create temp files");
    }

    // pre-conditions
    QVERIFY(m_subject->openDocuments().empty());
    QVERIFY(m_subject->documentForUrl(KUrl()) == 0);
    QVERIFY(m_subject->activeDocument() == 0);
}

void DocumentControllerTest::cleanup()
{
    // ensure there are not opened documents for next test
    foreach(IDocument* document, m_subject->openDocuments()) {
        document->close(IDocument::Discard);
    }

    Core::self()->documentControllerInternal()->cleanup();
}

void DocumentControllerTest::cleanupTestCase()
{
    TestCore::shutdown();
    m_tempDir.unlink();
}

void DocumentControllerTest::testSetEncoding()
{
    QString encoding("latin1");
    m_subject->setEncoding(encoding);
    QCOMPARE(m_subject->encoding(), encoding);
}

void DocumentControllerTest::testOpeningNewDocumentFromText()
{
    qRegisterMetaType<KDevelop::IDocument*>("KDevelop::IDocument*");
    QSignalSpy createdSpy(m_subject, SIGNAL(textDocumentCreated(KDevelop::IDocument*)));
    QVERIFY(createdSpy.isValid());
    QSignalSpy openedSpy(m_subject, SIGNAL(documentOpened(KDevelop::IDocument*)));
    QVERIFY(openedSpy.isValid());

    IDocument* document = m_subject->openDocumentFromText("");
    QVERIFY(document != 0);

    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(openedSpy.count(), 1);

    QVERIFY(!m_subject->openDocuments().empty());
    QVERIFY(m_subject->documentForUrl(document->url()) == document);
    QVERIFY(m_subject->activeDocument() == document);

    QEXPECT_FAIL("", "FIXME? DocumentController::encoding property has no effect on openDocumentFromText().", Continue);
    QCOMPARE(document->textDocument()->encoding(), m_subject->encoding());
}

void DocumentControllerTest::testOpeningDocumentFromUrl()
{
    KUrl url(m_file1.fileName());
    IDocument* document = m_subject->openDocument(url);
    QVERIFY(document != 0);
}

void DocumentControllerTest::testSaveSomeDocuments()
{
    // create documents
    KTempDir dir;
    IDocument *document1 = m_subject->openDocument(createFile(dir, "foo"));
    IDocument *document2 = m_subject->openDocument(createFile(dir, "bar"));
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Clean);

    // edit both documents
    document1->textDocument()->insertText(KTextEditor::Cursor(), "some text");
    document2->textDocument()->insertText(KTextEditor::Cursor(), "some text");
    QCOMPARE(document1->state(), IDocument::Modified);
    QCOMPARE(document2->state(), IDocument::Modified);

    // save one document (Silent == don't ask user)
    m_subject->saveSomeDocuments(QList<IDocument*>() << document1, IDocument::Silent);
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Modified);
}

void DocumentControllerTest::testSaveAllDocuments()
{
    // create documents
    KTempDir dir;
    IDocument *document1 = m_subject->openDocument(createFile(dir, "foo"));
    IDocument *document2 = m_subject->openDocument(createFile(dir, "bar"));
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Clean);

    // edit documents
    document1->textDocument()->insertText(KTextEditor::Cursor(), "some text");
    document2->textDocument()->insertText(KTextEditor::Cursor(), "some text");
    QCOMPARE(document1->state(), IDocument::Modified);
    QCOMPARE(document2->state(), IDocument::Modified);

    // save documents
    m_subject->saveAllDocuments(IDocument::Silent);
    QCOMPARE(document1->state(), IDocument::Clean);
    QCOMPARE(document2->state(), IDocument::Clean);
}


void DocumentControllerTest::testCloseAllDocuments()
{
    // create documents
    m_subject->openDocumentFromText("");
    m_subject->openDocumentFromText("");
    QVERIFY(!m_subject->openDocuments().empty());

    m_subject->closeAllDocuments();
    QVERIFY(m_subject->openDocuments().empty());
}



KUrl DocumentControllerTest::createFile(const KTempDir& dir, const QString& filename)
{
    QFile file(dir.name() + filename);
    bool success = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if(!success)
    {
        QWARN(QString("Failed to create file: " + dir.name() + filename).toAscii().data());
        return KUrl();
    }
    file.close();
    return KUrl(dir.name() + filename);
}

QTEST_MAIN(DocumentControllerTest);

