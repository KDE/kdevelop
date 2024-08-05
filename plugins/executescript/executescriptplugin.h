/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    explicit ExecuteScriptPlugin(QObject* parent, const KPluginMetaData& metaData,
                                 const QVariantList& = QVariantList());
    ~ExecuteScriptPlugin() override;

    static constexpr const char* interpreterEntry = "Interpreter";
    static constexpr const char* runCurrentFileEntry = "Run current file";
    static constexpr const char* executableEntry = "Executable";
    static constexpr const char* argumentsEntry = "Arguments";
    static constexpr const char* workingDirEntry = "Working Directory";
    // TODO: migrate to more consistent key term "EnvironmentProfile"
    static constexpr const char* environmentProfileEntry = "EnvironmentGroup";
    static constexpr const char* outputFilteringEntry = "Output Filtering Mode";
    static constexpr const char* executeOnRemoteHostEntry = "Execute on Remote Host";
    static constexpr const char* remoteHostEntry = "Remote Host";

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
