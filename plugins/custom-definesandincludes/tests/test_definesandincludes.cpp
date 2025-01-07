/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

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
    // used KDevDefinesAndIncludesManager has "X-KDevelop-LoadMode": "AlwaysOn", so no need to try to load explicitly
    AutoTestShell::init({QStringLiteral("KDevCustomBuildSystem"), QStringLiteral("KDevStandardOutputView")});
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

#include "moc_test_definesandincludes.cpp"
