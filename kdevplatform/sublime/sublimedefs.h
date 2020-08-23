/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_SUBLIMESUBLIMEDEFS_H
#define KDEVPLATFORM_SUBLIMESUBLIMEDEFS_H

#include <Qt>
#include <QFlags>

#include "sublimeexport.h"

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
