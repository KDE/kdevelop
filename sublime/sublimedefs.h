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

#include <QtCore/Qt>
#include <QtCore/QFlags>

#include "sublimeexport.h"

/**
@file sublimedefs.h
@brief Sublime UI enums and other common definitions
*/

namespace Sublime {

    /**Positions for toolviews in Sublime UI. To combine them, use
    Positions class (QFlags based).*/
    enum Position
    {
        Left = 1   /**< left toolview position */,
        Right = 2  /**< right toolview position */,
        Top = 4    /**< top toolview position */,
        Bottom = 8 /**< bottom toolview position */,
        AllPositions = Left|Right|Top|Bottom /**< all toolview positions */
    };
    Q_DECLARE_FLAGS(Positions, Position)

    KDEVPLATFORMSUBLIME_EXPORT Position dockAreaToPosition(Qt::DockWidgetArea dockArea);

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Sublime::Positions)

#endif

