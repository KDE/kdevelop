/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executeplasmoidplugin.h"
#include "plasmoidexecutionconfig.h"
#include "plasmoidexecutionjob.h"
#include "debug.h"

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include <KPluginFactory>

#include <QUrl>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevExecutePlasmoidFactory,"kdevexecuteplasmoid.json", registerPlugin<ExecutePlasmoidPlugin>(); )

ExecutePlasmoidPlugin::ExecutePlasmoidPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevexecuteplasmoid"), parent, metaData)
{
    m_configType = new PlasmoidExecutionConfigType();
    m_configType->addLauncher( new PlasmoidLauncher( this ) );
    qCDebug(EXECUTEPLASMOID) << "adding plasmoid launch config";
    core()->runController()->addConfigurationType( m_configType );
}

ExecutePlasmoidPlugin::~ExecutePlasmoidPlugin()
{}

void ExecutePlasmoidPlugin::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = nullptr;
}

QUrl ExecutePlasmoidPlugin::executable(ILaunchConfiguration* config, QString& /*error*/) const
{
    return QUrl::fromLocalFile(PlasmoidExecutionJob::executable(config));
}

QStringList ExecutePlasmoidPlugin::arguments(ILaunchConfiguration* config, QString& /*error*/) const
{
    return PlasmoidExecutionJob::arguments(config);
}

KJob* ExecutePlasmoidPlugin::dependencyJob(ILaunchConfiguration* config) const
{
    return PlasmoidLauncher::calculateDependencies(config);
}

QUrl ExecutePlasmoidPlugin::workingDirectory(ILaunchConfiguration* config) const
{
    return QUrl::fromLocalFile(PlasmoidExecutionJob::workingDirectory(config));
}

QString ExecutePlasmoidPlugin::environmentProfileName(ILaunchConfiguration* /*config*/) const
{
    return QString();
}

QString ExecutePlasmoidPlugin::nativeAppConfigTypeId() const
{
    return PlasmoidExecutionConfigType::typeId();
}

bool ExecutePlasmoidPlugin::useTerminal(ILaunchConfiguration* /*config*/) const
{
    return false;
}

QStringList ExecutePlasmoidPlugin::terminal(KDevelop::ILaunchConfiguration* config, QString& error) const
{
    Q_UNUSED(config)
    Q_UNUSED(error)
    return {};
}

QStringList ExecutePlasmoidPlugin::defaultExternalTerminalCommands() const
{
    return {};
}

#include "executeplasmoidplugin.moc"
#include "moc_executeplasmoidplugin.cpp"
