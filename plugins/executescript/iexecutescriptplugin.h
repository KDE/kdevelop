/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_IEXECUTESCRIPTPLUGIN_H
#define KDEVPLATFORM_PLUGIN_IEXECUTESCRIPTPLUGIN_H

#include <QObject>

namespace KDevelop
{
class ILaunchConfiguration;
}

class QUrl;
class QString;
#include <QStringList>

/**
@author Andreas Pakulat
@author Niko Sams
*/

class IExecuteScriptPlugin
{
public:
    virtual ~IExecuteScriptPlugin() {}

    virtual QString interpreter( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
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

