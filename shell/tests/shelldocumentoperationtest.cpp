/***************************************************************************
 *   Copyright 2008 Alexander Dymo <adymo@kdevelop.org>                    *
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
#include "shelldocumentoperationtest.h"

#include <tests/common/kdevtest.h>

#include <QtTest/QtTest>

#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kparts/mainwindow.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <sublime/area.h>

#include "documentcontroller.h"
#include "uicontroller.h"

void ShellDocumentOperationTest::init()
{
    AutoTestShell::init();
    KDevelop::Core::initialize();
}

void ShellDocumentOperationTest::cleanup()
{
}

void ShellDocumentOperationTest::testOpenDocumentFromText()
{
    //open some docs
    IDocumentController *documentController = Core::self()->documentController();
    documentController->openDocumentFromText("Test1");

    //test that we have this document in the list, signals are emitted and so on
    QCOMPARE(documentController->openDocuments().count(), 1);
    QCOMPARE(documentController->openDocuments()[0]->textDocument()->text(), QString("Test1"));

    Sublime::Area *area = Core::self()->uiControllerInternal()->activeArea();
    QCOMPARE(area->views().count(), 1);
    documentController->openDocuments()[0]->close(IDocument::Discard);

    // We used to have a bug where closing document failed to remove its
    // views from area, so check it here.
    QCOMPARE(area->views().count(), 0);    
}

void ShellDocumentOperationTest::testKateDocumentAndViewCreation()
{
    //create one document
    IDocumentController *documentController = Core::self()->documentController();
    documentController->openDocumentFromText("");
    QCOMPARE(documentController->openDocuments().count(), 1);

    //assure we have only one kate view for the newly created document
    KTextEditor::Document *doc = documentController->openDocuments()[0]->textDocument();
    QCOMPARE(doc->views().count(), 1);

    //also assure the view's xmlgui is plugged in
    KParts::MainWindow *main = Core::self()->uiControllerInternal()->activeMainWindow();
    QVERIFY(main);
    QVERIFY(main->guiFactory()->clients().contains(doc->views()[0]));

    //create the new view and activate it (using split action from mainwindow)
    QAction *splitAction = main->actionCollection()->action("split_vertical");
    QVERIFY(splitAction);
    splitAction->trigger();
    QCOMPARE(doc->views().count(), 2);

    //check that we did switch to the new xmlguiclient
    QVERIFY(!main->guiFactory()->clients().contains(doc->views()[0]));
    QVERIFY(main->guiFactory()->clients().contains(doc->views()[1]));

    documentController->openDocuments()[0]->close(IDocument::Discard);
}

KDEVTEST_MAIN(ShellDocumentOperationTest)

#include "shelldocumentoperationtest.moc"
