/*
   Copyright 2018 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "androiddockerruntime.h"
#include <KProcess>
#include <QStandardPaths>
#include <QAction>
#include <KLocalizedString>
#include <QFile>
#include <QDebug>
#include <QDir>

AndroidDockerRuntime::AndroidDockerRuntime()
//     : DockerRuntime(QStringLiteral("kdeorg/ci-android"))
    : DockerRuntime(QStringLiteral("asdk"))
{
}

static QByteArray s_installPrefix = "/opt/kdev-install/";

QByteArray AndroidDockerRuntime::getenv(const QByteArray& varname) const
{
    if (varname == "KDEV_DEFAULT_INSTALL_PREFIX") {
        return s_installPrefix;
    }
    return DockerRuntime::getenv(varname);
}

QStringList AndroidDockerRuntime::extraDockerArguments() const
{
    const QString installPrefixLocation = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/kdevdockerandroidprefix");
    const QString outputLocation = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QStringLiteral("/kdevandroid");
    QDir().mkpath(installPrefixLocation);
    QDir().mkpath(outputLocation);

    const QString installPrefixString = QFile::decodeName(s_installPrefix);
    auto ret = QStringList{ QStringLiteral("--volume"), (installPrefixLocation + QLatin1Char(':') + s_installPrefix) //the install prefix
           , QStringLiteral("--volume"), (outputLocation + QStringLiteral(":/output")) //the output location
           , QStringLiteral("-e"), (QStringLiteral("APP_INSTALL_PREFIX=") + s_installPrefix) //tells the build-cmake script where to install dependencies so we can reuse them
    };
    return ret;
}

QString AndroidDockerRuntime::name() const
{
    return QStringLiteral("Android + Qt");
}

QStringList AndroidDockerRuntime::args() const
{
    return { QStringLiteral("-DCMAKE_TOOLCHAIN_FILE=/opt/kdeandroid-deps/share/ECM/toolchain/Android.cmake"), QStringLiteral("-DKF5_HOST_TOOLING=/opt/nativetooling/lib/x86_64-linux-gnu/cmake/"), QStringLiteral("-DCMAKE_PREFIX_PATH=") + QString::fromUtf8(getenv("QT_ANDROID")) + QStringLiteral(";/opt/kdeandroid-deps;") + s_installPrefix };
}

QStringList AndroidDockerRuntime::extraProcessArguments(KProcess* process) const
{
    if (process->program().constFirst().endsWith(QLatin1String("/cmake")))
        return args();
    return {};
}

QStringList AndroidDockerRuntime::extraProcessArguments(QProcess* process) const
{
    if (process->program().endsWith(QLatin1String("/cmake")))
        return args();
    return {};
}

