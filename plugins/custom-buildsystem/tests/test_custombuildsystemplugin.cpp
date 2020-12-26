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

#include "test_custombuildsystemplugin.h"

#include <QTest>
#include <QDebug>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/kdevsignalspy.h>
#include <tests/projectsgenerator.h>
#include <shell/sessioncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <serialization/indexedstring.h>

#include "testconfig.h"

using KDevelop::Core;
using KDevelop::ICore;
using KDevelop::IProject;
using KDevelop::TestCore;
using KDevelop::AutoTestShell;
using KDevelop::KDevSignalSpy;
using KDevelop::ProjectsGenerator;
using KDevelop::Path;

QTEST_MAIN(TestCustomBuildSystemPlugin)

void TestCustomBuildSystemPlugin::cleanupTestCase()
{
    TestCore::shutdown();
}
void TestCustomBuildSystemPlugin::initTestCase()
{
    AutoTestShell::init({"KDevCustomBuildSystem", "KDevStandardOutputView"});
    TestCore::initialize();
}

void TestCustomBuildSystemPlugin::cleanup()
{
    ICore::self()->projectController()->closeAllProjects( );
}

void TestCustomBuildSystemPlugin::loadSimpleProject()
{
    m_currentProject = ProjectsGenerator::GenerateSimpleProject();
    QVERIFY( m_currentProject );
    QCOMPARE( m_currentProject->buildSystemManager()->buildDirectory( m_currentProject->projectItem() ),
              Path( QStringLiteral("file:///") + QDir::temp().absolutePath() + QStringLiteral("/simpleproject/build/") ) );
}

void TestCustomBuildSystemPlugin::buildDirProject()
{
    m_currentProject = ProjectsGenerator::GenerateEmptyBuildDirProject();
    QVERIFY( m_currentProject );
    QCOMPARE( m_currentProject->buildSystemManager()->buildDirectory( m_currentProject->projectItem() ),
              Path( QStringLiteral("file:///") + QDir::temp().absolutePath() + QStringLiteral("/simpleproject/build/") ) );
}

void TestCustomBuildSystemPlugin::loadMultiPathProject()
{
    m_currentProject = ProjectsGenerator::GenerateMultiPathProject();
    QVERIFY( m_currentProject );

    KDevelop::ProjectBaseItem* mainfile = nullptr;
    const auto& files = m_currentProject->fileSet();
    for (const auto& file : files) {
        const auto& filesForPath = m_currentProject->filesForPath(file);
        for (auto i: filesForPath) {
            if( i->text() == QLatin1String("main.cpp") ) {
                mainfile = i;
                break;
            }
        }
    }
    QVERIFY(mainfile);

    QCOMPARE( m_currentProject->buildSystemManager()->buildDirectory( mainfile ),
              Path( QStringLiteral("file:///") + QDir::temp().absolutePath() + QStringLiteral("/multipathproject/build/src") ) );
}

