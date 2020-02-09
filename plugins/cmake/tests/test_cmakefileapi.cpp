/* This file is part of KDevelop

    Copyright 2020 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QTest>
#include <QObject>
#include <QVersionNumber>
#include <QLoggingCategory>
#include <QDir>
#include <QJsonObject>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iprojectfilemanager.h>
#include <interfaces/ibuildsystemmanager.h>
#include <interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>

#include <KJob>

#include <cmakefileapi.h>
#include <cmakeutils.h>
#include <cmakeprojectdata.h>
#include <testhelpers.h>

using namespace KDevelop;

class TestCMakeFileApi : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestSuite()
    {
        QLoggingCategory::setFilterRules(QStringLiteral("default.debug=true\nkdevelop.projectmanagers.cmake.debug=true\n"));

        if (!CMake::FileApi::supported(CMake::findExecutable())) {
            QSKIP("cmake exe doesn't support file API");
        }

        AutoTestShell::init({"KDevCMakeManager", "KDevCMakeBuilder", "KDevMakeBuilder", "KDevStandardOutputView"});
        TestCore::initialize();
    }

    void cleanupTestCase()
    {
        TestCore::shutdown();
    }

    void testConfigure()
    {
        auto project = loadProject(QStringLiteral("single_subdirectory"));
        QVERIFY(project);

        auto bsm = project->buildSystemManager();
        const auto buildPath = bsm->buildDirectory(project->projectItem());
        const auto buildDir = buildPath.toLocalFile();
        QVERIFY(!buildDir.isEmpty());
        QVERIFY(QDir(buildDir).removeRecursively());

        auto builder = bsm->builder();
        auto configureJob = builder->configure(project);
        QVERIFY(configureJob);
        QVERIFY(configureJob->exec());

        const auto index = CMake::FileApi::findReplyIndexFile(buildDir);
        QVERIFY(!index.isEmpty());

        const auto projectData = CMake::FileApi::parseReplyIndexFile(index, project->path(), buildPath);
        QVERIFY(projectData.compilationData.isValid);
        QCOMPARE(projectData.targets.size(), 1);
        const auto subDirPath = Path(project->path(), "subdir");
        QVERIFY(projectData.targets.contains(subDirPath));
        const auto targets = projectData.targets[subDirPath];
        QCOMPARE(targets.size(), 1);
        const auto target = targets.first();
        QCOMPARE(target.name, "foo");
        QCOMPARE(target.type, CMakeTarget::Executable);
        const auto buildSubDirPath = Path(buildPath, "subdir");
        QCOMPARE(target.artifacts, {Path(buildSubDirPath, "foo")});
        const auto fooSrcPath = Path(subDirPath, "foo.cpp");
        QCOMPARE(target.sources, {fooSrcPath});

        QCOMPARE(projectData.compilationData.files.size(), 1);
        QVERIFY(projectData.compilationData.files.contains(fooSrcPath));
        const auto srcInfo = projectData.compilationData.files[fooSrcPath];
        QCOMPARE(srcInfo.language, "CXX");
        QCOMPARE(srcInfo.includes.size(), 3);
        QVERIFY(srcInfo.includes.contains(buildPath));
        QVERIFY(srcInfo.includes.contains(buildSubDirPath));
        QVERIFY(srcInfo.includes.contains(subDirPath));
    }
};

QTEST_MAIN(TestCMakeFileApi)
#include "test_cmakefileapi.moc"
