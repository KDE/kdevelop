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

#include "test_definesandincludes.h"
#include "projectsgenerator.h"

#include <QTest>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>

#include "idefinesandincludesmanager.h"

using namespace KDevelop;

static IProject* s_currentProject = nullptr;

void TestDefinesAndIncludes::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDefinesAndIncludes::initTestCase()
{
    AutoTestShell::init({QStringLiteral("kdevdefinesandincludesmanager"), QStringLiteral("KDevCustomBuildSystem"), QStringLiteral("KDevStandardOutputView")});
    TestCore::initialize();
}

void TestDefinesAndIncludes::cleanup()
{
    ICore::self()->projectController()->closeProject( s_currentProject );
}

void TestDefinesAndIncludes::loadSimpleProject()
{
    s_currentProject = ProjectsGenerator::GenerateSimpleProject();
    QVERIFY( s_currentProject );

    auto manager = IDefinesAndIncludesManager::manager();
    QVERIFY( manager );
    const auto actualIncludes = manager->includes( s_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined );
    const auto actualDefines = manager->defines( s_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined );

    qDebug() << actualDefines << actualIncludes;

    QCOMPARE( actualIncludes, Path::List() << Path( "/usr/include/mydir") );

    Defines defines;
    defines.insert( QStringLiteral("_DEBUG"), QLatin1String("") );
    defines.insert( QStringLiteral("VARIABLE"), QStringLiteral("VALUE") );
    QCOMPARE( actualDefines, defines );

    QVERIFY(!manager->parserArguments(s_currentProject->projectItem()).isEmpty());
    QVERIFY(!manager->parserArguments(QStringLiteral("/some/path/to/file.cpp")).isEmpty());
}

void TestDefinesAndIncludes::loadMultiPathProject()
{
    s_currentProject = ProjectsGenerator::GenerateMultiPathProject();
    QVERIFY( s_currentProject );

    auto manager = IDefinesAndIncludesManager::manager();
    QVERIFY( manager );
    Path::List includes = Path::List() << Path(QStringLiteral("/usr/include/otherdir"));

    QHash<QString,QString> defines;
    defines.insert(QStringLiteral("SOURCE"), QStringLiteral("CONTENT"));
    defines.insert(QStringLiteral("_COPY"), QLatin1String(""));

    QCOMPARE( manager->includes( s_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined ), includes );
    QCOMPARE( manager->defines( s_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined ), defines );
    QVERIFY(!manager->parserArguments(s_currentProject->projectItem()).isEmpty());

    ProjectBaseItem* mainfile = nullptr;
    for (const auto& file: s_currentProject->fileSet() ) {
        for (auto i: s_currentProject->filesForPath(file)) {
            if( i->text() == QLatin1String("main.cpp") ) {
                mainfile = i;
                break;
            }
        }
    }
    QVERIFY(mainfile);

    includes.prepend(Path(QStringLiteral("/usr/local/include/mydir")));
    defines.insert(QStringLiteral("BUILD"), QStringLiteral("debug"));
    qDebug() << includes << "VS" << manager->includes( mainfile, IDefinesAndIncludesManager::UserDefined );
    qDebug() << mainfile << mainfile->path();
    QCOMPARE(manager->includes( mainfile, IDefinesAndIncludesManager::UserDefined ), includes);
    QCOMPARE(defines, manager->defines( mainfile, IDefinesAndIncludesManager::UserDefined ));
    QVERIFY(!manager->parserArguments(mainfile).isEmpty());
}

void TestDefinesAndIncludes::testNoProjectIncludeDirectories()
{
    s_currentProject = ProjectsGenerator::GenerateSimpleProjectWithOutOfProjectFiles();
    QVERIFY(s_currentProject);

    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    QVERIFY(manager);

    auto projectIncludes = manager->includes(s_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined);

    Path includePath1(s_currentProject->path().path() + QDir::separator() + "include1.h");
    Path includePath2(s_currentProject->path().path() + QDir::separator() + "include2.h");

    QVERIFY(!projectIncludes.contains(includePath1));
    QVERIFY(!projectIncludes.contains(includePath2));

    auto noProjectIncludes = manager->includes(s_currentProject->path().path() + "/src/main.cpp");
    QVERIFY(noProjectIncludes.contains(includePath1));
    QVERIFY(noProjectIncludes.contains(includePath2));
    QVERIFY(!manager->parserArguments(s_currentProject->projectItem()).isEmpty());
}

void TestDefinesAndIncludes::testEmptyProject()
{
    s_currentProject = ProjectsGenerator::GenerateEmptyProject();
    QVERIFY(s_currentProject);

    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    QVERIFY(manager);

    auto projectIncludes = manager->includes(s_currentProject->projectItem());
    auto projectDefines = manager->defines(s_currentProject->projectItem());
    auto parserArguments = manager->parserArguments(s_currentProject->projectItem());

    QVERIFY(!projectIncludes.isEmpty());
    QVERIFY(!projectDefines.isEmpty());
    QVERIFY(!parserArguments.isEmpty());
}

QTEST_MAIN(TestDefinesAndIncludes)
