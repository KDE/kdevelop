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
#include <QtCore/QVariant>
#include <QtCore/QProcess>
#include "iexecutescriptplugin.h"

class ScriptAppConfigType;
class QUrl;
class KJob;

class ExecuteScriptPlugin : public KDevelop::IPlugin, public IExecuteScriptPlugin
{
    Q_OBJECT
    Q_INTERFACES( IExecuteScriptPlugin )

  public:
    ExecuteScriptPlugin(QObject *parent, const QVariantList & = QVariantList() );
    virtual ~ExecuteScriptPlugin();

    static QString _scriptAppConfigTypeId;
    static QString interpreterEntry;
    static QString workingDirEntry;
    static QString executableEntry;
    static QString executeOnRemoteHostEntry;
    static QString remoteHostEntry;
    static QString runCurrentFileEntry;
    static QString argumentsEntry;
    static QString isExecutableEntry;
    static QString environmentGroupEntry;
    //static QString useTerminalEntry;
    static QString userIdToRunEntry;
    static QString projectTargetEntry;
    static QString outputFilteringEntry;
    
    virtual void unload() override;
    
    QString interpreter( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QUrl script( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QString remoteHost(KDevelop::ILaunchConfiguration* , QString& err) const override;
    QStringList arguments( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QUrl workingDirectory( KDevelop::ILaunchConfiguration* ) const override;
    QString environmentGroup( KDevelop::ILaunchConfiguration* ) const override;
    //bool useTerminal( KDevelop::ILaunchConfiguration* ) const;
    QString scriptAppConfigTypeId() const override;
    int outputFilterModeId( KDevelop::ILaunchConfiguration* ) const override;
    bool runCurrentFile(KDevelop::ILaunchConfiguration*) const override;

    ScriptAppConfigType* m_configType;
};

#endif // KDEVPLATFORM_PLUGIN_EXECUTESCRIPTPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
