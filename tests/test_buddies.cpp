/***************************************************************************
 *   Copyright 2011 Martin Heide <martin.heide@gmx.net>                    *
 *   Copyright 2012 Milian Wolff <mail@milianw.de>                         *
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

#include "test_buddies.h"

#include <qtest_kde.h>

#include <QSplitter>
#include <QtTest/QtTest>

#include <KTempDir>
#include <KParts/MainWindow>
#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/mainwindow.h>
#include <sublime/container.h>
#include <sublime/document.h>
#include <sublime/urldocument.h>
#include <sublime/controller.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

Sublime::MainWindow* toSublimeWindow(KParts::MainWindow* window)
{
    Sublime::MainWindow* ret = dynamic_cast<Sublime::MainWindow*>(window);
    Q_ASSERT(ret);
    return ret;
}

void TestBuddies::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize();
    m_documentController = ICore::self()->documentController();
    m_uiController = ICore::self()->uiController();
    m_sublimeController = m_uiController->controller();
}

void TestBuddies::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestBuddies::verifyFilename(Sublime::View *view, const QString& endOfFilename)
{
    QVERIFY(view);
    if (view) {
        Sublime::UrlDocument *urlDoc = dynamic_cast<Sublime::UrlDocument *>(view->document());
        QVERIFY(urlDoc);
        if (urlDoc) {
            qDebug() << urlDoc->url().toLocalFile() << endOfFilename;
            QVERIFY(urlDoc->url().toLocalFile().endsWith(endOfFilename));
        }
    }
}

void TestBuddies::createFile(const KTempDir& dir, const QString& filename)
{
    QFile file(dir.name() + filename);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.close();
}

void TestBuddies::enableBuddies(bool enable)
{
    {
        KConfigGroup uiGroup = KGlobal::config()->group("UiSettings");
        uiGroup.writeEntry("TabBarArrangeBuddies", (enable ? 1 : 0));
        uiGroup.sync();
    }
    m_sublimeController->loadSettings();
    QCOMPARE(m_sublimeController->arrangeBuddies(), enable);
}

void TestBuddies::enableOpenAfterCurrent(bool enable)
{
    {
        KConfigGroup uiGroup = KGlobal::config()->group("UiSettings");
        uiGroup.writeEntry("TabBarOpenAfterCurrent", (enable ? 1 : 0));
        uiGroup.sync();
    }
    m_sublimeController->loadSettings();
    QCOMPARE(m_sublimeController->openAfterCurrent(), enable);
}

void TestBuddies::testDeclarationDefinitionOrder()
{
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent();

    KTempDir dirA;
    createFile(dirA, "a.cpp");
    createFile(dirA, "b.cpp");
    createFile(dirA, "c.cpp");
    createFile(dirA, "a.h");
    createFile(dirA, "b.h");
    createFile(dirA, "c.h");

    m_documentController->openDocument(KUrl(dirA.name() + "a.cpp"));
    m_documentController->openDocument(KUrl(dirA.name() + "b.h"));
    m_documentController->openDocument(KUrl(dirA.name() + "c.cpp"));
    m_documentController->openDocument(KUrl(dirA.name() + "b.cpp"));
    m_documentController->openDocument(KUrl(dirA.name() + "a.h"));
    m_documentController->openDocument(KUrl(dirA.name() + "c.h"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());
    QCOMPARE(m_documentController->openDocuments().count(), 6);
    //QCOMPARE(m_uiController->documents().count(), 6);
    QCOMPARE(areaIndex->viewCount(), 6);

    verifyFilename(areaIndex->views().value(0), "a.h");
    verifyFilename(areaIndex->views().value(1), "a.cpp");
    verifyFilename(areaIndex->views().value(2), "b.h");
    verifyFilename(areaIndex->views().value(3), "b.cpp");
    verifyFilename(areaIndex->views().value(4), "c.h");
    verifyFilename(areaIndex->views().value(5), "c.cpp");

    for(int i = 0; i < 6; i++)
        m_documentController->openDocuments()[0]->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

void TestBuddies::testMultiDotFilenames()
{
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent();

    KTempDir dirA;
    createFile(dirA, "a.cpp");
    createFile(dirA, "lots.of.dots.cpp");
    createFile(dirA, "b.cpp");
    createFile(dirA, "lots.of.dots.h");

    m_documentController->openDocument(KUrl(dirA.name() + "a.cpp"));
    m_documentController->openDocument(KUrl(dirA.name() + "lots.of.dots.cpp"));
    m_documentController->openDocument(KUrl(dirA.name() + "b.cpp"));
    m_documentController->openDocument(KUrl(dirA.name() + "lots.of.dots.h"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());
    QCOMPARE(m_documentController->openDocuments().count(), 4);
    //QCOMPARE(m_sublimeController->documents().count(), 4);
    QCOMPARE(areaIndex->viewCount(), 4);

    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "lots.of.dots.h");
    verifyFilename(areaIndex->views().value(2), "lots.of.dots.cpp");
    verifyFilename(areaIndex->views().value(3), "b.cpp");

    for(int i = 0; i < 4; i++)
        m_documentController->openDocuments()[0]->close(IDocument::Discard);

    QCOMPARE(m_documentController->openDocuments().count(), 0);
}


void TestBuddies::testActivation()
{
    QCOMPARE(m_documentController->openDocuments().count(), 0);

    enableBuddies();
    enableOpenAfterCurrent();

    KTempDir dirA;
    createFile(dirA, "a.h");
    createFile(dirA, "a.cpp");
    createFile(dirA, "b.cpp");

    m_documentController->openDocument(KUrl(dirA.name() + "a.cpp"));
    m_documentController->openDocument(KUrl(dirA.name() + "a.h"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "a.h");

    m_documentController->openDocument(KUrl(dirA.name() + "b.cpp"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "b.cpp");

    QCOMPARE(m_documentController->openDocuments().count(), 3);
    for(int i = 0; i < 3; i++)
        m_documentController->openDocuments()[0]->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

void TestBuddies::testDisableBuddies()
{
/*  3. Disactivate buddy option, Activate open next to active tab
       Open a.cpp a.h
       Verify order (a.cpp a.h)
       Verify that a.h is activated
       Activate a.cpp
       Open b.cpp
       Verify order (a.cpp b.cpp a.h) */
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies(false);
    enableOpenAfterCurrent();

    KTempDir dirA;
    createFile(dirA, "a.h");
    createFile(dirA, "a.cpp");
    createFile(dirA, "b.cpp");

    m_documentController->openDocument(QString(dirA.name() + "a.cpp"));
    m_documentController->openDocument(QString(dirA.name() + "a.h"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    // Buddies disabled => order of tabs should be the order of file opening
    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "a.h");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "a.h");

    //activate a.cpp => new doc should be opened right next to it
    toSublimeWindow(m_uiController->activeMainWindow())->activateView(areaIndex->views().value(0));

    m_documentController->openDocument(QString(dirA.name() + "b.cpp"));
    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "b.cpp");
    verifyFilename(areaIndex->views().value(2), "a.h");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "b.cpp");

    QCOMPARE(m_documentController->openDocuments().count(), 3);
    for(int i = 0; i < 3; i++)
        m_documentController->openDocuments()[0]->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

