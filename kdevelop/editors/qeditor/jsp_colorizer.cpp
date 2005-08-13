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


#include "jsp_colorizer.h"
#include "paragdata.h"

#include <qfont.h>
#include <qapplication.h>
#include <qsettings.h>
#include <private/qrichtext_p.h>

static const char *jsp_keywords[] = {
    // Java keywords
    "abstract",
    "break",
    "case",
    "catch",
    "class",
    "continue",
    "default",
    "do",
    "else",
    "extends",
    "false",
    "finally",
    "for",
    "goto",
    "if",
    "implements",
    "import",
    "instanceof",
    "interface",
    "native",
    "new",
    "null",
    "package",
    "private",
    "protected",
    "public",
    "return",
    "super",
    "strictfp",
    "switch",
    "synchronized",
    "this",
    "throws",
    "throw",
    "transient",
    "true",
    "try",
    "volatile",
    "while",
    "boolean",
    "byte",
    "char",
    "const",
    "double",
    "final",
    "float",
    "int",
    "long",
    "short",
    "static",
    "void",
    0
};

JspColorizer::JspColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    
    // xml
    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new StringHLItem( "<!--", Comment, 1 ) );
    context0->appendChild( new StringHLItem( "<%", Comment, 4 ) );
    context0->appendChild( new StringHLItem( "<", Normal, 2 ) );
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
    
    // begin java 
    
    // default context
    HLItemCollection* context4 = new HLItemCollection( Normal );
    context4->appendChild( new StringHLItem( "%>", Comment, 0 ) );
    context4->appendChild( new StartsWithHLItem( "//", Comment, 4 ) );
    context4->appendChild( new StringHLItem( "/*", Comment, 5 ) );
    context4->appendChild( new StringHLItem( "\"", String, 6 ) );
    context4->appendChild( new StringHLItem( "'", String, 7 ) );
    context4->appendChild( new KeywordsHLItem( jsp_keywords, Keyword, Normal, 4 ) );
    context4->appendChild( new NumberHLItem( Constant, 4 ) );

    // comment context
    HLItemCollection* context5 = new HLItemCollection( Comment );
    context5->appendChild( new StringHLItem( "*/", Comment, 4 ) );

    HLItemCollection* context6 = new HLItemCollection( String );
    context6->appendChild( new StringHLItem( "\\\\", String, 6 ) );
    context6->appendChild( new StringHLItem( "\\\"", String, 6 ) );
    context6->appendChild( new StringHLItem( "\"", String, 4 ) );

    HLItemCollection* context7 = new HLItemCollection( String );
    context7->appendChild( new StringHLItem( "\\\\", String, 7 ) );
    context7->appendChild( new StringHLItem( "\\'", String, 7 ) );
    context7->appendChild( new StringHLItem( "'", String, 4 ) );

    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
    m_items.append( context3 );
    m_items.append( context4 );
    m_items.append( context5 );
    m_items.append( context6 );
    m_items.append( context7 );
}

JspColorizer::~JspColorizer()
{
}

