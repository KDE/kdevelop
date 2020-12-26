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

#include <QTest>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>
#include <tests/projectsgenerator.h>

#include "idefinesandincludesmanager.h"

using namespace KDevelop;

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
    ICore::self()->projectController()->closeProject( m_currentProject );
}

void TestDefinesAndIncludes::loadSimpleProject()
{
    m_currentProject = ProjectsGenerator::GenerateSimpleProject();
    QVERIFY( m_currentProject );

    auto manager = IDefinesAndIncludesManager::manager();
    QVERIFY( manager );
    const auto actualIncludes = manager->includes( m_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined );
    const auto actualDefines = manager->defines( m_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined );

    qDebug() << actualDefines << actualIncludes;

    QCOMPARE( actualIncludes, Path::List() << Path( QDir::rootPath() + QStringLiteral("usr/include/mydir") ) );

    Defines defines;
    defines.insert( QStringLiteral("_DEBUG"), QString() );
    defines.insert( QStringLiteral("VARIABLE"), QStringLiteral("VALUE") );
    QCOMPARE( actualDefines, defines );

    QVERIFY(!manager->parserArguments(m_currentProject->path().path() + "/src/main.cpp").isEmpty());
}

void TestDefinesAndIncludes::loadMultiPathProject()
{
    m_currentProject = ProjectsGenerator::GenerateMultiPathProject();
    QVERIFY( m_currentProject );

    auto manager = IDefinesAndIncludesManager::manager();
    QVERIFY( manager );
    Path::List includes = Path::List() << Path( QDir::rootPath() + QStringLiteral("usr/include/otherdir") );

    QHash<QString,QString> defines;
    defines.insert(QStringLiteral("SOURCE"), QStringLiteral("CONTENT"));
    defines.insert(QStringLiteral("_COPY"), QString());

    QCOMPARE( manager->includes( m_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined ), includes );
    QCOMPARE( manager->defines( m_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined ), defines );

    ProjectBaseItem* mainfile = nullptr;
    const auto& fileSet = m_currentProject->fileSet();
    for (const auto& file : fileSet) {
        const auto& files = m_currentProject->filesForPath(file);
        for (auto i: files) {
            if( i->text() == QLatin1String("main.cpp") ) {
                mainfile = i;
                break;
            }
        }
    }
    QVERIFY(mainfile);

    includes.prepend(Path(QDir::rootPath() + QStringLiteral("usr/local/include/mydir")));
    defines.insert(QStringLiteral("BUILD"), QStringLiteral("debug"));
    qDebug() << includes << "VS" << manager->includes( mainfile, IDefinesAndIncludesManager::UserDefined );
    qDebug() << mainfile << mainfile->path();
    QCOMPARE(manager->includes( mainfile, IDefinesAndIncludesManager::UserDefined ), includes);
    QCOMPARE(defines, manager->defines( mainfile, IDefinesAndIncludesManager::UserDefined ));
    QVERIFY(!manager->parserArguments(mainfile).isEmpty());
}

void TestDefinesAndIncludes::testNoProjectIncludeDirectories()
{
    m_currentProject = ProjectsGenerator::GenerateSimpleProjectWithOutOfProjectFiles();
    QVERIFY(m_currentProject);

    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    QVERIFY(manager);

    auto projectIncludes = manager->includes(m_currentProject->projectItem(), IDefinesAndIncludesManager::UserDefined);

    Path includePath1(m_currentProject->path().path() + QDir::separator() + "include1.h");
    Path includePath2(m_currentProject->path().path() + QDir::separator() + "include2.h");

    QVERIFY(!projectIncludes.contains(includePath1));
    QVERIFY(!projectIncludes.contains(includePath2));

    auto noProjectIncludes = manager->includes(m_currentProject->path().path() + "/src/main.cpp");
    QVERIFY(noProjectIncludes.contains(includePath1));
    QVERIFY(noProjectIncludes.contains(includePath2));
}

void TestDefinesAndIncludes::testEmptyProject()
{
    m_currentProject = ProjectsGenerator::GenerateEmptyProject();
    QVERIFY(m_currentProject);

    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    QVERIFY(manager);

    auto projectIncludes = manager->includes(m_currentProject->projectItem());
    auto projectDefines = manager->defines(m_currentProject->projectItem());
    auto parserArguments = manager->parserArguments(m_currentProject->projectItem());

    QVERIFY(projectIncludes.isEmpty());
    QVERIFY(projectDefines.isEmpty());
    QVERIFY(parserArguments.isEmpty());
}

QTEST_MAIN(TestDefinesAndIncludes)
