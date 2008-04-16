/***************************************************************************
 *   Copyright 2008 Harald Fernengel <harry@kdevelop.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QtTest>
#include <qtest_kde.h>

#include <kross/core/action.h>
#include <kross/core/manager.h>

#include "../../test/common/autotestshell.h"

#include "idocumentcontroller.h"
#include "idocument.h"

class Script_Document: public QObject
{
    Q_OBJECT
public:
    Script_Document()
        : tmpFile("kdev")
    {}

private slots:
    void initTestCase();
    void cleanupTestCase();

    void activeDocument();

private:
    QTemporaryFile tmpFile;
    QString tmpFilePath;
};

void Script_Document::initTestCase()
{
    AutoTestShell::init();
    KDevelop::Core::initialize();

    // create a temporary file for testing
    QVERIFY2(tmpFile.open(), qPrintable(tmpFile.errorString()));
    tmpFilePath = QFileInfo(tmpFile).absoluteFilePath();

    // open it for testing
    QVERIFY(KDevelop::Core::self()->documentController()->openDocument("file://" + tmpFilePath));
}

void Script_Document::cleanupTestCase()
{
    KDevelop::Core::self()->documentController()->closeAllDocuments();
    // the QTemporaryFile destructor will make sure the file will be deleted.
}

void Script_Document::activeDocument()
{
    // make sure our file is active
    QVERIFY(KDevelop::Core::self()->documentController()->activeDocument());
    QCOMPARE(KDevelop::Core::self()->documentController()->activeDocument()->url().path(),
             tmpFilePath);

    // create an action that gets the url from our active document
    Kross::Action action(0, "activeDocument");
    action.setInterpreter("qtscript");

    action.setCode(QString("x = KDevTools.toDocumentController(KDevCore.documentController());\n"
                           "y = KDevTools.toDocument(x.activeDocument());\n"
                           "if (!y) { throw 'unable to get active document!' }\n"
                           "if (y.url() != '%1') { throw 'URL does not match!' }").arg("file://" + tmpFilePath).toLatin1()
                  );

    action.trigger();
    QVERIFY2(!action.hadError(), qPrintable(action.errorMessage()));
}

QTEST_KDEMAIN(Script_Document, GUI)

#include "script_document.moc"
