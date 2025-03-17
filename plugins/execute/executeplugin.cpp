/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executeplugin.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KShell>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>

#include "nativeappconfig.h"
#include "debug.h"
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <util/kdevstringhandler.h>
#include <util/shellutils.h>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevExecuteFactory, "kdevexecute.json", registerPlugin<ExecutePlugin>();)

ExecutePlugin::ExecutePlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevexecute"), parent, metaData)
{
    m_configType = new NativeAppConfigType();
    m_configType->addLauncher( new NativeAppLauncher() );
    qCDebug(PLUGIN_EXECUTE) << "adding native app launch config";
    core()->runController()->addConfigurationType( m_configType );
}

ExecutePlugin::~ExecutePlugin()
{
}

void ExecutePlugin::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = nullptr;
}

QStringList ExecutePlugin::arguments( KDevelop::ILaunchConfiguration* cfg, QString& err_ ) const
{

    if( !cfg )
    {
        return QStringList();
    }

    KShell::Errors err;
    QStringList args = KShell::splitArgs( cfg->config().readEntry( ExecutePlugin::argumentsEntry, "" ), KShell::TildeExpand | KShell::AbortOnMeta, &err );
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
        warnAboutSplitArgsError(*cfg, err, "arguments");
        args = QStringList();
    }
    return args;
}


KJob* ExecutePlugin::dependencyJob( KDevelop::ILaunchConfiguration* cfg ) const
{
    const QVariantList deps = KDevelop::stringToQVariant( cfg->config().readEntry( dependencyEntry, QString() ) ).toList();
    QString depAction = cfg->config().readEntry( dependencyActionEntry, "Nothing" );
    if( depAction != QLatin1String("Nothing") && !deps.isEmpty() )
    {
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        QList<KDevelop::ProjectBaseItem*> items;
        for (const QVariant& dep : deps) {
            KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex( dep.toStringList() ) );
            if( item )
            {
                items << item;
            }
            else
            {
                const QString messageText = i18n("Couldn't resolve the dependency: %1", dep.toString());
                auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
                ICore::self()->uiController()->postMessage(message);
            }
        }
        auto* job = new KDevelop::BuilderJob();
        if( depAction == QLatin1String("Build") )
        {
            job->addItems( KDevelop::BuilderJob::Build, items );
        } else if( depAction == QLatin1String("Install") )
        {
            job->addItems( KDevelop::BuilderJob::Install, items );
        }
        job->updateJobName();
        return job;
    }
    return nullptr;
}


QString ExecutePlugin::environmentProfileName(KDevelop::ILaunchConfiguration* cfg) const
{
    if( !cfg )
    {
        return QString();
    }

    return cfg->config().readEntry(ExecutePlugin::environmentProfileEntry, QString());
}


QUrl ExecutePlugin::executable( KDevelop::ILaunchConfiguration* cfg, QString& err ) const
{
    QUrl executable;
    if( !cfg )
    {
        return executable;
    }
    KConfigGroup grp = cfg->config();
    if( grp.readEntry(ExecutePlugin::isExecutableEntry, false ) )
    {
        executable = grp.readEntry( ExecutePlugin::executableEntry, QUrl() );
    } else
    {
        QStringList prjitem = grp.readEntry( ExecutePlugin::projectTargetEntry, QStringList() );
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex(prjitem) );
        if( item && item->executable() )
        {
            // TODO: Need an option in the gui to choose between installed and builddir url here, currently cmake only supports builddir url
            executable = item->executable()->builtUrl();
        }
    }
    if( executable.isEmpty() )
    {
        err = i18n("No valid executable specified");
        qCWarning(PLUGIN_EXECUTE) << "Launch Configuration:" << cfg->name() << "no valid executable set";
    } else
    {
        KShell::Errors err_;
        if( KShell::splitArgs( executable.toLocalFile(), KShell::TildeExpand | KShell::AbortOnMeta, &err_ ).isEmpty() || err_ != KShell::NoError )
        {
            executable = QUrl();
            if( err_ == KShell::BadQuoting )
            {
                err = i18n("There is a quoting error in the executable "
                "for the launch configuration '%1'. "
                "Aborting start.", cfg->name() );
            } else
            {
                err = i18n("A shell meta character was included in the "
                "executable for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", cfg->name() );
            }
            warnAboutSplitArgsError(*cfg, err_, "executable path");
        }
    }
    return executable;
}


bool ExecutePlugin::useTerminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return false;
    }

    return cfg->config().readEntry( ExecutePlugin::useTerminalEntry, false );
}


QString ExecutePlugin::terminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QString();
    }

    return cfg->config().readEntry( ExecutePlugin::terminalEntry, QString() );
}


QUrl ExecutePlugin::workingDirectory( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QUrl();
    }

    return cfg->config().readEntry( ExecutePlugin::workingDirEntry, QUrl() );
}


QString ExecutePlugin::nativeAppConfigTypeId() const
{
    return NativeAppConfigType::sharedId();
}


#include "executeplugin.moc"
#include "moc_executeplugin.cpp"
