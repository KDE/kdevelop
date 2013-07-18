/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "sessioncontrollertest.h"

#include <qtest_kde.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kconfiggroup.h>

#include "../core.h"
#include "../sessioncontroller.h"
#include "../session.h"
#include "../uicontroller.h"

Q_DECLARE_METATYPE( KDevelop::ISession* )

using namespace KDevelop;

using QTest::kWaitForSignal;

//////////////////// Helper Functions ////////////////////////////////////////

QString sessionDir( ISession* s )
{
    return SessionController::sessionDirectory(s->id().toString());
}

void verifySessionDir( const QString& sessiondir, const QString& name, bool exists )
{
    if( exists ) 
    {
        kDebug() << "checking existing session" << sessiondir;
        QVERIFY( QFileInfo( sessiondir ).exists() );
        QVERIFY( QFileInfo( sessiondir ).isDir() );
        QVERIFY( QFileInfo( sessiondir+"/sessionrc" ).exists() );
        KSharedConfig::Ptr cfg = KSharedConfig::openConfig( sessiondir+"/sessionrc" );
        QCOMPARE( name, cfg->group("").readEntry( Session::cfgSessionNameEntry, "" ) );
    } else {
        kDebug() << "checking not-existing dir: " << sessiondir;
        QVERIFY( !QFileInfo( sessiondir ).exists() );
    }
}

void verifySessionDir( ISession* s, bool exists = true )
{
    verifySessionDir(sessionDir(s), s->name(), exists);
}

////////////////////// Fixture ///////////////////////////////////////////////

void SessionControllerTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    m_core = Core::self();
    qRegisterMetaType<KDevelop::ISession*>();
}

void SessionControllerTest::init()
{
    m_sessionCtrl = m_core->sessionController();
}

void SessionControllerTest::cleanupTestCase()
{
    foreach( const Session* session, m_sessionCtrl->sessions() )
    {
        TryLockSessionResult lock = m_sessionCtrl->tryLockSession(session->id());
        if (lock.lock)
            m_sessionCtrl->deleteSession( lock.lock );
    }

    TestCore::shutdown();
}

void SessionControllerTest::createSession_data()
{
    QTest::addColumn<QString>( "sessionName" );
    QTest::newRow("SimpleName") << "TestSession";
    QTest::newRow("NonLetterChars") << "Test%$Session";
    QTest::newRow("NonAsciiChars") << QString::fromUtf8("TöstSession");
}

void SessionControllerTest::createSession()
{
    QFETCH(QString, sessionName);
    int sessionCount = m_sessionCtrl->sessionNames().count();
    Session* s = m_sessionCtrl->createSession( sessionName );
    QVERIFY( m_sessionCtrl->sessionNames().contains( sessionName )  );
    QCOMPARE( sessionCount+1, m_sessionCtrl->sessionNames().count() );
    verifySessionDir( s );
}

void SessionControllerTest::renameSession()
{
    const QString sessionName = "TestSession4";
    const QString newSessionName = "TestOtherSession4";
    KDevelop::Session *s = m_sessionCtrl->createSession( sessionName );
    QCOMPARE( sessionName, s->name() );
    verifySessionDir( s );
    QSignalSpy spy(s, SIGNAL(nameChanged(QString,QString)));
    s->setName( newSessionName );
    QCOMPARE( newSessionName, s->name() );
    
    QCOMPARE( spy.size(), 1 );
    QList<QVariant> arguments = spy.takeFirst();

    QCOMPARE( sessionName, arguments.at(1).toString() );
    QCOMPARE( newSessionName, arguments.at(0).toString() );

    verifySessionDir( s );
}

void SessionControllerTest::canRenameActiveSession()
{
    const QString sessionName = "TestSession5";
    const QString newSessionName = "TestOtherSession5";
    KDevelop::Session *s = m_sessionCtrl->createSession( sessionName );
    QCOMPARE( sessionName, s->name() );
    m_sessionCtrl->loadSession( sessionName );
    QSignalSpy spy(s, SIGNAL(nameChanged(QString,QString)));
    s->setName( newSessionName );
    QCOMPARE( newSessionName, s->name() );
    
    QCOMPARE( spy.size(), 1 );
    QList<QVariant> arguments = spy.takeFirst();

    QCOMPARE( sessionName, arguments.at(1).toString() );
    QCOMPARE( newSessionName, arguments.at(0).toString() );

    verifySessionDir( s );
}

void SessionControllerTest::deleteSession()
{
    const QString sessionName = "TestSession3";
    int sessionCount = m_sessionCtrl->sessionNames().count();
    Session* s = m_sessionCtrl->createSession( sessionName );
    QString sessionId = s->id().toString();
    QCOMPARE( sessionCount+1, m_sessionCtrl->sessionNames().count() );
    verifySessionDir( s );
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

    verifySessionDir( s, false );
}

void SessionControllerTest::cloneSession()
{
    QString sessionName = "CloneableSession";
    QString testgrp = "TestGroup";
    QString testentry = "TestEntry";
    QString testval = "TestValue";
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

void SessionControllerTest::readFromConfig()
{
    ISession* s = Core::self()->activeSession();
    KConfigGroup grp( s->config(), "TestGroup" );
    grp.writeEntry( "TestEntry", "Test1" );
    KConfigGroup grp2( s->config(), "TestGroup" );
    QCOMPARE(grp.readEntry( "TestEntry", "" ), QString( "Test1" ) );
}

void SessionControllerTest::temporary()
{
    ISession* s = Core::self()->activeSession();
    s->setTemporary(true);
    const QString oldName = s->name();
    const QString dir = sessionDir(s);

    verifySessionDir(s, true);
    Core::self()->sessionController()->cleanup();
    verifySessionDir(dir, oldName, false);
    Core::self()->sessionController()->initialize(oldName);
    QCOMPARE(Core::self()->activeSession()->name(), oldName);
    // dir / UID can be different, hence don't verifySessionDir
}

void SessionControllerTest::tryLockSession()
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

QTEST_KDEMAIN( SessionControllerTest, GUI)
#include "sessioncontrollertest.moc"
