/*
    SPDX-FileCopyrightText: 2011 Damien Flament <contact.damienflament@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_DOCUMENTCONTROLLER_H
#define KDEVPLATFORM_TEST_DOCUMENTCONTROLLER_H

#include <QObject>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QUrl>

namespace KDevelop
{
class IDocument;
class IDocumentController;
}

using namespace KDevelop;

class TestDocumentController : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testOpeningNewDocumentFromText();
    void testOpeningDocumentFromUrl();
    void testSaveSomeDocuments();
    void testSaveAllDocuments();
    void testCloseAllDocuments();
    void testEmptyUrl();

private :
    QUrl createFile(const QTemporaryDir& dir, const QString& filename);

    IDocumentController* m_subject;
    QTemporaryDir m_tempDir;
    QTemporaryFile m_file1;
    QTemporaryFile m_file2;
};

#endif // KDEVPLATFORM_TEST_DOCUMENTCONTROLLER_H
