/************************************************************************
*                                                                      *
* This file is part of KDevelop                                        *
*                                                                      *
* Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
* Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>         *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 or version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program; if not, see <http://www.gnu.org/licenses/>. *
************************************************************************/
#include "settingsconverter.h"

#include <KConfig>
#include <KConfigGroup>
#include <QString>
#include <QDataStream>

#include "settingsmanager.h"

namespace ConfigConstants
{
const QString definesKey("Defines");
const QString includesKey("Includes");
const QString projectPathPrefix("ProjectPath");
const QString projectPathKey("Path");
const QString customBuildSystemGroup("CustomBuildSystem");
}

QList< ConfigEntry > SettingsConverter::readSettings(KConfig* cfg) const
{
    KConfigGroup grp = cfg->group(ConfigConstants::customBuildSystemGroup);
    if (!grp.isValid())
        return {};

    QList<ConfigEntry>  paths;
    foreach (const QString& grpName, grp.groupList()) {
        KConfigGroup subgrp = grp.group(grpName);
        if (!subgrp.isValid())
            continue;

        foreach (const QString& subgrpName, subgrp.groupList()) {
            if (subgrpName.startsWith(ConfigConstants::projectPathPrefix)) {
                KConfigGroup pathgrp = subgrp.group(subgrpName);
                ConfigEntry path;
                path.path = pathgrp.readEntry(ConfigConstants::projectPathKey, "");
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
                pathgrp.deleteGroup();
            }
        }
    }
    if (!paths.isEmpty() && manager) {
        manager->writeSettings(cfg, paths);
    }
    return paths;
}
