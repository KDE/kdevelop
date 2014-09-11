/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
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

#ifndef CUSTOMBUILDSYSTEMCONFIG_H
#define CUSTOMBUILDSYSTEMCONFIG_H

#include <QMap>
#include <QVector>
#include <QList>
#include <QStringList>
#include <QUrl>

struct CustomBuildSystemTool
{
    enum ActionType { Build = 0, Configure, Install, Clean, Prune, Undefined };
    CustomBuildSystemTool() : enabled( false ), type( Undefined ) {}
    bool enabled;
    QUrl executable;
    QString arguments;
    QString envGrp;
    ActionType type;
};

Q_DECLARE_METATYPE( CustomBuildSystemTool )

struct CustomBuildSystemConfig
{
    QString title;
    QUrl buildDir;
    QVector<CustomBuildSystemTool> tools;

    CustomBuildSystemConfig()
    {
        for (int i = CustomBuildSystemTool::Build; i < CustomBuildSystemTool::Undefined; ++i) {
            CustomBuildSystemTool tool;
            tool.type = CustomBuildSystemTool::ActionType(i);
            tool.enabled = false;
            tools << tool;
        }
    }
};

#endif