void TestBuddies::testDisableOpenAfterCurrent()
{
/*  5. Enable buddy option, Disable open next to active tab
       Open foo.h bar.cpp foo.cpp
       Verify order (foo.h foo.cpp bar.cpp)
       Verify that foo.cpp is activated
       Open x.cpp => tab must be placed at the end
       Verify order (foo.h foo.cpp bar.cpp x.cpp)
       Verify that x.cpp is activated*/
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent(false);

    KTempDir dirA;
    createFile(dirA, "foo.h");
    createFile(dirA, "bar.cpp");
    createFile(dirA, "foo.cpp");
    createFile(dirA, "x.cpp");

    m_documentController->openDocument(QString(dirA.name() + "foo.h"));
    m_documentController->openDocument(QString(dirA.name() + "bar.cpp"));
    m_documentController->openDocument(QString(dirA.name() + "foo.cpp"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    verifyFilename(areaIndex->views().value(0), "foo.h");
    verifyFilename(areaIndex->views().value(1), "foo.cpp");
    verifyFilename(areaIndex->views().value(2), "bar.cpp");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "foo.cpp");

    m_documentController->openDocument(QString(dirA.name() + "x.cpp"));
    verifyFilename(areaIndex->views().value(0), "foo.h");
    verifyFilename(areaIndex->views().value(1), "foo.cpp");
    verifyFilename(areaIndex->views().value(2), "bar.cpp");
    verifyFilename(areaIndex->views().value(3), "x.cpp");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "x.cpp");

    QCOMPARE(m_documentController->openDocuments().count(), 4);
    for(int i = 0; i < 4; i++)
        m_documentController->openDocuments()[0]->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

void TestBuddies::testDisableAll()
{
/*  6. Disable buddy option, Disable open next to active tab
       Open       foo.cpp bar.h foo.h
       Activate   bar.h
       Open       bar.cpp
       Verify order (foo.cpp bar.h foo.h bar.cpp)
       Verify that bar.cpp is activated*/
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies(false);
    enableOpenAfterCurrent(false);

    KTempDir dirA;
    createFile(dirA, "foo.h");
    createFile(dirA, "foo.cpp");
    createFile(dirA, "bar.h");
    createFile(dirA, "bar.cpp");

    m_documentController->openDocument(QString(dirA.name() + "foo.cpp"));
    m_documentController->openDocument(QString(dirA.name() + "bar.h"));
    m_documentController->openDocument(QString(dirA.name() + "foo.h"));
    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    //activate bar.h
    toSublimeWindow(m_uiController->activeMainWindow())->activateView(areaIndex->views().value(1));

    m_documentController->openDocument(QString(dirA.name() + "bar.cpp"));

    verifyFilename(areaIndex->views().value(0), "foo.cpp");
    verifyFilename(areaIndex->views().value(1), "bar.h");
    verifyFilename(areaIndex->views().value(2), "foo.h");
    verifyFilename(areaIndex->views().value(3), "bar.cpp");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "bar.cpp");

    QCOMPARE(m_documentController->openDocuments().count(), 4);
    for(int i = 0; i < 4; i++)
        m_documentController->openDocuments()[0]->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}


