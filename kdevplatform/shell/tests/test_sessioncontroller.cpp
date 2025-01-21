/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_sessioncontroller.h"

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <KConfigGroup>
#include <KLocalizedString>

#include "../core.h"
#include "../sessioncontroller.h"
#include "../session.h"
#include <QDebug>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTest>

using namespace KDevelop;

//////////////////// Helper Functions ////////////////////////////////////////

void verifySessionDir( const QString& sessiondir, const QString& name, bool exists )
{
    if( exists )
    {
        qDebug() << "checking existing session" << sessiondir;
        QVERIFY( QFileInfo::exists( sessiondir ) );
        QVERIFY( QFileInfo( sessiondir ).isDir() );
        QVERIFY( QFileInfo::exists( sessiondir+"/sessionrc" ) );
        KSharedConfigPtr cfg = KSharedConfig::openConfig( sessiondir+"/sessionrc" );
        QCOMPARE( name, cfg->group("").readEntry( Session::cfgSessionNameEntry, "" ) );
    } else {
        qDebug() << "checking not-existing dir: " << sessiondir;
        QVERIFY( !QFileInfo::exists( sessiondir ) );
    }
}

void verifySessionDir( ISession* s, bool exists = true )
{
    verifySessionDir(s->dataDirectory(), s->name(), exists);
}

////////////////////// Fixture ///////////////////////////////////////////////

void TestSessionController::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    m_core = Core::self();
    qRegisterMetaType<KDevelop::ISession*>();
    qRegisterMetaType<KDevelop::Session*>();
}

void TestSessionController::init()
{
    m_sessionCtrl = m_core->sessionController();
}

void TestSessionController::cleanupTestCase()
{
    const auto sessions = m_sessionCtrl->sessions();
    for (const Session* session : sessions) {
        TryLockSessionResult lock = m_sessionCtrl->tryLockSession(session->id().toString());
        if (lock.lock)
            m_sessionCtrl->deleteSession( lock.lock );
    }

    TestCore::shutdown();
}

void TestSessionController::createSession_data()
{
    QTest::addColumn<QString>( "sessionName" );
    QTest::newRow("SimpleName") << "TestSession";
    QTest::newRow("NonLetterChars") << "Test%$Session";
    QTest::newRow("NonAsciiChars") << QStringLiteral("TöstSession");
}

void TestSessionController::createSession()
{
    QFETCH(QString, sessionName);
    int sessionCount = m_sessionCtrl->sessionNames().count();
    Session* s = m_sessionCtrl->createSession( sessionName );
    QVERIFY( m_sessionCtrl->sessionNames().contains( sessionName )  );
    QCOMPARE( sessionCount+1, m_sessionCtrl->sessionNames().count() );
    verifySessionDir( s );
}

void TestSessionController::renameSession()
{
    const QString sessionName = QStringLiteral("TestSession4");
    const QString newSessionName = QStringLiteral("TestOtherSession4");
    KDevelop::Session *s = m_sessionCtrl->createSession( sessionName );
    QCOMPARE( sessionName, s->name() );
    verifySessionDir( s );
    QSignalSpy spy(s, SIGNAL(sessionUpdated(KDevelop::ISession*)));
    s->setName( newSessionName );
    QCOMPARE( newSessionName, s->name() );
    QCOMPARE( spy.size(), 1 );

    verifySessionDir( s );
}

void TestSessionController::canRenameActiveSession()
{
    const QString sessionName = QStringLiteral("TestSession5");
    const QString newSessionName = QStringLiteral("TestOtherSession5");
    KDevelop::Session *s = m_sessionCtrl->createSession( sessionName );
    QCOMPARE( sessionName, s->name() );
    m_sessionCtrl->loadSession( sessionName );
    QSignalSpy spy(s, SIGNAL(sessionUpdated(KDevelop::ISession*)));
    s->setName( newSessionName );
    QCOMPARE( newSessionName, s->name() );
    QCOMPARE( spy.size(), 1 );

    verifySessionDir( s );
}

void TestSessionController::deleteSession()
{
    const QString sessionName = QStringLiteral("TestSession3");
    int sessionCount = m_sessionCtrl->sessionNames().count();
    QPointer<Session> s = m_sessionCtrl->createSession( sessionName );
    QString sessionId = s->id().toString();
    QCOMPARE( sessionCount+1, m_sessionCtrl->sessionNames().count() );
    verifySessionDir( s.data() );
    const auto sessionDir = s->dataDirectory();

    QSignalSpy spy(m_sessionCtrl, SIGNAL(sessionDeleted(QString)));
    {
        TryLockSessionResult lock = m_sessionCtrl->tryLockSession(sessionId);
        QVERIFY(lock.lock);
        m_sessionCtrl->deleteSession( lock.lock );
    }
    QCOMPARE( sessionCount, m_sessionCtrl->sessionNames().count() );
    QVERIFY( !m_sessionCtrl->sessionNames().contains(sessionId) );

    QCOMPARE(spy.size(), 1);
    QList<QVariant> arguments = spy.takeFirst();

    QString emittedSession = arguments.at(0).toString();
    QCOMPARE( sessionId, emittedSession );

    verifySessionDir( sessionDir, sessionName, false );
}

