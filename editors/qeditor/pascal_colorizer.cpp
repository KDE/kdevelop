/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "pascal_colorizer.h"
#include "qeditor.h"
#include "paragdata.h"

#include <qfont.h>
#include <qapplication.h>
#include <qsettings.h>
#include <private/qrichtext_p.h>

static const char *pascal_keywords[] = {
  // Pascal keywords taken from Free Pascal 1.0.6 reference manual
    "absolute",
    "and",
    "array",
    "asm",
    "begin",
    "break",
    "case",
    "const",
    "constructor",
    "continue",
    "destructor",
    "div",
    "do",
    "downto",
    "else",
    "end",
    "file",
    "for",
    "function",
    "goto",
    "if",
    "implementation",
    "in",
    "inherited",
    "inline",
    "interface",
    "label",
    "mod",
    "nil",
    "not",
    "object",
    "of",
    "on",
    "operator",
    "or",
    "packed",
    "procedure",
    "program",
    "record",
    "repeat",
    "self",
    "set",
    "shl",
    "shr",
    "string",
    "then",
    "to",
    "type",
    "unit",
    "until",
    "uses",
    "var",
    "while",
    "with",
    "xor",
    "as",
    "class",
    "except",
    "exports",
    "finalization",
    "finally",
    "initialization",
    "is",
    "library",
    "on",
    "property",
    "raise",
    "try",
    "dispose",
    "exit",
    "false",
    "new",
    "true",
    "absolute",
    "abstract",
    "alias",
    "assembler",
    "cdecl",
    "default",
    "export",
    "external",
    "far",
    "forward",
    "index",
    "name",
    "near",
    "override",
    "pascal",
    "popstack",
    "private",
    "protected",
    "public",
    "published",
    "read",
    "register",
    "saveregisters",
    "stdcall",
    "virtual",
    "write",
    0
};

 PascalColorizer::PascalColorizer( QEditor * editor)
    : QSourceColorizer( editor )
{
    //default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new StartsWithHLItem( "//", Comment, 0 ) );
    context0->appendChild( new StringHLItem( "{", Comment, 1 ) );
    context0->appendChild( new StringHLItem( "(*", Comment, 2 ) );
    context0->appendChild( new KeywordsHLItem( pascal_keywords, Keyword, Normal, 0 ) );
    context0->appendChild( new WhiteSpacesHLItem( Normal, 0 ) );
    context0->appendChild( new StringHLItem( "'", String, 3 ) );
    context0->appendChild( new NumberHLItem( Constant, 0 ) );
    context0->appendChild( new RegExpHLItem( "\\$[A-Fa-f0-9]*", Constant, 0 ) );
    context0->appendChild( new RegExpHLItem( "\\%[0-1]*", Constant, 0 ) );

    HLItemCollection* context1 = new HLItemCollection( Comment );
    context1->appendChild( new StringHLItem( "}", Comment, 0 ) );

    HLItemCollection* context2 = new HLItemCollection( Comment );
    context2->appendChild( new StringHLItem( "*)", Comment, 0 ) );

    HLItemCollection* context3 = new HLItemCollection( String );
    context3->appendChild( new StringHLItem( "'", String, 0 ) );

    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
    m_items.append( context3 );
}

 PascalColorizer::~PascalColorizer( )
{
}
