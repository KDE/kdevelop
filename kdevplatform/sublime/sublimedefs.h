/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMESUBLIMEDEFS_H
#define KDEVPLATFORM_SUBLIMESUBLIMEDEFS_H

#include "sublimeexport.h"

#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <QFlags>
#include <Qt>

/**
@file sublimedefs.h
@brief Sublime UI enums and other common definitions
*/

namespace Sublime {

    /**Positions for tool views in Sublime UI. To combine them, use
    Positions class (QFlags based).*/
    enum Position
    {
        Left = 1   /**< left tool view position */,
        Right = 2  /**< right tool view position */,
        Top = 4    /**< top tool view position */,
        Bottom = 8 /**< bottom tool view position */,
        AllPositions = Left|Right|Top|Bottom /**< all tool view positions */
    };
    Q_DECLARE_FLAGS(Positions, Position)
    Q_DECLARE_OPERATORS_FOR_FLAGS(Positions)

    KDEVPLATFORMSUBLIME_EXPORT Position dockAreaToPosition(Qt::DockWidgetArea dockArea);

}

#endif
