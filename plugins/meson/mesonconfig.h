/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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

struct MesonConfig
{
    int currentIndex = -1;
    QVector<BuildDir> buildDirs;

    int addBuildDir(BuildDir dir);
    bool removeBuildDir(int index);
};

KConfigGroup rootGroup(KDevelop::IProject* project);
BuildDir currentBuildDir(KDevelop::IProject* project);
MesonConfig getMesonConfig(KDevelop::IProject* project);
void writeMesonConfig(KDevelop::IProject* project, MesonConfig const& cfg);

}
