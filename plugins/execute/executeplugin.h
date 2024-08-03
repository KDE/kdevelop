/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

    static constexpr const char* projectTargetEntry = "Project Target";
    static constexpr const char* argumentsEntry = "Arguments";
    static constexpr const char* configuredByCTest = "ConfiguredByCTest";
    static constexpr const char* killBeforeExecutingAgain = "Kill Before Executing Again";
    static constexpr const char* dependencyActionEntry = "Dependency Action";
    static constexpr const char* dependencyEntry = "Dependencies";

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
