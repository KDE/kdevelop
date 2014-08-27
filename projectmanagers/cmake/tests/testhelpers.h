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
#include <kconfig.h>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <QSignalSpy>

static QString currentBuildDirKey = "Build Directory Path";
static QString currentCMakeBinaryKey = "CMake Binary";
static QString currentBuildTypeKey = "Build Type";
static QString currentInstallDirKey = "Install Directory";
static QString currentExtraArgumentsKey = "Extra Arguments";
static QString currentBuildDirectoryIndexKey = "Current Build Directory Index";
static QString projectBuildDirectoryCount = "ProjectRootRelative";
static QString projectRootRelativeKey = "ProjectRootRelative";
static QString projectBuildDirs = "BuildDirs";

struct TestProjectPaths {
    // foo/
    KUrl sourceDir;
    // foo/foo.kdev4
    KUrl projectFile;
    // foo/.kdev4/foo.kdev4
    KUrl configFile;
};

TestProjectPaths projectPaths(const QString& project, QString name = QString())
{
    TestProjectPaths paths;
    if(QDir::isRelativePath(project)) {
        QFileInfo info(QString(CMAKE_TESTS_PROJECTS_DIR)+"/"+project);
        Q_ASSERT(info.exists());
        paths.sourceDir = info.canonicalFilePath();
    } else {
        paths.sourceDir = project;
    }
    paths.sourceDir.adjustPath(KUrl::AddTrailingSlash);

    QString kdev4Name;
    if (name.isEmpty()) {
        QDir d(paths.sourceDir.toLocalFile());
        kdev4Name = d.entryList(QStringList("*.kdev4"), QDir::Files).takeFirst();
    } else
        kdev4Name = name+".kdev4";

    paths.projectFile = paths.sourceDir;
    paths.projectFile.addPath(kdev4Name);
    Q_ASSERT(QFile::exists(paths.projectFile.toLocalFile()));

    paths.configFile = paths.sourceDir;
    paths.configFile.addPath(".kdev4/" + kdev4Name);

    return paths;
}

/**
 * apply default configuration to project in @p sourceDir called @p projectName
 * 
 * this prevents the dialog to popup asking for user interaction
 * which should never happen in an automated unit test
 */
void defaultConfigure(const TestProjectPaths& paths)
{
    KConfig config(paths.configFile.toLocalFile());
    // clear config
    config.deleteGroup("CMake");

    // apply default configuration
    CMakeBuildDirChooser bd;
    bd.setSourceFolder(paths.sourceDir);
    // we don't want to execute, just pick the defaults from the dialog

    KConfigGroup cmakeGrp = config.group("CMake");
    {
        QDir buildFolder( bd.buildFolder().toLocalFile() );
        if ( !buildFolder.exists() ) {
            if ( !buildFolder.mkpath( buildFolder.absolutePath() ) ) {
                Q_ASSERT(false && "The build directory did not exist and could not be created.");
            }
        }
    }

    cmakeGrp.writeEntry( projectBuildDirectoryCount, 1);
    cmakeGrp.writeEntry( currentBuildDirectoryIndexKey, 0);

    KConfigGroup buildDirGrp = cmakeGrp.group(QStringLiteral("CMake Build Directory 0"));
    buildDirGrp.writeEntry<QUrl>( currentBuildDirKey, bd.buildFolder() );
    buildDirGrp.writeEntry<QUrl>( currentCMakeBinaryKey, bd.cmakeBinary() );
    buildDirGrp.writeEntry<QUrl>( currentInstallDirKey, bd.installPrefix() );
    buildDirGrp.writeEntry( currentExtraArgumentsKey, bd.extraArguments() );
    buildDirGrp.writeEntry( currentBuildTypeKey, bd.buildType() );
    buildDirGrp.writeEntry( projectBuildDirs, QStringList() << bd.buildFolder().toLocalFile());

    config.sync();
}

KDevelop::IProject* loadProject(const QString& name, const QString& relative = QString())
{
    const TestProjectPaths paths = projectPaths(name+relative, name);
    defaultConfigure(paths);

    KDevelop::ICore::self()->projectController()->openProject(paths.projectFile);

    qRegisterMetaType<KDevelop::IProject*>();
    const bool gotSignal = QSignalSpy(
            KDevelop::ICore::self()->projectController(),
            SIGNAL(projectOpened(KDevelop::IProject*))).wait(30000);
    if( !gotSignal )
        qFatal( "Timeout while waiting for opened signal" );


    KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->findProjectByName(name);
    Q_ASSERT(project);
    Q_ASSERT(project->buildSystemManager());
    Q_ASSERT(paths.projectFile == project->projectFileUrl());
    Q_ASSERT(project->folder() == project->folder());
    return project;
}

#endif
