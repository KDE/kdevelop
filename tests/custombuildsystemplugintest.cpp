/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 3 of the License, or    *
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

#include <qtest_kde.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/kdevsignalspy.h>
#include <interfaces/iprojectcontroller.h>
#include <KDebug>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

using KDevelop::Core;
using KDevelop::ICore;
using KDevelop::IProject;
using KDevelop::TestCore;
using KDevelop::AutoTestShell;
using KDevelop::KDevSignalSpy;

void CustomBuildSystemPluginTest::cleanupTestCase()
{
    m_core->cleanup();
    delete m_core;
}
void CustomBuildSystemPluginTest::initTestCase()
{
    AutoTestShell::init();
    m_core = new KDevelop::TestCore();
    m_core->initialize( Core::Default );
    
    QTimer timer;
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.setSingleShot(true);
    timer.start(5000);
    loop.exec();

    foreach( IProject* p, m_core->projectController()->projects() )
    {
        m_core->projectController()->closeProject( p );
    }
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
    KUrl::List includes = project->buildSystemManager()->includeDirectories( project->projectItem() );
    
    QHash<QString,QString> defines;
    defines.insert( "_DEBUG", "" );
    defines.insert( "VARIABLE", "VALUE" );
    QCOMPARE( includes, KUrl::List( QStringList() << "/usr/include/mydir" ) );
    QCOMPARE( project->buildSystemManager()->defines( project->projectItem() ), defines );
    QCOMPARE( project->buildSystemManager()->buildDirectory( project->projectItem() ), KUrl( "file:///home/andreas/projects/testcustom/build/" ) );
}

QTEST_KDEMAIN(CustomBuildSystemPluginTest, GUI)

#include "custombuildsystemplugintest.moc"
