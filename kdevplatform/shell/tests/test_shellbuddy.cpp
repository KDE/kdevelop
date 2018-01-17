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
#include "test_shellbuddy.h"

#include <QSplitter>
#include <QTest>
#include <QDebug>

#include <kparts/mainwindow.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/mainwindow.h>
#include <sublime/container.h>
#include <sublime/document.h>
#include <sublime/urldocument.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <interfaces/ibuddydocumentfinder.h>

#include "../documentcontroller.h"
#include "../uicontroller.h"

// groups files like foo.l.txt and foo.r.txt such that l is left of r
class TestBuddyFinder : public KDevelop::IBuddyDocumentFinder
{
    bool areBuddies(const QUrl& url1, const QUrl& url2) override
    {
        const QStringList name1 = url1.fileName().split('.');
        const QStringList name2 = url2.fileName().split('.');
        if (name1.size() != 3 || name2.size() != 3) {
            return false;
        }
        if (name1.last() != name2.last() || name1.first() != name2.first()) {
            return false;
        }
        if (name1.at(1) == name2.at(1)) {
            return false;
        }
        if (name1.at(1) != QLatin1String("l") && name1.at(1) != QLatin1String("r")) {
            return false;
        }
        if (name2.at(1) != QLatin1String("l") && name2.at(1) != QLatin1String("r")) {
            return false;
        }
        qDebug() << "found buddies: " << url1 << url2;
        return true;
    }
    bool buddyOrder(const QUrl& url1, const QUrl& /*url2*/) override
    {
        const QStringList name1 = url1.fileName().split('.');
        return name1.at(1) == QLatin1String("l");
    }

    QVector<QUrl> getPotentialBuddies(const QUrl& url) const override
    {
        Q_UNUSED(url);
        return QVector<QUrl>();
    }
};

void TestShellBuddy::initTestCase()
{
    AutoTestShell::init({{}}); // do not load plugins at all
    TestCore::initialize();
    m_documentController = Core::self()->documentController();
    m_uiController = Core::self()->uiControllerInternal();

    m_finder = new TestBuddyFinder;
    KDevelop::IBuddyDocumentFinder::addFinder(QStringLiteral("text/plain"), m_finder);
}

void TestShellBuddy::cleanupTestCase()
{
    KDevelop::IBuddyDocumentFinder::removeFinder(QStringLiteral("text/plain"));
    delete m_finder;
    m_finder = nullptr;
    TestCore::shutdown();
}

//NOTE: macro for proper line-numbers in test's output in case the check fails
#define verifyFilename(view, endOfFilename) \
    QVERIFY(view); \
    { \
        Sublime::UrlDocument *urlDoc = dynamic_cast<Sublime::UrlDocument *>(view->document()); \
        QVERIFY(urlDoc); \
        QVERIFY(urlDoc->url().toLocalFile().endsWith(QStringLiteral(endOfFilename))); \
    }

void TestShellBuddy::createFile(const QTemporaryDir& dir, const QString& filename)
{
    QFile file(dir.path() + '/' + filename);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.close();
}


void TestShellBuddy::enableBuddies(bool enable)
{
    {
        KConfigGroup uiGroup = KSharedConfig::openConfig()->group("UiSettings");
        uiGroup.writeEntry("TabBarArrangeBuddies", (enable ? 1 : 0));
        uiGroup.sync();
    }
    Core::self()->uiControllerInternal()->loadSettings();
    QCOMPARE(Core::self()->uiControllerInternal()->arrangeBuddies(), enable);
}


void TestShellBuddy::enableOpenAfterCurrent(bool enable)
{
    {
        KConfigGroup uiGroup = KSharedConfig::openConfig()->group("UiSettings");
        uiGroup.writeEntry("TabBarOpenAfterCurrent", (enable ? 1 : 0));
        uiGroup.sync();
    }
    Core::self()->uiControllerInternal()->loadSettings();
    QCOMPARE(Core::self()->uiControllerInternal()->openAfterCurrent(), enable);
}


// ------------------ Tests -------------------------------------------------


