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

#include "background_parser.h"
#include "qtimer.h"
#include "qeditor.h"
#include "paragdata.h"
#include "qsourcecolorizer.h"

#include <qapplication.h>
#include <qregexp.h>
#include <private/qrichtext_p.h>
#include <kdebug.h>

using namespace std;

BackgroundParser::BackgroundParser( QEditor* parent, const char* name )
    : QObject( parent, name ),
      m_editor( parent )
{
    m_lastParsed = 0;
    m_interval = 0;

    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
	     this, SLOT(continueParsing()) );

    m_lastParsed = m_editor->document()->firstParagraph();
    connect( m_editor, SIGNAL(textChanged()),
             this, SLOT(slotTextChanged()) );
    connect( this, SIGNAL(parsed()),
             m_editor, SIGNAL(parsed()) );

    continueParsing();
}

BackgroundParser::~BackgroundParser()
{
}

void BackgroundParser::ensureParsed( QTextParagraph* p )
{
    // kdDebug() << "BackgroundParser::ensureParsed" << endl;

    if( !m_editor || !m_lastParsed )
	return;

    while( !isParsed(p) )
        continueParsing();
}

void BackgroundParser::continueParsing()
{
    // kdDebug() << "BackgroundParser::continueParsing" << endl;

    if( !m_lastParsed )
        return;

    parseParag( m_lastParsed );
    m_lastParsed = m_lastParsed->next();

    if( m_lastParsed )
	m_timer->start( m_interval, TRUE );
    else {
        // kdDebug(9032) << "parsed!!!" << endl;
        emit parsed();
    }
}

void BackgroundParser::invalidate( QTextParagraph* from, QTextParagraph* to )
{
    QTextParagraph* p = from;

    while( p ){
        setParsed( p, false );

        if( p == to )
            break;

        p = p->next();
    }

    m_lastParsed = from;
}

void BackgroundParser::sync()
{
    if( !m_editor)
        return;

    m_interval = 0;
    ensureParsed( m_editor->document()->lastParagraph() );
}

void BackgroundParser::slotTextChanged()
{
    QTextParagraph* parag = m_editor->textCursor()->paragraph();

    invalidate( findGoodStartParag(parag),
                findGoodEndParag(parag) );

    continueParsing();
}

void BackgroundParser::reparse()
{
    if( !m_editor )
        return;

    invalidate( m_editor->document()->firstParagraph(),
                m_editor->document()->lastParagraph() );

    continueParsing();
}

void BackgroundParser::parseParag( QTextParagraph* p )
{
    // TODO: m_editor->ensureFormatted( p )
    setParsed( p, true );
}

void BackgroundParser::setParsed( QTextParagraph* p, bool parsed )
{
    ParagData* data = (ParagData*) p->extraData();
    if( data )
        data->setParsed( parsed );
}

bool BackgroundParser::isParsed( QTextParagraph* p ) const
{
    ParagData* data = (ParagData*) p->extraData();
    if( data )
        return data->isParsed();

    return 0;
}
#include "background_parser.moc"
