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
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */



#include "ocaml_colorizer.h"
#include "paragdata.h"
#include "qeditor_factory.h"

#include <qfont.h>
#include <qapplication.h>
#include <qsettings.h>
#include <private/qrichtext_p.h>

#include <kconfig.h>
#include <kglobalsettings.h>
#include <kinstance.h>

static const char* ocaml_keywords[] = {
    // OCaml keywords
    "and",
    "as",
    "assert",
    "asr",
    "begin",
    "class",
    "closed",
    "constraint",
    "do",
    "done",
    "downto",
    "else",
    "end",
    "exception",
    "external",
    "false",
    "for",
    "fun",
    "function",
    "functor",
    "if",
    "in",
    "include",
    "inherit",
    "land",
    "lazy",
    "let",
    "lor",
    "lsl",
    "lsr",
    "lxor",
    "match",
    "method",
    "mod",
    "module",
    "mutable",
    "new",
    "of",
    "open",
    "or",
    "parser",
    "private",
    "rec",
    "sig",
    "struct",
    "then",
    "to",
    "true",
    "try",
    "type",
    "val",
    "virtual",
    "with",
    "when",
    "while",
    0
};

enum {
    TypeVariable = QSourceColorizer::Custom + 100
};

OCamlColorizer::OCamlColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    QFont defaultFont = KGlobalSettings::fixedFont();
    KConfig* config = QEditorPartFactory::instance()->config();
    config->setGroup( "General" );  // or colorizer ?!?!?

    DECLARE_FORMAT_ITEM( TypeVariable, "TypeVariable", defaultFont, Qt::darkGray );

    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new StartsWithHLItem( "#", PreProcessor, 0 ) );
    context0->appendChild( new StringHLItem( "(*", Comment, 1 ) );
    context0->appendChild( new StringHLItem( "\"", String, 2 ) );
    context0->appendChild( new RegExpHLItem( "'[_a-z]+", TypeVariable, 0 ) );
    context0->appendChild( new KeywordsHLItem( ocaml_keywords, Keyword, Normal, 0 ) );
    context0->appendChild( new NumberHLItem( Constant, 0 ) );

    // comment context
    HLItemCollection* context1 = new HLItemCollection( Comment );
    context1->appendChild( new StringHLItem( "*)", Comment, 0 ) );

    HLItemCollection* context2 = new HLItemCollection( String );
    context2->appendChild( new StringHLItem( "\\", String, 2 ) );
    context2->appendChild( new StringHLItem( "\"", String, 0 ) );

    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
}

OCamlColorizer::~OCamlColorizer()
{
    KConfig* config = QEditorPartFactory::instance()->config();
    config->setGroup( "General" );
    
    STORE_FORMAT_ITEM( TypeVariable );
}
