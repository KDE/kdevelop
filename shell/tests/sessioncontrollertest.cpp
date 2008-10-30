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
#include <tests/common/autotestshell.h>

#include "../core.h"
#include "../sessioncontroller.h"
#include "../session.h"

Q_DECLARE_METATYPE( KDevelop::ISession* )

using KDevelop::SessionController;
using KDevelop::ISession;
using KDevelop::Core;

using QTest::kWaitForSignal;

////////////////////// Fixture ///////////////////////////////////////////////

void SessionControllerTest::initTestCase()
{
    AutoTestShell::init();
    Core::initialize( KDevelop::Core::NoUi );
    m_core = Core::self();
    qRegisterMetaType<KDevelop::ISession*>();
}

void SessionControllerTest::init()
{
    m_sessionCtrl = m_core->sessionController();
}

void SessionControllerTest::cleanup()
{
}

void SessionControllerTest::createSession()
{
    const QString sessionName = "TestSession";
    int sessionCount = m_sessionCtrl->sessions().count();
    m_sessionCtrl->createSession( sessionName );
    QVERIFY( m_sessionCtrl->sessions().contains( sessionName )  );
    QCOMPARE( sessionCount+1, m_sessionCtrl->sessions().count() );
    QString sessiondir = SessionController::sessionDirectory() + "/" + sessionName;
    QVERIFY( QFileInfo( sessiondir ).exists() );
    QVERIFY( QFileInfo( sessiondir ).isDir() );
    m_sessionCtrl->deleteSession( sessionName );
}

void SessionControllerTest::loadSession()
{
    const QString sessionName = "TestSession2";
    m_sessionCtrl->createSession( sessionName );
    ISession* s = m_sessionCtrl->activeSession();
    m_sessionCtrl->loadSession( sessionName );
    QEXPECT_FAIL("", "expecting a changed active session", Continue);
    QCOMPARE( s, m_sessionCtrl->activeSession()); 
    m_sessionCtrl->deleteSession( sessionName );
}

void SessionControllerTest::renameSession()
{
    const QString sessionName = "TestSession4";
    const QString newSessionName = "TestOtherSession4";
    KDevelop::Session *s = m_sessionCtrl->createSession( sessionName );
    QCOMPARE( sessionName, s->name() );
    QString sessiondir = SessionController::sessionDirectory() + "/" + sessionName;
    QVERIFY( QFileInfo( sessiondir ).exists() );
    QVERIFY( QFileInfo( sessiondir ).isDir() );
    QSignalSpy spy(s, SIGNAL(nameChanged(const QString&, const QString&)));
    s->setName( newSessionName );
    QCOMPARE( newSessionName, s->name() );
    
    QCOMPARE( spy.size(), 1 );
    QList<QVariant> arguments = spy.takeFirst();

    QCOMPARE( sessionName, arguments.at(1).toString() );
    QCOMPARE( newSessionName, arguments.at(0).toString() );

    sessiondir = SessionController::sessionDirectory() + "/" + newSessionName;
    QVERIFY( QFileInfo( sessiondir ).exists() );
    QVERIFY( QFileInfo( sessiondir ).isDir() );
    m_sessionCtrl->deleteSession( s->name() );
}

void SessionControllerTest::cannotRenameActiveSession()
{
    const QString sessionName = "TestSession5";
    const QString newSessionName = "TestOtherSession5";
    KDevelop::Session *s = m_sessionCtrl->createSession( sessionName );
    QCOMPARE( sessionName, s->name() );
    m_sessionCtrl->loadSession( sessionName );
    QSignalSpy spy(s, SIGNAL(nameChanged(const QString&, const QString&)));
    s->setName( newSessionName );
    QCOMPARE( sessionName, s->name() );
    
    QCOMPARE( spy.size(), 0 );
}

void SessionControllerTest::deleteSession()
{
    const QString sessionName = "TestSession3";
    int sessionCount = m_sessionCtrl->sessions().count();
    m_sessionCtrl->createSession( sessionName );
    QCOMPARE( sessionCount+1, m_sessionCtrl->sessions().count() );
    QString sessiondir = SessionController::sessionDirectory() + "/" + sessionName;
    QVERIFY( QFileInfo( sessiondir ).exists() );
    QVERIFY( QFileInfo( sessiondir ).isDir() );
    QSignalSpy spy(m_sessionCtrl, SIGNAL(sessionDeleted(const QString&)));
    m_sessionCtrl->deleteSession( sessionName );
    QCOMPARE( sessionCount, m_sessionCtrl->sessions().count() );

    QCOMPARE(spy.size(), 1);
    QList<QVariant> arguments = spy.takeFirst();

    QString emittedSession = arguments.at(0).toString();
    QCOMPARE( sessionName, emittedSession );

    QVERIFY( !QFileInfo( sessiondir ).exists() );
}

QTEST_KDEMAIN( SessionControllerTest, GUI)
#include "sessioncontrollertest.moc"
