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

#include <QMap>
#include <QStringList>
#include <QString>
#include <QDebug>

#include <KConfigGroup>
#include <QProcess>

#include <QRegularExpression>
#include <QProcessEnvironment>

namespace KDevelop {
class EnvironmentGroupListPrivate
{
public:
    QMap<QString, QMap<QString,QString> > m_groups;
    QString m_defaultGroup;
};
}

using namespace KDevelop;

namespace {

namespace Strings {
inline QString defaultEnvGroupKey() { return QStringLiteral("Default Environment Group"); }
inline QString envGroup() { return QStringLiteral("Environment Settings"); }
inline QString groupListKey() { return QStringLiteral("Group List"); }
inline QString defaultGroup() { return QStringLiteral("default"); }
}

void decode( KConfig* config, EnvironmentGroupListPrivate* d )
{
    KConfigGroup cfg( config, Strings::envGroup() );
    d->m_defaultGroup = cfg.readEntry( Strings::defaultEnvGroupKey(), Strings::defaultGroup() );
    QStringList grouplist = cfg.readEntry( Strings::groupListKey(), QStringList{Strings::defaultGroup()} );
    foreach( const QString &envgrpname, grouplist )
    {
        KConfigGroup envgrp( &cfg, envgrpname );
        QMap<QString,QString> variables;
        foreach( const QString &varname, envgrp.keyList() )
        {
            variables[varname] = envgrp.readEntry( varname, QString() );
        }
        d->m_groups.insert( envgrpname, variables );
    }
}

void encode( KConfig* config, EnvironmentGroupListPrivate* d )
{
    KConfigGroup cfg( config, Strings::envGroup() );
    cfg.writeEntry( Strings::defaultEnvGroupKey(), d->m_defaultGroup );
    cfg.writeEntry( Strings::groupListKey(), d->m_groups.keys() );
    foreach( const QString &group, cfg.groupList() )
    {
        if( !d->m_groups.contains( group ) )
        {
            cfg.deleteGroup( group );
        }
    }
    for(auto it = d->m_groups.cbegin(), itEnd = d->m_groups.cend(); it!=itEnd; ++it)
    {
        KConfigGroup envgrp( &cfg, it.key() );
        envgrp.deleteGroup();

        const auto val = it.value();
        for(auto it2 = val.cbegin(), it2End = val.cend(); it2!=it2End; ++it2)
        {
            envgrp.writeEntry( it2.key(), *it2 );
        }
    }
    cfg.sync();
}

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
    decode( config.data(), d );
}

EnvironmentGroupList::EnvironmentGroupList( KConfig* config )
    : d( new EnvironmentGroupListPrivate )
{
    decode( config, d );
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
    encode( config, d );
    config->sync();
}

void EnvironmentGroupList::loadSettings( KConfig* config )
{
    d->m_groups.clear();
    decode( config, d );
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

void KDevelop::expandVariables(QMap<QString, QString>& variables, const QProcessEnvironment& environment)
{
    QRegularExpression rVar(QStringLiteral("(?<!\\\\)(\\$\\w+)"));
    QRegularExpression rNotVar(QStringLiteral("\\\\\\$"));
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        QRegularExpressionMatch m;
        while ((m=rVar.match(it.value())).hasMatch()) {
            if (environment.contains(m.captured(1).midRef(1).toString())) {
                it.value().replace(m.capturedStart(0), m.capturedLength(0), environment.value(m.captured(0).midRef(1).toString()));
            } else {
                //TODO: an warning
                it.value().replace(m.capturedStart(0), m.capturedLength(0), QString());
            }
        }
        it.value().replace(rNotVar, QStringLiteral("$"));
    }
}

void KDevelop::restoreSystemEnvironment(QProcess* process)
{
    static const auto shouldChange = qEnvironmentVariableIsSet("_KDEV_OLD_LD_LIBRARY_PATH");
    if ( shouldChange ) {
        // running in AppImage
        auto env = process->processEnvironment();
        env.insert("LD_LIBRARY_PATH", qgetenv("_KDEV_OLD_LD_LIBRARY_PATH"));
        env.insert("QT_PLUGIN_PATH", qgetenv("_KDEV_OLD_QT_PLUGIN_PATH"));
        env.insert("XDG_DATA_DIRS", qgetenv("_KDEV_OLD_XDG_DATA_DIRS"));
        env.insert("PATH", qgetenv("_KDEV_OLD_PATH"));
        env.insert("KDE_FORK_SLAVES", qgetenv("_KDEV_OLD_KDE_FORK_SLAVES"));
        process->setProcessEnvironment(env);
    }
}
