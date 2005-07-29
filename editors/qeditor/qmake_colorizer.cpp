/*
 *  This file is part of Klint
 *  Copyright (C) 2001 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */


#include "qmake_colorizer.h"

#include <qapplication.h>
#include <private/qrichtext_p.h>


static const char *qmake_keywords[] = {
  // QMake keywords
  0
};

QMakeColorizer::QMakeColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new RegExpHLItem( "#.*", Comment, 0 ) );
    context0->appendChild( new RegExpHLItem( "[A-Z]+", Keyword, 0 ) );

    m_items.append( context0 );
}

QMakeColorizer::~QMakeColorizer()
{
}
