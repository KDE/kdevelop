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

class Script_DocumentController: public QObject
{
    Q_OBJECT
public:
    Script_DocumentController()
        : tmpFile("kdev")
    {}

private slots:
    void initTestCase();

    void openDocument();
    void documentLoaded();

private:
    QTemporaryFile tmpFile;
    QString tmpFilePath;
};

void Script_DocumentController::initTestCase()
{
    AutoTestShell::init();
    KDevelop::Core::initialize();

    // create a temporary file that we can open with the documentController
    QVERIFY2(tmpFile.open(), qPrintable(tmpFile.errorString()));
    tmpFilePath = QFileInfo(tmpFile).absoluteFilePath();
}

void Script_DocumentController::openDocument()
{
    // make sure no files are open
    QVERIFY(KDevelop::Core::self()->documentController()->openDocuments().isEmpty());

    // create an action that opens the tmp file with the documentController
    Kross::Action action(0, "openDocument");
    action.setInterpreter("qtscript");

    action.setCode(QString("x = KDevTools.toDocumentController(KDevCore.documentController());\n"
                           "x.openDocument('file://%1');").arg(tmpFilePath).toLatin1()
                  );

    action.trigger();
    QVERIFY2(!action.hadError(), qPrintable(action.errorMessage()));

    // make sure we have exactly one document in the list
    QCOMPARE(KDevelop::Core::self()->documentController()->openDocuments().count(), 1);
    // make sure that document is the one we opened
    QCOMPARE(KDevelop::Core::self()->documentController()->openDocuments().at(0)->url().path(),
             tmpFilePath);

    // clean up
    KDevelop::Core::self()->documentController()->closeAllDocuments();
}

void Script_DocumentController::documentLoaded()
{
    // make sure no files are open
    QVERIFY(KDevelop::Core::self()->documentController()->openDocuments().isEmpty());

    // this is a helper object that'll be modified from within the script if the right
    // signal is emitted
    QObject helperObject;

    // create an action that opens the tmp file with the documentController and connects
    // to the documentLoaded signal
    Kross::Action action(0, "documentLoaded");
    action.setInterpreter("qtscript");
    action.addQObject(&helperObject, "Helper");

    action.setCode(QString("function docLoaded(doc) {\n"
                           "    k = KDevTools.toDocument(doc);\n"
                           "    Helper.objectName = k.url();\n"
                           "}\n"
                           "x = KDevTools.toDocumentController(KDevCore.documentController());\n"
                           "x.documentLoaded.connect(docLoaded)\n"
                           "x.openDocument('file://%1');").arg(tmpFilePath).toLatin1()
                  );

    action.trigger();
    QVERIFY2(!action.hadError(), qPrintable(action.errorMessage()));

    QCOMPARE(helperObject.objectName(), "file://" + tmpFilePath);

    // clean up
    KDevelop::Core::self()->documentController()->closeAllDocuments();
}

QTEST_KDEMAIN(Script_DocumentController, GUI)

#include "script_documentcontroller.moc"
