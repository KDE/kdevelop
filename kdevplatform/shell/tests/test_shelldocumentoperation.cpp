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
#include "test_shelldocumentoperation.h"

#include <QTest>
#include <QAction>

#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kparts/mainwindow.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/movinginterface.h>

#include <sublime/area.h>
#include <sublime/view.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include "../documentcontroller.h"
#include "../uicontroller.h"

using namespace KDevelop;

void TestShellDocumentOperation::initTestCase()
{
    AutoTestShell::init({{}}); // do not load plugins at all
    TestCore::initialize();
}

void TestShellDocumentOperation::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestShellDocumentOperation::testOpenDocumentFromText()
{
    //open some docs
    IDocumentController *documentController = Core::self()->documentController();
    documentController->openDocumentFromText(QStringLiteral("Test1"));

    //test that we have this document in the list, signals are emitted and so on
    QCOMPARE(documentController->openDocuments().count(), 1);
    QCOMPARE(documentController->openDocuments().at(0)->textDocument()->text(), QStringLiteral("Test1"));

    Sublime::Area *area = Core::self()->uiControllerInternal()->activeArea();
    QCOMPARE(area->views().count(), 1);
    documentController->openDocuments().at(0)->close(IDocument::Discard);

    // We used to have a bug where closing document failed to remove its
    // views from area, so check it here.
    QCOMPARE(area->views().count(), 0);
}

void TestShellDocumentOperation::testClosing()
{
    // Test that both the view and the view widget is deleted when closing
    // document.
    {
        IDocumentController *documentController = Core::self()->documentController();
        documentController->openDocumentFromText(QStringLiteral("Test1"));
        Sublime::Area *area = Core::self()->uiControllerInternal()->activeArea();
        QCOMPARE(area->views().count(), 1);
        QPointer<Sublime::View> the_view = area->views().at(0);
        QPointer<QWidget> the_widget = the_view->widget();
        documentController->openDocuments().at(0)->close(IDocument::Discard);
        QCOMPARE(the_view.data(), (Sublime::View*)nullptr);
        QCOMPARE(the_widget.data(), (QWidget*)nullptr);
    }

    // Now try the same, where there are two open documents.
    {
        IDocumentController *documentController = Core::self()->documentController();
        // Annoying, the order of documents in
        // documentController->openDocuments() depends on how URLs hash. So,
        // to reliably close the second one, get hold of a pointer.
        IDocument* doc1 = documentController->openDocumentFromText(QStringLiteral("Test1"));
        IDocument* doc2 = documentController->openDocumentFromText(QStringLiteral("Test2"));
        Sublime::Area *area = Core::self()->uiControllerInternal()->activeArea();
        QCOMPARE(area->views().count(), 2);

        QPointer<Sublime::View> the_view = area->views().at(1);
        qDebug() << this << "see views " << area->views().at(0)
                     << " " << area->views().at(1);
        QPointer<QWidget> the_widget = the_view->widget();
        doc2->close(IDocument::Discard);
        QCOMPARE(the_view.data(), (Sublime::View*)nullptr);
        QCOMPARE(the_widget.data(), (QWidget*)nullptr);
        doc1->close(IDocument::Discard);
    }
}

void TestShellDocumentOperation::testKateDocumentAndViewCreation()
{
    //create one document
    IDocumentController *documentController = Core::self()->documentController();
    documentController->openDocumentFromText(QLatin1String(""));
    QCOMPARE(documentController->openDocuments().count(), 1);

    //assure we have only one kate view for the newly created document
    KTextEditor::Document *doc = documentController->openDocuments().at(0)->textDocument();
    QCOMPARE(doc->views().count(), 1);
    QCOMPARE(dynamic_cast<KTextEditor::MovingInterface*>(doc)->revision(), qint64(0));

    //also assure the view's xmlgui is plugged in
    KParts::MainWindow *main = Core::self()->uiControllerInternal()->activeMainWindow();
    QVERIFY(main);
    QVERIFY(main->guiFactory()->clients().contains(doc->views().at(0)));

    //KTextEditor::views is internally a QHash::keys() call: so the order of the views will vary
    const auto originalView = doc->views().at(0);

    //create the new view and activate it (using split action from mainwindow)
    QAction *splitAction = main->actionCollection()->action(QStringLiteral("split_vertical"));
    QVERIFY(splitAction);
    splitAction->trigger();
    const auto viewList = doc->views();
    QCOMPARE(viewList.count(), 2);

    const auto newlySplitView = originalView == viewList[0] ? viewList[1] : viewList[0];

    //check that we did switch to the new xmlguiclient
    QVERIFY(!main->guiFactory()->clients().contains(originalView));
    QVERIFY(main->guiFactory()->clients().contains(newlySplitView));

    documentController->openDocuments().at(0)->close(IDocument::Discard);
}

QTEST_MAIN(TestShellDocumentOperation)
