/*
    SPDX-FileCopyrightText: 2015 Nicolai Haehnle <nhaehnle@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "custombuildsystemconfig.h"

#include <KLocalizedString>

QString CustomBuildSystemTool::toolName(CustomBuildSystemTool::ActionType type)
{
    switch (type) {
        case Build:     return i18nc("@item:intext custom 'build' tool", "build");
        case Configure: return i18nc("@item:intext custom 'configure' tool", "configure");
        case Install:   return i18nc("@item:intext custom 'install' tool", "install");
        case Clean:     return i18nc("@item:intext custom 'clean' tool", "clean");
        case Prune:     return i18nc("@item:intext custom 'prune' tool", "prune");
        default:        return i18nc("@item:intext custom 'unknown' tool", "unknown");
    }
}
