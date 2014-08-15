/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "custombuildsystemplugintest.h"

#include <QtTest/QtTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/kdevsignalspy.h>
#include <shell/sessioncontroller.h>
#include <kio/netaccess.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>
#include <KDebug>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <kconfiggroup.h>

#include <language/duchain/indexedstring.h>

#include "testconfig.h"

using KDevelop::Core;
using KDevelop::ICore;
using KDevelop::IProject;
using KDevelop::TestCore;
using KDevelop::AutoTestShell;
using KDevelop::KDevSignalSpy;
using KDevelop::Path;

QTEST_MAIN(CustomBuildSystemPluginTest)

void CustomBuildSystemPluginTest::cleanupTestCase()
{
    TestCore::shutdown();
}
void CustomBuildSystemPluginTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();
}

void CustomBuildSystemPluginTest::loadSimpleProject()
{
    KUrl projecturl( PROJECTS_SOURCE_DIR"/simpleproject/simpleproject.kdev4" );
    KDevSignalSpy* projectSpy = new KDevSignalSpy( ICore::self()->projectController(), SIGNAL( projectOpened( KDevelop::IProject* ) ) );
    ICore::self()->projectController()->openProject( projecturl );
    // Wait for the project to be opened
    if( !projectSpy->wait( 20000 ) ) {
        kFatal() << "Expected project to be loaded within 20 seconds, but this didn't happen";
    }
    IProject* project = ICore::self()->projectController()->findProjectByName( "SimpleProject" );
    QVERIFY( project );

    QCOMPARE( project->buildSystemManager()->buildDirectory( project->projectItem() ),
              Path( "file:///home/andreas/projects/testcustom/build/" ) );
}

void CustomBuildSystemPluginTest::buildDirProject()
{
    KUrl projecturl( PROJECTS_SOURCE_DIR"/builddirproject/builddirproject.kdev4" );
    KDevSignalSpy* projectSpy = new KDevSignalSpy( ICore::self()->projectController(), SIGNAL( projectOpened( KDevelop::IProject* ) ) );
    ICore::self()->projectController()->openProject( projecturl );
    // Wait for the project to be opened
    if( !projectSpy->wait( 20000 ) ) {
        kFatal() << "Expected project to be loaded within 20 seconds, but this didn't happen";
    }
    IProject* project = ICore::self()->projectController()->findProjectByName( "BuilddirProject" );
    QVERIFY( project );
   
    Path currentBuilddir = project->buildSystemManager()->buildDirectory( project->projectItem() );

    QCOMPARE( currentBuilddir, Path( projecturl ).parent() );
}


void CustomBuildSystemPluginTest::loadMultiPathProject()
{
    KUrl projecturl( PROJECTS_SOURCE_DIR"/multipathproject/multipathproject.kdev4" );
    KDevSignalSpy* projectSpy = new KDevSignalSpy( ICore::self()->projectController(), SIGNAL( projectOpened( KDevelop::IProject* ) ) );
    ICore::self()->projectController()->openProject( projecturl );
    // Wait for the project to be opened
    if( !projectSpy->wait( 20000 ) ) {
        kFatal() << "Expected project to be loaded within 20 seconds, but this didn't happen";
    }
    IProject* project = ICore::self()->projectController()->findProjectByName( "MultiPathProject" );
    QVERIFY( project );
    KDevelop::ProjectBaseItem* mainfile = 0;
    for (const auto& file: project->fileSet() ) {
        for (auto i: project->filesForPath(file)) {
            if( i->text() == "main.cpp" ) {
                mainfile = i;
                break;
            }
        }
    }
    QVERIFY(mainfile);

    QCOMPARE( project->buildSystemManager()->buildDirectory( mainfile ),
              Path( "file:///home/andreas/projects/testcustom/build2/src" ) );
}

