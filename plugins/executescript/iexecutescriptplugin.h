/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_IEXECUTESCRIPTPLUGIN_H
#define KDEVPLATFORM_PLUGIN_IEXECUTESCRIPTPLUGIN_H

#include <QObject>
#include <QStringList>

namespace KDevelop
{
class ILaunchConfiguration;
}

class QUrl;

/**
@author Andreas Pakulat
@author Niko Sams
*/

class IExecuteScriptPlugin
{
public:
    virtual ~IExecuteScriptPlugin() {}

    /**
     * @return the interpreter command split into a string list
     *
     * The first element of the returned list is the interpreter program.
     * The remaining elements, if any, are the interpreter command line arguments.
     */
    [[nodiscard]] virtual QStringList interpreter(KDevelop::ILaunchConfiguration*, QString& error) const = 0;

    virtual QUrl script( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
    virtual QString remoteHost( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
    virtual QStringList arguments( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
    virtual QUrl workingDirectory( KDevelop::ILaunchConfiguration* ) const = 0;
    virtual QString environmentProfileName(KDevelop::ILaunchConfiguration*) const = 0;
    virtual QString scriptAppConfigTypeId() const = 0;
    virtual int outputFilterModeId( KDevelop::ILaunchConfiguration* ) const = 0;
    virtual bool runCurrentFile( KDevelop::ILaunchConfiguration* ) const = 0;
};

Q_DECLARE_INTERFACE( IExecuteScriptPlugin, "org.kdevelop.IExecuteScriptPlugin" )

#endif

