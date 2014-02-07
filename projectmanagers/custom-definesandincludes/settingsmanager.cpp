/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "settingsmanager.h"
#include "settingsconverter.h"

#include <interfaces/iproject.h>

#include <KConfig>
#include <QStringList>

namespace ConfigConstants
{
const QString configKey( "CustomDefinesAndIncludes" );
const QString definesKey( "Defines" );
const QString includesKey( "Includes" );
const QString projectPathPrefix( "ProjectPath" );
const QString projectPathKey( "Path" );
}

SettingsManager::SettingsManager()
{
}

void SettingsManager::writeSettings( KConfig* cfg, const QList<ConfigEntry>& paths )
{
       KConfigGroup grp = cfg->group( ConfigConstants::configKey );
    if ( !grp.isValid() )
        return;

    grp.deleteGroup();

    int pathIndex = 0;
    for ( const auto& path : paths ) {
        KConfigGroup pathgrp = grp.group( ConfigConstants::projectPathPrefix + QString::number( pathIndex++ ) );
        pathgrp.writeEntry( ConfigConstants::projectPathKey, path.path );
        {
            QByteArray tmp;
            QDataStream s(&tmp, QIODevice::WriteOnly);
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.includes;
            pathgrp.writeEntry( ConfigConstants::includesKey, tmp );
        }
        {
            QByteArray tmp;
            QDataStream s(&tmp, QIODevice::WriteOnly);
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.defines;
            pathgrp.writeEntry( ConfigConstants::definesKey, tmp );
        }
    }
}

QList<ConfigEntry> SettingsManager::readSettings( KConfig* cfg ) const
{
    KConfigGroup grp = cfg->group( ConfigConstants::configKey );
    if ( !grp.isValid() )
        return {};

    QList<ConfigEntry> paths;
    for( const QString &grpName: grp.groupList() ) {
        if ( grpName.startsWith( ConfigConstants::projectPathPrefix ) ) {
            KConfigGroup pathgrp = grp.group( grpName );

            ConfigEntry path;
            path.path = pathgrp.readEntry( ConfigConstants::projectPathKey, "" );

            {
                QByteArray tmp = pathgrp.readEntry(ConfigConstants::definesKey, QByteArray());
                QDataStream s(tmp);
                s.setVersion(QDataStream::Qt_4_5);
                s >> path.defines;
            }

            {
                QByteArray tmp = pathgrp.readEntry(ConfigConstants::includesKey, QByteArray());
                QDataStream s(tmp);
                s.setVersion(QDataStream::Qt_4_5);
                s >> path.includes;
            }

            paths << path;
        }
    }
    static SettingsConverter sc;
    auto convertedPaths = sc.readSettings(cfg);

    bool contains = false;
    for (auto cPath : convertedPaths) {
        for (auto path : paths) {
            if (path.path == cPath.path) {
                path.includes += cPath.includes;
                path.includes.removeDuplicates();

                path.defines.unite(cPath.defines);
                contains = true;
            }
        }
        if (!contains) {
            paths << cPath;
        }
        contains = false;
    }

    return paths;
}

SettingsManager* SettingsManager::self()
{
    static SettingsManager sm;
    return &sm;
}
