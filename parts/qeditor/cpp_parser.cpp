/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
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

#include "cpp_parser.h"
#include "paragdata.h"

#include <private/qrichtext_p.h>
#include <kdebug.h>

using namespace std;

CppParser::CppParser( QEditor* parent, const char* name )
    : BackgroundParser( parent, name )
{

}

CppParser::~CppParser()
{

}

void CppParser::parseParag( QTextParag* p )
{
    // m_editor->ensureFormatted( p )

    if( isParsed(p) )
        return;

    ParagData* data = (ParagData*) p->extraData();
    if( !data )
        return;

    int startLevel = 0;
    if( p->prev() ){
        ParagData* data = (ParagData*) p->prev()->extraData();
        startLevel = data->level();
    }

    // kdDebug(9032) << "CppParser::parseParag() -- " << p->paragId() << endl;

    int level = startLevel;

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

    level = level > 0 ? level : 0;

    if( level > startLevel ){
        data->setBlockStart( true );
    }

    int oldLevel = data->level();
    data->setLevel( level );

    if( p->next() && oldLevel != level ){
        setParsed( p->next(), false );
    }

    setParsed( p, true );
}

QTextParag* CppParser::findGoodStartParag( QTextParag* p )
{
    return p;
}

QTextParag* CppParser::findGoodEndParag( QTextParag* p )
{
    return p;
}
