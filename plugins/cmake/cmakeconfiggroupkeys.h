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
inline constexpr QLatin1String projectRootRelativeKey("ProjectRootRelative");
}

inline QString globalBuildDirIndexKey() { return QStringLiteral("Current Build Directory Index"); }
inline constexpr QLatin1String buildDirOverrideIndexKey("Temporary Build Directory Index");
inline constexpr QLatin1String buildDirCountKey("Build Directory Count");

//the used builddir will change for every runtime
inline QString buildDirIndexKey()
{
    const QString currentRuntime = KDevelop::ICore::self()->runtimeController()->currentRuntime()->name();
    return globalBuildDirIndexKey() + QLatin1Char('-') + currentRuntime;
}

namespace Specific
{
inline constexpr QLatin1String buildDirPathKey("Build Directory Path");
// TODO: migrate to more generic & consistent key term "CMake Executable"
// Support the old "CMake Binary" key too for backwards compatibility during
// a reasonable transition period. Both keys are saved at least until 5.2.0
// is released. Import support for the old key will need to remain for a
// considerably longer period, ideally.
inline constexpr QLatin1String cmakeBinaryKey("CMake Binary");
inline constexpr QLatin1String cmakeExecutableKey("CMake Executable");
inline constexpr QLatin1String cmakeBuildTypeKey("Build Type");
inline constexpr QLatin1String cmakeInstallDirKey("Install Directory");
inline constexpr QLatin1String cmakeEnvironmentKey("Environment Profile");
inline constexpr QLatin1String cmakeArgumentsKey("Extra Arguments");
inline constexpr QLatin1String buildDirRuntime("Runtime");
}

inline QString groupNameBuildDir(int buildDirIndex)
{
    return QLatin1String("CMake Build Directory ") + QString::number(buildDirIndex);
}

inline constexpr QLatin1String groupName("CMake");

} // namespace Config
