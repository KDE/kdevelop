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

QString LaunchConfiguration::LaunchConfigurationNameEntry = "Name";
QString LaunchConfiguration::LaunchConfigurationTypeEntry = "Type";

LaunchConfiguration::LaunchConfiguration(KConfigGroup grp, IProject* project, QObject* parent ) 
    : QObject( parent ), ILaunchConfiguration(), baseGroup( grp ), m_project( project )
{
    m_type = Core::self()->runControllerInternal()->launchConfigurationTypeForId( grp.readEntry(LaunchConfigurationTypeEntry, "") );
}

LaunchConfiguration::~LaunchConfiguration()
{
}

KConfigGroup LaunchConfiguration::config()
{
    return baseGroup.group( "Data" );
}

const KConfigGroup LaunchConfiguration::config() const
{
    return baseGroup.group( "Data" );
}

QString LaunchConfiguration::name() const
{
    return baseGroup.readEntry( LaunchConfigurationNameEntry, "" );
}

IProject* LaunchConfiguration::project() const
{
    return m_project;
}

LaunchConfigurationType* LaunchConfiguration::type() const
{
    return m_type;
}

void LaunchConfiguration::setName(const QString& name)
{
    baseGroup.writeEntry( LaunchConfigurationNameEntry, name );
    baseGroup.sync();
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
        baseGroup.deleteGroup("Data");
        m_type = t;
        baseGroup.writeEntry( LaunchConfigurationTypeEntry, m_type->id() );
        baseGroup.sync();
        emit typeChanged( t );
    }
}

void LaunchConfiguration::save()
{
    baseGroup.sync();
}

QString LaunchConfiguration::configGroupName() const
{
    return baseGroup.name();
}

QString LaunchConfiguration::launcherForMode(const QString& mode) const
{
    QStringList modes = baseGroup.readEntry( "Configured Launch Modes", QStringList() );
    int idx = modes.indexOf( mode );
    if( idx != -1 )
    {
        QStringList launchers = baseGroup.readEntry( "Configured Launchers", QStringList() );
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

    //No launcher configured, lets just try with the first one in the list and hope it works
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
    QStringList modes = baseGroup.readEntry( "Configured Launch Modes", QStringList() );
    int idx = modes.indexOf( mode );
    if( idx == -1 )
    {
        idx = modes.count();
        modes << mode;
        baseGroup.writeEntry( "Configured Launch Modes", modes );
    }
    QStringList launchers = baseGroup.readEntry( "Configured Launchers", QStringList() );
    if( launchers.count() > idx )
    {
        launchers.replace(idx, id);
    } else
    {
        launchers.append( id );
    }
    baseGroup.writeEntry( "Configured Launchers", launchers );
}


}

