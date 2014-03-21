/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>         *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the   *
 * License, or (at your option) any later version.                      *
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
#include "projectsgenerator.h"

#include <QtTest/QtTest>

#include <qtest_kde.h>

#include <KDebug>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <language/interfaces/idefinesandincludesmanager.h>

using KDevelop::ICore;
using KDevelop::IDefinesAndIncludesManager;
using KDevelop::IProject;
using KDevelop::TestCore;
using KDevelop::AutoTestShell;
using KDevelop::Path;

static IProject* s_currentProject = nullptr;

void PluginTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void PluginTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();
}

void PluginTest::cleanup()
{
    ICore::self()->projectController()->closeProject( s_currentProject );
}

void PluginTest::loadSimpleProject()
{
    s_currentProject = ProjectsGenerator::GenerateSimpleProject();
    QVERIFY( s_currentProject );

    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    QVERIFY( manager );
    Path::List includes = manager->includes( s_currentProject->projectItem() );

    QHash<QString,QString> defines;
    defines.insert( "_DEBUG", "" );
    defines.insert( "VARIABLE", "VALUE" );
    QCOMPARE( includes, Path::List() << Path( "/usr/include/mydir") );
    QCOMPARE( manager->defines( s_currentProject->projectItem() ), defines );
}

void PluginTest::loadMultiPathProject()
{
    s_currentProject = ProjectsGenerator::GenerateMultiPathProject();
    QVERIFY( s_currentProject );

    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    QVERIFY( manager );
    Path::List includes = Path::List() << Path("/usr/include/otherdir");

    QHash<QString,QString> defines;
    defines.insert("SOURCE", "CONTENT");
    defines.insert("_COPY", "");

    QCOMPARE( manager->includes( s_currentProject->projectItem()), includes );
    QCOMPARE( manager->defines( s_currentProject->projectItem()), defines );

    KDevelop::ProjectBaseItem* mainfile = 0;
    foreach( KDevelop::ProjectBaseItem* i, s_currentProject->files() ) {
        if( i->text() == "main.cpp" ) {
            mainfile = i;
            break;
        }
    }
    QVERIFY(mainfile);

    includes << Path("/usr/local/include/mydir");
    defines.insert("BUILD", "debug");
    QCOMPARE(manager->includes( mainfile ), includes);
    QCOMPARE(defines, manager->defines( mainfile ));
}

QTEST_KDEMAIN(PluginTest, GUI)


#include "plugintest.moc"
