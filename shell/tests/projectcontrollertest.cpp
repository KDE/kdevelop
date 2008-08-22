/***************************************************************************
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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

#include "projectcontrollertest.h"

#include <QtGui>
#define QT_GUI_LIB 1
#include <QtTest/QtTest>

#include <KUrl>
#include <KDebug>
#include <QFile>
#include <QDir>
#include <QKeyEvent>
#include <QSignalSpy>

#include <qtest_kde.h>
#include <tests/common/kdevtest.h>
#include <tests/common/autotestshell.h>

#include "../core.h"
#include "../projectcontroller.h"
#include "../project.h"


using namespace KDevelop;

Q_DECLARE_METATYPE(KDevelop::IProject*)

////////////////////// Fixture ///////////////////////////////////////////////

void ProjectControllerTest::initTestCase()
{
    AutoTestShell::init();
    Core::initialize();
    qRegisterMetaType<KDevelop::IProject*>();
}

void ProjectControllerTest::init()
{
    m_projName = "foo";
    m_projFileUrl = writeProjectConfig(m_projName);
    m_core = ICore::self();
    m_projCtrl = m_core->projectController();
    m_tmpConfigs << m_projFileUrl;
}

void ProjectControllerTest::cleanup()
{
    // also close any opened projects as we do not get a clean fixture,
    // following tests should start off clean.
    foreach(IProject* p, m_projCtrl->projects()) {
        m_projCtrl->closeProject(p);
    }
    foreach(KUrl cfg, m_tmpConfigs) {
        QFile::remove(cfg.pathOrUrl());
    }
}

////////////////////// Commands //////////////////////////////////////////////

void ProjectControllerTest::openProject()
{
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    if( !QTest::kWaitForSignal(m_projCtrl,SIGNAL(projectOpened(KDevelop::IProject*)), 20000) )
    {
        QFAIL("Timeout while waiting for opened signal");
    }
    QCOMPARE(m_projCtrl->projectCount(), 1);
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);

    m_projCtrl->closeProject(proj);

}

void ProjectControllerTest::closeProject()
{
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    if( !QTest::kWaitForSignal(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)),20000))
    {
        QFAIL("Timeout while waiting for opened signal");
    }
    IProject* proj = m_projCtrl->findProjectByName(m_projName);
    QASSERT(proj);

    QSignalSpy* spy1 = createClosedSpy();
    QSignalSpy* spy2 = createClosingSpy();
    m_projCtrl->closeProject(proj);


    QCOMPARE(m_projCtrl->projectCount(), 0);
    assertProjectClosed(proj);
    assertSpyCaughtProject(spy1, proj);
    assertSpyCaughtProject(spy2, proj);
}

void ProjectControllerTest::openCloseOpen()
{
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    if( !QTest::kWaitForSignal(m_projCtrl,SIGNAL(projectOpened(KDevelop::IProject*)), 20000) )
    {
        QFAIL("Timeout while waiting for opened signal");
    }
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    QVERIFY(m_projCtrl->closeProject(proj));
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    if( !QTest::kWaitForSignal(m_projCtrl,SIGNAL(projectOpened(KDevelop::IProject*)), 20000) )
    {
        QFAIL("Timeout while waiting for opened signal");
    }
    QCOMPARE(m_projCtrl->projectCount(), 1);
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);
}

// void ProjectControllerTest::reOpen()
// {
//     QVERIFY(m_projCtrl->openProject(m_projFileUrl));
//     clickAcceptOnReopenDialog();
//     QSignalSpy* spy = createOpenedSpy();
//     QVERIFY(m_projCtrl->openProject(m_projFileUrl));
//     if (!QTest::kWaitForSignal(this, SIGNAL(proceed()),2000)) {
//          QFAIL("Timeout while waiting for project to reopen.");
//     } 
//     QCOMPARE(m_projCtrl->projectCount(), 1);
//     IProject* proj;
//     assertProjectOpened(m_projName, proj);
//     assertSpyCaughtProject(spy, proj);
// }

void ProjectControllerTest::openMultiple()
{
    QString secondProj("bar");
    KUrl secondCfgUrl = writeProjectConfig(secondProj);
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    if( !QTest::kWaitForSignal(m_projCtrl,SIGNAL(projectOpened(KDevelop::IProject*)), 20000) )
    {
        QFAIL("Timeout while waiting for opened signal");
    }
    QVERIFY(m_projCtrl->openProject(secondCfgUrl));
    if( !QTest::kWaitForSignal(m_projCtrl,SIGNAL(projectOpened(KDevelop::IProject*)), 20000) )
    {
        QFAIL("Timeout while waiting for closed signal");
    }
 
    QCOMPARE(m_projCtrl->projectCount(), 2);
    IProject *proj1, *proj2;
    assertProjectOpened(m_projName, proj1);
    assertProjectOpened(secondProj, proj2); 

    QCOMPARE(spy->size(), 2);
    IProject* emittedProj1 = (*spy)[0][0].value<IProject*>();
    IProject* emittedProj2 = (*spy)[1][0].value<IProject*>();
    QCOMPARE(emittedProj1, proj1);
    QCOMPARE(emittedProj2, proj2);

    m_tmpConfigs << secondCfgUrl;
}

////////////////////// Helpers ///////////////////////////////////////////////

void ProjectControllerTest::clickAcceptOnReopenDialog()
{
    QTimer* t = new QTimer(this);
    t->setSingleShot(true);
    t->setInterval(1);
    connect(t, SIGNAL(timeout()), this, SLOT(waitForReopenBox()));
    t->start();
}

KUrl ProjectControllerTest::writeProjectConfig(const QString& name)
{
    KUrl configUrl = KUrl(QDir::tempPath() + "/" + name + ".kdev4");
    QFile f(configUrl.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    str << "[Project]\n"
        << "Name=" << name << "\n"
        << "Manager=KDevCMakeManager\n\n";
    f.close();
    return configUrl;
}

void ProjectControllerTest::waitForReopenBox()
{
    int i=0;
    for (; i<20; i++) {
        QTest::qWait(250);
        QWidget* reopenBox = QApplication::activeModalWidget();
        if (reopenBox) {
            for (int j=0; j<10; j++) {
                QTest::keyClick(reopenBox, Qt::Key_Enter, Qt::NoModifier, 20);
            }
            break;
        }
    }
    QVERIFY2(i < 20, "Timeout while waiting for project reopen dialog.");
    QTimer* t = new QTimer(this);
    t->setSingleShot(true);
    t->setInterval(100);
    connect(t, SIGNAL(timeout()), this, SIGNAL(proceed()));
    t->start();
}

////////////////// Custom assertions /////////////////////////////////////////

void ProjectControllerTest::assertProjectOpened(const QString& name, IProject*& proj)
{
    QVERIFY(proj = m_projCtrl->findProjectByName(name));
    QVERIFY(m_projCtrl->projects().contains(proj));
}

void ProjectControllerTest::assertSpyCaughtProject(QSignalSpy* spy, IProject* proj)
{
    QCOMPARE(spy->size(), 1);
    IProject* emittedProj = (*spy)[0][0].value<IProject*>();
    QCOMPARE(proj, emittedProj);
}

void ProjectControllerTest::assertProjectClosed(IProject* proj)
{
    qDebug() << proj << m_projCtrl;
    QString name = proj->name();
    IProject* p = m_projCtrl->findProjectByName(proj->name());
    QVERIFY(p == 0);
    QVERIFY(!m_projCtrl->projects().contains(proj));
}

///////////////////// Creation stuff /////////////////////////////////////////

QSignalSpy* ProjectControllerTest::createOpenedSpy()
{
    return new QSignalSpy(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)));
}

QSignalSpy* ProjectControllerTest::createClosedSpy()
{
    return new QSignalSpy(m_projCtrl, SIGNAL(projectClosed(KDevelop::IProject*)));
}

QSignalSpy* ProjectControllerTest::createClosingSpy()
{
    return new QSignalSpy(m_projCtrl, SIGNAL(projectClosing(KDevelop::IProject*)));
}

QTEST_KDEMAIN( ProjectControllerTest, GUI )
#include "projectcontrollertest.moc"
