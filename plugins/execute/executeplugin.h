/*
 * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_PLUGIN_EXECUTEPLUGIN_H
#define KDEVPLATFORM_PLUGIN_EXECUTEPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariant>
#include "iexecuteplugin.h"

class QUrl;
class KJob;

class NativeAppConfigType;

class ExecutePlugin : public KDevelop::IPlugin, public IExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES( IExecutePlugin )

  public:
    explicit ExecutePlugin(QObject *parent, const QVariantList & = QVariantList() );
    ~ExecutePlugin() override;

    static QString workingDirEntry;
    static QString executableEntry;
    static QString argumentsEntry;
    static QString isExecutableEntry;
    static QString dependencyEntry;
    static QString environmentProfileEntry;
    static QString useTerminalEntry;
    static QString terminalEntry;
    static QString dependencyActionEntry;
    static QString projectTargetEntry;
    static QString killBeforeExecutingAgain;
    static QString configuredByCTest;
    
    void unload() override;
    
    QUrl executable( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QStringList arguments( KDevelop::ILaunchConfiguration*, QString& err ) const override;
    QUrl workingDirectory( KDevelop::ILaunchConfiguration* ) const override;
    KJob* dependencyJob( KDevelop::ILaunchConfiguration* ) const override;
    QString environmentProfileName(KDevelop::ILaunchConfiguration*) const override;
    bool useTerminal( KDevelop::ILaunchConfiguration* ) const override;
    QString terminal( KDevelop::ILaunchConfiguration* ) const override;
    QString nativeAppConfigTypeId() const override;

    NativeAppConfigType* m_configType;
};

#endif // KDEVPLATFORM_PLUGIN_EXECUTEPLUGIN_H
