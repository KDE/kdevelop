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
    kDebug(9508) << "Reading environment variables";
    foreach( QString setting, cfg.readEntry( "Environment Variables", QStringList() ) )
    {
        QString profile_and_var = setting.section( '=', 0, 0 );
        QString profile = profile_and_var.section( '%', 0, 0 );
        QString var = profile_and_var.section( '%', 1, 1 );
        QString value = setting.section( '=', 1 );
        value = value.mid( 1, value.length() - 2 );
        d->m_groups[ profile ].insert( var, value );
    }
    d->m_defaultGroup = cfg.readEntry( "Default Environment Group", QString("default") );
    kDebug(9508) << "default group" << d->m_defaultGroup;
    kDebug(9508) << "groups:" << d->m_groups;

    // If the defaultgroup doesn't exist yet create it
    if( !d->m_groups.contains( d->m_defaultGroup ) )
    {
        d->m_groups.insert( d->m_defaultGroup, QMap<QString,QString>() );
    }
}

void encode( KConfigGroup cfg, EnvironmentGroupListPrivate* d )
{
    cfg.writeEntry( "Default Environment Group", d->m_defaultGroup );
    QStringList encodedsettings;
    foreach( QString group, d->m_groups.keys() )
    {
        foreach( QString var, d->m_groups[group].keys() )
        {
            encodedsettings << group+'%'+var+"=\""+d->m_groups[group][var]+'"';
        }
    }
    cfg.writeEntry( "Environment Variables", encodedsettings );
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
    KConfigGroup cfg(config, "EnvironmentSettings" );
    encode( cfg, d );
}

void EnvironmentGroupList::loadSettings( KConfig* config )
{
    d->m_groups.clear();
    KConfigGroup cfg(config, "EnvironmentSettings" );
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

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
