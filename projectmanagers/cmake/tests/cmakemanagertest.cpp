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

#include <icmakemanager.h>
#include <cmakebuilddirchooser.h>

#include <qtest.h>
#include <qtest_kde.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>
#include <tests/testcore.h>

QTEST_KDEMAIN(CMakeManagerTest, GUI )

#define WAIT_FOR_OPEN_SIGNAL \
{\
    bool gotSignal = QTest::kWaitForSignal(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), 30000);\
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");\
} void(0)

static QString currentBuildDirKey = "CurrentBuildDir";
static QString currentCMakeBinaryKey = "Current CMake Binary";
static QString currentBuildTypeKey = "CurrentBuildType";
static QString currentInstallDirKey = "CurrentInstallDir";
static QString currentExtraArgumentsKey = "Extra Arguments";
static QString projectRootRelativeKey = "ProjectRootRelative";
static QString projectBuildDirs = "BuildDirs";

/**
 * apply default configuration to project in @p sourceDir called @p projectName
 * 
 * this prevents the dialog to popup asking for user interaction
 * which should never happen in an automated unit test
 */
void defaultConfigure(const KUrl& sourceDir, const QString& projectName)
{
    QVERIFY(QDir(sourceDir.toLocalFile()).exists());

    KConfig config(sourceDir.toLocalFile(KUrl::AddTrailingSlash) + ".kdev4/" + projectName + ".kdev4");
    // clear config
    config.deleteGroup("CMake");

    // apply default configuration
    CMakeBuildDirChooser bd;
    bd.setSourceFolder(sourceDir);
    // we don't want to execute, just pick the defaults from the dialog

    KConfigGroup cmakeGrp = config.group("CMake");
    {
        QDir buildFolder( bd.buildFolder().toLocalFile() );
        if ( !buildFolder.exists() ) {
            if ( !buildFolder.mkpath( buildFolder.absolutePath() ) ) {
                QFAIL("The build directory did not exist and could not be created.");
            }
        }
    }

    cmakeGrp.writeEntry( currentBuildDirKey, bd.buildFolder() );
    cmakeGrp.writeEntry( currentCMakeBinaryKey, bd.cmakeBinary() );
    cmakeGrp.writeEntry( currentInstallDirKey, bd.installPrefix() );
    cmakeGrp.writeEntry( currentExtraArgumentsKey, bd.extraArguments() );
    cmakeGrp.writeEntry( currentBuildTypeKey, bd.buildType() );
    cmakeGrp.writeEntry( projectBuildDirs, QStringList() << bd.buildFolder().toLocalFile());

    config.sync();
}

using namespace KDevelop;

CMakeManagerTest::CMakeManagerTest(QObject* parent): QObject(parent)
{
    AutoTestShell::init();
    TestCore::initialize();
}

CMakeManagerTest::~CMakeManagerTest()
{
    TestCore::shutdown();
}

void CMakeManagerTest::testWithBuildDirProject()
{
    KUrl url(QFileInfo(CMAKE_TESTS_PROJECTS_DIR "/with_build_dir/with_build_dir.kdev4").canonicalFilePath());
    KUrl expected_source_Dir(QFileInfo(CMAKE_TESTS_PROJECTS_DIR "/with_build_dir").canonicalFilePath());
    expected_source_Dir.adjustPath(KUrl::AddTrailingSlash);

    defaultConfigure(expected_source_Dir, "with_build_dir");

    // Import project
    QList< ProjectFolderItem* > items;
    
    ICore::self()->projectController()->openProject(url);
    
    WAIT_FOR_OPEN_SIGNAL;
    
    IProject* project = ICore::self()->projectController()->findProjectByName("with_build_dir");
    QCOMPARE(url, project->projectFileUrl());
    QCOMPARE(expected_source_Dir, project->folder());
}

