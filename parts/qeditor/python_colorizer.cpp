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


#include "python_colorizer.h"
#include "qeditor.h"
#include "paragdata.h"

#include <qfont.h>
#include <qapplication.h>
#include <qsettings.h>
#include <private/qrichtext_p.h>

static const char *keywords[] = {
  // Python keywords
  "and",
  "assert",
  "break",
  "class",
  "continue",
  "def",
  "del",
  "elif",
  "else",
  "except",
  "exec",
  "finally",
  "for",
  "from",
  "global",
  "if",
  "import",
  "in",
  "is",
  "lambda",
  "not",
  "or",
  "pass",
  "print",
  "raise",
  "return",
  "try",
  "while",
  0
};

PythonColorizer::PythonColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new RegExpHLItem( "#.*", Comment, 0 ) );
    context0->appendChild( new StringHLItem( "'''", String, 4 ) );
    context0->appendChild( new StringHLItem( "\"\"\"", String, 5 ) );
    context0->appendChild( new StringHLItem( "\"", String, 2 ) );
    context0->appendChild( new StringHLItem( "'", String, 3 ) );
    context0->appendChild( new KeywordsHLItem( keywords, Keyword, 0 ) );
    context0->appendChild( new RegExpHLItem( "\\d+", Constant, 0 ) );
    context0->appendChild( new RegExpHLItem( "\\w+", Normal, 0 ) );

    // comment context
    HLItemCollection* context1 = new HLItemCollection( Comment );
    context1->appendChild( new StringHLItem( "*/", Comment, 0 ) );

    HLItemCollection* context2 = new HLItemCollection( String );
    context2->appendChild( new StringHLItem( "\\\\", String, 2 ) );
    context2->appendChild( new StringHLItem( "\\\"", String, 2 ) );
    context2->appendChild( new StringHLItem( "\"", String, 0 ) );

    HLItemCollection* context3 = new HLItemCollection( String );
    context3->appendChild( new StringHLItem( "\\'", String, 3 ) );
    context3->appendChild( new StringHLItem( "\\\\", String, 3 ) );
    context3->appendChild( new StringHLItem( "'", String, 0 ) );

    HLItemCollection* context4 = new HLItemCollection( String );
    context4->appendChild( new StringHLItem( "'''", String, 0 ) );

    HLItemCollection* context5 = new HLItemCollection( String );
    context5->appendChild( new StringHLItem( "\"\"\"", String, 0 ) );


    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
    m_items.append( context3 );
    m_items.append( context4 );
    m_items.append( context5 );
}

PythonColorizer::~PythonColorizer()
{
}

int PythonColorizer::computeLevel( QTextParag*, int startLevel )
{
    int level = startLevel;

    return level;
}
