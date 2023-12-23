/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "launchconfiguration.h"


#include <interfaces/iproject.h>
#include <interfaces/launchconfigurationtype.h>

#include "core.h"
#include "runcontroller.h"
#include <interfaces/ilauncher.h>

namespace KDevelop
{

class LaunchConfigurationPrivate
{
public:
    LaunchConfigurationPrivate(const KConfigGroup& grp, IProject* project)
        : baseGroup(grp)
        , project(project)
    {}

    KConfigGroup baseGroup;
    IProject* const project;
    LaunchConfigurationType* type;
};

QString LaunchConfiguration::LaunchConfigurationNameEntry()
{
    return QStringLiteral("Name");
}

QString LaunchConfiguration::LaunchConfigurationTypeEntry()
{
    return QStringLiteral("Type");
}

LaunchConfiguration::LaunchConfiguration(const KConfigGroup& grp, IProject* project, QObject* parent )
    : QObject(parent)
    , ILaunchConfiguration()
    , d_ptr(new LaunchConfigurationPrivate(grp, project))
{
    Q_D(LaunchConfiguration);

    d->type = Core::self()->runControllerInternal()->launchConfigurationTypeForId(grp.readEntry(LaunchConfigurationTypeEntry(), QString()));
}

LaunchConfiguration::~LaunchConfiguration()
{
}

KConfigGroup LaunchConfiguration::config()
{
    Q_D(LaunchConfiguration);

    return d->baseGroup.group(QStringLiteral("Data"));
}

const KConfigGroup LaunchConfiguration::config() const
{
    Q_D(const LaunchConfiguration);

    return d->baseGroup.group(QStringLiteral("Data"));
}

QString LaunchConfiguration::name() const
{
    Q_D(const LaunchConfiguration);

    return d->baseGroup.readEntry(LaunchConfigurationNameEntry(), QString());
}

IProject* LaunchConfiguration::project() const
{
    Q_D(const LaunchConfiguration);

    return d->project;
}

LaunchConfigurationType* LaunchConfiguration::type() const
{
    Q_D(const LaunchConfiguration);

    return d->type;
}

void LaunchConfiguration::setName(const QString& name)
{
    Q_D(LaunchConfiguration);

    d->baseGroup.writeEntry(LaunchConfigurationNameEntry(), name);
    d->baseGroup.sync();
    emit nameChanged( this );
}

void LaunchConfiguration::setType(const QString& typeId)
{
    Q_D(LaunchConfiguration);

    LaunchConfigurationType* t = Core::self()->runControllerInternal()->launchConfigurationTypeForId( typeId );
    // If this ever happens something seriously screwed in the launch config dialog, as that is 
    // the only place from where this method should be called
    Q_ASSERT(t);
    if( t )
    {
        d->baseGroup.deleteGroup(QStringLiteral("Data"));
        d->type = t;
        d->baseGroup.writeEntry(LaunchConfigurationTypeEntry(), d->type->id());
        d->baseGroup.sync();
        emit typeChanged( t );
    }
}

void LaunchConfiguration::save()
{
    Q_D(LaunchConfiguration);

    d->baseGroup.sync();
}

QString LaunchConfiguration::configGroupName() const
{
    Q_D(const LaunchConfiguration);

    return d->baseGroup.name();
}

QString LaunchConfiguration::launcherForMode(const QString& mode) const
{
    Q_D(const LaunchConfiguration);

    QStringList modes = d->baseGroup.readEntry("Configured Launch Modes", QStringList());
    int idx = modes.indexOf( mode );
    if( idx != -1 )
    {
        const QStringList launcherIds = d->baseGroup.readEntry("Configured Launchers", QStringList());
        if (launcherIds.count() > idx ) {
            const auto& id = launcherIds.at(idx);
            const auto typeLaunchers = type()->launchers();
            for (ILauncher* l : typeLaunchers) {
                if (l->id() == id) {
                    return id;
                }
            }
        }
    }

    // No launcher configured, if it's debug mode, prefer GDB if available.
    if( mode == QLatin1String("debug") )
    {
        const auto launchers = type()->launchers();
        for (ILauncher* l : launchers) {
            if( l->supportedModes().contains( mode ) && l->id() == QLatin1String("gdb") )
            {
                return l->id();
            }
        }
    }
    // Otherwise, lets just try with the first one in the list and hope it works
    const auto launchers = type()->launchers();
    for (ILauncher* l : launchers) {
        if( l->supportedModes().contains( mode ) )
        {
            return l->id();
        }
    }

    return QString();
}

void LaunchConfiguration::setLauncherForMode(const QString& mode, const QString& id)
{
    Q_D(LaunchConfiguration);

    QStringList modes = d->baseGroup.readEntry("Configured Launch Modes", QStringList());
    int idx = modes.indexOf( mode );
    if( idx == -1 )
    {
        idx = modes.count();
        modes << mode;
        d->baseGroup.writeEntry("Configured Launch Modes", modes);
    }
    QStringList launchers = d->baseGroup.readEntry("Configured Launchers", QStringList());
    if( launchers.count() > idx )
    {
        launchers.replace(idx, id);
    } else
    {
        launchers.append( id );
    }
    d->baseGroup.writeEntry("Configured Launchers", launchers);
}


}

#include "moc_launchconfiguration.cpp"
