/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef SUBLIMESUBLIMEDEFS_H
#define SUBLIMESUBLIMEDEFS_H

#include <QFlags>

/**
@file sublimedefs.h
@brief Sublime UI enums and other common definitions
*/

namespace Sublime {

    /**Positions for toolviews in Sublime UI. To combine them, use
    Positions class (QFlags based).*/
    enum Position { Left = 1, Right = 2, Top = 4, Bottom = 8, AllPositions = Left|Right|Top|Bottom };
    Q_DECLARE_FLAGS(Positions, Position)

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Sublime::Positions)

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
