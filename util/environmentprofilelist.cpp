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

#include "environmentprofilelist.h"

#include <QMap>
#include <QStringList>
#include <QString>

#include <KConfigGroup>

#include <QRegularExpression>
#include <QProcessEnvironment>

namespace KDevelop {
class EnvironmentProfileListPrivate
{
public:
    QMap<QString, QMap<QString,QString> > m_profiles;
    QString m_defaultProfileName;
};
}

using namespace KDevelop;

namespace {

namespace Strings {
// TODO: migrate to more consistent key term "Default Environment Profile"
inline QString defaultEnvProfileKey() { return QStringLiteral("Default Environment Group"); }
inline QString envGroup() { return QStringLiteral("Environment Settings"); }
// TODO: migrate to more consistent key term "Profile List"
inline QString profileListKey() { return QStringLiteral("Group List"); }
inline QString defaultProfileName() { return QStringLiteral("default"); }
}

void decode(KConfig* config, EnvironmentProfileListPrivate* d)
{
    KConfigGroup cfg( config, Strings::envGroup() );
    d->m_defaultProfileName = cfg.readEntry(Strings::defaultEnvProfileKey(), Strings::defaultProfileName());
    const QStringList profileNames = cfg.readEntry(Strings::profileListKey(), QStringList{Strings::defaultProfileName()});
    for (const auto& profileName : profileNames) {
        KConfigGroup envgrp(&cfg, profileName);
        QMap<QString,QString> variables;
        foreach( const QString &varname, envgrp.keyList() )
        {
            variables[varname] = envgrp.readEntry( varname, QString() );
        }
        d->m_profiles.insert(profileName, variables);
    }
}

void encode(KConfig* config, EnvironmentProfileListPrivate* d)
{
    KConfigGroup cfg( config, Strings::envGroup() );
    cfg.writeEntry(Strings::defaultEnvProfileKey(), d->m_defaultProfileName);
    cfg.writeEntry(Strings::profileListKey(), d->m_profiles.keys());
    foreach( const QString &group, cfg.groupList() )
    {
        if (!d->m_profiles.contains(group)) {
            cfg.deleteGroup( group );
        }
    }
    for (auto it = d->m_profiles.cbegin(), itEnd = d->m_profiles.cend(); it!=itEnd; ++it) {
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

EnvironmentProfileList::EnvironmentProfileList(const EnvironmentProfileList& rhs)
    : d(new EnvironmentProfileListPrivate(*rhs.d))
{
}

EnvironmentProfileList& EnvironmentProfileList::operator=(const EnvironmentProfileList& rhs)
{
    *d = *rhs.d;
    return *this;
}

EnvironmentProfileList::EnvironmentProfileList(KSharedConfigPtr config)
    : d( new EnvironmentProfileListPrivate )
{
    decode( config.data(), d );
}

EnvironmentProfileList::EnvironmentProfileList(KConfig* config)
    : d(new EnvironmentProfileListPrivate)
{
    decode( config, d );
}

EnvironmentProfileList::~EnvironmentProfileList()
{
    delete d;
}

QMap<QString, QString> EnvironmentProfileList::variables(const QString& profileName) const
{
    return d->m_profiles[profileName.isEmpty() ? d->m_defaultProfileName : profileName];
}

QMap<QString, QString>& EnvironmentProfileList::variables(const QString& profileName)
{
    return d->m_profiles[profileName.isEmpty() ? d->m_defaultProfileName : profileName];
}


QString EnvironmentProfileList::defaultProfileName() const
{
    return d->m_defaultProfileName;
}

void EnvironmentProfileList::setDefaultProfile(const QString& profileName)
{
    if (profileName.isEmpty() ||
        !d->m_profiles.contains(profileName)) {
        return;
    }

    d->m_defaultProfileName = profileName;
}

void EnvironmentProfileList::saveSettings(KConfig* config) const
{
    encode( config, d );
    config->sync();
}

void EnvironmentProfileList::loadSettings(KConfig* config)
{
    d->m_profiles.clear();
    decode( config, d );
}

QStringList EnvironmentProfileList::profileNames() const
{
    return d->m_profiles.keys();
}

void EnvironmentProfileList::removeProfile(const QString& profileName)
{
    d->m_profiles.remove(profileName);
}

EnvironmentProfileList::EnvironmentProfileList()
    : d(new EnvironmentProfileListPrivate)
{
}

QStringList EnvironmentProfileList::createEnvironment(const QString& profileName, const QStringList& defaultEnvironment) const
{
    QMap<QString, QString> retMap;
    foreach( const QString &line, defaultEnvironment )
    {
        QString varName = line.section( '=', 0, 0 );
        QString varValue = line.section( '=', 1 );
        retMap.insert( varName, varValue );
    }

    if (!profileName.isEmpty()) {
      const auto userMap = variables(profileName);

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