void TestShellBuddy::testDeclarationDefinitionOrder()
{
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir dirA;
    createFile(dirA, QStringLiteral("a.r.txt"));
    createFile(dirA, QStringLiteral("b.r.txt"));
    createFile(dirA, QStringLiteral("c.r.txt"));
    createFile(dirA, QStringLiteral("a.l.txt"));
    createFile(dirA, QStringLiteral("b.l.txt"));
    createFile(dirA, QStringLiteral("c.l.txt"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "a.r.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "b.l.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "c.r.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "b.r.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "a.l.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "c.l.txt"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(m_uiController->activeSublimeWindow()->activeView());
    QCOMPARE(m_documentController->openDocuments().count(), 6);
    //QCOMPARE(m_uiController->documents().count(), 6);
    QCOMPARE(areaIndex->viewCount(), 6);

    qDebug() << dynamic_cast<Sublime::UrlDocument*>(areaIndex->viewAt(0)->document())->url();
    verifyFilename(areaIndex->views().value(0), "a.l.txt");
    verifyFilename(areaIndex->views().value(1), "a.r.txt");
    verifyFilename(areaIndex->views().value(2), "b.l.txt");
    verifyFilename(areaIndex->views().value(3), "b.r.txt");
    verifyFilename(areaIndex->views().value(4), "c.l.txt");
    verifyFilename(areaIndex->views().value(5), "c.r.txt");

    for(int i = 0; i < 6; i++)
        m_documentController->openDocuments().at(0)->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

void TestShellBuddy::testActivation()
{
    QCOMPARE(m_documentController->openDocuments().count(), 0);

    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir dirA;
    createFile(dirA, QStringLiteral("a.l.txt"));
    createFile(dirA, QStringLiteral("a.r.txt"));
    createFile(dirA, QStringLiteral("b.r.txt"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "a.r.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "a.l.txt"));
    verifyFilename(m_uiController->activeSublimeWindow()->activeView(), "a.l.txt");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "b.r.txt"));
    verifyFilename(m_uiController->activeSublimeWindow()->activeView(), "b.r.txt");

    QCOMPARE(m_documentController->openDocuments().count(), 3);
    for(int i = 0; i < 3; i++)
        m_documentController->openDocuments().at(0)->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}


void TestShellBuddy::testDisableBuddies()
{
/*  3. Deactivate buddy option, Activate open next to active tab
       Open a.cpp a.l.txt
       Verify order (a.cpp a.l.txt)
       Verify that a.l.txt is activated
       Activate a.cpp
       Open b.cpp
       Verify order (a.cpp b.cpp a.l.txt) */
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies(false);
    enableOpenAfterCurrent();

    QTemporaryDir dirA;
    createFile(dirA, QStringLiteral("a.l.txt"));
    createFile(dirA, QStringLiteral("a.r.txt"));
    createFile(dirA, QStringLiteral("b.r.txt"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "a.r.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "a.l.txt"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(m_uiController->activeSublimeWindow()->activeView());

    // Buddies disabled => order of tabs should be the order of file opening
    verifyFilename(areaIndex->views().value(0), "a.r.txt");
    verifyFilename(areaIndex->views().value(1), "a.l.txt");
    verifyFilename(m_uiController->activeSublimeWindow()->activeView(), "a.l.txt");

    //activate a.cpp => new doc should be opened right next to it
    m_uiController->activeSublimeWindow()->activateView(areaIndex->views().value(0));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "b.r.txt"));
    verifyFilename(areaIndex->views().value(0), "a.r.txt");
    verifyFilename(areaIndex->views().value(1), "b.r.txt");
    verifyFilename(areaIndex->views().value(2), "a.l.txt");
    verifyFilename(m_uiController->activeSublimeWindow()->activeView(), "b.r.txt");

    QCOMPARE(m_documentController->openDocuments().count(), 3);
    for(int i = 0; i < 3; i++)
        m_documentController->openDocuments().at(0)->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}


void TestShellBuddy::testDisableOpenAfterCurrent()
{
/*  5. Enable buddy option, Disable open next to active tab
       Open foo.l.txt bar.cpp foo.cpp
       Verify order (foo.l.txt foo.cpp bar.cpp)
       Verify that foo.cpp is activated
       Open x.cpp => tab must be placed at the end
       Verify order (foo.l.txt foo.cpp bar.cpp x.cpp)
       Verify that x.cpp is activated*/
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent(false);

    QTemporaryDir dirA;
    createFile(dirA, QStringLiteral("foo.l.txt"));
    createFile(dirA, QStringLiteral("bar.r.txt"));
    createFile(dirA, QStringLiteral("foo.r.txt"));
    createFile(dirA, QStringLiteral("x.r.txt"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "foo.l.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "bar.r.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "foo.r.txt"));

    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(m_uiController->activeSublimeWindow()->activeView());

    verifyFilename(areaIndex->views().value(0), "foo.l.txt");
    verifyFilename(areaIndex->views().value(1), "foo.r.txt");
    verifyFilename(areaIndex->views().value(2), "bar.r.txt");
    verifyFilename(m_uiController->activeSublimeWindow()->activeView(), "foo.r.txt");

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "x.r.txt"));
    verifyFilename(areaIndex->views().value(0), "foo.l.txt");
    verifyFilename(areaIndex->views().value(1), "foo.r.txt");
    verifyFilename(areaIndex->views().value(2), "bar.r.txt");
    verifyFilename(areaIndex->views().value(3), "x.r.txt");
    verifyFilename(m_uiController->activeSublimeWindow()->activeView(), "x.r.txt");

    QCOMPARE(m_documentController->openDocuments().count(), 4);
    for(int i = 0; i < 4; i++)
        m_documentController->openDocuments().at(0)->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}


void TestShellBuddy::testDisableAll()
{
/*  6. Disable buddy option, Disable open next to active tab
       Open       foo.cpp bar.l.txt foo.l.txt
       Activate   bar.l.txt
       Open       bar.cpp
       Verify order (foo.cpp bar.l.txt foo.l.txt bar.cpp)
       Verify that bar.cpp is activated*/
    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies(false);
    enableOpenAfterCurrent(false);

    QTemporaryDir dirA;
    createFile(dirA, QStringLiteral("foo.l.txt"));
    createFile(dirA, QStringLiteral("foo.r.txt"));
    createFile(dirA, QStringLiteral("bar.l.txt"));
    createFile(dirA, QStringLiteral("bar.r.txt"));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "foo.r.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "bar.l.txt"));
    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "foo.l.txt"));
    Sublime::Area *area = m_uiController->activeArea();
    Sublime::AreaIndex* areaIndex = area->indexOf(m_uiController->activeSublimeWindow()->activeView());

    //activate bar.l.txt
    m_uiController->activeSublimeWindow()->activateView(areaIndex->views().value(1));

    m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "bar.r.txt"));

    verifyFilename(areaIndex->views().value(0), "foo.r.txt");
    verifyFilename(areaIndex->views().value(1), "bar.l.txt");
    verifyFilename(areaIndex->views().value(2), "foo.l.txt");
    verifyFilename(areaIndex->views().value(3), "bar.r.txt");
    verifyFilename(m_uiController->activeSublimeWindow()->activeView(), "bar.r.txt");

    QCOMPARE(m_documentController->openDocuments().count(), 4);
    for(int i = 0; i < 4; i++)
        m_documentController->openDocuments().at(0)->close(IDocument::Discard);
    QCOMPARE(m_documentController->openDocuments().count(), 0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void TestShellBuddy::testsplitViewBuddies()
{
    Sublime::MainWindow *pMainWindow = m_uiController->activeSublimeWindow();

    QCOMPARE(m_documentController->openDocuments().count(), 0);
    enableBuddies();
    enableOpenAfterCurrent();

    QTemporaryDir dirA;

    createFile(dirA, QStringLiteral("classA.r.txt"));
    createFile(dirA, QStringLiteral("classA.l.txt"));
    createFile(dirA, QStringLiteral("foo.txt"));

    Sublime::Area *pCodeArea = m_uiController->activeArea();
    QVERIFY(pCodeArea);

    IDocument *pClassAHeader = m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "classA.l.txt"));
    QVERIFY(pClassAHeader);
    Sublime::View *pClassAHeaderView = pMainWindow->activeView();
    pClassAHeaderView->setObjectName(QStringLiteral("classA.l.txt"));

    // now, create a split view of the active view (pClassAHeader)
    Sublime::View *pClassAHeaderSplitView = dynamic_cast<Sublime::Document*>(pClassAHeader)->createView();
    pClassAHeaderSplitView->setObjectName("splitOf" + pMainWindow->activeView()->objectName());
    pCodeArea->addView(pClassAHeaderSplitView, pMainWindow->activeView(), Qt::Vertical);
    // and activate it
    pMainWindow->activateView(pClassAHeaderSplitView);

    // get the current view's container from the mainwindow
    QWidget *pCentral = pMainWindow->centralWidget();
    QVERIFY(pCentral);
    QVERIFY(pCentral->inherits("QWidget"));

    QWidget *pSplitter = pCentral->findChild<QSplitter*>();
    QVERIFY(pSplitter);
    QVERIFY(pSplitter->inherits("QSplitter"));

    Sublime::Container *pLeftContainer  = pSplitter->findChildren<Sublime::Container*>().at(1);
    QVERIFY(pLeftContainer);
    Sublime::Container *pRightContainer = pSplitter->findChildren<Sublime::Container*>().at(0);
    QVERIFY(pRightContainer);

    // check that it only contains pClassAHeaderSplitView
    QVERIFY(pRightContainer->count() == 1 && pRightContainer->hasWidget(pClassAHeaderSplitView->widget()));

    // now open the correponding definition file, classA.r.txt
    IDocument *pClassAImplem = m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "classA.r.txt"));
    QVERIFY(pClassAImplem);
    pMainWindow->activeView()->setObjectName(QStringLiteral("classA.r.txt"));

    // and check its presence alongside pClassAHeaderSplitView in pRightContainer
    QVERIFY(pRightContainer->hasWidget(pClassAHeaderSplitView->widget()));
    QVERIFY(pRightContainer->hasWidget(pMainWindow->activeView()->widget()));

    // Now reactivate left side ClassAHeaderview
    pMainWindow->activateView(pClassAHeaderView);

    // open another file
    IDocument *pLeftSideCpp = m_documentController->openDocument(QUrl::fromLocalFile(dirA.path() + "foo.txt"));
    QVERIFY(pLeftSideCpp);
    pMainWindow->activeView()->setObjectName(QStringLiteral("foo.txt"));

    // and close left side ClassAHeaderview
    pCodeArea->closeView(pClassAHeaderView);

    // try to open classAImpl (which is already on the right)
    // but this time it should open on the left
    bool successfullyReOpened = m_documentController->openDocument(pClassAImplem);
    QVERIFY(successfullyReOpened);
    pMainWindow->activeView()->setObjectName(QStringLiteral("classA.r.txt"));

    // and check if it correctly opened on the left side
    QVERIFY(pLeftContainer->hasWidget(pMainWindow->activeView()->widget()));
}


QTEST_MAIN(TestShellBuddy)
