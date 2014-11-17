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

#include <QSplitter>
#include <QtTest/QtTest>

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
#include <tests/testfile.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ibuddydocumentfinder.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/parsingenvironment.h>

using namespace KDevelop;

#include <KSharedConfig>
#include <QMimeDatabase>
#include <QMimeType>

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

void TestBuddies::init()
{
    // Make sure we start with an empty document set
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

void TestBuddies::cleanup()
{
    m_documentController->closeAllDocuments();
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

void TestBuddies::createFile(const QDir& dir, const QString& filename)
{
    QFile file(dir.filePath(filename));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.close();
}

void TestBuddies::enableBuddies(bool enable)
{
    {
        KConfigGroup uiGroup = KSharedConfig::openConfig()->group("UiSettings");
        uiGroup.writeEntry("TabBarArrangeBuddies", (enable ? 1 : 0));
        uiGroup.sync();
    }
    m_sublimeController->loadSettings();
    QCOMPARE(m_sublimeController->arrangeBuddies(), enable);
}

void TestBuddies::enableOpenAfterCurrent(bool enable)
{
    {
        KConfigGroup uiGroup = KSharedConfig::openConfig()->group("UiSettings");
        uiGroup.writeEntry("TabBarOpenAfterCurrent", (enable ? 1 : 0));
        uiGroup.sync();
    }
    m_sublimeController->loadSettings();
    QCOMPARE(m_sublimeController->openAfterCurrent(), enable);
}

void TestBuddies::testDeclarationDefinitionOrder()
{
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, "a.cpp");
    createFile(dirA, "b.cpp");
    createFile(dirA, "c.cpp");
    createFile(dirA, "a.h");
    createFile(dirA, "b.h");
    createFile(dirA, "c.h");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("b.h")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("c.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("b.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.h")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("c.h")));

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
}

void TestBuddies::testMultiDotFilenames()
{
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, "a.cpp");
    createFile(dirA, "lots.of.dots.cpp");
    createFile(dirA, "b.cpp");
    createFile(dirA, "lots.of.dots.h");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("lots.of.dots.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("b.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("lots.of.dots.h")));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());
    QCOMPARE(m_documentController->openDocuments().count(), 4);
    //QCOMPARE(m_sublimeController->documents().count(), 4);
    QCOMPARE(areaIndex->viewCount(), 4);

    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "lots.of.dots.h");
    verifyFilename(areaIndex->views().value(2), "lots.of.dots.cpp");
    verifyFilename(areaIndex->views().value(3), "b.cpp");
}


void TestBuddies::testActivation()
{
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, "a.h");
    createFile(dirA, "a.cpp");
    createFile(dirA, "b.cpp");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.h")));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "a.h");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("b.cpp")));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "b.cpp");
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
    enableBuddies(false);
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, "a.h");
    createFile(dirA, "a.cpp");
    createFile(dirA, "b.cpp");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.h")));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    // Buddies disabled => order of tabs should be the order of file opening
    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "a.h");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "a.h");

    //activate a.cpp => new doc should be opened right next to it
    toSublimeWindow(m_uiController->activeMainWindow())->activateView(areaIndex->views().value(0));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("b.cpp")));
    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "b.cpp");
    verifyFilename(areaIndex->views().value(2), "a.h");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "b.cpp");
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
    enableBuddies();
    enableOpenAfterCurrent(false);

    QTemporaryDir tempDirA;
    QDir dirA;
    createFile(dirA, "foo.h");
    createFile(dirA, "bar.cpp");
    createFile(dirA, "foo.cpp");
    createFile(dirA, "x.cpp");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("foo.h")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("bar.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("foo.cpp")));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    verifyFilename(areaIndex->views().value(0), "foo.h");
    verifyFilename(areaIndex->views().value(1), "foo.cpp");
    verifyFilename(areaIndex->views().value(2), "bar.cpp");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "foo.cpp");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("x.cpp")));
    verifyFilename(areaIndex->views().value(0), "foo.h");
    verifyFilename(areaIndex->views().value(1), "foo.cpp");
    verifyFilename(areaIndex->views().value(2), "bar.cpp");
    verifyFilename(areaIndex->views().value(3), "x.cpp");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "x.cpp");
}

void TestBuddies::testDisableAll()
{
/*  6. Disable buddy option, Disable open next to active tab
       Open       foo.cpp bar.h foo.h
       Activate   bar.h
       Open       bar.cpp
       Verify order (foo.cpp bar.h foo.h bar.cpp)
       Verify that bar.cpp is activated*/
    enableBuddies(false);
    enableOpenAfterCurrent(false);

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, "foo.h");
    createFile(dirA, "foo.cpp");
    createFile(dirA, "bar.h");
    createFile(dirA, "bar.cpp");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("foo.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("bar.h")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("foo.h")));
    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    //activate bar.h
    toSublimeWindow(m_uiController->activeMainWindow())->activateView(areaIndex->views().value(1));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("bar.cpp")));

    verifyFilename(areaIndex->views().value(0), "foo.cpp");
    verifyFilename(areaIndex->views().value(1), "bar.h");
    verifyFilename(areaIndex->views().value(2), "foo.h");
    verifyFilename(areaIndex->views().value(3), "bar.cpp");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "bar.cpp");
}


