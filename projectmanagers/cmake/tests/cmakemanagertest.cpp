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


using namespace KDevelop;

CMakeManagerTest::CMakeManagerTest(QObject* parent): QObject(parent) {
    AutoTestShell::init();
    KDevelop::Core::initialize(0, KDevelop::Core::Default);
}

CMakeManagerTest::~CMakeManagerTest() {

}

void CMakeManagerTest::testWithBuildDirProject() {
    // Import project
    TestProject project;
    KSharedConfig::Ptr config = project.projectConfiguration();
    KConfigGroup group(config.data()->group("CMake"));
    group.writeEntry("ProjectRootRelative", "./");
    group.writeEntry("CurrentBuildDir", CMAKE_TESTS_PROJECTS_DIR  "/with_build_dir/builddir");
    project.set_projectFileUrl(KUrl(CMAKE_TESTS_PROJECTS_DIR "/with_build_dir/with_build_dir.kdev4"));
    IPlugin* i = Core::self()->pluginController()->loadPlugin( "KDevCMakeManager" );
    IProjectFileManager* manager = i->extension<IProjectFileManager>();
    manager->import(&project);
    ICMakeManager* cmakemanager = i->extension<ICMakeManager>();

    // Check that cache has been loaded by checking 2 arbitrary vraibles
    QString binary_dir = cmakemanager->cacheValue(&project, "with_build_dir_BINARY_DIR").first;
    QString source_dir = cmakemanager->cacheValue(&project, "with_build_dir_SOURCE_DIR").first;
    QString expected_binary_dir("/home/xxx/kdesvn/src/KDE/kdevelop/projectmanagers/cmake/tests/manual/with_build_dir/builddir");
    QString expected_source_Dir("/home/xxx/kdesvn/src/KDE/kdevelop/projectmanagers/cmake/tests/manual/with_build_dir");
    QCOMPARE(source_dir, expected_source_Dir);
    QCOMPARE(binary_dir, expected_binary_dir);
}

