/*  This file is part of KDevelop
    Copyright 2009 Andrea   s Pakulat <apaku@gmx.de>

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

#include "launchconfigurationtype.h"

#include "ilauncher.h"

namespace KDevelop
{

class LaunchConfigurationTypePrivate
{
public:
    QList<ILauncher*> starters;
};

LaunchConfigurationType::LaunchConfigurationType()
    : d_ptr(new LaunchConfigurationTypePrivate)
{
}

LaunchConfigurationType::~LaunchConfigurationType()
{
    Q_D(LaunchConfigurationType);

    qDeleteAll(d->starters);
}


void LaunchConfigurationType::addLauncher( ILauncher* starter )
{
    Q_D(LaunchConfigurationType);

    if( !d->starters.contains( starter ) )
    {
        d->starters.append( starter );
    }
}
void LaunchConfigurationType::removeLauncher( ILauncher* starter )
{
    Q_D(LaunchConfigurationType);

    d->starters.removeAll( starter );
}

QList<ILauncher*> LaunchConfigurationType::launchers() const
{
    Q_D(const LaunchConfigurationType);

    return d->starters;
}

ILauncher* LaunchConfigurationType::launcherForId(const QString& id) const
{
    Q_D(const LaunchConfigurationType);

    for (ILauncher* l : qAsConst(d->starters)) {
        if( l->id() == id ) {
           return l;
        }
    }
    return nullptr;
}

}

