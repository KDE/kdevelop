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

static const QString defaultEnvGroupKey = "Default Environment Group";
static const QString envGroup = "Environment Settings";
static const QString groupListKey = "Group List";

void decode( KConfigGroup cfg, EnvironmentGroupListPrivate* d )
{
    d->m_defaultGroup = cfg.readEntry( defaultEnvGroupKey, QString( "default" ) );
    QStringList grouplist = cfg.readEntry( groupListKey, QStringList() << "default" );
    foreach( const QString &envgrpname, grouplist )
    {
        KConfigGroup envgrp( &cfg, envgrpname );
        QMap<QString,QString> variables;
        foreach( const QString &varname, envgrp.keyList() )
        {
            variables[varname] = envgrp.readEntry( varname, QString("") );
        }
        d->m_groups.insert( envgrpname, variables );
    }
}

void encode( KConfigGroup cfg, EnvironmentGroupListPrivate* d )
{
    cfg.writeEntry( defaultEnvGroupKey, d->m_defaultGroup );
    cfg.writeEntry( groupListKey, d->m_groups.keys() );
    foreach( const QString &group, cfg.groupList() ) 
    {
        if( !d->m_groups.keys().contains( group ) ) 
        {
            cfg.deleteGroup( group );
        }
    }
    foreach( const QString &group, d->m_groups.keys() )
    {
        KConfigGroup envgrp( &cfg, group );
        envgrp.deleteGroup();
        foreach( const QString &var, d->m_groups[group].keys() )
        {
            envgrp.writeEntry( var, d->m_groups[group][var] );
        }
    }
    cfg.sync();
}

EnvironmentGroupList::EnvironmentGroupList( const EnvironmentGroupList& rhs )
    : d( new EnvironmentGroupListPrivate( *rhs.d ) )
{
}

EnvironmentGroupList& EnvironmentGroupList::operator=( const EnvironmentGroupList& rhs )
{
    *d = *rhs.d;
    return *this;
}

EnvironmentGroupList::EnvironmentGroupList( KSharedConfigPtr config )
    : d( new EnvironmentGroupListPrivate )
{
    KConfigGroup cfg( config, envGroup );
    decode( cfg, d );
}

EnvironmentGroupList::EnvironmentGroupList( KConfig* config )
    : d( new EnvironmentGroupListPrivate )
{
    KConfigGroup cfg( config, envGroup );
    decode( cfg, d );
}

EnvironmentGroupList::~EnvironmentGroupList()
{
    delete d;
}

const QMap<QString, QString> EnvironmentGroupList::variables( const QString& group ) const
{
    return d->m_groups[group.isEmpty() ? d->m_defaultGroup : group];
}

QMap<QString, QString>& EnvironmentGroupList::variables( const QString& group )
{
    return d->m_groups[group.isEmpty() ? d->m_defaultGroup : group];
}


QString EnvironmentGroupList::defaultGroup() const
{
    return d->m_defaultGroup;
}

void EnvironmentGroupList::setDefaultGroup( const QString& group )
{
    if( group.isEmpty() ) {
        return;
    }

    if( d->m_groups.contains( group ) )
    {
        d->m_defaultGroup = group;
    }
}

void EnvironmentGroupList::saveSettings( KConfig* config ) const
{
    KConfigGroup cfg(config, envGroup );
    encode( cfg, d );
    config->sync();
}

void EnvironmentGroupList::loadSettings( KConfig* config )
{
    d->m_groups.clear();
    KConfigGroup cfg(config, envGroup );
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
    : d( new EnvironmentGroupListPrivate )
{
}

QStringList EnvironmentGroupList::createEnvironment( const QString & group, const QStringList & defaultEnvironment ) const
{
    QMap<QString, QString> retMap;
    foreach( const QString &line, defaultEnvironment )
    {
        QString varName = line.section( '=', 0, 0 );
        QString varValue = line.section( '=', 1 );
        retMap.insert( varName, varValue );
    }

    if( !group.isEmpty() ) {
      QMap<QString, QString> userMap = variables(group);

      for( QMap<QString, QString>::const_iterator it = userMap.constBegin();
          it != userMap.constEnd(); ++it )
      {
          retMap.insert( it.key(), it.value() );
      }
    }

    QStringList env;
    for( QMap<QString, QString>::const_iterator it = retMap.constBegin();
        it != retMap.constEnd(); ++it )
    {
        env << it.key() + '=' + it.value();
    }

    return env;
}

}
