/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executescriptplugin.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KShell>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>
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

QUrl ExecuteScriptPlugin::script( KDevelop::ILaunchConfiguration* cfg, QString& err_ ) const
{
    QUrl script;

    if( !cfg )
    {
        return script;
    }
    KConfigGroup grp = cfg->config();

    script = grp.readEntry( ExecuteScriptPlugin::executableEntry, QUrl() );
    if( !script.isLocalFile() || script.isEmpty() )
    {
        err_ = i18n("No valid executable specified");
        qCWarning(PLUGIN_EXECUTESCRIPT) << "Launch Configuration:" << cfg->name() << "no valid script set";
    } else
    {
        KShell::Errors err;
        if( KShell::splitArgs( script.toLocalFile(), KShell::TildeExpand | KShell::AbortOnMeta, &err ).isEmpty() || err != KShell::NoError )
        {
            script = QUrl();
            if( err == KShell::BadQuoting )
            {
                err_ = i18n("There is a quoting error in the script "
                "for the launch configuration '%1'. "
                "Aborting start.", cfg->name() );
            } else
            {
                err_ = i18n("A shell meta character was included in the "
                "script for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", cfg->name() );
            }
            qCWarning(PLUGIN_EXECUTESCRIPT) << "Launch Configuration:" << cfg->name() << "script has meta characters";
        }
    }
    return script;
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

QStringList ExecuteScriptPlugin::arguments( KDevelop::ILaunchConfiguration* cfg, QString& err_ ) const
{
    if( !cfg )
    {
        return QStringList();
    }

    KShell::Errors err;
    QStringList args = KShell::splitArgs( cfg->config().readEntry( ExecuteScriptPlugin::argumentsEntry, "" ), KShell::TildeExpand | KShell::AbortOnMeta, &err );
    if( err != KShell::NoError )
    {

        if( err == KShell::BadQuoting )
        {
            err_ = i18n("There is a quoting error in the arguments for "
            "the launch configuration '%1'. Aborting start.", cfg->name() );
        } else
        {
            err_ = i18n("A shell meta character was included in the "
            "arguments for the launch configuration '%1', "
            "this is not supported currently. Aborting start.", cfg->name() );
        }
        args = QStringList();
        qCWarning(PLUGIN_EXECUTESCRIPT) << "Launch Configuration:" << cfg->name() << "arguments have meta characters";
    }
    return args;
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

QString ExecuteScriptPlugin::interpreter( KDevelop::ILaunchConfiguration* cfg, QString& err ) const
{
    QString interpreter;
    if( !cfg )
    {
        return interpreter;
    }
    KConfigGroup grp = cfg->config();

    interpreter = grp.readEntry( ExecuteScriptPlugin::interpreterEntry, QString() );

    if( interpreter.isEmpty() )
    {
        err = i18n("No valid interpreter specified");
        qCWarning(PLUGIN_EXECUTESCRIPT) << "Launch Configuration:" << cfg->name() << "no valid interpreter set";
    } else
    {
        KShell::Errors err_;
        if( KShell::splitArgs( interpreter, KShell::TildeExpand | KShell::AbortOnMeta, &err_ ).isEmpty() || err_ != KShell::NoError )
        {
            interpreter.clear();
            if( err_ == KShell::BadQuoting )
            {
                err = i18n("There is a quoting error in the interpreter "
                "for the launch configuration '%1'. "
                "Aborting start.", cfg->name() );
            } else
            {
                err = i18n("A shell meta character was included in the "
                "interpreter for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", cfg->name() );
            }
            qCWarning(PLUGIN_EXECUTESCRIPT) << "Launch Configuration:" << cfg->name() << "interpreter has meta characters";
        }
    }
    return interpreter;
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
