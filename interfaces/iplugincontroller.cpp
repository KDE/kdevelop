/* This file is part of the KDE project
Copyright 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright     2006 Matt Rogers <mattr@kde.org
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

Based on code from Kopete
Copyright 2002-2003 Martijn Klingens <klingens@kde.org>

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
#include "iplugincontroller.h"

#include <kservicetypetrader.h>
#include "ipluginversion.h"

namespace KDevelop
{

IPluginController::IPluginController( QObject* parent )
: QObject( parent )
{
}


IPluginController::~IPluginController()
{
}

KPluginInfo::List IPluginController::query( const QString &serviceType,
        const QString &constraint )
{
    KService::List serviceList = KServiceTypeTrader::self() ->query( serviceType,
            QString( "%1 and [X-KDevelop-Version] == %2" ).arg( constraint ).arg( KDEVELOP_PLUGIN_VERSION ) );

    return KPluginInfo::fromServices( serviceList );
}

KPluginInfo::List IPluginController::queryPlugins( const QString &constraint )
{
    return query( "KDevelop/Plugin", constraint );
}

QStringList IPluginController::argumentsFromService( const KService::Ptr &service )
{
    QStringList args;
    if ( !service )
        // service is a reference to a pointer, so a check whether it is 0 is still required
        return args;
    QVariant prop = service->property( "X-KDevelop-Args" );
    if ( prop.isValid() )
        args = prop.toString().split( ' ' );
    return args;
}

KPluginInfo::List IPluginController::queryExtensionPlugins(const QString &extension, const QStringList &constraints)
{
    QStringList c = constraints;
    c << QString("'%1' in [X-KDevelop-Interfaces]").arg( extension );
    return queryPlugins( c.join(" and ") );
}



}

#include "iplugincontroller.moc"
