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
        const auto buildDir = bsm->buildDirectory(project->projectItem()).toLocalFile();
        QVERIFY(!buildDir.isEmpty());
        QVERIFY(QDir(buildDir).removeRecursively());

        auto builder = bsm->builder();
        auto configureJob = builder->configure(project);
        QVERIFY(configureJob);
        QVERIFY(configureJob->exec());

        const auto index = CMake::FileApi::findReplyIndexFile(buildDir);
        QVERIFY(!index.isEmpty());
    }
};

QTEST_MAIN(TestCMakeFileApi)
#include "test_cmakefileapi.moc"
