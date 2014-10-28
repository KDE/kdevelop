/*
    Unit tests for DocumentController.
    Copyright 2011  Damien Flament <contact.damienflament@gmail.com>

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


#ifndef KDEVPLATFORM_TEST_DOCUMENTCONTROLLER_H
#define KDEVPLATFORM_TEST_DOCUMENTCONTROLLER_H

#include <QtCore/QObject>
#include <KTemporaryFile>
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
private slots :
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testSetEncoding();
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
    KTemporaryFile m_file1;
    KTemporaryFile m_file2;
};

#endif // KDEVPLATFORM_TEST_DOCUMENTCONTROLLER_H
