/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
    , d(new LaunchConfigurationPrivate(grp, project))
{
    d->type = Core::self()->runControllerInternal()->launchConfigurationTypeForId(grp.readEntry(LaunchConfigurationTypeEntry(), QString()));
}

LaunchConfiguration::~LaunchConfiguration()
{
}

KConfigGroup LaunchConfiguration::config()
{
    return d->baseGroup.group("Data");
}

const KConfigGroup LaunchConfiguration::config() const
{
    return d->baseGroup.group("Data");
}

QString LaunchConfiguration::name() const
{
    return d->baseGroup.readEntry(LaunchConfigurationNameEntry(), QString());
}

IProject* LaunchConfiguration::project() const
{
    return d->project;
}

LaunchConfigurationType* LaunchConfiguration::type() const
{
    return d->type;
}

void LaunchConfiguration::setName(const QString& name)
{
    d->baseGroup.writeEntry(LaunchConfigurationNameEntry(), name);
    d->baseGroup.sync();
    emit nameChanged( this );
}

void LaunchConfiguration::setType(const QString& typeId)
{
    LaunchConfigurationType* t = Core::self()->runControllerInternal()->launchConfigurationTypeForId( typeId );
    // If this ever happens something seriously screwed in the launch config dialog, as that is 
    // the only place from where this method should be called
    Q_ASSERT(t);
    if( t )
    {
        d->baseGroup.deleteGroup("Data");
        d->type = t;
        d->baseGroup.writeEntry(LaunchConfigurationTypeEntry(), d->type->id());
        d->baseGroup.sync();
        emit typeChanged( t );
    }
}

void LaunchConfiguration::save()
{
    d->baseGroup.sync();
}

QString LaunchConfiguration::configGroupName() const
{
    return d->baseGroup.name();
}

QString LaunchConfiguration::launcherForMode(const QString& mode) const
{
    QStringList modes = d->baseGroup.readEntry("Configured Launch Modes", QStringList());
    int idx = modes.indexOf( mode );
    if( idx != -1 )
    {
        QStringList launchers = d->baseGroup.readEntry("Configured Launchers", QStringList());
        if( launchers.count() > idx )
        {
            foreach( ILauncher* l, type()->launchers() )
            {
                if( l->id() == launchers.at( idx ) )
                {
                    return launchers.at( idx );
                }
            }
        }
    }

    // No launcher configured, if it's debug mode, prefer GDB if available.
    if( mode == QLatin1String("debug") )
    {
        foreach( ILauncher* l, type()->launchers() )
        {
            if( l->supportedModes().contains( mode ) && l->id() == QLatin1String("gdb") )
            {
                return l->id();
            }
        }
    }
    // Otherwise, lets just try with the first one in the list and hope it works
    foreach( ILauncher* l, type()->launchers() )
    {
        if( l->supportedModes().contains( mode ) )
        {
            return l->id();
        }
    }

    return QString();
}

void LaunchConfiguration::setLauncherForMode(const QString& mode, const QString& id)
{
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

