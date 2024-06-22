/*
    SPDX-FileCopyrightText: 2009 Andrea s Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

    for (ILauncher* l : std::as_const(d->starters)) {
        if( l->id() == id ) {
           return l;
        }
    }
    return nullptr;
}

}

#include "moc_launchconfigurationtype.cpp"
