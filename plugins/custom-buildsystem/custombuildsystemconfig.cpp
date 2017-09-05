/*
 * Custom Buildsystem Support
 *
 * Copyright 2015 Nicolai Haehnle <nhaehnle@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "custombuildsystemconfig.h"

#include <KLocalizedString>

QString CustomBuildSystemTool::toolName(CustomBuildSystemTool::ActionType type)
{
    switch (type) {
    case Build: return i18nc("Custom build tool", "build");
    case Configure: return i18nc("Custom build tool", "configure");
    case Install: return i18nc("Custom build tool", "install");
    case Clean: return i18nc("Custom build tool", "clean");
    case Prune: return i18nc("Custom build tool", "prune");
    default: return i18n("unknown");
    }
}