void TestBuddies::testMultipleFolders()
{
/*  4. Multiple folders:
       Activate buddy option
       Open f/a.cpp f/xyz.cpp g/a.h
       Verify g/a.h is activated
       Verify order (f/a.cpp f/xyz.cpp g/a.h)*/
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent();

    KTempDir dirA;
    createFile(dirA, "a.cpp");
    createFile(dirA, "x.cpp");
    KTempDir dirB;
    createFile(dirB, "a.h");  // different folder => not dirA/a.cpp's buddy!

    m_documentController->openDocument(QString(dirA.name() + "a.cpp"));
    m_documentController->openDocument(QString(dirA.name() + "x.cpp"));
    m_documentController->openDocument(QString(dirB.name() + "a.h"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "x.cpp");
    verifyFilename(areaIndex->views().value(2), "a.h");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "a.h");

    QCOMPARE(m_documentController->openDocuments().count(), 3);
    for(int i = 0; i < 3; i++)
        m_documentController->openDocuments()[0]->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

void TestBuddies::testSplitViewBuddies()
{
    Sublime::MainWindow *pMainWindow = toSublimeWindow(m_uiController->activeMainWindow());

    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent();

    KTempDir dirA;

    createFile(dirA, "classA.cpp");
    createFile(dirA, "classA.h");

    Sublime::Area *pCodeArea = m_uiController->activeArea();
    QVERIFY(pCodeArea);

    IDocument *pClassAHeader = m_documentController->openDocument(KUrl(dirA.name() + "classA.h"));
    QVERIFY(pClassAHeader);
    pMainWindow->activeView()->setObjectName("classA.h");

    // now, create a splitted view of the active view (pClassAHeader)
    Sublime::View *pNewView = pMainWindow->activeView()->document()->createView();
    pNewView->setObjectName("splitOf" + pMainWindow->activeView()->objectName());
    pCodeArea->addView(pNewView, pMainWindow->activeView(), Qt::Vertical);
    // and activate it
    pMainWindow->activateView(pNewView);

    // get the current view's container from the mainwindow
    QWidget *pCentral = pMainWindow->centralWidget();
    QVERIFY(pCentral);
    QVERIFY(pCentral->inherits("QWidget"));

    QWidget *pSplitter = pCentral->findChild<QSplitter*>();
    QVERIFY(pSplitter);
    QVERIFY(pSplitter->inherits("QSplitter"));

    Sublime::Container *pContainer = pSplitter->findChild<Sublime::Container*>();
    QVERIFY(pContainer);

    // check that it only contains pNewView
    QVERIFY(pContainer->count() == 1 && pContainer->hasWidget(pNewView->widget()));

    // now open the correponding definition file, classA.cpp
    IDocument *pClassAImplem = m_documentController->openDocument(KUrl(dirA.name() + "classA.cpp"));
    QVERIFY(pClassAImplem);
    pMainWindow->activeView()->setObjectName("classA.cpp");

    // and check its presence alongside pNewView in pContainer
    QVERIFY(pContainer->hasWidget(pNewView->widget()));
    QVERIFY(pContainer->hasWidget(pMainWindow->activeView()->widget()));
}

QTEST_MAIN(TestBuddies)

#include "test_buddies.moc"
