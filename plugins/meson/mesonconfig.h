/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <util/path.h>

#include <KConfigGroup>

namespace KDevelop
{
class IProject;
}

namespace Meson
{

struct BuildDir {
    KDevelop::Path buildDir;
    KDevelop::Path mesonExecutable;
    QString mesonBackend;
    QString mesonArgs;

    bool isValid() const;
    void canonicalizePaths();
};

struct MesonConfig {
    int currentIndex = -1;
    QVector<BuildDir> buildDirs;

    int addBuildDir(BuildDir dir);
    bool removeBuildDir(int index);
};

KConfigGroup rootGroup(KDevelop::IProject* project);
BuildDir currentBuildDir(KDevelop::IProject* project);
MesonConfig getMesonConfig(KDevelop::IProject* project);
void writeMesonConfig(KDevelop::IProject* project, const MesonConfig& cfg);

}
