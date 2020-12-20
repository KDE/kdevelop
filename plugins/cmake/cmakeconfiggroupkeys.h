/* This file is part of KDevelop

    Copyright 2020 Igor Kushnir <igorkuo@gmail.com>

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

#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>

#include <QByteArray>
#include <QString>

namespace Config
{
namespace Old
{
inline constexpr const char* currentBuildDirKey = "CurrentBuildDir";
inline constexpr const char* currentBuildTypeKey = "CurrentBuildType";
inline constexpr const char* currentInstallDirKey = "CurrentInstallDir";
inline constexpr const char* currentEnvironmentKey = "CurrentEnvironment";
inline constexpr const char* currentExtraArgumentsKey = "Extra Arguments";
inline constexpr const char* currentCMakeExecutableKey = "Current CMake Binary";
inline constexpr const char* projectRootRelativeKey = "ProjectRootRelative";
inline constexpr const char* projectBuildDirs = "BuildDirs";
}

inline QString globalBuildDirIndexKey() { return QStringLiteral("Current Build Directory Index"); }
inline constexpr const char* buildDirOverrideIndexKey = "Temporary Build Directory Index";
inline constexpr const char* buildDirCountKey = "Build Directory Count";

//the used builddir will change for every runtime
inline QString buildDirIndexKey()
{
    const QString currentRuntime = KDevelop::ICore::self()->runtimeController()->currentRuntime()->name();
    return globalBuildDirIndexKey() + QLatin1Char('-') + currentRuntime;
}

namespace Specific
{
inline constexpr const char* buildDirPathKey = "Build Directory Path";
// TODO: migrate to more generic & consistent key term "CMake Executable"
// Support the old "CMake Binary" key too for backwards compatibility during
// a reasonable transition period. Both keys are saved at least until 5.2.0
// is released. Import support for the old key will need to remain for a
// considerably longer period, ideally.
inline constexpr const char* cmakeBinaryKey = "CMake Binary";
inline constexpr const char* cmakeExecutableKey = "CMake Executable";
inline constexpr const char* cmakeBuildTypeKey = "Build Type";
inline constexpr const char* cmakeInstallDirKey = "Install Directory";
inline constexpr const char* cmakeEnvironmentKey = "Environment Profile";
inline constexpr const char* cmakeArgumentsKey = "Extra Arguments";
inline constexpr const char* buildDirRuntime = "Runtime";
}

inline QByteArray groupNameBuildDir(int buildDirIndex)
{
    return QByteArrayLiteral("CMake Build Directory ") + QByteArray::number(buildDirIndex);
}

inline constexpr const char* groupName = "CMake";

} // namespace Config
