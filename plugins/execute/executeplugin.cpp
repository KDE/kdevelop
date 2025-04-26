/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executeplugin.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

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

QStringList ExecutePlugin::arguments(ILaunchConfiguration* cfg, QString& err) const
{

    if( !cfg )
    {
        return QStringList();
    }
    const auto arguments = cfg->config().readEntry(ExecutePlugin::argumentsEntry, QString{});

    return splitLaunchConfigurationEntry(
        *cfg, arguments,
        LaunchConfigurationEntryName{"arguments", i18nc("command line arguments to an executable", "arguments")}, err);
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

    // Do not pass err to splitLocalFileLaunchConfigurationEntry(), because it may be nonempty from the beginning.
    QString initiallyEmptyErrorMessage;
    splitLocalFileLaunchConfigurationEntry(
        *cfg, executable,
        LaunchConfigurationEntryName{"executable path", i18nc("path to an executable", "executable path")},
        initiallyEmptyErrorMessage);
    if (initiallyEmptyErrorMessage.isEmpty()) {
        return executable;
    }

    err = std::move(initiallyEmptyErrorMessage);
    return {};
}


bool ExecutePlugin::useTerminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return false;
    }

    return cfg->config().readEntry( ExecutePlugin::useTerminalEntry, false );
}

QStringList ExecutePlugin::terminal(KDevelop::ILaunchConfiguration* cfg, QString& error) const
{
    if( !cfg )
    {
        return {};
    }
    auto terminalCommand = cfg->config().readEntry(ExecutePlugin::terminalEntry, QString{});

    // Keep old external terminal config working and (mostly) preserve backward compatibility:
    {
        // 1) remove an obsolete placeholder %exe from the end of the command
        constexpr QLatin1String exePlaceholder("%exe");
        if (terminalCommand.endsWith(exePlaceholder)) {
            terminalCommand.chop(exePlaceholder.size());
            qCWarning(PLUGIN_EXECUTE).nospace()
                << "the external terminal command for the launch configuration " << cfg->name()
                << " ends with an obsolete placeholder " << exePlaceholder << ", please remove it";
        }

        // 2) remove obsolete --workdir arguments to konsole
        constexpr QLatin1String workdirArguments("--workdir %workdir");
        const auto previousSize = terminalCommand.size();
        terminalCommand.remove(workdirArguments);
        if (terminalCommand.size() != previousSize) {
            Q_ASSERT(terminalCommand.size() + workdirArguments.size() <= previousSize);
            qCWarning(PLUGIN_EXECUTE).nospace()
                << "the external terminal command for the launch configuration " << cfg->name()
                << " contains obsolete arguments " << workdirArguments << ", please remove them";
        }
    }

    return splitNonemptyLaunchConfigurationEntry(
        *cfg, terminalCommand,
        LaunchConfigurationEntryName{"external terminal command", i18n("external terminal command")}, error);
}


QUrl ExecutePlugin::workingDirectory( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QUrl();
    }

    return cfg->config().readEntry( ExecutePlugin::workingDirEntry, QUrl() );
}

QStringList ExecutePlugin::defaultExternalTerminalCommands() const
{
    static const QStringList commands{QStringLiteral("konsole --hold -e"), QStringLiteral("xterm -hold -e"),
                                      QStringLiteral("xfce4-terminal --disable-server --hold -x"),
                                      QStringLiteral("gnome-terminal --wait --")};
    return commands;
}

QString ExecutePlugin::nativeAppConfigTypeId() const
{
    return NativeAppConfigType::sharedId();
}


#include "executeplugin.moc"
#include "moc_executeplugin.cpp"
