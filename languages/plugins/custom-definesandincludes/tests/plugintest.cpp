/************************************************************************
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

#include "plugintest.h"

#include <QtTest/QtTest>

#include <qtest_kde.h>

#include <KDebug>

#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <tests/autotestshell.h>
#include <tests/kdevsignalspy.h>
#include <tests/testcore.h>

#include <language/interfaces/idefinesandincludesmanager.h>

#include "testconfig.h"

using KDevelop::ICore;
using KDevelop::IProject;
using KDevelop::TestCore;
using KDevelop::AutoTestShell;
using KDevelop::KDevSignalSpy;
using KDevelop::Path;

void PluginTest::cleanupTestCase()
{
    TestCore::shutdown();
}
void PluginTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();
}

void PluginTest::loadSimpleProject()
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
    Path::List includes = KDevelop::IDefinesAndIncludesManager::manager()->includes( project->projectItem() );

    QHash<QString,QString> defines;
    defines.insert( "_DEBUG", "" );
    defines.insert( "VARIABLE", "VALUE" );
    QCOMPARE( includes, Path::List() << Path( "/usr/include/mydir") );
    QCOMPARE( KDevelop::IDefinesAndIncludesManager::manager()->defines( project->projectItem() ), defines );
}

void PluginTest::loadMultiPathProject()
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
    foreach( KDevelop::ProjectBaseItem* i, project->files() ) {
        if( i->text() == "main.cpp" ) {
            mainfile = i;
            break;
        }
    }
    QVERIFY(mainfile);
    Path::List includes = KDevelop::IDefinesAndIncludesManager::manager()->includes( mainfile );

    QHash<QString,QString> defines;
    defines.insert( "BUILD", "debug" );
    defines.insert("SOURCE", "CONTENT");
    defines.insert("_COPY", "");

    QCOMPARE( includes, Path::List() << Path("/usr/include/otherdir") << Path("/usr/local/include/mydir") );
    QCOMPARE( KDevelop::IDefinesAndIncludesManager::manager()->defines( mainfile ), defines );
}

QTEST_KDEMAIN(PluginTest, GUI)

#include "plugintest.moc"
