/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executescriptplugin.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <util/shellutils.h>

#include "scriptappconfig.h"
#include "debug.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevExecuteFactory, "kdevexecutescript.json", registerPlugin<ExecuteScriptPlugin>();)

ExecuteScriptPlugin::ExecuteScriptPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevexecutescript"), parent, metaData)
{
    m_configType = new ScriptAppConfigType();
    m_configType->addLauncher( new ScriptAppLauncher( this ) );
    qCDebug(PLUGIN_EXECUTESCRIPT) << "adding script launch config";
    core()->runController()->addConfigurationType( m_configType );
}

ExecuteScriptPlugin::~ExecuteScriptPlugin()
{
}


void ExecuteScriptPlugin::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = nullptr;
}

QUrl ExecuteScriptPlugin::script(ILaunchConfiguration* cfg, QString& err) const
{
    QUrl script;

    if( !cfg )
    {
        return script;
    }
    KConfigGroup grp = cfg->config();
    script = grp.readEntry( ExecuteScriptPlugin::executableEntry, QUrl() );

    // Do not pass err to splitLocalFileLaunchConfigurationEntry(), because it may be nonempty from the beginning.
    QString initiallyEmptyErrorMessage;
    splitLocalFileLaunchConfigurationEntry(
        *cfg, script, LaunchConfigurationEntryName{"script path", i18nc("path to a script", "script path")},
        initiallyEmptyErrorMessage);
    if (initiallyEmptyErrorMessage.isEmpty()) {
        return script;
    }

    err = std::move(initiallyEmptyErrorMessage);
    return {};
}

QString ExecuteScriptPlugin::remoteHost(ILaunchConfiguration* cfg, QString& err) const
{
    if (!cfg) return QString();
    KConfigGroup grp = cfg->config();
    if(grp.readEntry(ExecuteScriptPlugin::executeOnRemoteHostEntry, false)) {
        QString host = grp.readEntry(ExecuteScriptPlugin::remoteHostEntry, "");
        if (host.isEmpty()) {
            err = i18n("No remote host set for launch configuration '%1'. "
            "Aborting start.", cfg->name() );
            qCWarning(PLUGIN_EXECUTESCRIPT) << "Launch Configuration:" << cfg->name() << "no remote host set";
        }
        return host;
    }
    return QString();
}

QStringList ExecuteScriptPlugin::arguments(ILaunchConfiguration* cfg, QString& err) const
{
    if( !cfg )
    {
        return QStringList();
    }
    const auto arguments = cfg->config().readEntry(ExecuteScriptPlugin::argumentsEntry, QString{});

    return splitLaunchConfigurationEntry(
        *cfg, arguments,
        LaunchConfigurationEntryName{"arguments", i18nc("command line arguments to a script", "arguments")}, err);
}

QString ExecuteScriptPlugin::environmentProfileName(KDevelop::ILaunchConfiguration* cfg) const
{
    if( !cfg )
    {
        return QString();
    }

    return cfg->config().readEntry(ExecuteScriptPlugin::environmentProfileEntry, QString());
}

int ExecuteScriptPlugin::outputFilterModeId( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return 0;
    }

    return cfg->config().readEntry( ExecuteScriptPlugin::outputFilteringEntry, 0 );
}

bool ExecuteScriptPlugin::runCurrentFile(ILaunchConfiguration* cfg) const
{
    if( !cfg )
    {
        return false;
    }

    return cfg->config().readEntry( ExecuteScriptPlugin::runCurrentFileEntry, true );
}

QStringList ExecuteScriptPlugin::interpreter(KDevelop::ILaunchConfiguration* cfg, QString& err) const
{
    if( !cfg )
    {
        return {};
    }
    KConfigGroup grp = cfg->config();
    auto interpreterString = grp.readEntry(ExecuteScriptPlugin::interpreterEntry, QString{});

    return splitNonemptyLaunchConfigurationEntry(
        *cfg, interpreterString, LaunchConfigurationEntryName{"interpreter command", i18n("interpreter command")}, err);
}

QUrl ExecuteScriptPlugin::workingDirectory( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QUrl();
    }

    return cfg->config().readEntry( ExecuteScriptPlugin::workingDirEntry, QUrl() );
}


QString ExecuteScriptPlugin::scriptAppConfigTypeId() const
{
    return ScriptAppConfigType::sharedId();
}


#include "executescriptplugin.moc"
#include "moc_executescriptplugin.cpp"
