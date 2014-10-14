/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
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

#include "filtereditem.h"

namespace KDevelop
{

FilteredItem::FilteredItem( const QString& line )
: originalLine( line )
, type( InvalidItem )
, shortenedText( line )
, isActivatable(false)
, lineNo(-1)
, columnNo(-1)
{
}

FilteredItem::FilteredItem( const QString& line, FilteredOutputItemType type )
: originalLine( line )
, type( type )
, shortenedText( line )
, isActivatable(false)
, lineNo(-1)
, columnNo(-1)
{
}


} // namespace KDevelop

