/* KDevelop QMake Support
 *
 * Copyright 2011 Julien Desgats <julien.desgats@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "test_qmakeproject.h"
#include "../qmakeconfig.h"

#include <shell/core.h>
#include <tests/autotestshell.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <QTest>
#include <qtest_kde.h>
#include <KUrl>
#include <KConfigGroup>
#include <KDebug>

QTEST_KDEMAIN(TestQMakeProject, GUI);

using namespace KDevelop;

TestQMakeProject::TestQMakeProject(QObject* parent): QObject(parent)
{
    AutoTestShell::init();
    Core::initialize(0, Core::Default, "QMake_test_session");
}

TestQMakeProject::~TestQMakeProject()
{}


void TestQMakeProject::testBuildDirectory_data()
{
    QTest::addColumn<QString>("projectName");  // name of the project (both directory and .kde4 file)
    QTest::addColumn<QString>("target");       // directory to compile from project root
    QTest::addColumn<QString>("expected");     // expected build directory from build dir
    
    QTest::newRow("Basic Project") << "basic_project" << "" << "";
    QTest::newRow("Subdirs Project (root)") << "subdirs_project" << "" << "";
    QTest::newRow("Subdirs Project (dir_a)") << "subdirs_project" << "dir_a" << "dir_a";
}

void TestQMakeProject::testBuildDirectory()
{
    QFETCH(QString, projectName);
    QFETCH(QString, target);
    QFETCH(QString, expected);
    
    foreach(IProject *p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }

    // opens project with kdevelop
    KUrl url(QString("%1/%2/%3.kdev4").arg(QMAKE_TESTS_PROJECTS_DIR).arg(projectName).arg(projectName));
    ICore::self()->projectController()->openProject(url);
    
    // wait for loading finished
    //TODO: this pops the configuration dialog! Find a fox for that!
    bool gotSignal = QTest::kWaitForSignal(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), 30000);
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");
    
    IProject* project = ICore::self()->projectController()->findProjectByName(projectName);
    
    // reads build directory from configuration
    KConfigGroup cg(project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    KUrl buildDir = cg.readEntry(QMakeConfig::BUILD_FOLDER, KUrl(""));
    buildDir.addPath(expected);
    buildDir.cleanPath();
    
    ProjectBaseItem *rootItem = project->projectItem();
    IBuildSystemManager *buildManager = project->buildSystemManager();
    QString actual = buildManager->buildDirectory(rootItem).directory();
    expected = buildDir.directory();
    QCOMPARE(actual, expected);
}
