/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filtereditem.h"

namespace KDevelop
{

FilteredItem::FilteredItem( const QString& line )
: originalLine( line )
{
}

FilteredItem::FilteredItem( const QString& line, FilteredOutputItemType type )
: originalLine( line )
, type( type )
, isActivatable(false)
, lineNo(-1)
, columnNo(-1)
{
}


} // namespace KDevelop

