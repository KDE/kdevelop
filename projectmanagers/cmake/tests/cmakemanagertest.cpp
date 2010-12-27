/* This file is part of KDevelop
    Copyright 2010 Esben Mose Hansen<kde@mosehansen.dk>

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

#include "cmakemanagertest.h"
#include <tests/testproject.h>
#include <icmakemanager.h>
#include <qtest.h>
#include <qtest_kde.h>
#include <shell/core.h>
#include <tests/autotestshell.h>
#include <KConfigGroup>
#include <project/interfaces/iprojectfilemanager.h>
#include <interfaces/iplugincontroller.h>

QTEST_KDEMAIN(CMakeManagerTest, GUI )

#define WAIT_FOR_OPEN_SIGNAL \
{\
    bool gotSignal = QTest::kWaitForSignal(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), 30000);\
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");\
} void(0)

using namespace KDevelop;

CMakeManagerTest::CMakeManagerTest(QObject* parent): QObject(parent)
{
    AutoTestShell::init();
    KDevelop::Core::initialize(0, KDevelop::Core::Default);
}

CMakeManagerTest::~CMakeManagerTest()
{}

void CMakeManagerTest::testWithBuildDirProject()
{
    // Import project
    QList< ProjectFolderItem* > items;
    
    KUrl url(CMAKE_TESTS_PROJECTS_DIR "/with_build_dir/with_build_dir.kdev4");
    ICore::self()->projectController()->openProject(url);
    
    KUrl expected_source_Dir(CMAKE_TESTS_PROJECTS_DIR "/with_build_dir/");
    WAIT_FOR_OPEN_SIGNAL;
    
    IProject* project = ICore::self()->projectController()->findProjectByName("with_build_dir");
    QCOMPARE(url, project->projectFileUrl());
    QCOMPARE(expected_source_Dir, project->folder());
}

