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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */


#include "xml_colorizer.h"

#include <qfont.h>
#include <qapplication.h>
#include <qsettings.h>
#include <private/qrichtext_p.h>

static const char *xml_keywords[] = {
  // XML keywords
  0
};

XMLColorizer::XMLColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new RegExpHLItem( "<!--", Comment, 1 ) );
    context0->appendChild( new RegExpHLItem( "<", Normal, 2 ) );
    context0->appendChild( new RegExpHLItem( "&[\\w|_|!]+;", Constant, 0 ) );

    // comment context
    HLItemCollection* context1 = new HLItemCollection( Comment );
    context1->appendChild( new StringHLItem( "-->", Comment, 0 ) );

    HLItemCollection* context2 = new HLItemCollection( Normal );
    context2->appendChild( new StringHLItem( "\"", String, 3 ) );
    context2->appendChild( new RegExpHLItem( "[\\w|_]+", Keyword, 2 ) );
    context2->appendChild( new StringHLItem( ">", Normal, 0 ) );

    HLItemCollection* context3 = new HLItemCollection( String );
    context3->appendChild( new StringHLItem( "\\\"", String, 3 ) );
    context3->appendChild( new StringHLItem( "\"", String, 2 ) );

    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
    m_items.append( context3 );
}

XMLColorizer::~XMLColorizer()
{
}
