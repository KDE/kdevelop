/* $Id$
 *
 *  This file is part of Klint
 *  Copyright (C) 2001 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */


#include "cpp_colorizer.h"
#include "paragdata.h"

#include <qfont.h>
#include <qapplication.h>
#include <qsettings.h>
#include <private/qrichtext_p.h>

#include <kdebug.h>

enum {
	Normal,
	PreProcessor,
	Keyword,
	Comment,
	Constant,
	String,
	Definition,
	Hilite
};

static const char *keywords[] = {
	"break", "case", "continue", "default", "do", "else", "enum", "extern",
	"for", "goto", "if", "return", "sizeof", "struct",
	"switch", "typedef", "union", "while",
	"auto", "char", "uchar", "const", "double", "float", "int", "uint", "long", "register",
	"short", "signed", "static", "unsigned", "void", "volatile",
	"asm", "catch", "class", "const_cast", "const", "delete", "dynamic_cast",
	"explicit", "export", "false", "friend", "inline", "namespace", "new",
	"operator", "private", "protected", "public", "reinterpret_cast",
	"static_cast", "template", "this", "throw", "true", "try", "typeid",
	"typename", "using", "virtual",
	"and_eq", "and", "bitand", "bitor", "compl", "not_eq", "not", "or_eq", "or",
	"xor_eq", "xor",
	"slots", "signals",
	"bool", "wchar_t", "mutable",
	0
};

using namespace std;

CppColorizer::CppColorizer()
{
	refresh();

	// default context
	HLItemCollection* context0 = new HLItemCollection( 0 );
	context0->appendChild( new RegExpHLItem( "^\\s*#", PreProcessor, 4 ) );
	context0->appendChild( new StringHLItem( "'", String, 1 ) );
	context0->appendChild( new StringHLItem( "\"", String, 2 ) );
	context0->appendChild( new StringHLItem( "/*", Comment, 3 ) );
	context0->appendChild( new RegExpHLItem( "//.*", Comment, 0 ) );
	context0->appendChild( new KeywordsHLItem( keywords, Keyword, 0 ) );
	context0->appendChild( new RegExpHLItem( "0[x|X]\\d+", Constant, 0 ) );
	context0->appendChild( new RegExpHLItem( "\\d+", Constant, 0 ) );
	context0->appendChild( new RegExpHLItem( "[_\\w]+", Normal, 0 ) );

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
	context4->appendChild( new RegExpHLItem( ".*", PreProcessor, 0 ) );


	m_items.append( context0 );
	m_items.append( context1 );
	m_items.append( context2 );
	m_items.append( context3 );
	m_items.append( context4 );
}

CppColorizer::~CppColorizer()
{
	QString keybase = "/Klint/0.1/CodeEditor/";
	QSettings config;

	config.writeEntry( keybase + "Family", font.family() );
	config.writeEntry( keybase + "Size", font.pointSize() );
}

void CppColorizer::refresh()
{
	m_formats.clear();

	QString keybase = "/Klint/0.1/CodeEditor/";
	QSettings config;

	font = QFont( "courier", 10 );
	font.setFamily( config.readEntry( keybase + "Family", font.family() ) );
	font.setPointSize( config.readNumEntry( keybase + "Size", font.pointSize() ) );

	m_formats.insert( Normal, new QTextFormat( font, Qt::black ) );
	m_formats.insert( PreProcessor, new QTextFormat( font, Qt::darkGreen ) );
	m_formats.insert( Keyword, new QTextFormat( font, QColor( 0xff, 0x77, 0x00 ) ) );
	m_formats.insert( Comment, new QTextFormat( font, QColor( 0xdd, 0x00, 0x00 ) ) );
	m_formats.insert( Constant, new QTextFormat( font, Qt::darkBlue ) );
	m_formats.insert( String, new QTextFormat( font, QColor( 0x00, 0xaa, 0x00 ) ) );
	m_formats.insert( Definition, new QTextFormat( font, QColor( 0x00, 0x00, 0xff ) ) );
	m_formats.insert( Hilite, new QTextFormat( font, QColor( 0x00, 0x00, 0x68 ) ) );
}

int CppColorizer::computeLevel( QTextParag* parag, int startLevel )
{
    int level = startLevel;

    ParagData* data = (ParagData*) parag->extraData();
    if( !data ){
        return startLevel;
    }

    data->setBlockStart( false );

    QValueList<Symbol> symbols = data->symbolList();
    QValueList<Symbol>::Iterator it = symbols.begin();
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
