/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sublimedefs.h"

namespace Sublime {

Position dockAreaToPosition(Qt::DockWidgetArea dockArea)
{
    switch (dockArea) {
        case Qt::LeftDockWidgetArea: return Sublime::Left;
        case Qt::RightDockWidgetArea: return Sublime::Right;
        case Qt::TopDockWidgetArea: return Sublime::Top;
        case Qt::BottomDockWidgetArea: return Sublime::Bottom;
        default: return Sublime::Left;
    }
    return Sublime::Left;
}

}

