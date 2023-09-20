/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_IEXECUTEPLUGIN_H
#define KDEVPLATFORM_PLUGIN_IEXECUTEPLUGIN_H

#include <QString>

namespace KDevelop
{
class ILaunchConfiguration;
}

class KJob;
class QUrl;
#include <QStringList>

/**
@author Andreas Pakulat
*/

class IExecutePlugin
{
public:
    static constexpr const char* isExecutableEntry = "isExecutable";
    static constexpr const char* executableEntry = "Executable";
    static constexpr const char* workingDirEntry = "Working Directory";
    // TODO: migrate to more consistent key term "EnvironmentProfile"
    static constexpr const char* environmentProfileEntry = "EnvironmentGroup";
    static constexpr const char* useTerminalEntry = "Use External Terminal";
    static constexpr const char* terminalEntry = "External Terminal";

    virtual ~IExecutePlugin() {}

    virtual QUrl executable( KDevelop::ILaunchConfiguration* config, QString& error) const = 0;
    virtual QStringList arguments( KDevelop::ILaunchConfiguration* config, QString& error) const = 0;
    virtual QUrl workingDirectory( KDevelop::ILaunchConfiguration* config) const = 0;
    virtual KJob* dependencyJob( KDevelop::ILaunchConfiguration* config) const = 0;
    virtual QString environmentProfileName(KDevelop::ILaunchConfiguration* config) const = 0;
    virtual bool useTerminal( KDevelop::ILaunchConfiguration* config) const = 0;
    virtual QString terminal( KDevelop::ILaunchConfiguration* config) const = 0;
    virtual QString nativeAppConfigTypeId() const = 0;
};

Q_DECLARE_INTERFACE( IExecutePlugin, "org.kdevelop.IExecutePlugin" )

#endif

