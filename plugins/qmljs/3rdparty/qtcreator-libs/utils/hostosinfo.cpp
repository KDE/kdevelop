/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "hostosinfo.h"

#include <QCoreApplication>

#if !defined(QT_NO_OPENGL) && defined(QT_GUI_LIB)
#include <QOpenGLContext>
#endif

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#endif

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#ifdef Q_OS_MACOS
#include <sys/sysctl.h>
#endif

using namespace Utils;

Qt::CaseSensitivity HostOsInfo::m_overrideFileNameCaseSensitivity = Qt::CaseSensitive;
bool HostOsInfo::m_useOverrideFileNameCaseSensitivity = false;

#ifdef Q_OS_WIN
static WORD hostProcessorArchitecture()
{
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    return info.wProcessorArchitecture;
}
#endif

HostOsInfo::HostArchitecture HostOsInfo::hostArchitecture()
{
#ifdef Q_OS_WIN
    static const WORD processorArchitecture = hostProcessorArchitecture();
    switch (processorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        return HostOsInfo::HostArchitectureAMD64;
    case PROCESSOR_ARCHITECTURE_INTEL:
        return HostOsInfo::HostArchitectureX86;
    case PROCESSOR_ARCHITECTURE_IA64:
        return HostOsInfo::HostArchitectureItanium;
    case PROCESSOR_ARCHITECTURE_ARM:
        return HostOsInfo::HostArchitectureArm;
    case PROCESSOR_ARCHITECTURE_ARM64:
        return HostOsInfo::HostArchitectureArm64;
    default:
        return HostOsInfo::HostArchitectureUnknown;
    }
#else
    return HostOsInfo::HostArchitectureUnknown;
#endif
}

bool HostOsInfo::isRunningUnderRosetta()
{
#ifdef Q_OS_MACOS
    int translated = 0;
    auto size = sizeof(translated);
    if (sysctlbyname("sysctl.proc_translated", &translated, &size, nullptr, 0) == 0)
        return translated;
#endif
    return false;
}

void HostOsInfo::setOverrideFileNameCaseSensitivity(Qt::CaseSensitivity sensitivity)
{
    m_useOverrideFileNameCaseSensitivity = true;
    m_overrideFileNameCaseSensitivity = sensitivity;
}

void HostOsInfo::unsetOverrideFileNameCaseSensitivity()
{
    m_useOverrideFileNameCaseSensitivity = false;
}

bool HostOsInfo::canCreateOpenGLContext(QString *errorMessage)
{
#if defined(QT_NO_OPENGL) || !defined(QT_GUI_LIB)
    Q_UNUSED(errorMessage)
    return false;
#else
    static const bool canCreate = QOpenGLContext().create();
    if (!canCreate)
        *errorMessage = QCoreApplication::translate("Utils::HostOsInfo",
                                                    "Cannot create OpenGL context.");
    return canCreate;
#endif
}

optional<quint64> HostOsInfo::totalMemoryInstalledInBytes()
{
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == -1)
        return {};
    return info.totalram;
#elif defined(Q_OS_WIN)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof statex;
    if (!GlobalMemoryStatusEx(&statex))
        return {};
    return statex.ullTotalPhys;
#elif defined(Q_OS_MACOS)
    int mib[] = {CTL_HW, HW_MEMSIZE};
    int64_t ram;
    size_t length = sizeof(int64_t);
    if (sysctl(mib, 2, &ram, &length, nullptr, 0) == -1)
        return {};
    return ram;
#endif
    return {};
}
