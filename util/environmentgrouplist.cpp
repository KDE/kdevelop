/* This file is part of KDevelop
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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

#include "environmentgrouplist.h"

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QString>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

namespace KDevelop
{

class EnvironmentGroupListPrivate
{
public:
    QMap<QString, QMap<QString,QString> > m_groups;
    QString m_defaultGroup;
};


void decode( KConfigGroup cfg, EnvironmentGroupListPrivate* d )
{
    d->m_defaultGroup = cfg.readEntry( "Default EnvironmentGroup", QString( "default" ) );
    foreach( QString envgrpname, cfg.groupList() )
    {
        KConfigGroup envgrp( &cfg, envgrpname );
        QMap<QString,QString> variables;
        foreach( QString varname, envgrp.keyList() )
        {
            variables[varname] = envgrp.readEntry( varname, QString("") );
        }
        d->m_groups.insert( envgrpname, variables );
    }

    // If the defaultgroup doesn't exist yet create it
    if( !d->m_groups.contains( d->m_defaultGroup ) )
    {
        d->m_groups.insert( d->m_defaultGroup, QMap<QString,QString>() );
    }
}

void encode( KConfigGroup cfg, EnvironmentGroupListPrivate* d )
{
    cfg.writeEntry( "Default Environment Group", d->m_defaultGroup );
    foreach( QString group, d->m_groups.keys() )
    {
        KConfigGroup envgrp( &cfg, group );
        foreach( QString var, d->m_groups[group].keys() )
        {
            envgrp.writeEntry( var, d->m_groups[group][var] );
        }
    }
    cfg.sync();
}

EnvironmentGroupList::EnvironmentGroupList( KSharedConfigPtr config )
    : d(new EnvironmentGroupListPrivate)
{
    KConfigGroup cfg( config, "Environment Settings" );
    decode( cfg, d );
}

EnvironmentGroupList::EnvironmentGroupList( KConfig* config )
    : d(new EnvironmentGroupListPrivate)
{
    KConfigGroup cfg( config, "Environment Settings" );
    decode( cfg, d );
}


EnvironmentGroupList::~EnvironmentGroupList()
{
    delete d;
}

const QMap<QString, QString> EnvironmentGroupList::variables( const QString& group ) const
{
    return d->m_groups[group];
}

QMap<QString, QString>& EnvironmentGroupList::variables( const QString& group )
{
    return d->m_groups[group];
}


QString EnvironmentGroupList::defaultGroup() const
{
    return d->m_defaultGroup;
}

void EnvironmentGroupList::setDefaultGroup( const QString& group )
{
    if( d->m_groups.contains( group ) )
    {
        d->m_defaultGroup = group;
    }
}

void EnvironmentGroupList::saveSettings( KConfig* config ) const
{
    KConfigGroup cfg(config, "Environment Settings" );
    encode( cfg, d );
}

void EnvironmentGroupList::loadSettings( KConfig* config )
{
    d->m_groups.clear();
    KConfigGroup cfg(config, "Environment Settings" );
    decode( cfg, d );
}

QStringList EnvironmentGroupList::groups() const
{
    return d->m_groups.keys();
}

void EnvironmentGroupList::removeGroup( const QString& group )
{
    d->m_groups.remove( group );
}

EnvironmentGroupList::EnvironmentGroupList()
    : d( new EnvironmentGroupListPrivate)
{
}

}

