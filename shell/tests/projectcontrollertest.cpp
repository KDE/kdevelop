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

#include <QFile>
#include <QSignalSpy>

#include <qtest_kde.h>
#include <tests/common/autotestshell.h>

#include "../core.h"
#include "../projectcontroller.h"
#include "../project.h"

using KDevelop::ProjectController;
using KDevelop::IProjectDialogProvider;
using KDevelop::IProject;
using KDevelop::Core;

using QTest::kWaitForSignal;

Q_DECLARE_METATYPE(KDevelop::IProject*)

namespace {

class DialogProviderFake : public IProjectDialogProvider
{
Q_OBJECT
public:
    DialogProviderFake() : m_reopen(true) {}
    virtual ~DialogProviderFake() {}
    bool m_reopen;

public slots:
    virtual KUrl askProjectConfigLocation() { return KUrl(); }
    virtual bool userWantsReopen() { return m_reopen; }
};

}

////////////////////// Fixture ///////////////////////////////////////////////

void ProjectControllerTest::initTestCase()
{
    AutoTestShell::init();
    Core::initialize();
    qRegisterMetaType<KDevelop::IProject*>();
    m_core = Core::self();
    m_scratchDir = QDir(QDir::tempPath());
    m_scratchDir.mkdir("prjctrltest");
    m_scratchDir.cd("prjctrltest");
}

void ProjectControllerTest::init()
{
    m_projName = "foo";
    m_projFileUrl = writeProjectConfig(m_projName);
    m_projCtrl = m_core->projectControllerInternal();
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

#define WAIT_FOR_OPEN_SIGNAL \
{\
    bool gotSignal = kWaitForSignal(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)), 20000);\
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");\
} void(0)

void ProjectControllerTest::openProject()
{
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(m_projCtrl->projectCount(), 1);
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);

    m_projCtrl->closeProject(proj);

}

void ProjectControllerTest::closeProject()
{
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    IProject* proj = m_projCtrl->findProjectByName(m_projName);
    Q_ASSERT(proj);

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
    WAIT_FOR_OPEN_SIGNAL;
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    QVERIFY(m_projCtrl->closeProject(proj));
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(m_projCtrl->projectCount(), 1);
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);
}

void ProjectControllerTest::reopen()
{
    m_projCtrl->setDialogProvider(new DialogProviderFake);
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(m_projCtrl->projectCount(), 1);
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);
}

void ProjectControllerTest::reopenWhileLoading()
{
    // Open the same project again while the first is still
    // loading. The second open request should be blocked.
    m_projCtrl->setDialogProvider(new DialogProviderFake);
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    QVERIFY(!m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    // wait a bit for a second signal, this should timeout
    QVERIFY2(!kWaitForSignal(m_projCtrl,
        SIGNAL(projectOpened(KDevelop::IProject*)), 100),
        "Received 2 projectOpened signals.");
    QCOMPARE(m_projCtrl->projectCount(), 1);
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);
}

void ProjectControllerTest::openMultiple()
{
    QString secondProj("bar");
    KUrl secondCfgUrl = writeProjectConfig(secondProj);
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QVERIFY(m_projCtrl->openProject(secondCfgUrl));
    WAIT_FOR_OPEN_SIGNAL;

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

KUrl ProjectControllerTest::writeProjectConfig(const QString& name)
{
    KUrl configUrl = KUrl(m_scratchDir.absolutePath() + "/" + name + ".kdev4");
    QFile f(configUrl.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    str << "[Project]\n"
        << "Name=" << name << "\n";
    f.close();
    return configUrl;
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

QTEST_KDEMAIN( ProjectControllerTest, GUI)
#include "moc_projectcontrollertest.cpp"
#include "projectcontrollertest.moc"
