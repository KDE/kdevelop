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


#include "qsourcecolorizer.h"
#include "paragdata.h"

#include <qapplication.h>
#include <qregexp.h>

QSourceColorizer::QSourceColorizer()
    : QTextPreProcessor()
{
    m_formats.setAutoDelete( TRUE );
    m_items.setAutoDelete( TRUE );
    setSyntaxTable( "{[(", "}])" );
}

QSourceColorizer::~QSourceColorizer()
{
}


void QSourceColorizer::process( QTextDocument* doc, QTextParag* parag, int,
                                bool invalidate )
{
//    qDebug( "QSourceColorizer::process()" );
    int state = 0;
    int pos = 0;
    int startLevel = 0;

    if( parag->prev() ){
        if( parag->prev()->endState() == -1 )
            process( doc, parag->prev(), 0, FALSE );
        state = parag->prev()->endState();
        startLevel = ((ParagData*) parag->prev()->extraData())->level();
    }

    ParagData* extra = (ParagData*) parag->extraData();
    if( extra ){
        extra->clear();
    } else {
        extra = new ParagData();
        parag->setExtraData( extra );
    }

    HLItemCollection* ctx = m_items.at( state );
    while( pos < parag->length() ){
        int attr = 0;
        int next = state;
        int npos = ctx->checkHL( doc, parag, pos, &attr, &next );
        if( npos > pos ){
            state = next;
            ctx = m_items.at( state );
            parag->setFormat( pos, npos, format(attr) );
            pos = npos;
        } else {
            QChar ch = parag->at( pos )->c;
            int a = ctx->attr();
            if( !a ){
                if( m_left.find(ch) != -1 ){
                    extra->add( Symbol::Left, ch, pos );
                } else if( m_right.find(ch) != -1 ){
                    extra->add( Symbol::Right, ch, pos );
                }
            }
            parag->setFormat( pos, pos+1, format(a) );
            ++pos;
        }
    }

    int oldState = parag->endState();
    parag->setEndState( state );
    parag->setFirstPreProcess( FALSE );

    int oldLevel = extra->level();
    int level = computeLevel( parag, startLevel );
    extra->setLevel( level );

    if ( invalidate && parag->next() &&
	 !parag->next()->firstPreProcess() &&
         (oldState != parag->endState() || oldLevel != level) &&
         parag->next()->endState() != -1 ) {
	QTextParag *p = parag->next();
	while ( p ) {
	    if ( p->endState() == -1 )
		return;
	    p->setEndState( -1 );
	    p = p->next();
	}
    }
}

void QSourceColorizer::insertHLItem( int id, HLItemCollection* item )
{
    m_items.insert( id, item );
}

void QSourceColorizer::setSyntaxTable( const QString& l, const QString& r )
{
    m_left = l;
    m_right = r;
}

QChar QSourceColorizer::matchFor( const QChar& ch ) const
{
    int idx = m_left.find( ch );
    if( idx != -1 ){
        return m_right[ idx ];
    } else {
        idx = m_right.find( ch );
        if( idx != -1 ){
            return m_left[ idx ];
        }
    }
    return QChar::null;
}

int QSourceColorizer::computeLevel( QTextParag*, int startLevel )
{
    return startLevel;
}
