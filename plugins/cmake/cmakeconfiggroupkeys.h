/*
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
inline constexpr const char* projectRootRelativeKey = "ProjectRootRelative";
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
