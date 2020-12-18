/*
 * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_EXECUTESCRIPTPLUGIN_H
#define KDEVPLATFORM_PLUGIN_EXECUTESCRIPTPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariant>
#include "iexecutescriptplugin.h"

class ScriptAppConfigType;
class QUrl;

class ExecuteScriptPlugin : public KDevelop::IPlugin, public IExecuteScriptPlugin
{
    Q_OBJECT
    Q_INTERFACES( IExecuteScriptPlugin )

  public:
    explicit ExecuteScriptPlugin(QObject *parent, const QVariantList & = QVariantList() );
    ~ExecuteScriptPlugin() override;

    static QString interpreterEntry;
    static QString workingDirEntry;
    static QString executableEntry;
    static QString executeOnRemoteHostEntry;
    static QString remoteHostEntry;
    static QString runCurrentFileEntry;
    static QString argumentsEntry;
    static QString environmentProfileEntry;
    static QString outputFilteringEntry;
    
    void unload() override;
    
    QString interpreter( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QUrl script( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QString remoteHost(KDevelop::ILaunchConfiguration* , QString& err) const override;
    QStringList arguments( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QUrl workingDirectory( KDevelop::ILaunchConfiguration* ) const override;
    QString environmentProfileName(KDevelop::ILaunchConfiguration*) const override;
    QString scriptAppConfigTypeId() const override;
    int outputFilterModeId( KDevelop::ILaunchConfiguration* ) const override;
    bool runCurrentFile(KDevelop::ILaunchConfiguration*) const override;

    ScriptAppConfigType* m_configType;
};

#endif // KDEVPLATFORM_PLUGIN_EXECUTESCRIPTPLUGIN_H
