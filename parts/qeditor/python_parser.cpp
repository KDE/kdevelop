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

#include "python_parser.h"
#include "paragdata.h"
#include "qeditor.h"

#include <private/qrichtext_p.h>
#include <kdebug.h>

static int indentation( const QString &s, int tabwidth )
{
    if ( s.simplifyWhiteSpace().length() == 0 )
        return 0;
    int i = 0;
    int ind = 0;
    while ( i < (int)s.length() ) {
        QChar c = s.at( i );
        if ( c == ' ' ){
            ind++;
        } else if ( c == '\t' ){
            ind += tabwidth;
        } else {
            break;
        }
        ++i;
    }
    return ind;
}

PythonParser::PythonParser( QEditor* parent, const char* name )
    : BackgroundParser( parent, name ),
      m_startClassRx( "^\\s*class\\s+([\\w_]+).*$" ),
      m_startMethodRx( "^\\s*def\\s+([\\w_]+).*$" )
{

}

PythonParser::~PythonParser()
{

}

void PythonParser::parseParag( QTextParagraph* p )
{
    // editor()->ensureFormatted( p )

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

    // kdDebug(9032) << "PythonParser::parseParag() -- " << p->paragId() << endl;

    int level = startLevel;

    data->setBlockStart( false );

    QString text = p->string()->toString();
    text.truncate( text.length() - 1 );

    // #### compute level

    data->setMark( data->mark() & ~0x100 );
    data->setMark( data->mark() & ~0x200 );

    if( !p->prev() || p->prev()->endState() == 0 ){
        if( m_startClassRx.exactMatch(text) ){
            kdDebug(9032) << "Declared class " << m_startClassRx.cap(1)
                          << " at line " << p->paragId() << endl;
            data->setMark( data->mark() | 0x100 );
        } else if( m_startMethodRx.exactMatch(text) ){
            kdDebug(9032) << "Declared method " << m_startMethodRx.cap(1)
                          << " at line " << p->paragId() << endl;
            data->setMark( data->mark() | 0x200 );
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

QTextParagraph* PythonParser::findGoodStartParag( QTextParagraph* p )
{
    return p;
}

QTextParagraph* PythonParser::findGoodEndParag( QTextParagraph* p )
{
    return p;
}
