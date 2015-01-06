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

#include "executeplasmoidplugin.h"
#include "plasmoidexecutionconfig.h"
#include "plasmoidexecutionjob.h"
#include "debug.h"
#include <kpluginfactory.h>
#include <KAboutData>
#include <QUrl>
#include <interfaces/icore.h>

using namespace KDevelop;

Q_LOGGING_CATEGORY(EXECUTEPLASMOID, "kdevelop.kdeintegration.executeplasmoid")
K_PLUGIN_FACTORY(KDevExecutePlasmoidFactory, registerPlugin<ExecutePlasmoidPlugin>(); )

ExecutePlasmoidPlugin::ExecutePlasmoidPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin("kdevexecuteplasmoid", parent)
{
    KDEV_USE_EXTENSION_INTERFACE( IExecutePlugin )
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
    m_configType = 0;
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

QString ExecutePlasmoidPlugin::environmentGroup(ILaunchConfiguration* /*config*/) const
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

QString ExecutePlasmoidPlugin::terminal(ILaunchConfiguration* /*config*/) const
{
    return QString();
}

#include "executeplasmoidplugin.moc"
