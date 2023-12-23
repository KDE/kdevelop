/*
    SPDX-FileCopyrightText: 2011 Martin Heide <martin.heide@gmx.net>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_buddies.h"

#include <QSplitter>
#include <QTest>
#include <QLoggingCategory>

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
    auto* ret = qobject_cast<Sublime::MainWindow*>(window);
    Q_ASSERT(ret);
    return ret;
}

void TestBuddies::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    AutoTestShell::init({QStringLiteral("kdevclangsupport")});
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
        auto* urlDoc = qobject_cast<Sublime::UrlDocument*>(view->document());
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
        KConfigGroup uiGroup = KSharedConfig::openConfig()->group(QStringLiteral("UiSettings"));
        uiGroup.writeEntry("TabBarArrangeBuddies", (enable ? 1 : 0));
        uiGroup.sync();
    }
    m_sublimeController->loadSettings();
    QCOMPARE(m_sublimeController->arrangeBuddies(), enable);
}

void TestBuddies::enableOpenAfterCurrent(bool enable)
{
    {
        KConfigGroup uiGroup = KSharedConfig::openConfig()->group(QStringLiteral("UiSettings"));
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
    createFile(dirA, QStringLiteral("a.cpp"));
    createFile(dirA, QStringLiteral("b.cpp"));
    createFile(dirA, QStringLiteral("c.cpp"));
    createFile(dirA, QStringLiteral("a.h"));
    createFile(dirA, QStringLiteral("b.h"));
    createFile(dirA, QStringLiteral("c.h"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("b.h"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("c.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("b.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.h"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("c.h"))));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());
    QCOMPARE(m_documentController->openDocuments().count(), 6);
    //QCOMPARE(m_uiController->documents().count(), 6);
    QCOMPARE(areaIndex->viewCount(), 6);

    verifyFilename(areaIndex->views().value(0), QStringLiteral("a.h"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("a.cpp"));
    verifyFilename(areaIndex->views().value(2), QStringLiteral("b.h"));
    verifyFilename(areaIndex->views().value(3), QStringLiteral("b.cpp"));
    verifyFilename(areaIndex->views().value(4), QStringLiteral("c.h"));
    verifyFilename(areaIndex->views().value(5), QStringLiteral("c.cpp"));
}

void TestBuddies::testMultiDotFilenames()
{
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, QStringLiteral("a.cpp"));
    createFile(dirA, QStringLiteral("lots.of.dots.cpp"));
    createFile(dirA, QStringLiteral("b.cpp"));
    createFile(dirA, QStringLiteral("lots.of.dots.h"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("lots.of.dots.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("b.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("lots.of.dots.h"))));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());
    QCOMPARE(m_documentController->openDocuments().count(), 4);
    //QCOMPARE(m_sublimeController->documents().count(), 4);
    QCOMPARE(areaIndex->viewCount(), 4);

    verifyFilename(areaIndex->views().value(0), QStringLiteral("a.cpp"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("lots.of.dots.h"));
    verifyFilename(areaIndex->views().value(2), QStringLiteral("lots.of.dots.cpp"));
    verifyFilename(areaIndex->views().value(3), QStringLiteral("b.cpp"));
}


void TestBuddies::testActivation()
{
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, QStringLiteral("a.h"));
    createFile(dirA, QStringLiteral("a.cpp"));
    createFile(dirA, QStringLiteral("b.cpp"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.h"))));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("a.h"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("b.cpp"))));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("b.cpp"));
}

void TestBuddies::testDisableBuddies()
{
/*  3. Deactivate buddy option, Activate open next to active tab
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
    createFile(dirA, QStringLiteral("a.h"));
    createFile(dirA, QStringLiteral("a.cpp"));
    createFile(dirA, QStringLiteral("b.cpp"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.h"))));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    // Buddies disabled => order of tabs should be the order of file opening
    verifyFilename(areaIndex->views().value(0), QStringLiteral("a.cpp"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("a.h"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("a.h"));

    //activate a.cpp => new doc should be opened right next to it
    toSublimeWindow(m_uiController->activeMainWindow())->activateView(areaIndex->views().value(0));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("b.cpp"))));
    verifyFilename(areaIndex->views().value(0), QStringLiteral("a.cpp"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("b.cpp"));
    verifyFilename(areaIndex->views().value(2), QStringLiteral("a.h"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("b.cpp"));
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
    createFile(dirA, QStringLiteral("foo.h"));
    createFile(dirA, QStringLiteral("bar.cpp"));
    createFile(dirA, QStringLiteral("foo.cpp"));
    createFile(dirA, QStringLiteral("x.cpp"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("foo.h"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("bar.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("foo.cpp"))));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    verifyFilename(areaIndex->views().value(0), QStringLiteral("foo.h"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("foo.cpp"));
    verifyFilename(areaIndex->views().value(2), QStringLiteral("bar.cpp"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("foo.cpp"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("x.cpp"))));
    verifyFilename(areaIndex->views().value(0), QStringLiteral("foo.h"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("foo.cpp"));
    verifyFilename(areaIndex->views().value(2), QStringLiteral("bar.cpp"));
    verifyFilename(areaIndex->views().value(3), QStringLiteral("x.cpp"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("x.cpp"));
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
    createFile(dirA, QStringLiteral("foo.h"));
    createFile(dirA, QStringLiteral("foo.cpp"));
    createFile(dirA, QStringLiteral("bar.h"));
    createFile(dirA, QStringLiteral("bar.cpp"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("foo.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("bar.h"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("foo.h"))));
    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    //activate bar.h
    toSublimeWindow(m_uiController->activeMainWindow())->activateView(areaIndex->views().value(1));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("bar.cpp"))));

    verifyFilename(areaIndex->views().value(0), QStringLiteral("foo.cpp"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("bar.h"));
    verifyFilename(areaIndex->views().value(2), QStringLiteral("foo.h"));
    verifyFilename(areaIndex->views().value(3), QStringLiteral("bar.cpp"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("bar.cpp"));
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
    createFile(dirA, QStringLiteral("a.cpp"));
    createFile(dirA, QStringLiteral("x.cpp"));
    QTemporaryDir tempDirB;
    QDir dirB(tempDirB.path());
    createFile(dirB, QStringLiteral("a.h"));  // different folder => not dirA/a.cpp's buddy!

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("a.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("x.cpp"))));
    m_documentController->openDocument(QUrl::fromLocalFile(dirB.filePath(QStringLiteral("a.h"))));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(toSublimeWindow(m_uiController->activeMainWindow())->activeView());

    verifyFilename(areaIndex->views().value(0), QStringLiteral("a.cpp"));
    verifyFilename(areaIndex->views().value(1), QStringLiteral("x.cpp"));
    verifyFilename(areaIndex->views().value(2), QStringLiteral("a.h"));
    verifyFilename(toSublimeWindow(m_uiController->activeMainWindow())->activeView(), QStringLiteral("a.h"));
}

void TestBuddies::testSplitViewBuddies()
{
    Sublime::MainWindow *pMainWindow = toSublimeWindow(m_uiController->activeMainWindow());

    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir tempDirA;
    QDir dirA(tempDirA.path());
    createFile(dirA, QStringLiteral("classA.cpp"));
    createFile(dirA, QStringLiteral("classA.h"));

    Sublime::Area *pCodeArea = m_uiController->activeArea();
    QVERIFY(pCodeArea);

    IDocument *pClassAHeader = m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("classA.h"))));
    QVERIFY(pClassAHeader);
    pMainWindow->activeView()->setObjectName(QStringLiteral("classA.h"));

    // now, create a split view of the active view (pClassAHeader)
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

    auto *pContainer = pSplitter->findChild<Sublime::Container*>();
    QVERIFY(pContainer);

    // check that it only contains pNewView
    QVERIFY(pContainer->count() == 1 && pContainer->hasWidget(pNewView->widget()));

    // now open the corresponding definition file, classA.cpp
    IDocument *pClassAImplem = m_documentController->openDocument(QUrl::fromLocalFile(dirA.filePath(QStringLiteral("classA.cpp"))));
    QVERIFY(pClassAImplem);
    pMainWindow->activeView()->setObjectName(QStringLiteral("classA.cpp"));

    // and check its presence alongside pNewView in pContainer
    QVERIFY(pContainer->hasWidget(pNewView->widget()));
    QVERIFY(pContainer->hasWidget(pMainWindow->activeView()->widget()));
}

void TestBuddies::testDUChainBuddy()
{
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir dirA;

    TestFile header(QStringLiteral("void myfunction();\n"), QStringLiteral("h"), nullptr, dirA.path());
    TestFile other(QStringLiteral("void otherfunction() {}\n"), QStringLiteral("cpp"), nullptr, dirA.path());
    TestFile source(
        QStringLiteral("#include \"%1\"\nvoid myfunction() {}\n").arg(header.url().toUrl().fileName()),
        QStringLiteral("cpp"), nullptr, dirA.path()
    );

    header.parseAndWait();
    other.parseAndWait();
    source.parseAndWait();

    // Test IBuddyDocumentFinder::potentialBuddies()
    QMimeDatabase db;
    IBuddyDocumentFinder* sourceBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(source.url().toUrl()).name());
    QVector< QUrl > sourceBuddies = sourceBuddyFinder->potentialBuddies(source.url().toUrl());
    if (!sourceBuddies.contains(header.url().toUrl())) {
        qDebug() << "got source buddies: " << sourceBuddies;
        qDebug() << "expected: " << header.url().toUrl();
        QFAIL("Failed to find buddy for source file");
    }
    QVERIFY2(!sourceBuddies.contains(other.url().toUrl()), "source buddy list contains unrelated file");

    IBuddyDocumentFinder* headerBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(header.url().toUrl()).name());
    QVector< QUrl > headerBuddies = headerBuddyFinder->potentialBuddies(header.url().toUrl());
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

    TestFile header(QStringLiteral("void func1();\nvoid func2();\nvoid func3();\n"), QStringLiteral("h"), nullptr, dirA.path());
    TestFile source1(
        QStringLiteral("#include \"%1\"\nvoid func1() {}\n").arg(header.url().toUrl().fileName()),
        QStringLiteral("cpp"), nullptr, dirA.path()
    );
    TestFile source2(
        QStringLiteral("#include \"%1\"\nvoid func2() {}\nvoid func3() {}\n").arg(header.url().toUrl().fileName()),
        QStringLiteral("cpp"), nullptr, dirA.path()
    );

    // -> buddy(header) should resolve to source2

    header.parseAndWait();
    source1.parseAndWait();
    source2.parseAndWait();

    // Test IBuddyDocumentFinder::potentialBuddies()
    QMimeDatabase db;
    IBuddyDocumentFinder* sourceBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(source1.url().toUrl()).name());
    QVector< QUrl > sourceBuddies = sourceBuddyFinder->potentialBuddies(source1.url().toUrl());
    if (!sourceBuddies.contains(header.url().toUrl())) {
        qDebug() << "got source buddies: " << sourceBuddies;
        qDebug() << "expected: " << header.url().toUrl();
        QFAIL("Failed to find buddy for source file");
    }

    IBuddyDocumentFinder* source2BuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(source2.url().toUrl()).name());
    QVector< QUrl > source2Buddies = source2BuddyFinder->potentialBuddies(source2.url().toUrl());
    if (!source2Buddies.contains(header.url().toUrl())) {
        qDebug() << "got source2 buddies: " << source2Buddies;
        qDebug() << "expected: " << header.url().toUrl();
        QFAIL("Failed to find buddy for source2 file");
    }

    IBuddyDocumentFinder* headerBuddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(header.url().toUrl()).name());
    QVector< QUrl > headerBuddies = headerBuddyFinder->potentialBuddies(header.url().toUrl());
    if (!headerBuddies.contains(source2.url().toUrl())) {
        qDebug() << "got header buddies: " << headerBuddies;
        qDebug() << "expected: " << source2.url().toUrl();
        QFAIL("Failed to find buddy for header file");
    }
    QVERIFY2(!headerBuddies.contains(source1.url().toUrl()), "header buddy list contains weaker file");
}

QTEST_MAIN(TestBuddies)

#include "moc_test_buddies.cpp"
