/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#include "cmake-test-paths.h"
#include <cmakebuilddirchooser.h>
#include <cmakeconfiggroupkeys.h>
#include <KConfig>
#include <KConfigGroup>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <QSignalSpy>

struct TestProjectPaths {
    // foo/
    KDevelop::Path sourceDir;
    // foo/foo.kdev4
    KDevelop::Path projectFile;
    // foo/.kdev4/foo.kdev4
    KDevelop::Path configFile;
};

/// TODO: replace Q_ASSERT with QFAIL, QVERIFY, QCOMPARE in the functions below;
/// define and use a macro based on DO_AND_RETURN_IF_TEST_FAILED for the void function;
/// insert RETURN_IF_TEST_FAILED after each usage of these functions.

inline TestProjectPaths projectPaths(const QString& project, const QString& name = QString())
{
    TestProjectPaths paths;
    if(QDir::isRelativePath(project)) {
        QFileInfo info(QStringLiteral(CMAKE_TESTS_PROJECTS_DIR)+"/"+project);
        Q_ASSERT(info.exists());
        paths.sourceDir = KDevelop::Path(info.canonicalFilePath());
    } else {
        paths.sourceDir = KDevelop::Path(project);
    }
    Q_ASSERT(QFile::exists(paths.sourceDir.toLocalFile()));

    QString kdev4Name;
    if (name.isEmpty()) {
        QDir d(paths.sourceDir.toLocalFile());
        kdev4Name = d.entryList(QStringList(QStringLiteral("*.kdev4")), QDir::Files).takeFirst();
    } else
        kdev4Name = name+".kdev4";

    paths.projectFile = KDevelop::Path(paths.sourceDir, kdev4Name);
    Q_ASSERT(QFile::exists(paths.projectFile.toLocalFile()));

    paths.configFile = KDevelop::Path(paths.sourceDir, QString(QStringLiteral(".kdev4/") + kdev4Name));

    return paths;
}

/**
 * apply default configuration to project in @p sourceDir called @p projectName
 * 
 * this prevents the dialog to popup asking for user interaction
 * which should never happen in an automated unit test
 */
inline void defaultConfigure(const TestProjectPaths& paths)
{
    KConfig config(paths.configFile.toLocalFile());
    // clear config
    config.deleteGroup(Config::groupName);

    // apply default configuration
    CMakeBuildDirChooser bd;
    bd.setSourceFolder(paths.sourceDir);
    bd.setBuildFolder(KDevelop::Path(CMAKE_TESTS_BINARY_DIR + QStringLiteral("/build-") + paths.sourceDir.lastPathSegment()));
    // we don't want to execute, just pick the defaults from the dialog

    KConfigGroup cmakeGroup = config.group(Config::groupName);
    {
        QDir buildFolder( bd.buildFolder().toLocalFile() );
        if ( !buildFolder.exists() ) {
            if ( !buildFolder.mkpath( buildFolder.absolutePath() ) ) {
                Q_ASSERT(false && "The build directory did not exist and could not be created.");
            }
        }
    }

    cmakeGroup.writeEntry(Config::buildDirCountKey, 1);
    cmakeGroup.writeEntry(Config::buildDirIndexKey(), 0);

    KConfigGroup buildDirGroup = cmakeGroup.group(Config::groupNameBuildDir(0));
    buildDirGroup.writeEntry(Config::Specific::buildDirPathKey, bd.buildFolder().toLocalFile());
    buildDirGroup.writeEntry(Config::Specific::cmakeExecutableKey, bd.cmakeExecutable().toLocalFile());
    buildDirGroup.writeEntry(Config::Specific::cmakeInstallDirKey, bd.installPrefix().toLocalFile());
    buildDirGroup.writeEntry(Config::Specific::cmakeArgumentsKey, bd.extraArguments());
    buildDirGroup.writeEntry(Config::Specific::cmakeBuildTypeKey, bd.buildType());
    buildDirGroup.writeEntry(Config::Specific::buildDirRuntime, KDevelop::ICore::self()->runtimeController()->currentRuntime()->name());

    config.sync();
}

inline KDevelop::IProject* loadProject(const QString& name, const QString& relative = QString())
{
    qRegisterMetaType<KDevelop::IProject*>();

    const TestProjectPaths paths = projectPaths(name+relative, name);
    defaultConfigure(paths);

    QSignalSpy spy(KDevelop::ICore::self()->projectController(),
                   SIGNAL(projectOpened(KDevelop::IProject*)));
    Q_ASSERT(spy.isValid());

    KDevelop::ICore::self()->projectController()->openProject(paths.projectFile.toUrl());

    if ( spy.isEmpty() && !spy.wait(30000) ) {
        qFatal( "Timeout while waiting for opened signal" );
    }

    KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->findProjectByName(name);
    Q_ASSERT(project);
    Q_ASSERT(project->buildSystemManager());
    Q_ASSERT(paths.projectFile == project->projectFile());
    return project;
}

#endif
