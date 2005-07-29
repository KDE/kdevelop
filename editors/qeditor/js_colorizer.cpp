/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
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


#include "js_colorizer.h"
#include "qeditor_part.h"
#include "paragdata.h"

#include <qfont.h>
#include <private/qrichtext_p.h>

#include <qdom.h>
#include <qfile.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>


static const char *js_keywords[] = {
"null",
"true",
"false",
"abstract",
"break",
"case",
"catch",
"class",
"const",
"constructor",
"default",
"extends",
"final",
"finally",
"for",
"import",
"instanceof",
"is",
"new",
"var",
"continue",
"function",
"return",
"void",
"delete",
"if",
"this",
"do",
"while",
"else",
"in",
"package",
"private",
"public",
"static",
"switch",
"throw",
"try",
"typeof",
"with",
"abstract",
"boolean",
"byte",
"char",
"debugger",
"double",
"enum",
"export",
"final",
"float",
"goto",
"implements",
"int",
"interface",
"long",
"native",
"private",
"protected",
"public",
"short",
"static",
"super",
"synchronized",
"throws",
"transient",
"volatile",
    0
};


using namespace std;

JSColorizer::JSColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    loadDynamicKeywords();

    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new WhiteSpacesHLItem( Normal, 0 ) );
    context0->appendChild( new StringHLItem( "'", String, 1 ) );
    context0->appendChild( new StringHLItem( "\"", String, 2 ) );
    context0->appendChild( new StringHLItem( "/*", Comment, 3 ) );
    context0->appendChild( new StartsWithHLItem( "//", Comment, 0 ) );
    context0->appendChild( new HexHLItem( Constant, 0 ) );
    context0->appendChild( new NumberHLItem( Constant, 0 ) );
    context0->appendChild( new KeywordsHLItem( m_dynamicKeywords, BuiltInClass, Normal, 0, false ) );
    context0->appendChild( new KeywordsHLItem( js_keywords, Keyword, Normal, 0 ) );

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


    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
    m_items.append( context3 );
}

JSColorizer::~JSColorizer()
{
}

void JSColorizer::loadDynamicKeywords()
{
    QString strFileNameTag( "name" );
    QString strClassNameTag( "name" );

    m_dynamicKeywords.clear();

    QString hlFileDir = KGlobal::dirs()->findResourceDir( "data", "qeditorpart/highlight/highlighting.xml" );

    hlFileDir += "qeditorpart/highlight/";

    //kdDebug(9032) << "Highlighting Dir: " << hlFileDir << endl;

    if( hlFileDir.isNull() )
	return;

    QDomDocument hlFile( "hlfile" ), curDoc ( "classlist" );
    QFile hlRawFile( hlFileDir + "highlighting.xml" );
    int keywordIndex = 0;
    if( !hlRawFile.open( IO_ReadOnly ) )
	return;
    if( !hlFile.setContent( &hlRawFile ) ) {
	hlRawFile.close();
	return;
    }
    hlRawFile.close();

    QDomElement e = hlFile.documentElement();
    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
	e = n.toElement();
	if( !e.isNull() ) {

	    // kdDebug(9032) << "Loading classes-file: " << (hlFileDir + e.attribute( strFileNameTag )) << endl;

	    QFile clsRawFile( hlFileDir + e.attribute( strFileNameTag ) );
	    if( clsRawFile.open( IO_ReadOnly ) && curDoc.setContent( &clsRawFile ) ) {

		QDomElement e = curDoc.documentElement();
		QDomNode n = e.firstChild();
		while( !n.isNull() ) {
		    e = n.toElement();
		    if( !e.isNull()) {
			// kdDebug(9032) << "Adding dynamic keyword: '" << e.attribute( strClassNameTag ) << "'" << endl;
			m_dynamicKeywords.insert( e.attribute( strClassNameTag ), keywordIndex++ );
		    }
		    n = n.nextSibling();
		}

	    }
	    clsRawFile.close();
	}
	n = n.nextSibling();
    }
}

int JSColorizer::computeLevel( QTextParagraph* parag, int startLevel )
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

