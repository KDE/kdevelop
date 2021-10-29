/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef CUSTOMBUILDSYSTEMCONFIG_H
#define CUSTOMBUILDSYSTEMCONFIG_H

#include <QVector>
#include <QUrl>
#include <QMetaType>

struct CustomBuildSystemTool
{
    enum ActionType { Build = 0, Configure, Install, Clean, Prune, Undefined };
    static QString toolName(ActionType type);

    CustomBuildSystemTool() {}
    bool enabled = false;
    QUrl executable;
    QString arguments;
    QString envGrp;
    ActionType type = Undefined;
};

Q_DECLARE_METATYPE( CustomBuildSystemTool )
Q_DECLARE_TYPEINFO(CustomBuildSystemTool, Q_MOVABLE_TYPE);

struct CustomBuildSystemConfig
{
    QString title;
    QUrl buildDir;
    QVector<CustomBuildSystemTool> tools;

    CustomBuildSystemConfig()
    {
        tools.reserve(CustomBuildSystemTool::Undefined - CustomBuildSystemTool::Build);
        for (int i = CustomBuildSystemTool::Build; i < CustomBuildSystemTool::Undefined; ++i) {
            CustomBuildSystemTool tool;
            tool.type = CustomBuildSystemTool::ActionType(i);
            tool.enabled = false;
            tools << tool;
        }
    }
};

Q_DECLARE_TYPEINFO(CustomBuildSystemConfig, Q_MOVABLE_TYPE);

#endif