void TestBuddies::testMultipleFolders()
{
/*  4. Multiple folders:
       Activate buddy option
       Open f/a.cpp f/xyz.cpp g/a.h
       Verify g/a.h is activated
       Verify order (f/a.cpp f/xyz.cpp g/a.h)*/
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, "a.cpp");
    createFile(dirA, "x.cpp");
    QTemporaryDir tempDirB;
    QDir dirB(tempDirB.path());
    createFile(dirB, "a.h");  // different folder => not dirA/a.cpp's buddy!

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("a.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("x.cpp")));
    m_documentController->openDocument(QUrl::fromLocalFile(dirB.filePath("a.h")));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    verifyFilename(areaIndex->views().value(0), "a.cpp");
    verifyFilename(areaIndex->views().value(1), "x.cpp");
    verifyFilename(areaIndex->views().value(2), "a.h");
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), "a.h");
}

void TestBuddies::testSplitViewBuddies()
{
    Sublime::MainWindow *pMainWindow = toSublimeWindow(m_uiController->activeMainWindow());

    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, "classA.cpp");
    createFile(dirA, "classA.h");

    Sublime::Area *pCodeArea = m_uiController->activeArea();
    QVERIFY(pCodeArea);

    IDocument *pClassAHeader = m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("classA.h")));
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
    IDocument *pClassAImplem = m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath("classA.cpp")));
    QVERIFY(pClassAImplem);
    pMainWindow->activeView()->setObjectName("classA.cpp");

    // and check its presence alongside pNewView in pContainer
    QVERIFY(pContainer->hasWidget(pNewView->widget()));
    QVERIFY(pContainer->hasWidget(pMainWindow->activeView()->widget()));
}

void TestBuddies::testDUChainBuddy()
{
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir dirA;

    TestFile header("void myfunction();\n", "h", 0, dirA.path());
    TestFile other("void otherfunction() {}\n", "cpp", 0, dirA.path());
    TestFile source(
        QString("#include \"%1\"\nvoid myfunction() {}\n").arg(header.url().toUrl().fileName()),
        "cpp", 0, dirA.path()
    );

    header.parseAndWait();
    other.parseAndWait();
    source.parseAndWait();

    // Test IBuddyDocumentFinder::getPotentialBuddies()
    QMimeDatabase db;
    IBuddyDocumentFinder* sourceBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(source.url().toUrl()).name());
    QVector< QUrl > sourceBuddies = sourceBuddyFinder->getPotentialBuddies(source.url().toUrl());
    if (!sourceBuddies.contains(header.url().toUrl())) {
        qDebug() << "got source buddies: " << sourceBuddies;
        qDebug() << "expected: " << header.url().toUrl();
        QFAIL("Failed to find buddy for source file");
    }
    QVERIFY2(!sourceBuddies.contains(other.url().toUrl()), "source buddy list contains unrelated file");

    IBuddyDocumentFinder* headerBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(header.url().toUrl()).name());
    QVector< QUrl > headerBuddies = headerBuddyFinder->getPotentialBuddies(header.url().toUrl());
    if (!headerBuddies.contains(source.url().toUrl())) {
        qDebug() << "got header buddies: " << headerBuddies;
        qDebug() << "expected: " << source.url().toUrl();
        QFAIL("Failed to find buddy for header file");
    }
    QVERIFY2(!headerBuddies.contains(other.url().toUrl()), "header buddy list contains unrelated file");

    // Test IBuddyDocumentFinder::areBuddies()
    QVERIFY(sourceBuddyFinder->areBuddies(source.url().toUrl(), header.url().toUrl()));
}

void TestBuddies::testDUChainBuddyVote()
{
    /*
     * Test that the DUChain buddy system finds the buddy file with the most
     * common declarations/definitions
     */

    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir dirA;

    TestFile header("void func1();\nvoid func2();\nvoid func3();\n", "h", 0, dirA.path());
    TestFile source1(
        QString("#include \"%1\"\nvoid func1() {}\n").arg(header.url().toUrl().fileName()),
        "cpp", 0, dirA.path()
    );
    TestFile source2(
        QString("#include \"%1\"\nvoid func2() {}\nvoid func3() {}\n").arg(header.url().toUrl().fileName()),
        "cpp", 0, dirA.path()
    );

    // -> buddy(header) should resolve to source2

    header.parseAndWait();
    source1.parseAndWait();
    source2.parseAndWait();

    // Test IBuddyDocumentFinder::getPotentialBuddies()
    QMimeDatabase db;
    IBuddyDocumentFinder* sourceBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(source1.url().toUrl()).name());
    QVector< QUrl > sourceBuddies = sourceBuddyFinder->getPotentialBuddies(source1.url().toUrl());
    if (!sourceBuddies.contains(header.url().toUrl())) {
        qDebug() << "got source buddies: " << sourceBuddies;
        qDebug() << "expected: " << header.url().toUrl();
        QFAIL("Failed to find buddy for source file");
    }

    IBuddyDocumentFinder* source2BuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(source2.url().toUrl()).name());
    QVector< QUrl > source2Buddies = source2BuddyFinder->getPotentialBuddies(source2.url().toUrl());
    if (!source2Buddies.contains(header.url().toUrl())) {
        qDebug() << "got source2 buddies: " << source2Buddies;
        qDebug() << "expected: " << header.url().toUrl();
        QFAIL("Failed to find buddy for source2 file");
    }

    IBuddyDocumentFinder* headerBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(header.url().toUrl()).name());
    QVector< QUrl > headerBuddies = headerBuddyFinder->getPotentialBuddies(header.url().toUrl());
    if (!headerBuddies.contains(source2.url().toUrl())) {
        qDebug() << "got header buddies: " << headerBuddies;
        qDebug() << "expected: " << source2.url().toUrl();
        QFAIL("Failed to find buddy for header file");
    }
    QVERIFY2(!headerBuddies.contains(source1.url().toUrl()), "header buddy list contains weaker file");
}

QTEST_MAIN(TestBuddies)