void TestSessionController::cloneSession()
{
    QString sessionName = QStringLiteral("CloneableSession");
    QString testgrp = QStringLiteral("TestGroup");
    QString testentry = QStringLiteral("TestEntry");
    QString testval = QStringLiteral("TestValue");
    int sessionCount = m_sessionCtrl->sessionNames().count();
    m_sessionCtrl->createSession( sessionName );
    Session* s = m_sessionCtrl->session( sessionName );
    s->config()->group( testgrp ).writeEntry( testentry, testval );
    s->config()->sync();
    QCOMPARE( sessionCount+1, m_sessionCtrl->sessionNames().count() );
    QVERIFY( m_sessionCtrl->session( sessionName ) );

    QString newSession = m_sessionCtrl->cloneSession( sessionName );
    QVERIFY( m_sessionCtrl->session( newSession ) );
    QCOMPARE( sessionCount+2, m_sessionCtrl->sessionNames().count() );
    Session* news = m_sessionCtrl->session( newSession );
    QCOMPARE( testval, news->config()->group( testgrp ).readEntry( testentry, "" ) );
    QCOMPARE( i18n( "Copy of %1", sessionName ), news->name() );

    verifySessionDir( news );


}

void TestSessionController::readFromConfig()
{
    ISession* s = Core::self()->activeSession();
    KConfigGroup grp( s->config(), "TestGroup" );
    grp.writeEntry( "TestEntry", "Test1" );
    KConfigGroup grp2( s->config(), "TestGroup" );
    QCOMPARE(grp.readEntry( "TestEntry", "" ), QStringLiteral( "Test1" ) );
}

void TestSessionController::temporary()
{
    ISession* s = Core::self()->activeSession();
    s->setTemporary(true);
    const QString oldName = s->name();
    const auto dir = s->dataDirectory();

    verifySessionDir(s, true);
    Core::self()->sessionController()->cleanup();
    verifySessionDir(dir, oldName, false);
    Core::self()->sessionController()->initialize(oldName);
    QCOMPARE(Core::self()->activeSession()->name(), oldName);
    // dir / UID can be different, hence don't verifySessionDir
}

void TestSessionController::tryLockSession()
{
    const QString id1 = QUuid::createUuid().toString();
    m_sessionCtrl->createSession( id1 )->setTemporary(true);
    const QString id2 = QUuid::createUuid().toString();
    m_sessionCtrl->createSession( id2 )->setTemporary(true);
    {
        // acquired scoped lock
        QVERIFY(!SessionController::isSessionRunning(id1));
        QCOMPARE(SessionController::sessionRunInfo(id1), SessionRunInfo());
        TryLockSessionResult initial = SessionController::tryLockSession(id1);
        QVERIFY(initial.lock);
        QCOMPARE(initial.lock->id(), id1);
        QCOMPARE(initial.runInfo, SessionRunInfo());
        QVERIFY(SessionController::isSessionRunning(id1));

        SessionRunInfo info = SessionController::sessionRunInfo(id1);
        QVERIFY(info != initial.runInfo);
        QVERIFY(info.isRunning);
        QCOMPARE(info.holderApp, QCoreApplication::applicationName());
        QCOMPARE(info.holderPid, static_cast<int>(QCoreApplication::applicationPid()));

        // this should fail
        TryLockSessionResult repeated = SessionController::tryLockSession(id1);
        QVERIFY(!repeated.lock);
        QCOMPARE(repeated.runInfo, info);

        // this should pass (different id)
        QVERIFY(!SessionController::isSessionRunning(id2));
        TryLockSessionResult other = SessionController::tryLockSession(id2);
        QVERIFY(other.lock);
        QCOMPARE(other.lock->id(), id2);
        QCOMPARE(other.runInfo, SessionRunInfo());
        QVERIFY(SessionController::isSessionRunning(id2));
    }

    // scope left, sessions are now unlocked again
    QVERIFY(!SessionController::isSessionRunning(id1));
    QCOMPARE(SessionController::sessionRunInfo(id1), SessionRunInfo());
    QVERIFY(!SessionController::isSessionRunning(id2));
    QCOMPARE(SessionController::sessionRunInfo(id2), SessionRunInfo());

    // can re-lock it here
    TryLockSessionResult final = SessionController::tryLockSession(id1);
    QVERIFY(SessionController::isSessionRunning(id1));
    QVERIFY(final.lock);
    QCOMPARE(final.lock->id(), id1);
    QCOMPARE(final.runInfo, SessionRunInfo());
}

QTEST_GUILESS_MAIN(TestSessionController)

#include "moc_test_sessioncontroller.cpp"
