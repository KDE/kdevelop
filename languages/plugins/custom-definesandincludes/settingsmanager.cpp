/*
 * This file is part of KDevelop
 *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#include <KConfig>

#include <interfaces/iproject.h>

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

/// Reads and converts paths from old (Custom Build System's) format to the current one.
void addConvertedPaths(const SettingsManager* sm, KConfig* cfg, QList<ConfigEntry>& paths)
{
    SettingsConverter sc{sm};
    auto convertedPaths = sc.readSettings(cfg);

    bool contains = false;
    for (auto cPath : convertedPaths) {
        for (auto path : paths) {
            if (path.path == cPath.path) {
                path.includes += cPath.includes;
                path.includes.removeDuplicates();

                for (auto it = cPath.defines.constBegin(); it != cPath.defines.constEnd(); it++) {
                    if (!path.defines.contains(it.key())) {
                        path.defines[it.key()] = it.value();
                    }
                }
                contains = true;
            }
        }
        if (!contains) {
            paths << cPath;
        }
        contains = false;
    }
}

void SettingsManager::writeSettings( KConfig* cfg, const QList<ConfigEntry>& paths ) const
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
    addConvertedPaths(this, cfg, paths);

    return paths;
}
