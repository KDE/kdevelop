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


#include "cs_colorizer.h"
#include "qeditor_part.h"
//Added by qt3to4:
#include <Q3ValueList>
#include "paragdata.h"

#include <qfont.h>
#include <private/qrichtext_p.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>


static const char *cs_keywords[] = {
    "abstract",
    "as",
    "base",
    "bool",
    "break",
    "byte",
    "case",
    "catch",
    "char",
    "checked",
    "class",
    "const",
    "continue",
    "decimal",
    "default",
    "delegate",
    "do",
    "double",
    "else",
    "enum",
    "event",
    "explicit",
    "extern",
    "false",
    "finally",
    "fixed",
    "float",
    "for",
    "foreach",
    "goto",
    "if",
    "implicit",
    "in",
    "int",
    "interface",
    "internal",
    "is",
    "lock",
    "long",
    "namespace",
    "new",
    "null",
    "object",
    "operator",
    "out",
    "override",
    "params",
    "private",
    "protected",
    "public",
    "readonly",
    "ref",
    "return",
    "sbyte",
    "sealed",
    "short",
    "sizeof",
    "stackalloc",
    "static",
    "string",
    "struct",
    "switch",
    "this",
    "throw",
    "true",
    "try",
    "typeof",
    "uint",
    "ulong",
    "unchecked"
    "unsafe",
    "ushort",
    "using",
    "virtual",
    "void",
    "volatile",
    "while",
    0
};

using namespace std;

CSharpColorizer::CSharpColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );

    context0->appendChild( new RegExpHLItem( "^\\s*#", PreProcessor, 4 ) );
    context0->appendChild( new WhiteSpacesHLItem( Normal, 0 ) );
    context0->appendChild( new StringHLItem( "'", String, 1 ) );
    context0->appendChild( new StringHLItem( "\"", String, 2 ) );
    context0->appendChild( new StringHLItem( "/*", Comment, 3 ) );
    context0->appendChild( new StartsWithHLItem( "//", Comment, 0 ) );
    context0->appendChild( new KeywordsHLItem( cs_keywords, Keyword, Normal, 0 ) );
    context0->appendChild( new HexHLItem( Constant, 0 ) );
    context0->appendChild( new NumberHLItem( Constant, 0 ) );
    context0->appendChild( new RegExpHLItem("@[_\\w]+", Normal, 0) );

    HLItemCollection* context1 = new HLItemCollection( String );
    context1->appendChild( new StringHLItem( "\\\\", String, 1 ) );
    context1->appendChild( new StringHLItem( "\\'", String, 1 ) );
    context1->appendChild( new StringHLItem( "'", String, 0 ) );

    HLItemCollection* context2 = new HLItemCollection( String );
    context2->appendChild( new StringHLItem( "\\\\", String, 2 ) );
    context2->appendChild( new StringHLItem( "\\\"", String, 2 ) );
    context2->appendChild( new StringHLItem( "\"", String, 0 ) );

    HLItemCollection* context3 = new HLItemCollection( Comment );
    context3->appendChild( new StringHLItem( "*/", Comment, 0 ) );

    HLItemCollection* context4 = new HLItemCollection( PreProcessor );
    context4->appendChild( new RegExpHLItem( ".*\\\\\\s*$", PreProcessor, 4 ) );
    context4->appendChild( new StartsWithHLItem( "", PreProcessor, 0 ) );


    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
    m_items.append( context3 );
    m_items.append( context4 );
}

CSharpColorizer::~CSharpColorizer()
{
}

int CSharpColorizer::computeLevel( QTextParagraph* parag, int startLevel )
{
    int level = startLevel;

    ParagData* data = (ParagData*) parag->extraData();
    if( !data ){
        return startLevel;
    }

    data->setBlockStart( false );

    Q3ValueList<Symbol> symbols = data->symbolList();
    Q3ValueList<Symbol>::Iterator it = symbols.begin();
    while( it != symbols.end() ){
        Symbol sym = *it++;
        if( sym.ch() == '{' ){
            ++level;
        } else if( sym.ch() == '}' ){
            --level;
        }
    }

    if( level > startLevel ){
        data->setBlockStart( true );
    }

    return level;
}

