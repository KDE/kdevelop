/****************************************************************************
** Implementation of the internal Qt classes dealing with rich text
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qrichtext_p.h"
#include "qstringlist.h"
#include "qfont.h"
#include "qtextstream.h"
#include "qfile.h"
#include "qregexp.h"
#include "qapplication.h"
#include "qclipboard.h"
#include "qmap.h"
#include "qfileinfo.h"
#include "qstylesheet.h"
#include "qmime.h"
#include "qregexp.h"
#include "qimage.h"
#include "qdragobject.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qcursor.h"
#include "qstack.h"
#include "qcomplextext_p.h"

#include <stdlib.h>

using namespace Qt3;

//#define PARSER_DEBUG
//#define DEBUG_COLLECTION
//#define DEBUG_TABLE_RENDERING

static QTextFormatCollection *qFormatCollection = 0;

#if defined(PARSER_DEBUG)
static QString debug_indent;
#endif

static double scale_factor( double v )
{
    return v;
}

static bool is_printer( QPainter * )
{
    return FALSE;
#if 0
    if ( !p || !p->device() )
	return FALSE;
    return p->device()->devType() == QInternal::Printer;
#endif
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void QTextCommandHistory::addCommand( QTextCommand *cmd )
{
    if ( current < (int)history.count() - 1 ) {
	QPtrList<QTextCommand> commands;
	commands.setAutoDelete( FALSE );

	for( int i = 0; i <= current; ++i ) {
	    commands.insert( i, history.at( 0 ) );
	    history.take( 0 );
	}

	commands.append( cmd );
	history.clear();
	history = commands;
	history.setAutoDelete( TRUE );
    } else {
	history.append( cmd );
    }

    if ( (int)history.count() > steps )
	history.removeFirst();
    else
	++current;
}

QTextCursor *QTextCommandHistory::undo( QTextCursor *c )
{
    if ( current > -1 ) {
	QTextCursor *c2 = history.at( current )->unexecute( c );
	--current;
	return c2;
    }
    return 0;
}

QTextCursor *QTextCommandHistory::redo( QTextCursor *c )
{
    if ( current > -1 ) {
	if ( current < (int)history.count() - 1 ) {
	    ++current;
	    return history.at( current )->execute( c );
	}
    } else {
	if ( history.count() > 0 ) {
	    ++current;
	    return history.at( current )->execute( c );
	}
    }
    return 0;
}

bool QTextCommandHistory::isUndoAvailable()
{
    return current > -1;
}

bool QTextCommandHistory::isRedoAvailable()
{
   return current > -1 || history.count() > 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextDeleteCommand::QTextDeleteCommand( QTextDocument *d, int i, int idx, const QMemArray<QTextStringChar> &str,
					const QValueList< QPtrVector<QStyleSheetItem> > &os,
					const QValueList<QStyleSheetItem::ListStyle> &ols,
					const QMemArray<int> &oas)
    : QTextCommand( d ), id( i ), index( idx ), parag( 0 ), text( str ), oldStyles( os ), oldListStyles( ols ), oldAligns( oas )
{
    for ( int j = 0; j < (int)text.size(); ++j ) {
	if ( text[ j ].format() )
	    text[ j ].format()->addRef();
    }
}

QTextDeleteCommand::QTextDeleteCommand( QTextParag *p, int idx, const QMemArray<QTextStringChar> &str )
    : QTextCommand( 0 ), id( -1 ), index( idx ), parag( p ), text( str )
{
    for ( int i = 0; i < (int)text.size(); ++i ) {
	if ( text[ i ].format() )
	    text[ i ].format()->addRef();
    }
}

QTextDeleteCommand::~QTextDeleteCommand()
{
    for ( int i = 0; i < (int)text.size(); ++i ) {
	if ( text[ i ].format() )
	    text[ i ].format()->removeRef();
    }
    text.resize( 0 );
}

QTextCursor *QTextDeleteCommand::execute( QTextCursor *c )
{
    QTextParag *s = doc ? doc->paragAt( id ) : parag;
    if ( !s ) {
	qWarning( "can't locate parag at %d, last parag: %d", id, doc->lastParag()->paragId() );
	return 0;
    }

    cursor.setParag( s );
    cursor.setIndex( index );
    int len = text.size();
    if ( c )
	*c = cursor;
    if ( doc ) {
	doc->setSelectionStart( QTextDocument::Temp, &cursor );
	for ( int i = 0; i < len; ++i )
	    cursor.gotoRight();
	doc->setSelectionEnd( QTextDocument::Temp, &cursor );
	doc->removeSelectedText( QTextDocument::Temp, &cursor );
	if ( c )
	    *c = cursor;
    } else {
	s->remove( index, len );
    }

    return c;
}

QTextCursor *QTextDeleteCommand::unexecute( QTextCursor *c )
{
    QTextParag *s = doc ? doc->paragAt( id ) : parag;
    if ( !s ) {
	qWarning( "can't locate parag at %d, last parag: %d", id, doc->lastParag()->paragId() );
	return 0;
    }

    cursor.setParag( s );
    cursor.setIndex( index );
    QString str = QTextString::toString( text );
    cursor.insert( str, TRUE, &text );
    cursor.setParag( s );
    cursor.setIndex( index );
    if ( c ) {
	c->setParag( s );
	c->setIndex( index );
	for ( int i = 0; i < (int)text.size(); ++i )
	    c->gotoRight();
    }

    QValueList< QPtrVector<QStyleSheetItem> >::Iterator it = oldStyles.begin();
    QValueList<QStyleSheetItem::ListStyle>::Iterator lit = oldListStyles.begin();
    int i = 0;
    QTextParag *p = s;
    bool end = FALSE;
    while ( p ) {
	if ( it != oldStyles.end() )
	    p->setStyleSheetItems( *it );
	else
	    end = TRUE;
	if ( lit != oldListStyles.end() )
	    p->setListStyle( *lit );
	else
	    end = TRUE;
	if ( i < (int)oldAligns.size() )
	    p->setAlignment( oldAligns.at( i ) );
	else
	    end = TRUE;
	if ( end )
	    break;
	p = p->next();
	++it;
	++lit;
	++i;
    }

    s = cursor.parag();
    while ( s ) {
	s->format();
	s->setChanged( TRUE );
	if ( s == c->parag() )
	    break;
	s = s->next();
    }

    return &cursor;
}

QTextFormatCommand::QTextFormatCommand( QTextDocument *d, int sid, int sidx, int eid, int eidx,
					const QMemArray<QTextStringChar> &old, QTextFormat *f, int fl )
    : QTextCommand( d ), startId( sid ), startIndex( sidx ), endId( eid ), endIndex( eidx ), format( f ), oldFormats( old ), flags( fl )
{
    format = d->formatCollection()->format( f );
    for ( int j = 0; j < (int)oldFormats.size(); ++j ) {
	if ( oldFormats[ j ].format() )
	    oldFormats[ j ].format()->addRef();
    }
}

QTextFormatCommand::~QTextFormatCommand()
{
    format->removeRef();
    for ( int j = 0; j < (int)oldFormats.size(); ++j ) {
	if ( oldFormats[ j ].format() )
	    oldFormats[ j ].format()->removeRef();
    }
}

QTextCursor *QTextFormatCommand::execute( QTextCursor *c )
{
    QTextParag *sp = doc->paragAt( startId );
    QTextParag *ep = doc->paragAt( endId );
    if ( !sp || !ep )
	return c;

    QTextCursor start( doc );
    start.setParag( sp );
    start.setIndex( startIndex );
    QTextCursor end( doc );
    end.setParag( ep );
    end.setIndex( endIndex );

    doc->setSelectionStart( QTextDocument::Temp, &start );
    doc->setSelectionEnd( QTextDocument::Temp, &end );
    doc->setFormat( QTextDocument::Temp, format, flags );
    doc->removeSelection( QTextDocument::Temp );
    *c = end;
    return c;
}

QTextCursor *QTextFormatCommand::unexecute( QTextCursor *c )
{
    QTextParag *sp = doc->paragAt( startId );
    QTextParag *ep = doc->paragAt( endId );
    if ( !sp || !ep )
	return 0;

    int idx = startIndex;
    int fIndex = 0;
    while ( TRUE ) {
	if ( oldFormats.at( fIndex ).c == '\n' ) {
	    if ( idx > 0 ) {
		if ( idx < sp->length() && fIndex > 0 )
		    sp->setFormat( idx, 1, oldFormats.at( fIndex - 1 ).format() );
		if ( sp == ep )
		    break;
		sp = sp->next();
		idx = 0;
	    }
	    fIndex++;
	}
	if ( oldFormats.at( fIndex ).format() )
	    sp->setFormat( idx, 1, oldFormats.at( fIndex ).format() );
	idx++;
	fIndex++;
	if ( fIndex >= (int)oldFormats.size() )
	    break;
	if ( idx >= sp->length() ) {
	    if ( sp == ep )
		break;
	    sp = sp->next();
	    idx = 0;
	}
    }

    QTextCursor end( doc );
    end.setParag( ep );
    end.setIndex( endIndex );
    *c = end;
    return c;
}

QTextAlignmentCommand::QTextAlignmentCommand( QTextDocument *d, int fParag, int lParag, int na, const QMemArray<int> &oa )
    : QTextCommand( d ), firstParag( fParag ), lastParag( lParag ), newAlign( na ), oldAligns( oa )
{
}

QTextCursor *QTextAlignmentCommand::execute( QTextCursor *c )
{
    QTextParag *p = doc->paragAt( firstParag );
    if ( !p )
	return c;
    while ( p ) {
	p->setAlignment( newAlign );
	if ( p->paragId() == lastParag )
	    break;
	p = p->next();
    }
    return c;
}

QTextCursor *QTextAlignmentCommand::unexecute( QTextCursor *c )
{
    QTextParag *p = doc->paragAt( firstParag );
    if ( !p )
	return c;
    int i = 0;
    while ( p ) {
	if ( i < (int)oldAligns.size() )
	    p->setAlignment( oldAligns.at( i ) );
	if ( p->paragId() == lastParag )
	    break;
	p = p->next();
	++i;
    }
    return c;
}

QTextParagTypeCommand::QTextParagTypeCommand( QTextDocument *d, int fParag, int lParag, bool l,
					      QStyleSheetItem::ListStyle s, const QValueList< QPtrVector<QStyleSheetItem> > &os,
					      const QValueList<QStyleSheetItem::ListStyle> &ols )
    : QTextCommand( d ), firstParag( fParag ), lastParag( lParag ), list( l ), listStyle( s ), oldStyles( os ), oldListStyles( ols )
{
}

QTextCursor *QTextParagTypeCommand::execute( QTextCursor *c )
{
    QTextParag *p = doc->paragAt( firstParag );
    if ( !p )
	return c;
    while ( p ) {
	p->setList( list, (int)listStyle );
	if ( p->paragId() == lastParag )
	    break;
	p = p->next();
    }
    return c;
}

QTextCursor *QTextParagTypeCommand::unexecute( QTextCursor *c )
{
    QTextParag *p = doc->paragAt( firstParag );
    if ( !p )
	return c;
    QValueList< QPtrVector<QStyleSheetItem> >::Iterator it = oldStyles.begin();
    QValueList<QStyleSheetItem::ListStyle>::Iterator lit = oldListStyles.begin();
    while ( p ) {
	if ( it != oldStyles.end() )
	    p->setStyleSheetItems( *it );
	if ( lit != oldListStyles.end() )
	    p->setListStyle( *lit );
	if ( p->paragId() == lastParag )
	    break;
	p = p->next();
	++it;
	++lit;
    }
    return c;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextCursor::QTextCursor( QTextDocument *d )
    : doc( d ), ox( 0 ), oy( 0 )
{
    nested = FALSE;
    idx = 0;
    string = doc ? doc->firstParag() : 0;
    tmpIndex = -1;
}

QTextCursor::QTextCursor()
{
}

QTextCursor::QTextCursor( const QTextCursor &c )
{
    doc = c.doc;
    ox = c.ox;
    oy = c.oy;
    nested = c.nested;
    idx = c.idx;
    string = c.string;
    tmpIndex = c.tmpIndex;
    indices = c.indices;
    parags = c.parags;
    xOffsets = c.xOffsets;
    yOffsets = c.yOffsets;
}

QTextCursor &QTextCursor::operator=( const QTextCursor &c )
{
    doc = c.doc;
    ox = c.ox;
    oy = c.oy;
    nested = c.nested;
    idx = c.idx;
    string = c.string;
    tmpIndex = c.tmpIndex;
    indices = c.indices;
    parags = c.parags;
    xOffsets = c.xOffsets;
    yOffsets = c.yOffsets;

    return *this;
}

bool QTextCursor::operator==( const QTextCursor &c ) const
{
    return doc == c.doc && string == c.string && idx == c.idx;
}

int QTextCursor::totalOffsetX() const
{
    if ( !nested )
	return 0;
    QValueStack<int>::ConstIterator xit = xOffsets.begin();
    int xoff = ox;
    for ( ; xit != xOffsets.end(); ++xit )
	xoff += *xit;
    return xoff;
}

int QTextCursor::totalOffsetY() const
{
    if ( !nested )
	return 0;
    QValueStack<int>::ConstIterator yit = yOffsets.begin();
    int yoff = oy;
    for ( ; yit != yOffsets.end(); ++yit )
	yoff += *yit;
    return yoff;
}

void QTextCursor::gotoIntoNested( const QPoint &globalPos )
{
    if ( !doc )
	return;
    push();
    ox = 0;
    int bl, y;
    string->lineHeightOfChar( idx, &bl, &y );
    oy = y + string->rect().y();
    nested = TRUE;
    QPoint p( globalPos.x() - offsetX(), globalPos.y() - offsetY() );
    Q_ASSERT( string->at( idx )->isCustom() );
    string->at( idx )->customItem()->enterAt( this, doc, string, idx, ox, oy, p );
}

void QTextCursor::invalidateNested()
{
    if ( nested ) {
	QValueStack<QTextParag*>::Iterator it = parags.begin();
	QValueStack<int>::Iterator it2 = indices.begin();
	for ( ; it != parags.end(); ++it, ++it2 ) {
	    if ( *it == string )
		continue;
	    (*it)->invalidate( 0 );
	    if ( (*it)->at( *it2 )->isCustom() )
		(*it)->at( *it2 )->customItem()->invalidate();
	}
    }
}

void QTextCursor::insert( const QString &str, bool checkNewLine, QMemArray<QTextStringChar> *formatting )
{
    tmpIndex = -1;
    bool justInsert = TRUE;
    QString s( str );
#if defined(Q_WS_WIN)
    if ( checkNewLine )
	s = s.replace( QRegExp( "\\r" ), "" );
#endif
    if ( checkNewLine )
	justInsert = s.find( '\n' ) == -1;
    if ( justInsert ) {
	string->insert( idx, s );
	if ( formatting ) {
	    for ( int i = 0; i < (int)s.length(); ++i ) {
		if ( formatting->at( i ).format() ) {
		    formatting->at( i ).format()->addRef();
		    string->string()->setFormat( idx + i, formatting->at( i ).format(), TRUE );
		}
	    }
	}
	idx += s.length();
    } else {
	QStringList lst = QStringList::split( '\n', s, TRUE );
	QStringList::Iterator it = lst.begin();
	int y = string->rect().y() + string->rect().height();
	int lastIndex = 0;
	for ( ; it != lst.end(); ++it ) {
	    if ( it != lst.begin() ) {
		splitAndInsertEmptyParag( FALSE, FALSE );
		string->setEndState( -1 );
		string->prev()->format( -1, FALSE );
	    }
	    QString s = *it;
	    if ( s.isEmpty() )
		continue;
	    string->insert( idx, s );
	    if ( formatting ) {
		int len = s.length();
		if ( it != --lst.end() )
		    len++;
		for ( int i = 0; i < len; ++i ) {
		    if ( formatting->at( i + lastIndex ).format() ) {
			formatting->at( i + lastIndex ).format()->addRef();
			string->string()->setFormat( i + idx, formatting->at( i + lastIndex ).format(), TRUE );
		    }
		}
		lastIndex += len;
	    }

	    idx += s.length();
	}
	string->format( -1, FALSE );
	int dy = string->rect().y() + string->rect().height() - y;
	QTextParag *p = string;
	p->setParagId( p->prev()->paragId() + 1 );
	p = p->next();
	while ( p ) {
	    p->setParagId( p->prev()->paragId() + 1 );
	    p->move( dy );
	    p->invalidate( 0 );
	    p->setEndState( -1 );
	    p = p->next();
	}
    }

    int h = string->rect().height();
    string->format( -1, TRUE );
    if ( h != string->rect().height() )
	invalidateNested();
    else if ( doc && doc->parent() )
	doc->nextDoubleBuffered = TRUE;
}

void QTextCursor::gotoLeft()
{
    tmpIndex = -1;

    if ( idx > 0 ) {
	idx--;
    } else if ( string->prev() ) {
	string = string->prev();
	while ( !string->isVisible() )
	    string = string->prev();
	idx = string->length() - 1;
    } else {
	if ( nested ) {
	    pop();
	    processNesting( Prev );
	    if ( idx == -1 ) {
		pop();
		if ( idx > 0 ) {
		    idx--;
		} else if ( string->prev() ) {
		    string = string->prev();
		    idx = string->length() - 1;
		}
	    }
	}
    }

    if ( string->at( idx )->isCustom() &&
	 string->at( idx )->customItem()->isNested() ) {
	processNesting( EnterEnd );
    }
}

void QTextCursor::push()
{
    indices.push( idx );
    parags.push( string );
    xOffsets.push( ox );
    yOffsets.push( oy );
    nestedStack.push( nested );
}

void QTextCursor::pop()
{
    if ( !doc )
	return;
    idx = indices.pop();
    string = parags.pop();
    ox = xOffsets.pop();
    oy = yOffsets.pop();
    if ( doc->parent() )
	doc = doc->parent();
    nested = nestedStack.pop();
}

void QTextCursor::restoreState()
{
    while ( !indices.isEmpty() )
	pop();
}

bool QTextCursor::place( const QPoint &p, QTextParag *s )
{
    QPoint pos( p );
    QRect r;
    if ( pos.y() < s->rect().y() )
	pos.setY( s->rect().y() );
    while ( s ) {
	r = s->rect();
	r.setWidth( doc ? doc->width() : QWIDGETSIZE_MAX );
	if ( pos.y() >= r.y() && pos.y() <= r.y() + r.height() || !s->next() )
	    break;
	s = s->next();
        //if ( !s )
        //    break;
    }

    if ( !s )
	return FALSE;

    setParag( s, FALSE );
    int y = s->rect().y();
    int lines = s->lines();
    QTextStringChar *chr = 0;
    int index = 0;
    int i = 0;
    int cy = 0;
    int ch = 0;
    for ( ; i < lines; ++i ) {
	chr = s->lineStartOfLine( i, &index );
	cy = s->lineY( i );
	ch = s->lineHeight( i );
	if ( !chr )
	    return FALSE;
	if ( pos.y() >= y + cy && pos.y() <= y + cy + ch )
	    break;
    }
    int nextLine;
    if ( i < lines - 1 )
	s->lineStartOfLine( i+1, &nextLine );
    else
	nextLine = s->length();
    i = index;
    int x = s->rect().x();
    if ( pos.x() < x )
	pos.setX( x + 1 );
    int cw;
    int curpos = s->length()-1;
    int dist = 10000000;
    bool inCustom = FALSE;
    while ( i < nextLine ) {
	chr = s->at(i);
	int cpos = x + chr->x;
	cw = s->string()->width( i );
	if ( chr->isCustom() ) {
	    if ( pos.x() >= cpos && pos.x() <= cpos + cw &&
		 pos.y() >= y + cy && pos.y() <= y + cy + chr->height() ) {
		inCustom = TRUE;
		curpos = i;
		break;
	    }
	} else {
	    if( chr->rightToLeft )
		cpos += cw;
	    int d = cpos - pos.x();
	    bool dm = d < 0 ? !chr->rightToLeft : chr->rightToLeft;
	    if ( QABS( d ) < dist || (dist == d && dm == TRUE ) ) {
		dist = QABS( d );
		curpos = i;
	    }
	}
	i++;
    }
    setIndex( curpos, FALSE );

    if ( inCustom && doc && parag()->at( curpos )->isCustom() && parag()->at( curpos )->customItem()->isNested() ) {
	gotoIntoNested( pos );
	QPoint p( pos.x() - offsetX(), pos.y() - offsetY() );
	if ( !place( p, document()->firstParag() ) )
	    pop();
    }
    return TRUE;
}

void QTextCursor::processNesting( Operation op )
{
    if ( !doc )
	return;
    push();
    ox = 0;
    int bl, y;
    string->lineHeightOfChar( idx, &bl, &y );
    oy = y + string->rect().y();
    nested = TRUE;

    switch ( op ) {
    case EnterBegin:
	string->at( idx )->customItem()->enter( this, doc, string, idx, ox, oy );
	break;
    case EnterEnd:
	string->at( idx )->customItem()->enter( this, doc, string, idx, ox, oy, TRUE );
	break;
    case Next:
	string->at( idx )->customItem()->next( this, doc, string, idx, ox, oy );
	break;
    case Prev:
	string->at( idx )->customItem()->prev( this, doc, string, idx, ox, oy );
	break;
    case Down:
	string->at( idx )->customItem()->down( this, doc, string, idx, ox, oy );
	break;
    case Up:
	string->at( idx )->customItem()->up( this, doc, string, idx, ox, oy );
	break;
    }
}

void QTextCursor::gotoRight()
{
    tmpIndex = -1;

    if ( string->at( idx )->isCustom() &&
	 string->at( idx )->customItem()->isNested() ) {
	processNesting( EnterBegin );
	return;
    }

    if ( idx < string->length() - 1 ) {
	idx++;
    } else if ( string->next() ) {
	string = string->next();
	while ( !string->isVisible() )
	    string = string->next();
	idx = 0;
    } else {
	if ( nested ) {
	    pop();
	    processNesting( Next );
	    if ( idx == -1 ) {
		pop();
		if ( idx < string->length() - 1 ) {
		    idx++;
		} else if ( string->next() ) {
		    string = string->next();
		    idx = 0;
		}
	    }
	}
    }
}

void QTextCursor::gotoUp()
{
    int indexOfLineStart;
    int line;
    QTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    tmpIndex = QMAX( tmpIndex, idx - indexOfLineStart );
    if ( indexOfLineStart == 0 ) {
	if ( !string->prev() ) {
	    if ( !nested )
		return;
	    pop();
	    processNesting( Up );
	    if ( idx == -1 ) {
		pop();
		if ( !string->prev() )
		    return;
		idx = tmpIndex = 0;
	    } else {
		tmpIndex = -1;
		return;
	    }
	}
	string = string->prev();
	while ( !string->isVisible() )
	    string = string->prev();
	int lastLine = string->lines() - 1;
	if ( !string->lineStartOfLine( lastLine, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < string->length() )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = string->length() - 1;
    } else {
	--line;
	int oldIndexOfLineStart = indexOfLineStart;
	if ( !string->lineStartOfLine( line, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < oldIndexOfLineStart )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = oldIndexOfLineStart - 1;
    }
}

void QTextCursor::gotoDown()
{
    int indexOfLineStart;
    int line;
    QTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    tmpIndex = QMAX( tmpIndex, idx - indexOfLineStart );
    if ( line == string->lines() - 1 ) {
	if ( !string->next() ) {
	    if ( !nested )
		return;
	    pop();
	    processNesting( Down );
	    if ( idx == -1 ) {
		pop();
		if ( !string->next() )
		    return;
		idx = tmpIndex = 0;
	    } else {
		tmpIndex = -1;
		return;
	    }
	}
	string = string->next();
	while ( !string->isVisible() )
	    string = string->next();
	if ( !string->lineStartOfLine( 0, &indexOfLineStart ) )
	    return;
	int end;
	if ( string->lines() == 1 )
	    end = string->length();
	else
	    string->lineStartOfLine( 1, &end );
	if ( indexOfLineStart + tmpIndex < end )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = end - 1;
    } else {
	++line;
	int end;
	if ( line == string->lines() - 1 )
	    end = string->length();
	else
	    string->lineStartOfLine( line + 1, &end );
	if ( !string->lineStartOfLine( line, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < end )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = end - 1;
    }
}

void QTextCursor::gotoLineEnd()
{
    tmpIndex = -1;
    int indexOfLineStart;
    int line;
    QTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    if ( line == string->lines() - 1 ) {
	idx = string->length() - 1;
    } else {
	c = string->lineStartOfLine( ++line, &indexOfLineStart );
	indexOfLineStart--;
	idx = indexOfLineStart;
    }
}

void QTextCursor::gotoLineStart()
{
    tmpIndex = -1;
    int indexOfLineStart;
    int line;
    QTextStringChar *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    idx = indexOfLineStart;
}

void QTextCursor::gotoHome()
{
    tmpIndex = -1;
    if ( doc )
	string = doc->firstParag();
    idx = 0;
}

void QTextCursor::gotoEnd()
{
    if ( doc && !doc->lastParag()->isValid() )
    {
        qDebug("Last parag, %d, is invalid - aborting gotoEnd() !",doc->lastParag()->paragId());
	return;
    }

    tmpIndex = -1;
    if ( doc )
	string = doc->lastParag();
    idx = string->length() - 1;
    qDebug("gotoEnd: going to parag %d, index %d",string->paragId(),idx);
}

void QTextCursor::gotoPageUp( int visibleHeight )
{
    tmpIndex = -1;
    QTextParag *s = string;
    int h = visibleHeight;
    int y = s->rect().y();
    while ( s ) {
	if ( y - s->rect().y() >= h )
	    break;
	s = s->prev();
    }

    if ( !s && doc )
	s = doc->firstParag();

    string = s;
    idx = 0;
}

void QTextCursor::gotoPageDown( int visibleHeight )
{
    tmpIndex = -1;
    QTextParag *s = string;
    int h = visibleHeight;
    int y = s->rect().y();
    while ( s ) {
	if ( s->rect().y() - y >= h )
	    break;
	s = s->next();
    }

    if ( !s && doc ) {
	s = doc->lastParag();
	string = s;
	idx = string->length() - 1;
	return;
    }

    if ( !s->isValid() )
	return;

    string = s;
    idx = 0;
}

void QTextCursor::gotoWordLeft()
{
    gotoLeft();
    tmpIndex = -1;
    QTextString *s = string->string();
    bool allowSame = FALSE;
    for ( int i = idx - 1; i >= 0; --i ) {
	if ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' ) {
	    if ( !allowSame && s->at( i ).c == s->at( idx ).c )
		continue;
	    idx = i + 1;
	    return;
	}
	if ( !allowSame && s->at( i ).c != s->at( idx ).c )
	    allowSame = TRUE;
    }

    if ( string->prev() ) {
	string = string->prev();
	while ( !string->isVisible() )
	    string = string->prev();
	idx = string->length() - 1;
    } else {
	gotoLineStart();
    }
}

void QTextCursor::gotoWordRight()
{
    tmpIndex = -1;
    QTextString *s = string->string();
    bool allowSame = FALSE;
    for ( int i = idx + 1; i < (int)s->length(); ++i ) {
	if ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' ) {
	    if ( !allowSame &&  s->at( i ).c == s->at( idx ).c )
		continue;
	    idx = i;
	    return;
	}
	if ( !allowSame && s->at( i ).c != s->at( idx ).c )
	    allowSame = TRUE;
    }

    if ( string->next() ) {
	string = string->next();
	while ( !string->isVisible() )
	    string = string->next();
	idx = 0;
    } else {
	gotoLineEnd();
    }
}

bool QTextCursor::atParagStart()
{
    return idx == 0;
}

bool QTextCursor::atParagEnd()
{
    return idx == string->length() - 1;
}

void QTextCursor::splitAndInsertEmptyParag( bool ind, bool updateIds )
{
    if ( !doc )
	return;
    tmpIndex = -1;
    QTextFormat *f = 0;
    if ( doc->useFormatCollection() ) {
	f = string->at( idx )->format();
	//if ( idx == string->length() - 1 && idx > 0 )
	//    f = string->at( idx - 1 )->format();
	if ( f->isMisspelled() ) {
	    f->removeRef();
	    f = doc->formatCollection()->format( f->font(), f->color() );
	}
    }

    if ( atParagStart() ) {
	QTextParag *p = string->prev();
	QTextParag *s = doc->createParag( doc, p, string, updateIds );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->copyParagData( string );
	if ( ind ) {
	    s->indent();
	    s->format();
	    indent();
	    string->format();
	}
    } else if ( atParagEnd() ) {
	QTextParag *n = string->next();
	QTextParag *s = doc->createParag( doc, string, n, updateIds );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->copyParagData( string );
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    string = s;
	    idx = ni;
	} else {
	    string = s;
	    idx = 0;
	}
    } else {
	QString str = string->string()->toString().mid( idx, 0xFFFFFF );
	QTextParag *n = string->next();
	QTextParag *s = doc->createParag( doc, string, n, updateIds );
	s->copyParagData( string );
	s->remove( 0, 1 );
	s->append( str, TRUE );
	for ( uint i = 0; i < str.length(); ++i ) {
	    s->setFormat( i, 1, string->at( idx + i )->format(), TRUE );
	    if ( string->at( idx + i )->isCustom() ) {
		QTextCustomItem * item = string->at( idx + i )->customItem();
		s->at( i )->setCustomItem( item );
		string->at( idx + i )->loseCustomItem();
#if 0
		s->addCustomItem();
		string->removeCustomItem();
#endif
		doc->unregisterCustomItem( item, string );
		doc->registerCustomItem( item, s );
	    }
	}
	string->truncate( idx );
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    string = s;
	    idx = ni;
	} else {
	    string = s;
	    idx = 0;
	}
    }

    invalidateNested();
}

bool QTextCursor::remove()
{
    tmpIndex = -1;
    if ( !atParagEnd() ) {
	string->remove( idx, 1 );
	int h = string->rect().height();
	string->format( -1, TRUE );
	if ( h != string->rect().height() )
	    invalidateNested();
	else if ( doc && doc->parent() )
	    doc->nextDoubleBuffered = TRUE;
	return FALSE;
    } else if ( string->next() ) {
	string->join( string->next() );
	invalidateNested();
	return TRUE;
    }
    return FALSE;
}

void QTextCursor::killLine()
{
    if ( atParagEnd() )
	return;
    string->remove( idx, string->length() - idx - 1 );
    int h = string->rect().height();
    string->format( -1, TRUE );
    if ( h != string->rect().height() )
	invalidateNested();
    else if ( doc && doc->parent() )
	doc->nextDoubleBuffered = TRUE;
}

void QTextCursor::indent()
{
    int oi = 0, ni = 0;
    string->indent( &oi, &ni );
    if ( oi == ni )
	return;

    if ( idx >= oi )
	idx += ni - oi;
    else
	idx = ni;
}

void QTextCursor::setDocument( QTextDocument *d )
{
    doc = d;
    string = d->firstParag();
    idx = 0;
    nested = FALSE;
    restoreState();
    tmpIndex = -1;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextDocument::QTextDocument( QTextDocument *p )
    : par( p ), parParag( 0 ), tc( 0 ), tArray( 0 ), tStopWidth( 0 )
{
    fCollection = new QTextFormatCollection;
    init();
}

QTextDocument::QTextDocument( QTextDocument *p, QTextFormatCollection *c )
    : par( p ), parParag( 0 ), tc( 0 ), tArray( 0 ), tStopWidth( 0 )
{
    fCollection = c;
    init();
}

void QTextDocument::init()
{
#if defined(PARSER_DEBUG)
    qDebug( "new QTextDocument (%p)", this );
#endif
    if ( par )
	par->insertChild( this );
    pProcessor = 0;
    useFC = TRUE;
    pFormatter = 0;
    indenter = 0;
    fParag = 0;
    txtFormat = Qt::AutoText;
    preferRichText = FALSE;
    pages = FALSE;
    focusIndicator.parag = 0;
    minw = 0;
    minwParag = 0;
    align = Qt3::AlignAuto;
    nSelections = 1;
    addMargs = FALSE;

    sheet_ = QStyleSheet::defaultSheet();
    factory_ = QMimeSourceFactory::defaultFactory();
    contxt = QString::null;
    fCollection->setStyleSheet( sheet_ );

    linkC = Qt::blue;
    underlLinks = TRUE;
    backBrush = 0;
    buf_pixmap = 0;
    nextDoubleBuffered = FALSE;

    if ( par )
	withoutDoubleBuffer = par->withoutDoubleBuffer;
    else
	withoutDoubleBuffer = FALSE;

    lParag = fParag = createParag( this, 0, 0 );
    tmpCursor = 0;

    cx = 0;
    cy = 2;
    if ( par )
	cx = cy = 0;
    cw = 600;
    vw = 0;
    flow_ = new QTextFlow;
    flow_->setWidth( cw );

    selectionColors[ Standard ] = QApplication::palette().color( QPalette::Active, QColorGroup::Highlight );
    selectionText[ Standard ] = TRUE;
    commandHistory = new QTextCommandHistory( 100 );
    tStopWidth = formatCollection()->defaultFormat()->width( 'x' ) * 8;
}

QTextDocument::~QTextDocument()
{
    if ( par )
	par->removeChild( this );
    clear();
    delete commandHistory;
    delete flow_;
    if ( !par )
	delete pFormatter;
    delete fCollection;
    delete pProcessor;
    delete buf_pixmap;
    delete indenter;
    delete backBrush;
    if ( tArray )
	delete [] tArray;
}

void QTextDocument::clear( bool createEmptyParag )
{
    if ( flow_ )
	flow_->clear();
    while ( fParag ) {
	QTextParag *p = fParag->next();
	delete fParag;
	fParag = p;
    }
    fParag = lParag = 0;
    if ( createEmptyParag )
	fParag = lParag = createParag( this );
    selections.clear();
}

int QTextDocument::widthUsed() const
{
    QTextParag *p = fParag;
    int w = 0;
    while ( p ) {
	int a = p->alignment();
	p->setAlignment( Qt::AlignLeft );
	p->invalidate( 0 );
	p->format();
	w = QMAX( w, p->rect().width() );
	p->setAlignment( a );
	p->invalidate( 0 );
	p = p->next();
    }
    return w;
}

QTextParag *QTextDocument::createParag( QTextDocument *d, QTextParag *pr, QTextParag *nx, bool updateIds )
{
    return new QTextParag( d, pr, nx, updateIds );
}

bool QTextDocument::setMinimumWidth( int w, QTextParag *p )
{
    if ( w == -1 ) {
	minw = 0;
	p = 0;
    }
    if ( p == minwParag ) {
	minw = w;
	emit minimumWidthChanged( minw );
    } else if ( w > minw ) {
	minw = w;
	minwParag = p;
	emit minimumWidthChanged( minw );
    }
    cw = QMAX( minw, cw );
    return TRUE;
}

void QTextDocument::setPlainText( const QString &text )
{
    clear();
    preferRichText = FALSE;

    QString s;
    lParag = 0;
    QStringList lst = QStringList::split( '\n', text, TRUE );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	lParag = createParag( this, lParag, 0 );
	if ( !fParag )
	    fParag = lParag;
	s = *it;
	if ( !s.isEmpty() ) {
	    if ( s[ (int)s.length() - 1 ] == '\r' )
		s.remove( s.length() - 1, 1 );
	    lParag->append( s );
	}
    }

    if ( !lParag )
	lParag = fParag = createParag( this, 0, 0 );
}

struct Q_EXPORT Tag {
    Tag(){}
    Tag( const QString&n, const QStyleSheetItem* s ):name(n),style(s) {
	wsm = QStyleSheetItem::WhiteSpaceNormal;
    }
    QString name;
    const QStyleSheetItem* style;
    QStyleSheetItem::WhiteSpaceMode wsm;
    QTextFormat format;

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const Tag& ) const { return FALSE; }
#endif
};

#define NEWPAR       if ( !curpar || ( curtag.name != "table" && curtag.name != "li" ) || curpar->length() > 1 ) { if ( !hasNewPar ) curpar = createParag( this, curpar ); \
		    hasNewPar = TRUE;  \
		    QPtrVector<QStyleSheetItem> vec( tags.count() ); \
		    int i = 0; \
		    for ( QValueStack<Tag>::Iterator it = tags.begin(); it != tags.end(); ++it ) \
			vec.insert( i++, (*it).style );	\
		    curpar->setStyleSheetItems( vec ); }while(FALSE)
#define NEWPAROPEN(nstyle)       if ( !curpar || ( curtag.name != "table" && curtag.name != "li" ) || curpar->length() > 1 )  { if ( !hasNewPar ) curpar = createParag( this, curpar ); \
		    hasNewPar = TRUE;  \
		    QPtrVector<QStyleSheetItem> vec( tags.count()+1 ); \
		    int i = 0; \
		    for ( QValueStack<Tag>::Iterator it = tags.begin(); it != tags.end(); ++it ) \
			vec.insert( i++, (*it).style );	\
		    vec.insert( i, nstyle ); \
		    curpar->setStyleSheetItems( vec ); }while(FALSE)


void QTextDocument::setRichText( const QString &text, const QString &context )
{
    setTextFormat( Qt::RichText );
    if ( !context.isEmpty() )
	setContext( context );
    clear();
    fParag = lParag = createParag( this );
    setRichTextInternal( text );
}

void QTextDocument::setRichTextInternal( const QString &text )
{
    QTextParag* curpar = lParag;
    int pos = 0;
    QValueStack<Tag> tags;
    Tag curtag( "", sheet_->item("") );
    curtag.format = *formatCollection()->defaultFormat();
    bool space = FALSE;

    QString doc = text;
    int depth = 0;
    bool hasNewPar = TRUE;
    while ( pos < int( doc.length() ) ) {
	if (hasPrefix(doc, pos, '<' ) ){
	    if (!hasPrefix(doc, pos+1, QChar('/'))) {
		// open tag
		QMap<QString, QString> attr;
		bool emptyTag = FALSE;
		QString tagname = parseOpenTag(doc, pos, attr, emptyTag);
		if ( tagname.isEmpty() )
		    continue; // nothing we could do with this, probably parse error
		while ( eat( doc, pos, '\n' ) )
		    ; // eliminate newlines right after openings

		const QStyleSheetItem* nstyle = sheet_->item(tagname);
		if ( nstyle ) {
		    // we might have to close some 'forgotten' tags
		    while ( !nstyle->allowedInContext( curtag.style ) ) {
			QString msg;
			msg.sprintf( "QText Warning: Document not valid ( '%s' not allowed in '%s' #%d)",
				     tagname.ascii(), curtag.style->name().ascii(), pos);
			sheet_->error( msg );
			if ( tags.isEmpty() )
			    break;
			curtag = tags.pop();
			depth--;
		    }
		}

		QTextCustomItem* custom =  0;
		// some well-known empty tags
		if ( tagname == "br" ) {
		    emptyTag = TRUE;
		    NEWPAR;
		}  else if ( tagname == "hr" ) {
		    emptyTag = TRUE;
		    custom = sheet_->tag( tagname, attr, contxt, *factory_ , emptyTag, this );
		    NEWPAR;
		} else if ( tagname == "table" ) {
		    QTextFormat format = curtag.format.makeTextFormat(  nstyle, attr );
		    custom = parseTable( attr, format, doc, pos, curpar );
		    (void ) eatSpace( doc, pos );
		    emptyTag = TRUE;
		} else {
		    custom = sheet_->tag( tagname, attr, contxt, *factory_ , emptyTag, this );
		}

		if ( !nstyle && !custom ) // we have no clue what this tag could be, ignore it
		    continue;

		if ( custom ) {
		    QTextFormat format = curtag.format.makeTextFormat(  nstyle, attr );
		    int index = curpar->length() - 1;
		    if ( index < 0 )
			index = 0;
		    curpar->append( QChar('b') );
		    curpar->setFormat( index, 1, &format );
		    curpar->at( index )->setCustomItem( custom );
		    curpar->addCustomItem();
		    registerCustomItem( custom, curpar );
		} else if ( !emptyTag ) {
		    tags += curtag;
		    if ( nstyle ) {
			// ignore whitespace for inline elements if there was already one
			if ( nstyle->whiteSpaceMode() == QStyleSheetItem::WhiteSpaceNormal
			     && ( space || nstyle->displayMode() != QStyleSheetItem::DisplayInline ) )
			    eatSpace( doc, pos );

			// some styles are not self nesting
			if ( nstyle == curtag.style && !nstyle->selfNesting() )
			    (void) tags.pop();

			if ( curtag.style->displayMode() == QStyleSheetItem::DisplayListItem )
			    hasNewPar = FALSE; // we want empty paragraphs in this case
			if ( nstyle->displayMode() != QStyleSheetItem::DisplayInline )
			    NEWPAROPEN(nstyle);
			if ( nstyle->displayMode() == QStyleSheetItem::DisplayListItem )
			    curpar->setListStyle( curtag.style->listStyle() );
			curtag.style = nstyle;
			curtag.wsm = nstyle->whiteSpaceMode();
			curtag.format = curtag.format.makeTextFormat( nstyle, attr );
			if ( nstyle->displayMode() != QStyleSheetItem::DisplayInline )
			    curpar->setFormat( &curtag.format );
		    }
		    curtag.name = tagname;
		    if ( curtag.name == "a" && attr.find( "name" ) != attr.end() && doc[ pos] == '<' )	// hack to be sure
			doc.insert( pos, " " );						// <a name=".."></a> formats or inserted
		    if ( attr.find( "align" ) != attr.end() &&
			 ( curtag.name == "p" || curtag.name == "li" || curtag.name[ 0 ] == 'h' ) ) {
			if ( *attr.find( "align" ) == "center" )
			    curpar->setAlignment( Qt::AlignCenter );
			else if ( *attr.find( "align" ) == "right" )
			    curpar->setAlignment( Qt::AlignRight );
			else if ( *attr.find( "align" ) == "justify" )
			    curpar->setAlignment( Qt3::AlignJustify );
		    }
		    depth++;
		}
	    } else {
		// close tag
		QString tagname = parseCloseTag( doc, pos );
		if ( tagname.isEmpty() )
		    continue; // nothing we could do with this, probably parse error
		while ( eat( doc, pos, '\n' ) )
		    ; // eliminate newlines right after closings
		if ( !sheet_->item( tagname ) ) // ignore unknown tags
		    continue;
		depth--;
		while ( curtag.name != tagname ) {
		    QString msg;
		    msg.sprintf( "QText Warning: Document not valid ( '%s' not closed before '%s' #%d)",
				 curtag.name.ascii(), tagname.ascii(), pos);
		    sheet_->error( msg );
		    if ( !hasNewPar && curtag.style->displayMode() != QStyleSheetItem::DisplayInline
			 && curtag.wsm == QStyleSheetItem::WhiteSpaceNormal ) {
			eatSpace( doc, pos );
			NEWPAR;
		    }
		    if ( tags.isEmpty() )
			break;
		    curtag = tags.pop();
		    depth--;
		}

		if ( !hasNewPar && curtag.style->displayMode() != QStyleSheetItem::DisplayInline
		     && curtag.wsm == QStyleSheetItem::WhiteSpaceNormal ) {
		    eatSpace( doc, pos );
		    NEWPAR;
		}
		if ( !tags.isEmpty() )
		    curtag = tags.pop();
	    }
	} else {
	    // normal contents
	    QString s;
	    QChar c;
	    bool hadNonSpace = !curpar->string()->toString().simplifyWhiteSpace().isEmpty();
	    while ( pos < int( doc.length() ) && !hasPrefix(doc, pos, '<' ) ){
		c = parseChar( doc, pos, curtag.wsm );
		space = c.isSpace();
		hadNonSpace = hadNonSpace || !space;
		if ( c == '\n' ) // happens in WhiteSpacePre mode
		    break;
		if ( !hadNonSpace && space && curtag.wsm == QStyleSheetItem::WhiteSpaceNormal )
		    continue;
		if ( c == '\r' )
		    continue;
		s += c;
	    }
	    if ( !s.isEmpty() && curtag.style->displayMode() != QStyleSheetItem::DisplayNone ) {
		hasNewPar = FALSE;
		int index = curpar->length() - 1;
		if ( index < 0 )
		    index = 0;
		curpar->append( s );
		curpar->setFormat( index, s.length(), &curtag.format );
	    }
	    if ( c == '\n' )
		NEWPAR;

	}
    }
}

void QTextDocument::setText( const QString &text, const QString &context )
{
    oText = text;
    focusIndicator.parag = 0;
    selections.clear();
    if ( txtFormat == Qt::AutoText && QStyleSheet::mightBeRichText( text ) ||
	 txtFormat == Qt::RichText )
	setRichText( text, context );
    else
	setPlainText( text );
}

QString QTextDocument::plainText( QTextParag *p ) const
{
    if ( !p ) {
	QString buffer;
	QString s;
	QTextParag *p = fParag;
	while ( p ) {
	    s = p->string()->toString();
	    s.remove( s.length() - 1, 1 );
	    if ( p->next() )
		s += "\n";
	    buffer += s;
	    p = p->next();
	}
	return buffer;
    } else {
	return p->string()->toString();
    }
}

static QString align_to_string( const QString &tag, int a )
{
    if ( tag == "p" || tag == "li" || tag[ 0 ] == 'h' ) {
	if ( a & Qt::AlignRight )
	    return " align=right ";
	if ( a & Qt::AlignCenter )
	    return " align=center ";
	if ( a & Qt3::AlignJustify )
	    return " align=justify ";
    }
    return "";
}

QString QTextDocument::richText( QTextParag *p ) const
{
    QString s;
    if ( !p ) {
	p = fParag;
	QPtrVector<QStyleSheetItem> lastItems, items;
	while ( p ) {
	    items = p->styleSheetItems();
	    if ( items.size() ) {
		QStyleSheetItem *item = items[ items.size() - 1 ];
		items.resize( items.size() - 1 );
		if ( items.size() > lastItems.size() ) {
		    for ( int i = lastItems.size(); i < (int)items.size(); ++i ) {
			if ( items[ i ]->name().isEmpty() )
			    continue;
			s += "<" + items[ i ]->name() + align_to_string( items[ i ]->name(), p->alignment() ) + ">";
		    }
		} else {
		    QString end;
		    for ( int i = items.size(); i < (int)lastItems.size(); ++i ) {
			if ( lastItems[ i ]->name().isEmpty() )
			    continue;
			end.prepend( "</" + lastItems[ i ]->name() + ">" );
		    }
		    s += end;
		}
		lastItems = items;
		s += "<" + item->name() + align_to_string( item->name(), p->alignment() ) + ">" +
		     p->richText() + "</" + item->name() + ">\n";
	    } else {
		QString end;
		for ( int i = 0; i < (int)lastItems.size(); ++i ) {
		    if ( lastItems[ i ]->name().isEmpty() )
			continue;
		    end.prepend( "</" + lastItems[ i ]->name() + ">" );
		}
		s += end;
		s += "<p" + align_to_string( "p", p->alignment() ) + ">" + p->richText() + "</p>\n";
		lastItems = items;
	    }
	    p = p->next();
	}
    } else {
	s = p->richText();
    }

    return s;
}

QString QTextDocument::text() const
{
    if ( plainText().simplifyWhiteSpace().isEmpty() )
	return QString::null;
    if ( txtFormat == Qt::AutoText && preferRichText || txtFormat == Qt::RichText )
	return richText();
    return plainText( 0 );
}

QString QTextDocument::text( int parag ) const
{
    QTextParag *p = paragAt( parag );
    if ( !p )
	return QString::null;

    if ( txtFormat == Qt::AutoText && preferRichText || txtFormat == Qt::RichText )
	return richText( p );
    else
	return plainText( p );
}

void QTextDocument::invalidate()
{
    QTextParag *s = fParag;
    while ( s ) {
	s->invalidate( 0 );
	s = s->next();
    }
}

void QTextDocument::selectionStart( int id, int &paragId, int &index )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;
    QTextDocumentSelection &sel = *it;
    paragId = !sel.swapped ? sel.startCursor.parag()->paragId() : sel.endCursor.parag()->paragId();
    index = !sel.swapped ? sel.startCursor.index() : sel.endCursor.index();
}

QTextCursor QTextDocument::selectionStartCursor( int id)
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return QTextCursor( this );
    QTextDocumentSelection &sel = *it;
    if ( sel.swapped )
	return sel.endCursor;
    return sel.startCursor;
}

QTextCursor QTextDocument::selectionEndCursor( int id)
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return QTextCursor( this );
    QTextDocumentSelection &sel = *it;
    if ( !sel.swapped )
	return sel.endCursor;
    return sel.startCursor;
}

void QTextDocument::selectionEnd( int id, int &paragId, int &index )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;
    QTextDocumentSelection &sel = *it;
    paragId = sel.swapped ? sel.startCursor.parag()->paragId() : sel.endCursor.parag()->paragId();
    index = sel.swapped ? sel.startCursor.index() : sel.endCursor.index();
}

QTextParag *QTextDocument::selectionStart( int id )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return 0;
    QTextDocumentSelection &sel = *it;
    if ( sel.startCursor.parag()->paragId() <  sel.endCursor.parag()->paragId() )
	return sel.startCursor.parag();
    return sel.endCursor.parag();
}

QTextParag *QTextDocument::selectionEnd( int id )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return 0;
    QTextDocumentSelection &sel = *it;
    if ( sel.startCursor.parag()->paragId() >  sel.endCursor.parag()->paragId() )
	return sel.startCursor.parag();
    return sel.endCursor.parag();
}

void QTextDocument::addSelection( int id )
{
    nSelections = QMAX( nSelections, id + 1 );
}

bool QTextDocument::setSelectionEnd( int id, QTextCursor *cursor )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;

    QTextDocumentSelection &sel = *it;

    QTextCursor start = sel.startCursor;
    QTextCursor end = *cursor;

    if ( sel.endCursor.parag() == end.parag() ) {
	QTextCursor c1 = start;
	QTextCursor c2 = end;
	if ( sel.swapped ) {
	    c1 = end;
	    c2 = start;
	}

	c1.parag()->removeSelection( id );
	c2.parag()->removeSelection( id );
	if ( c1.parag() != c2.parag() ) {
	    c1.parag()->setSelection( id, c1.index(), c1.parag()->length() - 1 );
	    c2.parag()->setSelection( id, 0, c2.index() );
	} else {
	    c1.parag()->setSelection( id, QMIN( c1.index(), c2.index() ), QMAX( c1.index(), c2.index() ) );
	}

	sel.startCursor = start;
	sel.endCursor = end;
	if ( sel.startCursor.parag() == sel.endCursor.parag() )
	    sel.swapped = sel.startCursor.index() > sel.endCursor.index();

	return TRUE;
    }

    bool inSelection = FALSE;
    QTextCursor c( this );
    QTextCursor tmp = sel.startCursor;
    if ( sel.swapped )
	tmp = sel.endCursor;
    tmp.restoreState();
    QTextCursor tmp2 = *cursor;
    tmp2.restoreState();
    c.setParag( tmp.parag()->paragId() < tmp2.parag()->paragId() ? tmp.parag() : tmp2.parag() );
    QTextCursor old;
    bool hadStart = FALSE;
    bool hadEnd = FALSE;
    bool hadStartParag = FALSE;
    bool hadEndParag = FALSE;
    bool hadOldStart = FALSE;
    bool hadOldEnd = FALSE;
    bool leftSelection = FALSE;
    sel.swapped = FALSE;
    while ( TRUE ) {
	if ( c == start )
	    hadStart = TRUE;
	if ( c == end )
	    hadEnd = TRUE;
	if ( c.parag() == start.parag() )
	    hadStartParag = TRUE;
	if ( c.parag() == end.parag() )
	    hadEndParag = TRUE;
	if ( c == sel.startCursor )
	    hadOldStart = TRUE;
	if ( c == sel.endCursor )
	    hadOldEnd = TRUE;

	if ( !sel.swapped &&
	     ( hadEnd && !hadStart ||
	       hadEnd && hadStart && start.parag() == end.parag() && start.index() > end.index() ) )
	    sel.swapped = TRUE;

	if ( c == end && hadStartParag ||
	     c == start && hadEndParag ) {
	    QTextCursor tmp = c;
	    tmp.restoreState();
	    if ( tmp.parag() != c.parag() ) {
		int sstart = tmp.parag()->selectionStart( id );
		tmp.parag()->removeSelection( id );
		tmp.parag()->setSelection( id, sstart, tmp.index() );
	    }
	}

	if ( inSelection &&
	     ( c == end && hadStart || c == start && hadEnd ) )
	     leftSelection = TRUE;
	else if ( !leftSelection && !inSelection && ( hadStart || hadEnd ) )
	    inSelection = TRUE;

	bool noSelectionAnymore = hadOldStart && hadOldEnd && leftSelection && !inSelection && !c.parag()->hasSelection( id ) && c.atParagEnd();
	if ( !c.parag()->hasChanged() ) {
	    c.parag()->removeSelection( id );
	    if ( inSelection ) {
		if ( c.parag() == start.parag() && start.parag() == end.parag() ) {
		    c.parag()->setSelection( id, QMIN( start.index(), end.index() ), QMAX( start.index(), end.index() ) );
		} else if ( c.parag() == start.parag() && !hadEndParag ) {
		    c.parag()->setSelection( id, start.index(), c.parag()->length() - 1 );
		} else if ( c.parag() == end.parag() && !hadStartParag ) {
		    c.parag()->setSelection( id, end.index(), c.parag()->length() - 1 );
		} else if ( c.parag() == end.parag() && hadEndParag ) {
		    c.parag()->setSelection( id, 0, end.index() );
		} else if ( c.parag() == start.parag() && hadStartParag ) {
		    c.parag()->setSelection( id, 0, start.index() );
		} else {
		    c.parag()->setSelection( id, 0, c.parag()->length() - 1 );
		}
	    }
	}

	if ( leftSelection )
	    inSelection = FALSE;

	old = c;
	c.gotoRight();
	if ( old == c || noSelectionAnymore )
	    break;
    }

    if ( !sel.swapped )
	sel.startCursor.parag()->setSelection( id, sel.startCursor.index(), sel.startCursor.parag()->length() - 1 );

    sel.startCursor = start;
    sel.endCursor = end;
    if ( sel.startCursor.parag() == sel.endCursor.parag() )
	sel.swapped = sel.startCursor.index() > sel.endCursor.index();

    return TRUE;
}

void QTextDocument::selectAll( int id )
{
    removeSelection( id );

    QTextDocumentSelection sel;
    sel.swapped = FALSE;
    QTextCursor c( this );

    c.setParag( fParag );
    c.setIndex( 0 );
    sel.startCursor = c;

    c.setParag( lParag );
    c.setIndex( lParag->length() - 1 );
    sel.endCursor = c;

    QTextParag *p = fParag;
    while ( p ) {
	p->setSelection( id, 0, p->length() - 1 );
	p = p->next();
    }

    selections.insert( id, sel );
}

bool QTextDocument::removeSelection( int id )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;

    QTextDocumentSelection &sel = *it;

    QTextCursor c( this );
    QTextCursor tmp = sel.startCursor;
    if ( sel.swapped )
	tmp = sel.endCursor;
    tmp.restoreState();
    c.setParag( tmp.parag() );
    QTextCursor old;
    bool hadStart = FALSE;
    bool hadEnd = FALSE;
    QTextParag *lastParag = 0;
    bool leftSelection = FALSE;
    bool inSelection = FALSE;
    sel.swapped = FALSE;
    while ( TRUE ) {
	if ( c.parag() == sel.startCursor.parag() )
	    hadStart = TRUE;
	if ( c.parag() == sel.endCursor.parag() )
	    hadEnd = TRUE;

	if ( inSelection &&
	     ( c == sel.endCursor && hadStart || c == sel.startCursor && hadEnd ) )
	     leftSelection = TRUE;
	else if ( !leftSelection && !inSelection && ( c.parag() == sel.startCursor.parag() || c.parag() == sel.endCursor.parag() ) )
	    inSelection = TRUE;

	bool noSelectionAnymore = leftSelection && !inSelection && !c.parag()->hasSelection( id ) && c.atParagEnd();

	if ( lastParag != c.parag() )
	    c.parag()->removeSelection( id );

	old = c;
	lastParag = c.parag();
	c.gotoRight();
	if ( old == c || noSelectionAnymore )
	    break;
    }

    selections.remove( id );
    return TRUE;
}

QString QTextDocument::selectedText( int id ) const
{
    // ######## TODO: look at textFormat() and return rich text or plain text (like the text() method!)
    QMap<int, QTextDocumentSelection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return QString::null;

    QTextDocumentSelection sel = *it;

    QTextCursor c1 = sel.startCursor;
    QTextCursor c2 = sel.endCursor;
    if ( sel.swapped ) {
	c2 = sel.startCursor;
	c1 = sel.endCursor;
    }

    c2.restoreState();
    c1.restoreState();

    if ( c1.parag() == c2.parag() )
	return c1.parag()->string()->toString().mid( c1.index(), c2.index() - c1.index() );

    QString s;
    s += c1.parag()->string()->toString().mid( c1.index() ) + "\n";
    QTextParag *p = c1.parag()->next();
    while ( p && p != c2.parag() ) {
	s += p->string()->toString() + "\n";
	p = p->next();
    }
    s += c2.parag()->string()->toString().left( c2.index() );
    return s;
}

void QTextDocument::setFormat( int id, QTextFormat *f, int flags )
{
    QMap<int, QTextDocumentSelection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    QTextDocumentSelection sel = *it;

    QTextCursor c1 = sel.startCursor;
    QTextCursor c2 = sel.endCursor;
    if ( sel.swapped ) {
	c2 = sel.startCursor;
	c1 = sel.endCursor;
    }

    c2.restoreState();
    c1.restoreState();

    if ( c1.parag() == c2.parag() ) {
	c1.parag()->setFormat( c1.index(), c2.index() - c1.index(), f, TRUE, flags );
	return;
    }

    c1.parag()->setFormat( c1.index(), c1.parag()->length() - c1.index(), f, TRUE, flags );
    QTextParag *p = c1.parag()->next();
    while ( p && p != c2.parag() ) {
	p->setFormat( 0, p->length() - 1, f, TRUE, flags );
	p = p->next();
    }
    c2.parag()->setFormat( 0, c2.index(), f, TRUE, flags );
}

void QTextDocument::copySelectedText( int id )
{
#ifndef QT_NO_CLIPBOARD
    if ( !hasSelection( id ) )
	return;

    QApplication::clipboard()->setText( selectedText( id ) );
#endif
}

void QTextDocument::removeSelectedText( int id, QTextCursor *cursor )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    QTextDocumentSelection sel = *it;

    QTextCursor c1 = sel.startCursor;
    QTextCursor c2 = sel.endCursor;
    if ( sel.swapped ) {
	c2 = sel.startCursor;
	c1 = sel.endCursor;
    }

    // ### no support for editing tables yet
    if ( c1.nestedDepth() || c2.nestedDepth() )
	return;

    c2.restoreState();
    c1.restoreState();

    *cursor = c1;
    removeSelection( id );

    if ( c1.parag() == c2.parag() ) {
	c1.parag()->remove( c1.index(), c2.index() - c1.index() );
	return;
    }

    if (  c1.index() == 0 )
	cursor->gotoLeft();

    c1.parag()->remove( c1.index(), c1.parag()->length() - c1.index() );
    QTextParag *p = c1.parag()->next();
    int dy = 0;
    QTextParag *tmp;
    while ( p && p != c2.parag() ) {
	tmp = p->next();
	dy -= p->rect().height();
	delete p;
	p = tmp;
    }
    c2.parag()->remove( 0, c2.index() );
    while ( p ) {
	p->move( dy );
	p->invalidate( 0 );
	p->setEndState( -1 );
	p = p->next();
    }

    c1.parag()->join( c2.parag() );
}

void QTextDocument::indentSelection( int id )
{
    QMap<int, QTextDocumentSelection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    QTextDocumentSelection sel = *it;
    QTextParag *startParag = sel.startCursor.parag();
    QTextParag *endParag = sel.endCursor.parag();
    if ( sel.endCursor.parag()->paragId() < sel.startCursor.parag()->paragId() ) {
	endParag = sel.startCursor.parag();
	startParag = sel.endCursor.parag();
    }

    QTextParag *p = startParag;
    while ( p && p != endParag ) {
	p->indent();
	p = p->next();
    }
}

void QTextDocument::addCommand( QTextCommand *cmd )
{
    commandHistory->addCommand( cmd );
}

QTextCursor *QTextDocument::undo( QTextCursor *c )
{
    return commandHistory->undo( c );
}

QTextCursor *QTextDocument::redo( QTextCursor *c )
{
    return commandHistory->redo( c );
}

bool QTextDocument::find( const QString &expr, bool cs, bool wo, bool forward,
			      int *parag, int *index, QTextCursor *cursor )
{
    QTextParag *p = forward ? fParag : lParag;
    if ( parag )
	p = paragAt( *parag );
    else if ( cursor )
	p = cursor->parag();
    bool first = TRUE;

    while ( p ) {
	QString s = p->string()->toString();
	s.remove( s.length() - 1, 1 ); // get rid of trailing space
	int start = forward ? 0 : s.length() - 1;
	if ( first && index )
	    start = *index;
	else if ( first )
	    start = cursor->index();
	first = FALSE;
	int res = forward ? s.find( expr, start, cs ) : s.findRev( expr, start, cs );
	if ( res != -1 ) {
	    bool ok = TRUE;
	    if ( wo ) {
		int end = res + expr.length();
		if ( ( res == 0 || s[ res ].isSpace() || s[ res ].isPunct() ) &&
		     ( end == (int)s.length() - 1 || s[ end ].isSpace() || s[ end ].isPunct() ) )
		    ok = TRUE;
		else
		    ok = FALSE;
	    }
	    if ( ok ) {
		cursor->setParag( p );
		cursor->setIndex( res );
		setSelectionStart( Standard, cursor );
		cursor->setIndex( res + expr.length() );
		setSelectionEnd( Standard, cursor );
		if ( parag )
		    *parag = p->paragId();
		if ( index )
		    *index = res;
		return TRUE;
	    }
	}
	p = forward ? p->next() : p->prev();
    }

    return FALSE;
}

void QTextDocument::setTextFormat( Qt::TextFormat f )
{
    txtFormat = f;
}

Qt::TextFormat QTextDocument::textFormat() const
{
    return txtFormat;
}

bool QTextDocument::inSelection( int selId, const QPoint &pos ) const
{
    QMap<int, QTextDocumentSelection>::ConstIterator it = selections.find( selId );
    if ( it == selections.end() )
	return FALSE;

    QTextDocumentSelection sel = *it;
    QTextParag *startParag = sel.startCursor.parag();
    QTextParag *endParag = sel.endCursor.parag();
    if ( sel.startCursor.parag() == sel.endCursor.parag() &&
	 sel.startCursor.parag()->selectionStart( selId ) == sel.endCursor.parag()->selectionEnd( selId ) )
	return FALSE;
    if ( sel.endCursor.parag()->paragId() < sel.startCursor.parag()->paragId() ) {
	endParag = sel.startCursor.parag();
	startParag = sel.endCursor.parag();
    }

    QTextParag *p = startParag;
    while ( p ) {
	if ( p->rect().contains( pos ) ) {
	    bool inSel = FALSE;
	    int selStart = p->selectionStart( selId );
	    int selEnd = p->selectionEnd( selId );
	    int y = 0;
	    int h = 0;
	    for ( int i = 0; i < p->length(); ++i ) {
		if ( i == selStart )
		    inSel = TRUE;
		if ( i == selEnd )
		    break;
		if ( p->at( i )->lineStart ) {
		    y = (*p->lineStarts.find( i ))->y;
		    h = (*p->lineStarts.find( i ))->h;
		}
		if ( pos.y() - p->rect().y() >= y && pos.y() - p->rect().y() <= y + h ) {
		    if ( inSel && pos.x() >= p->at( i )->x &&
			 pos.x() <= p->at( i )->x + p->at( i )->format()->width( p->at( i )->c ) )
			return TRUE;
		}
	    }
	}
	if ( pos.y() < p->rect().y() )
	    break;
	if ( p == endParag )
	    break;
	p = p->next();
    }

    return FALSE;
}

void QTextDocument::doLayout( QPainter *p, int w )
{
    withoutDoubleBuffer = ( p != 0 );
    flow_->setWidth( w );
    cw = w;
    vw = w;
    if ( !par && is_printer( p ) )
	fCollection->setPainter( p );
    QTextParag *parag = fParag;
    while ( parag ) {
	parag->invalidate( 0 );
	if ( is_printer( p ) )
	    parag->setPainter( p );
	parag->format();
	parag = parag->next();
    }
    if ( !par && is_printer( p ) ) {
	fCollection->setPainter( 0 );
	parag = fParag;
	while ( parag ) {
	    parag->setPainter( 0 );
	    parag = parag->next();
	}
    }
}

QPixmap *QTextDocument::bufferPixmap( const QSize &s )
{
    if ( !buf_pixmap ) {
	buf_pixmap = new QPixmap( s );
    } else {
	if ( buf_pixmap->width() < s.width() ||
	     buf_pixmap->height() < s.height() ) {
	    buf_pixmap->resize( QMAX( s.width(), buf_pixmap->width() ),
				QMAX( s.height(), buf_pixmap->height() ) );
	}
    }

    return buf_pixmap;
}

void QTextDocument::draw( QPainter *p, const QRegion &reg, const QColorGroup &cg, const QBrush *paper )
{
    if ( !firstParag() )
	return;

    if ( paper ) {
//QT2HACK
//	p->setBrushOrigin( -(int)p->translationX(),
//			   -(int)p->translationY() );
	p->setBrushOrigin( -(int)p->worldMatrix().dx(),
			   -(int)p->worldMatrix().dy() );
	p->fillRect( reg.boundingRect(), *paper );
    }

    QTextParag *parag = firstParag();
    QRect cr;
    if ( !reg.isNull() )
	cr = reg.boundingRect();
    while ( parag ) {
	if ( !parag->isValid() )
	    parag->format();
	int y = parag->rect().y();
	QRect pr( parag->rect() );
	pr.setX( 0 );
	pr.setWidth( QWIDGETSIZE_MAX );
	if ( !reg.isNull() && !cr.isNull() && !cr.intersects( pr ) ) {
	    parag = parag->next();
	    continue;
	}
	p->translate( 0, y );
	parag->paint( *p, cg, 0, FALSE );
	p->translate( 0, -y );
	parag = parag->next();
    }
}

void QTextDocument::drawParag( QPainter *p, QTextParag *parag, int cx, int cy, int cw, int ch,
			       QPixmap *&doubleBuffer, const QColorGroup &cg,
			       bool drawCursor, QTextCursor *cursor, bool resetChanged )
{
    QPainter *painter = 0;
    if ( resetChanged )
	parag->setChanged( FALSE );
    QRect ir( parag->rect() );
    bool useDoubleBuffer = !parag->document()->parent();
    if ( !useDoubleBuffer && parag->document()->nextDoubleBuffered )
	useDoubleBuffer = TRUE;
    if ( p->device()->devType() == QInternal::Printer )
	useDoubleBuffer = FALSE;

    if ( useDoubleBuffer  ) {
	painter = new QPainter;
	if ( cx >= 0 && cy >= 0 )
	    ir = ir.intersect( QRect( cx, cy, cw, ch ) );
	if ( !doubleBuffer ||
	     ir.width() > doubleBuffer->width() ||
	     ir.height() > doubleBuffer->height() ) {
	    doubleBuffer = bufferPixmap( ir.size() );
	    painter->begin( doubleBuffer );
	} else {
	    painter->begin( doubleBuffer );
	}
    } else {
	painter = p;
	painter->translate( ir.x(), ir.y() );
    }

    painter->setBrushOrigin( -ir.x(), -ir.y() );

    if ( useDoubleBuffer ) {
	painter->fillRect( QRect( 0, 0, ir.width(), ir.height() ),
		       cg.brush( QColorGroup::Base ) );
    } else {
	if ( cursor && cursor->parag() == parag ) {
	    painter->fillRect( QRect( parag->at( cursor->index() )->x, 0, 2, ir.height() ),
			       cg.brush( QColorGroup::Base ) );
	}
    }

    painter->translate( -( ir.x() - parag->rect().x() ),
		       -( ir.y() - parag->rect().y() ) );
    parag->paint( *painter, cg, drawCursor ? cursor : 0, TRUE, cx, cy, cw, ch );
    if ( !flow()->isEmpty() ) {
	painter->translate( 0, -parag->rect().y() );
	QRect cr( cx, cy, cw, ch );
	cr = cr.intersect( QRect( 0, parag->rect().y(), parag->rect().width(), parag->rect().height() ) );
	flow()->drawFloatingItems( painter, cr.x(), cr.y(), cr.width(), cr.height(), cg );
	painter->translate( 0, +parag->rect().y() );
    }

    if ( useDoubleBuffer ) {
	delete painter;
	painter = 0;
	p->drawPixmap( ir.topLeft(), *doubleBuffer, QRect( QPoint( 0, 0 ), ir.size() ) );
    } else {
	painter->translate( -ir.x(), -ir.y() );
    }

    if ( parag->rect().x() + parag->rect().width() < parag->document()->x() + parag->document()->width() ) {
	p->fillRect( parag->rect().x() + parag->rect().width(), parag->rect().y(),
		     ( parag->document()->x() + parag->document()->width() ) -
		     ( parag->rect().x() + parag->rect().width() ),
		     parag->rect().height(), cg.brush( QColorGroup::Base ) );
    }

    if ( verticalBreak() && parag->lastInFrame && parag->document()->flow() )
	parag->document()->flow()->eraseAfter( parag, p, cg );

    parag->document()->nextDoubleBuffered = FALSE;
}

QTextParag *QTextDocument::draw( QPainter *p, int cx, int cy, int cw, int ch, const QColorGroup &cg,
				 bool onlyChanged, bool drawCursor, QTextCursor *cursor, bool resetChanged )
{
    if ( withoutDoubleBuffer || par && par->withoutDoubleBuffer ) {
	withoutDoubleBuffer = TRUE;
	QRegion rg;
	draw( p, rg, cg );
	return 0;
    }
    withoutDoubleBuffer = FALSE;

    if ( !firstParag() )
	return 0;

    if ( drawCursor && cursor )
	tmpCursor = cursor;
    if ( cx < 0 && cy < 0 ) {
	cx = 0;
	cy = 0;
	cw = width();
	ch = height();
    }

    QTextParag *lastFormatted = 0;
    QTextParag *parag = firstParag();

    QPixmap *doubleBuffer = 0;
    QPainter painter;

    while ( parag ) {
	//qDebug("draw: formatting %d", parag->paragId());
	lastFormatted = parag;
	if ( !parag->isValid() )
	    parag->format();

	if ( !parag->rect().intersects( QRect( cx, cy, cw, ch ) ) ) {
	    QRect pr( parag->rect() );
	    pr.setWidth( parag->document()->width() );
	    if ( pr.intersects( QRect( cx, cy, cw, ch ) ) )
		p->fillRect( pr.intersect( QRect( cx, cy, cw, ch ) ), cg.brush( QColorGroup::Base ) );
	    if ( parag->rect().y() > cy + ch ) {
		tmpCursor = 0;
		if ( buf_pixmap && buf_pixmap->height() > 300 ) {
		    delete buf_pixmap;
		    buf_pixmap = 0;
		}
		if ( verticalBreak() && flow() )
		    flow()->draw( p, cx, cy, cw, ch );

		//qDebug("draw: we're past cy+ch=%d returning lastFormatted=%d", cy+ch, parag->paragId());
		return lastFormatted;
	    }
	    parag = parag->next();
	    continue;
	}

	if ( !parag->hasChanged() && onlyChanged ) {
	    parag = parag->next();
	    continue;
	}

	drawParag( p, parag, cx, cy, cw, ch, doubleBuffer, cg, drawCursor, cursor, resetChanged );
	parag = parag->next();
    }

    parag = lastParag();
    if ( parag->rect().y() + parag->rect().height() < parag->document()->height() ) {
	p->fillRect( 0, parag->rect().y() + parag->rect().height(), parag->document()->width(),
		     parag->document()->height() - ( parag->rect().y() + parag->rect().height() ),
		     cg.brush( QColorGroup::Base ) );
	if ( !flow()->isEmpty() ) {
	    QRect cr( cx, cy, cw, ch );
	    cr = cr.intersect( QRect( 0, parag->rect().y() + parag->rect().height(), parag->document()->width(),
				      parag->document()->height() - ( parag->rect().y() + parag->rect().height() ) ) );
	    flow()->drawFloatingItems( p, cr.x(), cr.y(), cr.width(), cr.height(), cg );
	}
    }

    if ( buf_pixmap && buf_pixmap->height() > 300 ) {
	delete buf_pixmap;
	buf_pixmap = 0;
    }

    if ( verticalBreak() && flow() )
	flow()->draw( p, cx, cy, cw, ch );

    tmpCursor = 0;
    //qDebug("draw: all done. returning lastFormatted=%d", parag->paragId());
    return lastFormatted;
}

void QTextDocument::setDefaultFont( const QFont &f )
{
    updateFontSizes( f.pointSize() );
}

void QTextDocument::registerCustomItem( QTextCustomItem *i, QTextParag *p )
{
    if ( i && i->placement() != QTextCustomItem::PlaceInline ) {
	flow_->registerFloatingItem( i, i->placement() == QTextCustomItem::PlaceRight );
	p->registerFloatingItem( i );
    }
    i->setParagraph( p );
    customItems.append( i );
}

void QTextDocument::unregisterCustomItem( QTextCustomItem *i, QTextParag *p )
{
    flow_->unregisterFloatingItem( i );
    p->unregisterFloatingItem( i );
    customItems.removeRef( i );
    i->setParagraph( 0L );
}

bool QTextDocument::focusNextPrevChild( bool next )
{
    if ( !focusIndicator.parag ) {
	if ( next ) {
	    focusIndicator.parag = fParag;
	    focusIndicator.start = 0;
	    focusIndicator.len = 0;
	} else {
	    focusIndicator.parag = lParag;
	    focusIndicator.start = lParag->length();
	    focusIndicator.len = 0;
	}
    } else {
	focusIndicator.parag->setChanged( TRUE );
    }
    focusIndicator.href = QString::null;

    if ( next ) {
	QTextParag *p = focusIndicator.parag;
	int index = focusIndicator.start + focusIndicator.len;
	while ( p ) {
	    for ( int i = index; i < p->length(); ++i ) {
		if ( p->at( i )->format()->isAnchor() ) {
		    p->setChanged( TRUE );
		    focusIndicator.parag = p;
		    focusIndicator.start = i;
		    focusIndicator.len = 0;
		    focusIndicator.href = p->at( i )->format()->anchorHref();
		    while ( i < p->length() ) {
			if ( !p->at( i )->format()->isAnchor() )
			    return TRUE;
			focusIndicator.len++;
			i++;
		    }
		}
	    }
	    index = 0;
	    p = p->next();
	}
    } else {
	QTextParag *p = focusIndicator.parag;
	int index = focusIndicator.start - 1;
	while ( p ) {
	    for ( int i = index; i >= 0; --i ) {
		if ( p->at( i )->format()->isAnchor() ) {
		    p->setChanged( TRUE );
		    focusIndicator.parag = p;
		    focusIndicator.start = i;
		    focusIndicator.len = 0;
		    focusIndicator.href = p->at( i )->format()->anchorHref();
		    while ( i >= -1 ) {
			if ( i < 0 || !p->at( i )->format()->isAnchor() ) {
			    focusIndicator.start++;
			    return TRUE;
			}
			if ( i < 0 )
			    break;
			focusIndicator.len++;
			focusIndicator.start--;
			i--;
		    }
		}
	    }
	    p = p->prev();
	    if ( p )
		index = p->length() - 1;
	}
    }

    return FALSE;
}

int QTextDocument::length() const
{
    int l = 0;
    QTextParag *p = fParag;
    while ( p ) {
	l += p->length();
	p = p->next();
    }
    return l;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int QTextFormat::width( const QChar &c ) const
{
    if ( c.unicode() == 0xad ) // soft hyphen
	return 0;
    if ( !painter || !painter->isActive() ) {
	if ( c == '\t' )
	    return fm.width( 'x' ) * 8;
	if ( ha == AlignNormal ) {
	    int w;
	    if ( c.row() )
		w = fm.width( c );
	    else
		w = widths[ c.unicode() ];
	    if ( w == 0 && !c.row() ) {
		w = fm.width( c );
		( (QTextFormat*)this )->widths[ c.unicode() ] = w;
	    }
	    return w;
	} else {
	    QFont f( fn );
	    f.setPointSize( ( f.pointSize() * 2 ) / 3 );
	    QFontMetrics fm_( f );
	    return fm_.width( c );
	}
    }

    QFont f( fn );
    if ( ha != AlignNormal )
	f.setPointSize( ( f.pointSize() * 2 ) / 3 );
    painter->setFont( f );

    return painter->fontMetrics().width( c );
}

int QTextFormat::width( const QString &str, int pos ) const
{
    int w;
    if ( str[ pos ].unicode() == 0xad )
	return 0;
    if ( !painter || !painter->isActive() ) {
	if ( ha == AlignNormal ) {
//          w = fm.charWidth( str, pos );
            w = fm.width( str[pos] );
	} else {
	    QFont f( fn );
	    f.setPointSize( ( f.pointSize() * 2 ) / 3 );
	    QFontMetrics fm_( f );
            w = fm_.width( str[pos] );
//	    w = fm_.charWidth( str, pos );
	}
    } else {
	QFont f( fn );
	if ( ha != AlignNormal )
	    f.setPointSize( ( f.pointSize() * 2 ) / 3 );
	painter->setFont( f );
	w = painter->fontMetrics().width( str[pos] );
//	w = painter->fontMetrics().charWidth( str, pos );
    }
    return w;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextString::QTextString()
{
    textChanged = FALSE;
    bidi = FALSE;
    rightToLeft = FALSE;
}

QTextString::QTextString( const QTextString &s )
{
    textChanged = s.textChanged;
    bidi = s.bidi;
    rightToLeft = s.rightToLeft;
    data = s.subString();
}

void QTextString::insert( int index, const QString &s, QTextFormat *f )
{
    int os = data.size();
    data.resize( data.size() + s.length() );
    if ( index < os ) {
	memmove( data.data() + index + s.length(), data.data() + index,
		 sizeof( QTextStringChar ) * ( os - index ) );
    }
    for ( int i = 0; i < (int)s.length(); ++i ) {
	data[ (int)index + i ].x = 0;
	data[ (int)index + i ].lineStart = 0;
	data[ (int)index + i ].d.format = 0;
	data[ (int)index + i ].type = QTextStringChar::Regular;
	data[ (int)index + i ].rightToLeft = 0;
	data[ (int)index + i ].startOfRun = 0;
#if defined(Q_WS_X11)
	//### workaround for broken courier fonts on X11
	if ( s[ i ] == QChar( 0x00a0U ) )
	    data[ (int)index + i ].c = ' ';
	else
	    data[ (int)index + i ].c = s[ i ];
#else
	data[ (int)index + i ].c = s[ i ];
#endif
#ifdef DEBUG_COLLECTION
	qDebug("QTextString::insert setting format %p to character %d",f,(int)index+i);
#endif
	data[ (int)index + i ].setFormat( f );
    }
    textChanged = TRUE;
}

QTextString::~QTextString()
{
    clear();
}

void QTextString::insert( int index, QTextStringChar *c )
{
    int os = data.size();
    data.resize( data.size() + 1 );
    if ( index < os ) {
	memmove( data.data() + index + 1, data.data() + index,
		 sizeof( QTextStringChar ) * ( os - index ) );
    }
    data[ (int)index ].c = c->c;
    data[ (int)index ].x = 0;
    data[ (int)index ].lineStart = 0;
    data[ (int)index ].rightToLeft = 0;
    data[ (int)index ].d.format = 0;
    data[ (int)index ].type = QTextStringChar::Regular;
    data[ (int)index ].setFormat( c->format() );
    textChanged = TRUE;
}

void QTextString::truncate( int index )
{
    index = QMAX( index, 0 );
    index = QMIN( index, (int)data.size() - 1 );
    if ( index < (int)data.size() ) {
	for ( int i = index + 1; i < (int)data.size(); ++i ) {
	    if ( data[ i ].isCustom() ) {
		delete data[ i ].customItem();
		if ( data[ i ].d.custom->format )
		    data[ i ].d.custom->format->removeRef();
		data[ i ].d.custom = 0;
	    } else if ( data[ i ].format() ) {
		data[ i ].format()->removeRef();
	    }
	}
    }
    data.truncate( index );
    textChanged = TRUE;
}

void QTextString::remove( int index, int len )
{
    for ( int i = index; i < (int)data.size() && i - index < len; ++i ) {
	if ( data[ i ].isCustom() ) {
	    delete data[ i ].customItem();
	    if ( data[ i ].d.custom->format )
		data[ i ].d.custom->format->removeRef();
	    data[ i ].d.custom = 0;
	} else if ( data[ i ].format() ) {
	    data[ i ].format()->removeRef();
	}
    }
    memmove( data.data() + index, data.data() + index + len,
	     sizeof( QTextStringChar ) * ( data.size() - index - len ) );
    data.resize( data.size() - len );
    textChanged = TRUE;
}

void QTextString::clear()
{
    for ( int i = 0; i < (int)data.count(); ++i ) {
	if ( data[ i ].isCustom() ) {
	    delete data[ i ].customItem();
	    if ( data[ i ].d.custom->format )
		data[ i ].d.custom->format->removeRef();
	    delete data[ i ].d.custom;
	    data[ i ].d.custom = 0;
	} else if ( data[ i ].format() ) {
	    data[ i ].format()->removeRef();
	}
    }
    data.resize( 0 );
}

void QTextString::setFormat( int index, QTextFormat *f, bool useCollection )
{
//    qDebug("QTextString::setFormat index=%d f=%p",index,f);
    if ( useCollection && data[ index ].format() )
    {
        //qDebug("QTextString::setFormat removing ref on old format %p",data[ index ].format());
	data[ index ].format()->removeRef();
    }
    data[ index ].setFormat( f );
}

void QTextString::checkBidi() const
{
    int len = data.size();
    const QTextStringChar *c = data.data();
    ((QTextString *)this)->bidi = FALSE;
    ((QTextString *)this)->rightToLeft = FALSE;
    while( len ) {
	uchar row = c->c.row();
	if( (row > 0x04 && row < 0x09) || row > 0xfa ) {
	    ((QTextString *)this)->bidi = TRUE;
	    basicDirection();
	    return;
	}
	len--;
	++c;
    }
}

void QTextString::basicDirection() const
{
    int pos = 0;
    ((QTextString *)this)->rightToLeft = FALSE;
    while( pos < length() ) {
	switch( at(pos).c.direction() )
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    return;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    ((QTextString *)this)->rightToLeft = TRUE;
	    return;
	default:
	    break;
	}
	++pos;
    }
    return;
}


void QTextDocument::setStyleSheet( QStyleSheet *s )
{
    if ( !s )
	return;
    sheet_ = s;
    fCollection->setStyleSheet( s );
    updateStyles();
}

void QTextDocument::updateStyles()
{
    invalidate();
    fCollection->updateStyles();
    for ( QTextDocument *d = childList.first(); d; d = childList.next() )
	d->updateStyles();
}

void QTextDocument::updateFontSizes( int base )
{
    for ( QTextDocument *d = childList.first(); d; d = childList.next() )
	d->updateFontSizes( base );
    invalidate();
    fCollection->updateFontSizes( base );
}

void QTextDocument::updateFontAttributes( const QFont &f, const QFont &old )
{
    for ( QTextDocument *d = childList.first(); d; d = childList.next() )
	d->updateFontAttributes( f, old );
    invalidate();
    fCollection->updateFontAttributes( f, old );
}

void QTextStringChar::setFormat( QTextFormat *f )
{
    if ( type == Regular ) {
	d.format = f;
    } else {
	if ( !d.custom ) {
	    d.custom = new CustomData;
	    d.custom->custom = 0;
	}
	d.custom->format = f;
    }
}

void QTextStringChar::setCustomItem( QTextCustomItem *i )
{
    if ( !isCustom() ) {
	QTextFormat *f = format();
	d.custom = new CustomData;
	d.custom->format = f;
	type = Custom;
    } else {
	delete d.custom->custom;
    }
    d.custom->custom = i;
}

void QTextStringChar::loseCustomItem() // setRegular() might be a better name
{
    if ( isCustom() ) {
	QTextFormat *f = d.custom->format;
	d.custom->custom = 0;
	delete d.custom;
	type = Regular;
	d.format = f;
    }
}

int QTextString::width(int idx) const
{
     int w = 0;
     QTextStringChar *c = &at( idx );
     if ( c->c.unicode() == 0xad )
	 return 0;
     if( c->isCustom() ) {
	 if( c->customItem()->placement() == QTextCustomItem::PlaceInline )
	     w = c->customItem()->width;
     } else if ( c->type == QTextStringChar::Mark ) {
	 return 0;
     } else {
	 int r = c->c.row();
	 if( r < 0x06 || r > 0x1f )
	     w = c->format()->width( c->c );
	 else {
	     // complex text. We need some hacks to get the right metric here
	     QString str;
	     int pos = 0;
	     if( idx > 3 )
		 pos = idx - 3;
	     int off = idx - pos;
	     int end = QMIN( length(), idx + 3 );
	     while ( pos < end ) {
		 str += at(pos).c;
		 pos++;
	     }
	     w = c->format()->width( str, off );
	 }
     }
     return w;
}

QMemArray<QTextStringChar> QTextString::subString( int start, int len ) const
{
    if ( len == 0xFFFFFF )
	len = data.size();
    QMemArray<QTextStringChar> a;
    a.resize( len );
    for ( int i = 0; i < len; ++i ) {
	QTextStringChar *c = &data[ i + start ];
	a[ i ].c = c->c;
	a[ i ].x = 0;
	a[ i ].lineStart = 0;
	a[ i ].rightToLeft = 0;
	a[ i ].d.format = 0;
	a[ i ].type = QTextStringChar::Regular;
	a[ i ].setFormat( c->format() );
	if ( c->format() )
	    c->format()->addRef();
    }
    return a;
}

QTextStringChar *QTextStringChar::clone() const
{
    QTextStringChar *chr = new QTextStringChar;
    chr->c = c;
    chr->x = 0;
    chr->lineStart = 0;
    chr->rightToLeft = 0;
    chr->d.format = 0;
    chr->type = QTextStringChar::Regular;
    chr->setFormat( format() );
    if ( chr->format() )
	chr->format()->addRef();
    return chr;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextParag::QTextParag( QTextDocument *d, QTextParag *pr, QTextParag *nx, bool updateIds )
    : invalid( 0 ), p( pr ), n( nx ), doc( d ), align( -1 ), numSubParag( -1 ),
      tm( -1 ), bm( -1 ), lm( -1 ), rm( -1 ), flm( -1 ), tc( 0 ),
      numCustomItems( 0 ), pFormatter( 0 ),
      tArray( 0 ), tabStopWidth( 0 ), eData( 0 ), pntr( 0 )
{
    visible = TRUE;
    newLinesAllowed = FALSE;
    splittedInside = FALSE;
    lastInFrame = FALSE;
    defFormat = formatCollection()->defaultFormat();
    if ( !doc ) {
	tabStopWidth = defFormat->width( 'x' ) * 8;
	commandHistory = new QTextCommandHistory( 100 );
    }
#if defined(PARSER_DEBUG)
    qDebug( "new QTextParag" );
#endif
    fullWidth = TRUE;

    if ( p ) {
	p->n = this;
	if ( p->tc )
	    tc = p->tc;
    }
    if ( n ) {
	n->p = this;
	if ( n->tc )
	    tc = n->tc;
    }

    if ( !tc && d && d->tableCell() )
	tc = d->tableCell();

    if ( !p && doc )
	doc->setFirstParag( this );
    if ( !n && doc )
	doc->setLastParag( this );

    changed = FALSE;
    firstFormat = TRUE;
    state = -1;
    needPreProcess = FALSE;

    if ( p )
	id = p->id + 1;
    else
	id = 0;
    if ( n && updateIds ) {
	QTextParag *s = n;
	while ( s ) {
	    s->id = s->p->id + 1;
	    s->numSubParag = -1;
	    s->lm = s->rm = s->tm = s->bm = -1, s->flm = -1;
	    s = s->n;
	}
    }
    firstPProcess = TRUE;

    str = new QTextString();
    formatCollection()->defaultFormat()->addRef();
    str->insert( 0, " ", formatCollection()->defaultFormat() );
}

QTextParag::~QTextParag()
{
    //qDebug("QTextParag::~QTextParag %p id=%d",this,paragId());
    delete str;
    if ( doc && p == doc->minwParag ) {
	doc->minwParag = 0;
	doc->minw = 0;
    }
    if ( !doc ) {
	delete pFormatter;
	delete commandHistory;
    }
    if ( tArray )
	delete [] tArray;
    delete eData;
    QMap<int, QTextParagLineStart*>::Iterator it = lineStarts.begin();
    for ( ; it != lineStarts.end(); ++it )
	delete *it;
}

void QTextParag::setNext( QTextParag *s )
{
    n = s;
    if ( !n && doc )
	doc->setLastParag( this );
}

void QTextParag::setPrev( QTextParag *s )
{
    p = s;
    if ( !p && doc )
	doc->setFirstParag( this );
}

void QTextParag::invalidate( int chr )
{
    if ( invalid < 0 )
	invalid = chr;
    else
	invalid = QMIN( invalid, chr );
    for ( QTextCustomItem *i = floatingItems.first(); i; i = floatingItems.next() )
	i->ypos = -1;
    lm = rm = bm = tm = flm = -1;
}

void QTextParag::insert( int index, const QString &s )
{
    if ( doc && !doc->useFormatCollection() && doc->preProcessor() )
	str->insert( index, s,
		     doc->preProcessor()->format( QTextPreProcessor::Standard ) );
    else
	str->insert( index, s, formatCollection()->defaultFormat() );
    invalidate( index );
    needPreProcess = TRUE;
}

void QTextParag::truncate( int index )
{
    str->truncate( index );
    insert( length(), " " );
    needPreProcess = TRUE;
}

void QTextParag::remove( int index, int len )
{
    if ( index + len - str->length() > 0 )
	return;
    for ( int i = index; i < len; ++i ) {
	QTextStringChar *c = at( i );
	if ( doc && c->isCustom() ) {
	    doc->unregisterCustomItem( c->customItem(), this );
	    removeCustomItem();
	}
    }
    str->remove( index, len );
    invalidate( 0 );
    needPreProcess = TRUE;
}

void QTextParag::join( QTextParag *s )
{
    //qDebug("QTextParag::join this=%d (length %d) with %d (length %d)",paragId(),length(),s->paragId(),s->length());
    int oh = r.height() + s->r.height();
    n = s->n;
    if ( n )
	n->p = this;
    else if ( doc )
	doc->setLastParag( this );

  bool hasTrailingSpace = length() > 0 && at( length() - 1 )->c == ' ';
  // keep existing trailing space if we already have one and we're joining to an empty parag
  if ( !hasTrailingSpace || s->length() > 1 )
  {
    int start = str->length();
    if ( hasTrailingSpace ) {
	remove( length() - 1, 1 );
	--start;
    }
    append( s->str->toString(), TRUE );
    //qDebug("copying %d chars", s->length());
    for ( int i = 0; i < s->length(); ++i ) {
	if ( !doc || doc->useFormatCollection() ) {
	    s->str->at( i ).format()->addRef();
	    str->setFormat( i + start, s->str->at( i ).format(), TRUE );
	}
	if ( s->str->at( i ).isCustom() ) {
	    QTextCustomItem * item = s->str->at( i ).customItem();
	    str->at( i + start ).setCustomItem( item );
	    s->str->at( i ).loseCustomItem();
	    doc->unregisterCustomItem( item, s );
	    doc->registerCustomItem( item, this );
	}
    }
  }
  //else
    //qDebug("join: empty parag -> not copying");
  ASSERT(str->at(str->length()-1).c == ' ');

    if ( !extraData() && s->extraData() ) {
	setExtraData( s->extraData() );
	s->setExtraData( 0 );
    } else if ( extraData() && s->extraData() ) {
	extraData()->join( s->extraData() );
    }
    delete s;
    invalidate( 0 );
    r.setHeight( oh );
    needPreProcess = TRUE;
    if ( n ) {
	QTextParag *s = n;
	while ( s ) {
	    s->id = s->p->id + 1;
	    s->state = -1;
	    s->needPreProcess = TRUE;
	    s->changed = TRUE;
	    s = s->n;
	}
    }
    format();
    state = -1;
}

void QTextParag::move( int &dy )
{
    //qDebug("QTextParag::move paragId=%d dy=%d",paragId(),dy);
    if ( dy == 0 )
	return;
    changed = TRUE;
    r.moveBy( 0, dy );
    for ( QTextCustomItem *i = floatingItems.first(); i; i = floatingItems.next() )
	i->ypos += dy;
    if ( p )
	p->lastInFrame = FALSE;
    if ( doc && doc->verticalBreak() ) {
	const int oy = r.y();
	int y = oy;
	doc->flow()->adjustFlow( y, r.width(), r.height(), this, TRUE );
	if ( oy != y ) {
	    if ( p ) {
		p->lastInFrame = TRUE;
		p->setChanged( TRUE );
	    }
	    int oh = r.height();
	    r.setY( y );
	    r.setHeight( oh );
	    dy += y - oy;
            //qDebug("QTextParag::move done. paragId=%d dy=%d lastInFrame=true",paragId(),dy);
	}
    }
}

void QTextParag::format( int start, bool doMove )
{
    if ( str->length() == 0 || !formatter() )
	return;

    if ( doc &&
	 doc->preProcessor() &&
	 ( needPreProcess || state == -1 ) )
	doc->preProcessor()->process( doc, this, invalid <= 0 ? 0 : invalid );
    needPreProcess = FALSE;

    if ( invalid == -1 )
	return;

    //qDebug("QTextParag::format id=%d invalid, formatting (moving after previous parag)",paragId());
    r.moveTopLeft( QPoint( documentX(), p ? p->r.y() + p->r.height() : documentY() ) );
    r.setWidth( documentWidth() );
    if ( p )
	p->lastInFrame = FALSE;
formatAgain:
    if ( doc ) {
	for ( QTextCustomItem *i = floatingItems.first(); i; i = floatingItems.next() ) {
	    i->ypos = r.y();
	    if ( i->placement() == QTextCustomItem::PlaceRight )
		i->xpos = r.x() + r.width() - i->width;
	    doc->flow()->updateHeight( i );
	}
    }
    QMap<int, QTextParagLineStart*> oldLineStarts = lineStarts;
    lineStarts.clear();
    int y = formatter()->format( doc, this, start, oldLineStarts );
    r.setWidth( QMAX( r.width(), minimumWidth() ) );
    QMap<int, QTextParagLineStart*>::Iterator it = oldLineStarts.begin();
    for ( ; it != oldLineStarts.end(); ++it )
	delete *it;

    QTextStringChar *c = 0;
    if ( lineStarts.count() == 1 && ( !doc || doc->flow()->isEmpty() ) && !string()->isBidi() ) {
	c = &str->at( str->length() - 1 );
	r.setWidth( c->x + str->width( str->length() - 1 ) );
    }

    if ( newLinesAllowed ) {
	it = lineStarts.begin();
	int usedw = 0;
	for ( ; it != lineStarts.end(); ++it )
	    usedw = QMAX( usedw, (*it)->w );
	r.setWidth( QMIN( usedw, r.width() ) );
    }

    if ( y != r.height() )
	r.setHeight( y );

    if ( !visible )
	r.setHeight( 0 );

    splittedInside = FALSE;
    if ( doc && doc->verticalBreak() ) {
	const int oy = r.y();
	int y = oy;
	doc->flow()->adjustFlow( y, r.width(), r.height(), this, TRUE );
	if ( oy != y ) {
	    if ( p ) {
		p->lastInFrame = TRUE;
		p->setChanged( TRUE );
	    }
	    int oh = r.height();
	    r.setY( y );
	    r.setHeight( oh );
	    goto formatAgain;
	}

    }

    if ( n && doMove && n->invalid == -1 && r.y() + r.height() != n->r.y() ) {
	int dy = ( r.y() + r.height() ) - n->r.y();
	QTextParag *s = n;
        bool makeInvalid = p && p->lastInFrame;
        //qDebug("moving. previous's lastInFrame (=makeInvalid): %d",makeInvalid);
	while ( s && dy ) {
	    if ( !s->isFullWidth() )
		makeInvalid = TRUE;
	    if ( makeInvalid )
		s->invalidate( 0 );
	    s->move( dy );
	    if ( s->lastInFrame )
		makeInvalid = TRUE;
	    s = s->n;
	}
    }

    firstFormat = FALSE;
    changed = TRUE;
    invalid = -1;
    string()->setTextChanged( FALSE );
}

int QTextParag::lineHeightOfChar( int i, int *bl, int *y ) const
{
    if ( !isValid() )
	( (QTextParag*)this )->format();

    QMap<int, QTextParagLineStart*>::ConstIterator it = lineStarts.end();
    --it;
    for ( ;; ) {
	if ( i >= it.key() ) {
	    if ( bl )
		*bl = ( *it )->baseLine;
	    if ( y )
		*y = ( *it )->y;
	    return ( *it )->h;
	}
	if ( it == lineStarts.begin() )
	    break;
	--it;
    }

    qWarning( "QTextParag::lineHeightOfChar: couldn't find lh for %d", i );
    return 15;
}

QTextStringChar *QTextParag::lineStartOfChar( int i, int *index, int *line ) const
{
    if ( !isValid() )
	( (QTextParag*)this )->format();

    int l = lineStarts.count() - 1;
    QMap<int, QTextParagLineStart*>::ConstIterator it = lineStarts.end();
    --it;
    for ( ;; ) {
	if ( i >= it.key() ) {
	    if ( index )
		*index = it.key();
	    if ( line )
		*line = l;
	    return &str->at( it.key() );
	}
	if ( it == lineStarts.begin() )
	    break;
	--it;
	--l;
    }

    qWarning( "QTextParag::lineStartOfChar: couldn't find %d", i );
    return 0;
}

int QTextParag::lines() const
{
    if ( !isValid() )
	( (QTextParag*)this )->format();

    return lineStarts.count();
}

QTextStringChar *QTextParag::lineStartOfLine( int line, int *index ) const
{
    if ( !isValid() )
	( (QTextParag*)this )->format();

    if ( line >= 0 && line < (int)lineStarts.count() ) {
	QMap<int, QTextParagLineStart*>::ConstIterator it = lineStarts.begin();
	while ( line-- > 0 )
	    ++it;
	int i = it.key();
	if ( index )
	    *index = i;
	return &str->at( i );
    }

    qWarning( "QTextParag::lineStartOfLine: couldn't find %d", line );
    return 0;
}

void QTextParag::setFormat( int index, int len, QTextFormat *f, bool useCollection, int flags )
{
    if ( index < 0 )
	index = 0;
    if ( index > str->length() - 1 )
	index = str->length() - 1;
    if ( index + len >= str->length() )
	len = str->length() - index;

    QTextFormatCollection *fc = 0;
    if ( useCollection )
	fc = formatCollection();
    QTextFormat *of;
    for ( int i = 0; i < len; ++i ) {
	of = str->at( i + index ).format();
	if ( !changed && f->key() != of->key() )
	    changed = TRUE;
	if ( invalid == -1 &&
	     ( f->font().family() != of->font().family() ||
	       f->font().pointSize() != of->font().pointSize() ||
	       f->font().weight() != of->font().weight() ||
	       f->font().italic() != of->font().italic() ||
	       f->vAlign() != of->vAlign() ) ) {
	    invalidate( 0 );
	}
	if ( flags == -1 || flags == QTextFormat::Format || !fc ) {
#ifdef DEBUG_COLLECTION
	    qDebug(" QTextParag::setFormat, will use format(f) %p %s", f, f->key().latin1());
#endif
	    if ( fc )
		f = fc->format( f );
	    str->setFormat( i + index, f, useCollection );
	} else {
#ifdef DEBUG_COLLECTION
	    qDebug(" QTextParag::setFormat, will use format(of,f,flags) ");
#endif
	    QTextFormat *fm = fc->format( of, f, flags );
#ifdef DEBUG_COLLECTION
	    qDebug(" QTextParag::setFormat, format(of,f,flags) returned %p %s ", fm,fm->key().latin1());
#endif
	    str->setFormat( i + index, fm, useCollection );
	}
    }
}

void QTextParag::indent( int *oldIndent, int *newIndent )
{
    if ( !doc || !doc->indent() || style() && style()->displayMode() != QStyleSheetItem::DisplayBlock ) {
	if ( oldIndent )
	    *oldIndent = 0;
	if ( newIndent )
	    *newIndent = 0;
	if ( oldIndent && newIndent )
	    *newIndent = *oldIndent;
	return;
    }
    doc->indent()->indent( doc, this, oldIndent, newIndent );
}

void QTextParag::paint( QPainter &painter, const QColorGroup &cg, QTextCursor *cursor, bool drawSelections,
			int clipx, int clipy, int clipw, int cliph )
{
    if ( !visible )
	return;
    QTextStringChar *chr = at( 0 );
    ASSERT( chr );
    if (!chr) { qDebug("paragraph %p %d, can't paint, EMPTY !", this, paragId()); return; }
    int i = 0;
    int h = 0;
    int baseLine = 0, lastBaseLine = 0;
    QTextFormat *lastFormat = 0;
    int lastY = -1;
    int startX = 0;
    int bw = 0;
    int cy = 0;
    int curx = -1, cury = 0, curh = 0;
    bool lastDirection = chr->rightToLeft;
    QTextStringChar::Type lastType = chr->type;
    int tw = 0;

    QString qstr = str->toString();

    const int nSels = doc ? doc->numSelections() : 1;
    QMemArray<int> selectionStarts( nSels );
    QMemArray<int> selectionEnds( nSels );
    if ( drawSelections ) {
	bool hasASelection = FALSE;
	for ( i = 0; i < nSels; ++i ) {
	    if ( !hasSelection( i ) ) {
		selectionStarts[ i ] = -1;
		selectionEnds[ i ] = -1;
	    } else {
		hasASelection = TRUE;
		selectionStarts[ i ] = selectionStart( i );
		int end = selectionEnd( i );
		if ( end == length() - 1 && n && n->hasSelection( i ) )
		    end++;
		selectionEnds[ i ] = end;
	    }
	}
	if ( !hasASelection )
	    drawSelections = FALSE;
    }

    int line = -1;
    int cw;
    bool didListLabel = FALSE;
    int paintStart = 0;
    int paintEnd = -1;
    int lasth = 0;
    for ( i = 0; i < length(); i++ ) {
	chr = at( i );
	if ( !str->isBidi() && is_printer( &painter ) ) { // ### fix our broken ps-printer
	    if ( !chr->lineStart )
		chr->x = QMAX( chr->x, tw );
	    else
		tw = 0;
	}
	cw = string()->width( i );
	if ( chr->c == '\t' && i < length() - 1 )
	    cw = at( i + 1 )->x - chr->x + 1;
	if ( chr->c.unicode() == 0xad && i < length() - 1 )
	    cw = 0;

	// init a new line
	if ( chr->lineStart ) {
	    tw = 0;
	    ++line;
	    lineInfo( line, cy, h, baseLine );
	    lasth = h;
	    if ( clipy != -1 && cy > clipy - r.y() + cliph ) // outside clip area, leave
		break;
	    if ( lastBaseLine == 0 )
		lastBaseLine = baseLine;
	}

	// draw bullet list items
	if ( !didListLabel && line == 0 && style() && style()->displayMode() == QStyleSheetItem::DisplayListItem ) {
	    didListLabel = TRUE;
	    drawLabel( &painter, chr->x, cy, 0, 0, baseLine, cg );
	}

	// check for cursor mark
	if ( cursor && this == cursor->parag() && i == cursor->index() ) {
	    curx = chr->x;
	    if ( chr->rightToLeft )
		curx += cw;
	    curh = h;
	    cury = cy;
	}

	// first time - start again...
	if ( !lastFormat || lastY == -1 ) {
	    lastFormat = chr->format();
	    lastY = cy;
	    startX = chr->x;
	    if ( !chr->isCustom() && chr->c != '\n' )
		paintEnd = i;
	    bw = cw;
	    if ( !chr->isCustom() )
		continue;
	}

	// check if selection state changed
	bool selectionChange = FALSE;
	if ( drawSelections ) {
	    for ( int j = 0; j < nSels; ++j ) {
		selectionChange = selectionStarts[ j ] == i || selectionEnds[ j ] == i;
		if ( selectionChange )
		    break;
	    }
	}

	//if something (format, etc.) changed, draw what we have so far
	if ( ( ( ( alignment() & Qt3::AlignJustify ) == Qt3::AlignJustify && at(paintEnd)->c.isSpace() ) ||
	       lastDirection != (bool)chr->rightToLeft ||
	       chr->startOfRun ||
	       lastY != cy || chr->format() != lastFormat ||
	       ( paintEnd != -1 && at( paintEnd )->c =='\t' ) || chr->c == '\t' ||
	       ( paintEnd != -1 && at( paintEnd )->c.unicode() == 0xad ) || chr->c.unicode() == 0xad ||
	       selectionChange || chr->isCustom() ) ) {
	    if ( paintStart <= paintEnd ) {
		// ### temporary hack until I get the new placement/shaping stuff working
		int x = startX;
		if ( lastType == QTextStringChar::Mark && i > 0 ) {
		    if ( !lastDirection )
			x += str->at(i - 1).d.mark->xoff;
		    else if ( i > 1 )
			x -= str->at(i - 1).d.mark->xoff + str->width( i - 2 );
		}
		drawParagString( painter, qstr, paintStart, paintEnd - paintStart + 1, x, lastY,
				 lastBaseLine, bw, lasth, drawSelections,
				 lastFormat, i, selectionStarts, selectionEnds, cg, lastDirection );
	    }
	    if ( !str->isBidi() && is_printer( &painter ) ) { // ### fix our broken ps-printer
		if ( !chr->lineStart ) {
		    // ### the next line doesn't look 100% correct for arabic
		    tw = startX + painter.fontMetrics().width( qstr.mid(paintStart, paintEnd - paintStart +1) );
		    chr->x = QMAX( chr->x, tw );
		} else {
		    tw = 0;
		}
	    }
	    if ( !chr->isCustom() ) {
		if ( chr->c != '\n' ) {
		    paintStart = i;
		    paintEnd = i;
		} else {
		    paintStart = i+1;
		    paintEnd = -1;
		}
		lastFormat = chr->format();
		lastY = cy;
		startX = chr->x;
		bw = cw;
	    } else {
		if ( chr->customItem()->placement() == QTextCustomItem::PlaceInline ) {
		    chr->customItem()->draw( &painter, chr->x, cy, clipx - r.x(), clipy - r.y(), clipw, cliph, cg );
		    paintStart = i+1;
		    paintEnd = -1;
		    lastFormat = chr->format();
		    lastY = cy;
		    startX = chr->x + string()->width( i );
		    bw = 0;
		} else {
		    chr->customItem()->resize( pntr, chr->customItem()->width );
		    paintStart = i+1;
		    paintEnd = -1;
		    lastFormat = chr->format();
		    lastY = cy;
		    startX = chr->x + string()->width( i );
		    bw = 0;
		}
	    }
	} else {
	    if ( chr->c != '\n' ) {
		if( chr->rightToLeft ) {
		    startX = chr->x;
		}
		paintEnd = i;
	    }
	    bw += cw;
	}
	lastBaseLine = baseLine;
	lasth = h;
	lastDirection = chr->rightToLeft;
	lastType = chr->type;
    }

    // if we are through the parag, but still have some stuff left to draw, draw it now
    if ( paintStart <= paintEnd ) {
	bool selectionChange = FALSE;
	if ( drawSelections ) {
	    for ( int j = 0; j < nSels; ++j ) {
		selectionChange = selectionStarts[ j ] == i || selectionEnds[ j ] == i;
		if ( selectionChange )
		    break;
	    }
	}
	// ### temporary hack until I get the new placement/shaping stuff working
	int x = startX;
	if ( lastType == QTextStringChar::Mark && i > 0 ) {
	    if ( !lastDirection )
		x += str->at(i - 1).d.mark->xoff;
	    else if ( i > 1 )
		x -= str->at(i - 1).d.mark->xoff + str->width( i - 2 );
	}
	drawParagString( painter, qstr, paintStart, paintEnd-paintStart+1, x, lastY,
			 lastBaseLine, bw, h, drawSelections,
			 lastFormat, i, selectionStarts, selectionEnds, cg, lastDirection );
    }

    // if we should draw a cursor, draw it now
    if ( curx != -1 && cursor ) {
	painter.fillRect( QRect( curx, cury, 1, curh - lineSpacing() ), Qt::black );
	painter.save();
	if ( string()->isBidi() ) {
	    const int d = 4;
	    if ( at( cursor->index() )->rightToLeft ) {
		painter.setPen( Qt::black );
		painter.drawLine( curx, cury, curx - d / 2, cury + d / 2 );
		painter.drawLine( curx, cury + d, curx - d / 2, cury + d / 2 );
	    } else {
		painter.setPen( Qt::black );
		painter.drawLine( curx, cury, curx + d / 2, cury + d / 2 );
		painter.drawLine( curx, cury + d, curx + d / 2, cury + d / 2 );
	    }
	}
	painter.restore();
    }
}

void QTextParag::drawParagString( QPainter &painter, const QString &s, int start, int len, int startX,
				      int lastY, int baseLine, int bw, int h, bool drawSelections,
				      QTextFormat *lastFormat, int i, const QMemArray<int> &selectionStarts,
				      const QMemArray<int> &selectionEnds, const QColorGroup &cg, bool /*rightToLeft*/ )
{
    QString str( s );
    if ( str[ (int)str.length() - 1 ].unicode() == 0xad )
	str.remove( str.length() - 1, 1 );
    painter.setPen( QPen( lastFormat->color() ) );
    painter.setFont( lastFormat->font() );

    if ( doc && lastFormat->isAnchor() && !lastFormat->anchorHref().isEmpty() && lastFormat->useLinkColor() ) {
	painter.setPen( QPen( Qt::blue /* cg.link() */ ) ); // QT2HACK
	if ( doc->underlineLinks() ) {
	    QFont fn = lastFormat->font();
	    fn.setUnderline( TRUE );
	    painter.setFont( fn );
	}
    }

    if ( drawSelections ) {
	const int nSels = doc ? doc->numSelections() : 1;
	const int startSel = painter.device()->devType() != QInternal::Printer ? 0 : 1;
	for ( int j = startSel; j < nSels; ++j ) {
	    if ( i > selectionStarts[ j ] && i <= selectionEnds[ j ] ) {
		if ( !doc || doc->invertSelectionText( j ) )
		    painter.setPen( QPen( cg.color( QColorGroup::HighlightedText ) ) );
		if ( j == QTextDocument::Standard )
		    painter.fillRect( startX, lastY, bw, h, cg.color( QColorGroup::Highlight ) );
		else
		    painter.fillRect( startX, lastY, bw, h, doc ? doc->selectionColor( j ) : cg.color( QColorGroup::Highlight ) );
	    }
	}
    }
    //QT2HACK
    //QPainter::TextDirection dir = QPainter::LTR;
    //if ( rightToLeft )
	//dir = QPainter::RTL;
    if ( str[start] != '\t' && str[ start ].unicode() != 0xad ) {
	if ( lastFormat->vAlign() == QTextFormat::AlignNormal ) {
            //QT2HACK painter.drawText( startX, lastY + baseLine, str, start, len, dir );
            painter.drawText( startX, lastY + baseLine, str.mid(start), len );
	} else if ( lastFormat->vAlign() == QTextFormat::AlignSuperScript ) {
	    QFont f( painter.font() );
	    f.setPointSize( ( f.pointSize() * 2 ) / 3 );
	    painter.setFont( f );
	    //QT2HACK painter.drawText( startX, lastY + baseLine - ( h - painter.fontMetrics().height() ), str, start, len, dir );
	    painter.drawText( startX, lastY + baseLine - ( h - painter.fontMetrics().height() ), str.mid(start), len );
	} else if ( lastFormat->vAlign() == QTextFormat::AlignSubScript ) {
	    QFont f( painter.font() );
	    f.setPointSize( ( f.pointSize() * 2 ) / 3 );
	    painter.setFont( f );
	    //QT2HACK painter.drawText( startX, lastY + baseLine, str, start, len, dir );
	    painter.drawText( startX, lastY + baseLine, str.mid(start), len );
	}
    }
    if ( i + 1 < length() && at( i + 1 )->lineStart && at( i )->c.unicode() == 0xad ) {
	painter.drawText( startX + bw, lastY + baseLine, "\xad" );
    }
    if ( lastFormat->isMisspelled() ) {
	painter.save();
	painter.setPen( QPen( Qt::red, 1, Qt::DotLine ) );
	painter.drawLine( startX, lastY + baseLine + 1, startX + bw, lastY + baseLine + 1 );
	painter.restore();
    }

    i -= len;
    if ( doc && lastFormat->isAnchor() && !lastFormat->anchorHref().isEmpty() &&
	 doc->focusIndicator.parag == this &&
	 doc->focusIndicator.start >= i &&
	 doc->focusIndicator.start + doc->focusIndicator.len <= i + len ) {
	painter.drawWinFocusRect( QRect( startX, lastY, bw, h ) );
    }

}

void QTextParag::drawLabel( QPainter* p, int x, int y, int w, int h, int base, const QColorGroup& cg )
{
    if ( !style() )
	return;
    QRect r ( x, y, w, h );
    QStyleSheetItem::ListStyle s = listStyle();

    QFont font = p->font();
    p->setFont( defFormat->font() );
    QFontMetrics fm( p->fontMetrics() );
    int size = fm.lineSpacing() / 3;

    switch ( s ) {
    case QStyleSheetItem::ListDecimal:
    case QStyleSheetItem::ListLowerAlpha:
    case QStyleSheetItem::ListUpperAlpha:
	{
	    int n = numberOfSubParagraph();
	    QString l;
	    switch ( s ) {
	    case QStyleSheetItem::ListLowerAlpha:
		if ( n < 27 ) {
		    l = QChar( ('a' + (char) (n-1)));
		    break;
		}
	    case QStyleSheetItem::ListUpperAlpha:
		if ( n < 27 ) {
		    l = QChar( ('A' + (char) (n-1)));
		    break;
		}
		break;
	    default:  //QStyleSheetItem::ListDecimal:
		l.setNum( n );
		break;
	    }
	    l += QString::fromLatin1(". ");
	    p->drawText( r.right() - fm.width( l ), r.top() + base, l );
	}
	break;
    case QStyleSheetItem::ListSquare:
	{
	    QRect er( r.right() - size * 2, r.top() + base - fm.boundingRect( 'A' ).height() / 2 - size / 2 - 1, size, size );
	    p->fillRect( er , cg.brush( QColorGroup::Foreground ) );
	}
	break;
    case QStyleSheetItem::ListCircle:
	{
	    QRect er( r.right()-size*2, r.top() + base - fm.boundingRect('A').height()/2 - size/2 - 1, size, size);
	    p->drawEllipse( er );
	}
	break;
    case QStyleSheetItem::ListDisc:
    default:
	{
	    p->setBrush( cg.brush( QColorGroup::Foreground ));
	    QRect er( r.right()-size*2, r.top() + base - fm.boundingRect('A').height()/2 - size/2 - 1, size, size);
	    p->drawEllipse( er );
	    p->setBrush( Qt::NoBrush );
	}
	break;
    }

    p->setFont( font );
}

void QTextParag::setStyleSheetItems( const QPtrVector<QStyleSheetItem> &vec )
{
    styleSheetItemsVec = vec;
    invalidate( 0 );
    lm = rm = tm = bm = flm = -1;
    numSubParag = -1;
}

void QTextParag::setList( bool b, int listStyle )
{
    if ( !doc )
	return;

    if ( !style() ) {
	styleSheetItemsVec.resize( 2 );
	styleSheetItemsVec.insert( 0, doc->styleSheet()->item( "html" ) );
	styleSheetItemsVec.insert( 1, doc->styleSheet()->item( "p" ) );
    }

    if ( b ) {
	if ( style()->displayMode() != QStyleSheetItem::DisplayListItem || this->listStyle() != listStyle ) {
	    styleSheetItemsVec.remove( styleSheetItemsVec.size() - 1 );
	    QStyleSheetItem *item = styleSheetItemsVec[ styleSheetItemsVec.size() - 2 ];
	    if ( item )
		styleSheetItemsVec.remove( styleSheetItemsVec.size() - 2 );
	    styleSheetItemsVec.insert( styleSheetItemsVec.size() - 2,
				       listStyle == QStyleSheetItem::ListDisc || listStyle == QStyleSheetItem::ListCircle
				       || listStyle == QStyleSheetItem::ListSquare ?
				       doc->styleSheet()->item( "ul" ) : doc->styleSheet()->item( "ol" ) );
	    styleSheetItemsVec.insert( styleSheetItemsVec.size() - 1, doc->styleSheet()->item( "li" ) );
	    setListStyle( (QStyleSheetItem::ListStyle)listStyle );
	} else {
	    return;
	}
    } else {
	if ( style()->displayMode() != QStyleSheetItem::DisplayBlock ) {
	    styleSheetItemsVec.remove( styleSheetItemsVec.size() - 1 );
	    if ( styleSheetItemsVec.size() >= 2 ) {
		styleSheetItemsVec.remove( styleSheetItemsVec.size() - 2 );
		styleSheetItemsVec.resize( styleSheetItemsVec.size() - 2 );
	    } else {
		styleSheetItemsVec.resize( styleSheetItemsVec.size() - 1 );
	    }
	} else {
	    return;
	}
    }
    invalidate( 0 );
    lm = rm = tm = bm = flm = -1;
    numSubParag = -1;
    if ( next() ) {
	QTextParag *s = next();
	while ( s ) {
	    s->numSubParag = -1;
	    s->lm = s->rm = s->tm = s->bm = flm = -1;
	    s->numSubParag = -1;
	    s->invalidate( 0 );
	    s = s->next();
	}
    }
}

void QTextParag::incDepth()
{
    if ( !style() || !doc )
	return;
    if ( style()->displayMode() != QStyleSheetItem::DisplayListItem )
	return;
    styleSheetItemsVec.resize( styleSheetItemsVec.size() + 1 );
    styleSheetItemsVec.insert( styleSheetItemsVec.size() - 1, styleSheetItemsVec[ styleSheetItemsVec.size() - 2 ] );
    styleSheetItemsVec.insert( styleSheetItemsVec.size() - 2,
			       listStyle() == QStyleSheetItem::ListDisc || listStyle() == QStyleSheetItem::ListCircle ||
			       listStyle() == QStyleSheetItem::ListSquare ?
			       doc->styleSheet()->item( "ul" ) : doc->styleSheet()->item( "ol" ) );
    invalidate( 0 );
    lm = -1;
    flm = -1;
}

void QTextParag::decDepth()
{
    if ( !style() || !doc )
	return;
    if ( style()->displayMode() != QStyleSheetItem::DisplayListItem )
	return;
    int numLists = 0;
    QStyleSheetItem *lastList = 0;
    int lastIndex = 0;
    int i;
    for ( i = 0; i < (int)styleSheetItemsVec.size(); ++i ) {
	QStyleSheetItem *item = styleSheetItemsVec[ i ];
	if ( item->name() == "ol" || item->name() == "ul" ) {
	    lastList = item;
	    lastIndex = i;
	    numLists++;
	}
    }

    if ( !lastList )
	return;
    styleSheetItemsVec.remove( lastIndex );
    for ( i = lastIndex; i < (int)styleSheetItemsVec.size() - 1; ++i )
	styleSheetItemsVec.insert( i, styleSheetItemsVec[ i + 1 ] );
    styleSheetItemsVec.resize( styleSheetItemsVec.size() - 1 );
    if ( numLists == 1 )
	setList( FALSE, -1 );
    invalidate( 0 );
    lm = -1;
    flm = -1;
}

int QTextParag::nextTab( int, int x )
{
    int *ta = tArray;
    if ( doc ) {
	if ( !ta )
	   ta = doc->tabArray();
	tabStopWidth = doc->tabStopWidth();
    }
    if ( ta ) {
	int i = 0;
	while ( ta[ i ] ) {
	    if ( ta[ i ] >= x )
		return tArray[ i ];
	    ++i;
	}
	return tArray[ 0 ];
    } else {
	int d;
	if ( tabStopWidth != 0 )
	    d = x / tabStopWidth;
	else
	    return x;
	return tabStopWidth * ( d + 1 );
    }
}

void QTextParag::setPainter( QPainter *p )
{
    pntr = p;
    for ( int i = 0; i < length(); ++i ) {
	if ( at( i )->isCustom() )
	    at( i )->customItem()->adjustToPainter( p );
    }
}

QTextFormatCollection *QTextParag::formatCollection() const
{
    if ( doc )
	return doc->formatCollection();
    if ( !qFormatCollection )
	qFormatCollection = new QTextFormatCollection;
    return qFormatCollection;
}

QString QTextParag::richText() const
{
    QString s;
    QTextFormat *lastFormat = 0;
    for ( int i = 0; i < length(); ++i ) {
	QTextStringChar *c = &str->at( i );
	if ( !lastFormat || ( lastFormat->key() != c->format()->key() && c->c != ' ' ) ) {
	    s += c->format()->makeFormatChangeTags( lastFormat );
	    lastFormat = c->format();
	}
	if ( c->c == '<' ) {
	    s += "&lt;";
	} else if ( c->c == '>' ) {
	    s += "&gt;";
	} else if ( c->isCustom() ) {
	    s += c->customItem()->richText();
	} else {
	    s += c->c;
	}
    }
    return s;
}

void QTextParag::addCommand( QTextCommand *cmd )
{
    if ( !doc )
	commandHistory->addCommand( cmd );
    else
	doc->commands()->addCommand( cmd );
}

QTextCursor *QTextParag::undo( QTextCursor *c )
{
    if ( !doc )
	return commandHistory->undo( c );
    return doc->commands()->undo( c );
}

QTextCursor *QTextParag::redo( QTextCursor *c )
{
    if ( !doc )
	return commandHistory->redo( c );
    return doc->commands()->redo( c );
}

int QTextParag::topMargin() const
{
    if ( !p && ( !doc || !doc->addMargins() ) )
	return 0;
    if ( tm != -1 )
	return tm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->tm = 0;
	return 0;
    }

    int m = 0;
    if ( item->margin( QStyleSheetItem::MarginTop ) != QStyleSheetItem::Undefined )
	m = item->margin( QStyleSheetItem::MarginTop );
    QStyleSheetItem *it = 0;
    QStyleSheetItem *p = prev() ? prev()->style() : 0;
    for ( int i = (int)styleSheetItemsVec.size() - 2 ; i >= 0; --i ) {
	it = styleSheetItemsVec[ i ];
	if ( it != p )
	    break;
	int mar = it->margin( QStyleSheetItem::MarginTop );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
	if ( it->displayMode() != QStyleSheetItem::DisplayInline )
	    break;
    }

    if ( is_printer( painter() ) ) {
	QPaintDeviceMetrics metrics( painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	m = (int)( (double)m * yscale );
    }

    ( (QTextParag*)this )->tm = m;
    return tm;
}

int QTextParag::bottomMargin() const
{
    if ( bm != -1 )
	return bm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->bm = 0;
	return 0;
    }

    int m = 0;
    if ( item->margin( QStyleSheetItem::MarginBottom ) != QStyleSheetItem::Undefined )
	m = item->margin( QStyleSheetItem::MarginBottom );
    QStyleSheetItem *it = 0;
    QStyleSheetItem *n = next() ? next()->style() : 0;
    for ( int i =(int)styleSheetItemsVec.size() - 2 ; i >= 0; --i ) {
	it = styleSheetItemsVec[ i ];
	if ( it != n )
	    break;
	int mar = it->margin( QStyleSheetItem::MarginBottom );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
	if ( it->displayMode() != QStyleSheetItem::DisplayInline )
	    break;
    }

    if ( is_printer( painter() ) ) {
	QPaintDeviceMetrics metrics( painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	m = (int)( (double)m * yscale );
    }

    ( (QTextParag*)this )->bm = m;
    return bm;
}

int QTextParag::leftMargin() const
{
    if ( lm != -1 )
	return lm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->lm = 0;
	return 0;
    }
    int m = 0;
    for ( int i = 0; i < (int)styleSheetItemsVec.size(); ++i ) {
	item = styleSheetItemsVec[ i ];
	int mar = item->margin( QStyleSheetItem::MarginLeft );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
	if ( item->name() == "ol" || item->name() == "ul" ) {
	    m += defFormat->width( '1' ) +
		 defFormat->width( '2' ) +
		 defFormat->width( '3' ) +
		 defFormat->width( '.' );
	}
    }

    if ( is_printer( painter() ) ) {
	QPaintDeviceMetrics metrics( painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	m = (int)( (double)m * yscale );
    }

    ( (QTextParag*)this )->lm = m;
    return lm;
}

int QTextParag::firstLineMargin() const
{
    if ( flm != -1 )
	return lm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->flm = 0;
	return 0;
    }
    int m = 0;
    for ( int i = 0; i < (int)styleSheetItemsVec.size(); ++i ) {
	item = styleSheetItemsVec[ i ];
	int mar = item->margin( QStyleSheetItem::MarginFirstLine );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
    }

    if ( is_printer( painter() ) ) {
	QPaintDeviceMetrics metrics( painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	m = (int)( (double)m * yscale );
    }

    ( (QTextParag*)this )->flm = m;
    return flm;
}

int QTextParag::rightMargin() const
{
    if ( rm != -1 )
	return rm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->rm = 0;
	return 0;
    }
    int m = 0;
    for ( int i = 0; i < (int)styleSheetItemsVec.size(); ++i ) {
	item = styleSheetItemsVec[ i ];
	int mar = item->margin( QStyleSheetItem::MarginRight );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
    }

    if ( is_printer( painter() ) ) {
	QPaintDeviceMetrics metrics( painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	m = (int)( (double)m * yscale );
    }

    ( (QTextParag*)this )->rm = m;
    return rm;
}

int QTextParag::lineSpacing() const
{
    QStyleSheetItem *item = style();
    if ( !item )
	return 0;

    int ls = item->lineSpacing();
    if ( ls == QStyleSheetItem::Undefined )
	return 0;
    if ( is_printer( painter() ) ) {
	QPaintDeviceMetrics metrics( painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	ls = (int)( (double)ls * yscale );
    }

    return ls;
}

void QTextParag::copyParagData( QTextParag *parag )
{
    setStyleSheetItems( parag->styleSheetItems() );
    setListStyle( parag->listStyle() );
    setAlignment( parag->alignment() );
}

void QTextParag::show()
{
    if ( visible || !doc )
	return;
    visible = TRUE;
}

void QTextParag::hide()
{
    if ( !visible || !doc )
	return;
    visible = FALSE;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


QTextPreProcessor::QTextPreProcessor()
{
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextFormatter::QTextFormatter()
    : wrapEnabled( TRUE ), wrapColumn( -1 ), biw( FALSE )
{
}

/* only used for bidi or complex text reordering
 */
QTextParagLineStart *QTextFormatter::formatLine( QTextParag * /*parag*/, QTextString *string, QTextParagLineStart *line,
						   QTextStringChar *startChar, QTextStringChar *lastChar, int align, int space )
{
//QT2HACK
#if 0
    if( string->isBidi() )
	return bidiReorderLine( parag, string, line, startChar, lastChar, align, space );
#endif
    space = QMAX( space, 0 ); // #### with nested tables this gets negative because of a bug I didn't find yet, so workaround for now. This also means non-left aligned nested tables do not work at the moment
    int start = (startChar - &string->at(0));
    int last = (lastChar - &string->at(0) );
    // do alignment Auto == Left in this case
    if ( align & Qt::AlignHCenter || align & Qt::AlignRight ) {
	if ( align & Qt::AlignHCenter )
	    space /= 2;
	for ( int j = start; j <= last; ++j )
	    string->at( j ).x += space;
    } else if ( align & Qt3::AlignJustify ) {
	int numSpaces = 0;
	for ( int j = start; j < last; ++j ) {
	    if( isBreakable( string, j ) ) {
		numSpaces++;
	    }
	}
	int toAdd = 0;
	for ( int k = start + 1; k <= last; ++k ) {
	    if( isBreakable( string, k ) && numSpaces ) {
		int s = space / numSpaces;
		toAdd += s;
		space -= s;
		numSpaces--;
	    }
	    string->at( k ).x += toAdd;
	}
    }

    if ( last >= 0 && last < string->length() )
	line->w = string->at( last ).x + string->width( last ); // #### Lars, I guess this breaks for Bidi
    else
	line->w = 0;

    return new QTextParagLineStart();
}

//QT2HACK
#if 0

#ifdef BIDI_DEBUG
#include <iostream>
#endif

// collects one line of the paragraph and transforms it to visual order
QTextParagLineStart *QTextFormatter::bidiReorderLine( QTextParag *parag, QTextString *text, QTextParagLineStart *line,
							QTextStringChar *startChar, QTextStringChar *lastChar, int align, int space )
{
    int start = (startChar - &text->at(0));
    int last = (lastChar - &text->at(0) );
    //printf("doing BiDi reordering from %d to %d!\n", start, last);

    QBidiControl *control = new QBidiControl( line->context(), line->status );
    QString str;
    str.setUnicode( 0, last - start + 1 );
    // fill string with logically ordered chars.
    QTextStringChar *ch = startChar;
    QChar *qch = (QChar *)str.unicode();
    while( ch <= lastChar ) {
	*qch = ch->c;
	qch++;
	ch++;
    }

    QPtrList<QTextRun> *runs;
    runs = QComplexText::bidiReorderLine(control, str, 0, last - start + 1);

    // now construct the reordered string out of the runs...

    int left = parag->document() ? parag->leftMargin() + 4 : 4;
    int x = left + ( parag->document() ? parag->firstLineMargin() : 0 );
    if ( parag->document() )
	x = parag->document()->flow()->adjustLMargin( parag->rect().y(), parag->rect().height(), left, 4 );
    int numSpaces = 0;
    // set the correct alignment. This is a bit messy....
    if( align == Qt3::AlignAuto ) {
	// align according to directionality of the paragraph...
	if ( text->isRightToLeft() )
	    align = Qt::AlignRight;
    }

    if ( align & Qt::AlignHCenter )
	x += space/2;
    else if ( align & Qt::AlignRight )
	x += space;
    else if ( align & Qt3::AlignJustify ) {
	for ( int j = start; j < last; ++j ) {
	    if( isBreakable( text, j ) ) {
		numSpaces++;
	    }
	}
    }
    int toAdd = 0;

    // in rtl text the leftmost character is usually a space
    // this space should not take up visible space on the left side, to get alignment right.
    // the following bool is used for that purpose
    bool first = TRUE;
    QTextRun *r = runs->first();
    while ( r ) {
	if(r->level %2) {
	    // odd level, need to reverse the string
	    int pos = r->stop + start;
	    while(pos >= r->start + start) {
		QTextStringChar *c = &text->at(pos);
		if( numSpaces && !first && isBreakable( text, pos ) ) {
		    int s = space / numSpaces;
		    toAdd += s;
		    space -= s;
		    numSpaces--;
		}
		if ( first ) {
		    first = FALSE;
		    if ( c->c == ' ' )
			x -= text->width( pos );
		}
		c->x = x + toAdd;
		c->rightToLeft = TRUE;
		c->startOfRun = FALSE;
		int ww = 0;
		if ( c->c.unicode() >= 32 || c->c == '\t' || c->isCustom() ) {
		    ww = text->width( pos );
		} else {
		    ww = c->format()->width( ' ' );
		}
		//qDebug("setting char %d at pos %d width=%d", pos, x, ww);
		x += ww;
		pos--;
	    }
	} else {
	    int pos = r->start + start;
	    while(pos <= r->stop + start) {
		QTextStringChar* c = &text->at(pos);
		if( numSpaces && !first && isBreakable( text, pos ) ) {
		    int s = space / numSpaces;
		    toAdd += s;
		    space -= s;
		    numSpaces--;
		}
		if ( first ) {
		    first = FALSE;
		    if ( c->c == ' ' )
			x -= text->width( pos );
		}
		c->x = x + toAdd;
		c->rightToLeft = FALSE;
		c->startOfRun = FALSE;
		int ww = 0;
		if ( c->c.unicode() >= 32 || c->c == '\t' || c->isCustom() ) {
		    ww = text->width( pos );
		} else {
		    ww = c->format()->width( ' ' );
		}
		//qDebug("setting char %d at pos %d", pos, x);
		x += ww;
		pos++;
	    }
	}
	text->at( r->start ).startOfRun = TRUE;
	r = runs->next();
    }
    QTextParagLineStart *ls = new QTextParagLineStart( control->context, control->status );
    delete control;
    delete runs;
    return ls;
}
#endif

bool QTextFormatter::isBreakable( QTextString *string, int pos ) const
{
    const QChar &c = string->at( pos ).c;
    char ch = c.latin1();
    if ( c.isSpace() && ch != '\n' )
	return TRUE;
    if ( c.unicode() == 0xad ) // soft hyphen
	return TRUE;
    if ( !ch ) {
	// not latin1, need to do more sophisticated checks for other scripts
	uchar row = c.row();
	if ( row == 0x0e ) {
	    // 0e00 - 0e7f == Thai
	    if ( c.cell() < 0x80 ) {
#ifdef HAVE_THAI_BREAKS
		// check for thai
		if( string != cachedString ) {
		    // build up string of thai chars
		    QTextCodec *thaiCodec = QTextCodec::codecForMib(2259);
		    if ( !thaiCache )
			thaiCache = new QCString;
		    if ( !thaiIt )
			thaiIt = ThBreakIterator::createWordInstance();
		    *thaiCache = thaiCodec->fromUnicode( s->string() );
		}
		thaiIt->setText(thaiCache->data());
		for(int i = thaiIt->first(); i != thaiIt->DONE; i = thaiIt->next() ) {
		    if( i == pos )
			return TRUE;
		    if( i > pos )
			return FALSE;
		}
		return FALSE;
#else
		// if we don't have a thai line breaking lib, allow
		// breaks everywhere except directly before punctuation.
		return TRUE;
#endif
	    } else
		return FALSE;
	}
	if ( row < 0x11 ) // no asian font
	    return FALSE;
	if ( row > 0x2d && row < 0xfb || row == 0x11 )
	    // asian line breaking. Everywhere allowed except directly
	    // in front of a punctuation character.
	    return TRUE;
    }
    return FALSE;
}

void QTextFormatter::insertLineStart( QTextParag *parag, int index, QTextParagLineStart *ls )
{
    if ( index > 0 ) { // we can assume that only first line starts are insrted multiple times
	parag->lineStartList().insert( index, ls );
	return;
    }
    QMap<int, QTextParagLineStart*>::Iterator it;
    if ( ( it = parag->lineStartList().find( index ) ) == parag->lineStartList().end() ) {
	parag->lineStartList().insert( index, ls );
    } else {
	delete *it;
	parag->lineStartList().remove( it );
	parag->lineStartList().insert( index, ls );
    }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextFormatterBreakInWords::QTextFormatterBreakInWords()
{
}

int QTextFormatterBreakInWords::format( QTextDocument *doc,QTextParag *parag,
					int start, const QMap<int, QTextParagLineStart*> & )
{
    QTextStringChar *c = 0;
    QTextStringChar *firstChar = 0;
    int left = doc ? parag->leftMargin() + 4 : 4;
    int x = left + ( doc ? parag->firstLineMargin() : 0 );
    int dw = parag->documentVisibleWidth() - ( doc ? 8 : 0 );
    int y = doc->addMargins() ? parag->topMargin() : 0;
    int h = y;
    int len = parag->length();
    if ( doc )
	x = doc->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), x, 4 );
    int rm = parag->rightMargin();
    int w = dw - ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), parag->rect().height(), rm, 4 ) : 0 );
    bool fullWidth = TRUE;
    int minw = 0;

    start = 0;
    if ( start == 0 )
	c = &parag->string()->at( 0 );

    int i = start;
    QTextParagLineStart *lineStart = new QTextParagLineStart( y, y, 0 );
    insertLineStart( parag, 0, lineStart );

    int col = 0;
    int ww = 0;
    QChar lastChr;
    for ( ; i < len; ++i, ++col ) {
	if ( c )
	    lastChr = c->c;
	c = &parag->string()->at( i );
	if ( i > 0 ) {
	    c->lineStart = 0;
	} else {
	    c->lineStart = 1;
	    firstChar = c;
	}
	if ( c->c.unicode() >= 32 || c->isCustom() ) {
	    ww = parag->string()->width( i );
	} else if ( c->c == '\t' ) {
	    int nx = parag->nextTab( i, x );
	    if ( nx < x )
		ww = w - x;
	    else
		ww = nx - x + 1;
	} else {
	    ww = c->format()->width( ' ' );
	}

	if ( c->isCustom() && c->customItem()->ownLine() ) {
	    if ( doc )
		x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), left, 4 ) : left;
	    w = dw - ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), parag->rect().height(), rm, 4 ) : 0 );
	    c->customItem()->resize( parag->painter(), dw );
	    if ( x != left || w != dw )
		fullWidth = FALSE;
	    w = dw;
	    y += h;
	    h = c->height();
	    lineStart = new QTextParagLineStart( y, h, h );
	    insertLineStart( parag, i, lineStart );
	    c->lineStart = 1;
	    firstChar = c;
	    x = 0xffffff;
	    continue;
	}

	if ( isWrapEnabled() &&
	     ( wrapAtColumn() == -1 && x + ww > w ||
	       wrapAtColumn() != -1 && col >= wrapAtColumn() ) ||
	       parag->isNewLinesAllowed() && lastChr == '\n' ) {
	    x = doc ? parag->document()->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), left, 4 ) : left;
	    if ( x != left )
		fullWidth = FALSE;
	    w = dw;
	    y += h;
	    h = c->height();
	    lineStart = formatLine( parag, parag->string(), lineStart, firstChar, c-1 );
	    lineStart->y = y;
	    insertLineStart( parag, i, lineStart );
	    lineStart->baseLine = c->ascent();
	    lineStart->h = c->height();
	    c->lineStart = 1;
	    firstChar = c;
	    col = 0;
	    if ( wrapAtColumn() != -1 )
		minw = QMAX( minw, w );
	} else if ( lineStart ) {
	    lineStart->baseLine = QMAX( lineStart->baseLine, c->ascent() );
	    h = QMAX( h, c->height() );
	    lineStart->h = h;
	}

	c->x = x;
	x += ww;
    }

    int m = parag->bottomMargin();
    if ( parag->next() && !doc->addMargins() )
	m = QMAX( m, parag->next()->topMargin() );
    parag->setFullWidth( fullWidth );
    if ( is_printer( parag->painter() ) ) {
	QPaintDeviceMetrics metrics( parag->painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	m = (int)( (double)m * yscale );
    }
    y += h + m;
    if ( !isWrapEnabled() )
	minw = QMAX( minw, c->x + ww ); // #### Lars: Fix this for BiDi, please
    if ( doc )
	doc->setMinimumWidth( minw, parag );
    return y;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextFormatterBreakWords::QTextFormatterBreakWords()
{
}

int QTextFormatterBreakWords::format( QTextDocument *doc, QTextParag *parag,
				      int start, const QMap<int, QTextParagLineStart*> & )
{
    QTextStringChar *c = 0;
    QTextStringChar *firstChar = 0;
    QTextString *string = parag->string();
    int left = doc ? parag->leftMargin() + 4 : 0;
    int x = left + ( doc ? parag->firstLineMargin() : 0 );
    int curLeft = left;
    int y = doc->addMargins() ? parag->topMargin() : 0;
    int h = y;
    int len = parag->length();
    if ( doc )
	x = doc->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), x, 4 );
    int dw = parag->documentVisibleWidth() - ( doc ? ( left != x ? 0 : 8 ) : -4 );

    curLeft = x;
    int rm = parag->rightMargin();
    int rdiff = doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), parag->rect().height(), rm, 4 ) : 0;
    int w = dw - rdiff;
    bool fullWidth = TRUE;
    int marg = left + rdiff;
    int minw = 0;
    int tminw = marg;
    int ls = doc ? parag->lineSpacing() : 0;

    start = 0;
    if ( start == 0 )
	c = &parag->string()->at( 0 );

    int i = start;
    QTextParagLineStart *lineStart = new QTextParagLineStart( y, y, 0 );
    insertLineStart( parag, 0, lineStart );
    int lastBreak = -1;
    int tmpBaseLine = 0, tmph = 0;
    bool lastWasNonInlineCustom = FALSE;

    int align = parag->alignment();
    if ( align == Qt3::AlignAuto && doc && doc->alignment() != Qt3::AlignAuto )
 	align = doc->alignment();

    int col = 0;
    int ww = 0;
    QChar lastChr;
    for ( ; i < len; ++i, ++col ) {
	if ( c )
	    lastChr = c->c;
	c = &string->at( i );
	if ( i > 0 && x > curLeft || lastWasNonInlineCustom ) {
	    c->lineStart = 0;
	} else {
	    c->lineStart = 1;
	    firstChar = c;
	}

	if ( c->isCustom() && c->customItem()->placement() != QTextCustomItem::PlaceInline )
	    lastWasNonInlineCustom = TRUE;
	else
	    lastWasNonInlineCustom = FALSE;

	if ( c->c.unicode() >= 32 || c->isCustom() ) {
	    ww = string->width( i );
	} else if ( c->c == '\t' ) {
	    int nx = parag->nextTab( i, x );
	    if ( nx < x )
		ww = w - x;
	    else
		ww = nx - x + 1;
	} else {
	    ww = c->format()->width( ' ' );
	}

	if ( c->isCustom() && c->customItem()->ownLine() ) {
	    x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), left, 4 ) : left;
	    w = dw - ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), parag->rect().height(), rm, 4 ) : 0 );
	    QTextParagLineStart *lineStart2 = formatLine( parag, string, lineStart, firstChar, c-1, align, w - x );
	    c->customItem()->resize( parag->painter(), dw );
	    if ( x != left || w != dw )
		fullWidth = FALSE;
	    curLeft = x;
	    if ( i == 0 || !isBreakable( string, i - 1 ) || string->at( i - 1 ).lineStart == 0 ) {
		y += QMAX( h, tmph );
		tmph = c->height() + ls;
		h = tmph;
		lineStart = lineStart2;
		lineStart->y = y;
		insertLineStart( parag, i, lineStart );
		c->lineStart = 1;
		firstChar = c;
	    } else {
		tmph = c->height() + ls;
		h = tmph;
		delete lineStart2;
	    }
	    lineStart->h = h;
	    lineStart->baseLine = h;
	    tmpBaseLine = lineStart->baseLine;
	    lastBreak = -2;
	    x = 0xffffff;
	    minw = QMAX( minw, tminw );
	    int tw = QMAX( c->customItem()->minimumWidth(), QMIN( c->customItem()->widthHint(), c->customItem()->width ) );
	    if ( tw < 32000 )
		tminw = tw;
	    else
		tminw = marg;
	    continue;
	}
	if ( isWrapEnabled() && ( lastBreak != -1 || allowBreakInWords() ) &&
	     ( wrapAtColumn() == -1 && x + ww > w && lastBreak != -1 ||
	       wrapAtColumn() == -1 && x + ww > w - 4 && lastBreak == -1 && allowBreakInWords() ||
	       wrapAtColumn() != -1 && col >= wrapAtColumn() ) ||
	       parag->isNewLinesAllowed() && lastChr == '\n' ) {
	    if ( wrapAtColumn() != -1 )
		minw = QMAX( minw, x + ww );
	    if ( lastBreak < 0 ) {
		if ( lineStart ) {
		    lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
		    h = QMAX( h, tmph );
		    lineStart->h = h;
		}
		lineStart = formatLine( parag, string, lineStart, firstChar, c-1, align, w - x );
		x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), left, 4 ) : left;
		w = dw - ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), parag->rect().height(), rm, 4 ) : 0 );
		if ( parag->isNewLinesAllowed() && c->c == '\t' ) {
		    int nx = parag->nextTab( i, x );
		    if ( nx < x )
			ww = w - x;
		    else
			ww = nx - x + 1;
		}
		if ( x != left || w != dw )
		    fullWidth = FALSE;
		curLeft = x;
		y += h;
		tmph = c->height() + ls;
		h = 0;
		lineStart->y = y;
		insertLineStart( parag, i, lineStart );
		lineStart->baseLine = c->ascent();
		lineStart->h = c->height();
		c->lineStart = 1;
		firstChar = c;
		tmpBaseLine = lineStart->baseLine;
		lastBreak = -1;
		col = 0;
	    } else {
		i = lastBreak;
		lineStart = formatLine( parag, string, lineStart, firstChar, parag->at( lastBreak ), align, w - string->at( i ).x );
		x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), left, 4 ) : left;
		w = dw - ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), parag->rect().height(), rm, 4 ) : 0 );
		if ( parag->isNewLinesAllowed() && c->c == '\t' ) {
		    int nx = parag->nextTab( i, x );
		    if ( nx < x )
			ww = w - x;
		    else
			ww = nx - x + 1;
		}
		if ( x != left || w != dw )
		    fullWidth = FALSE;
		curLeft = x;
		y += h;
		tmph = c->height() + ls;
		h = tmph;
		lineStart->y = y;
		insertLineStart( parag, i + 1, lineStart );
		lineStart->baseLine = c->ascent();
		lineStart->h = c->height();
		c->lineStart = 1;
		firstChar = c;
		tmpBaseLine = lineStart->baseLine;
		lastBreak = -1;
		col = 0;
		tminw = marg;
		continue;
	    }
	} else if ( lineStart && ( isBreakable( string, i ) || parag->isNewLinesAllowed() && c->c == '\n' ) ) {
	    if ( len < 2 || i < len - 1 ) {
		tmpBaseLine = QMAX( tmpBaseLine, c->ascent() );
		tmph = QMAX( tmph, c->height() + ls );
	    }
	    minw = QMAX( minw, tminw );
	    tminw = marg + ww;
	    lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
	    h = QMAX( h, tmph );
	    lineStart->h = h;
	    if ( i < len - 2 || c->c != ' ' )
		lastBreak = i;
	} else {
	    tminw += ww;
            //qDebug( "BLBUG: tmpBaseLine = max ( tmpBaseLine=%d, c->ascent()=%d )",  tmph, c->ascent() );
	    tmpBaseLine = QMAX( tmpBaseLine, c->ascent() );
            //qDebug( "BLBUG: tmph = max ( tmph=%d, c->height()+ls=%d )",  tmph, c->height() + ls );
	    tmph = QMAX( tmph, c->height() + ls );
	}

	c->x = x;
	x += ww;
    }

    if ( lineStart ) {
	lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
	h = QMAX( h, tmph );
	lineStart->h = h;
	// last line in a paragraph is not justified
	if ( align == Qt3::AlignJustify )
	    align = Qt3::AlignAuto;
	lineStart = formatLine( parag, string, lineStart, firstChar, c, align, w - x );
	delete lineStart;
    }

    minw = QMAX( minw, tminw );

    int m = parag->bottomMargin();
    if ( parag->next() && !doc->addMargins() )
	m = QMAX( m, parag->next()->topMargin() );
    parag->setFullWidth( fullWidth );
    if ( is_printer( parag->painter() ) ) {
	QPaintDeviceMetrics metrics( parag->painter()->device() );
	double yscale = scale_factor( metrics.logicalDpiY() );
	m = (int)( (double)m * yscale );
    }
    y += h + m;

    if ( !isWrapEnabled() )
	minw = QMAX( minw, c->x + ww ); // #### Lars: Fix this for BiDi, please
    if ( doc ) {
	if ( minw < 32000 )
	    doc->setMinimumWidth( minw, parag );
    }

    return y;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextIndent::QTextIndent()
{
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextFormatCollection::QTextFormatCollection()
    : cKey( 307 ), sheet( 0 )
{
#ifdef DEBUG_COLLECTION
    qDebug("QTextFormatCollection::QTextFormatCollection %p", this);
#endif
    defFormat = new QTextFormat( QApplication::font(),
				     QApplication::palette().color( QPalette::Active, QColorGroup::Text ) );
    lastFormat = cres = 0;
    cflags = -1;
    cKey.setAutoDelete( TRUE );
    cachedFormat = 0;
}

QTextFormatCollection::~QTextFormatCollection()
{
#ifdef DEBUG_COLLECTION
    qDebug("QTextFormatCollection::~QTextFormatCollection %p", this);
#endif
    delete defFormat;
}

QTextFormat *QTextFormatCollection::format( QTextFormat *f )
{
    if ( f->parent() == this || f == defFormat ) {
#ifdef DEBUG_COLLECTION
	qDebug( " format(f) need '%s', best case!", f->key().latin1() );
#endif
	lastFormat = f;
	lastFormat->addRef();
	return lastFormat;
    }

    if ( f == lastFormat || ( lastFormat && f->key() == lastFormat->key() ) ) {
#ifdef DEBUG_COLLECTION
	qDebug( " format(f) need '%s', good case!", f->key().latin1() );
#endif
	lastFormat->addRef();
	return lastFormat;
    }

#if 0 // #### disabled, because if this format is not in the
 // formatcollection, it doesn't get the painter through
 // QTextFormatCollection::setPainter() which breaks printing on
 // windows
    if ( f->isAnchor() ) {
	lastFormat = createFormat( *f );
	lastFormat->collection = 0;
	return lastFormat;
    }
#endif

    QTextFormat *fm = cKey.find( f->key() );
    if ( fm ) {
#ifdef DEBUG_COLLECTION
	qDebug( " format(f) need '%s', normal case!", f->key().latin1() );
#endif
	lastFormat = fm;
	lastFormat->addRef();
	return lastFormat;
    }

#ifdef DEBUG_COLLECTION
    qDebug( " format(f) need '%s', worst case!", f->key().latin1() );
#endif
    lastFormat = createFormat( *f );
    lastFormat->collection = this;
    cKey.insert( lastFormat->key(), lastFormat );
    return lastFormat;
}

QTextFormat *QTextFormatCollection::format( QTextFormat *of, QTextFormat *nf, int flags )
{
    if ( cres && kof == of->key() && knf == nf->key() && cflags == flags ) {
#ifdef DEBUG_COLLECTION
	qDebug( " format(of,nf,flags) mix of '%s' and '%s, best case!", of->key().latin1(), nf->key().latin1() );
#endif
	cres->addRef();
	return cres;
    }

    cres = createFormat( *of );
    kof = of->key();
    knf = nf->key();
    cflags = flags;

    cres->copyFormat( *nf, flags );

    QTextFormat *fm = cKey.find( cres->key() );
    if ( !fm ) {
#ifdef DEBUG_COLLECTION
	qDebug( " format(of,nf,flags) mix of '%s' and '%s, worst case!", of->key().latin1(), nf->key().latin1() );
#endif
	cres->collection = this;
	cKey.insert( cres->key(), cres );
    } else {
#ifdef DEBUG_COLLECTION
	qDebug( " format(of,nf,flags) mix of '%s' and '%s, good case!", of->key().latin1(), nf->key().latin1() );
#endif
	delete cres;
	cres = fm;
	cres->addRef();
    }

    return cres;
}

QTextFormat *QTextFormatCollection::format( const QFont &f, const QColor &c )
{
    if ( cachedFormat && cfont == f && ccol == c ) {
#ifdef DEBUG_COLLECTION
	qDebug( " format of font and col '%s' - best case", cachedFormat->key().latin1() );
#endif
	cachedFormat->addRef();
	return cachedFormat;
    }

    QString key = QTextFormat::getKey( f, c, FALSE, QString::null, QString::null, QTextFormat::AlignNormal );
    cachedFormat = cKey.find( key );
    cfont = f;
    ccol = c;

    if ( cachedFormat ) {
#ifdef DEBUG_COLLECTION
	qDebug( " format of font and col '%s' - good case", cachedFormat->key().latin1() );
#endif
	cachedFormat->addRef();
	return cachedFormat;
    }

    cachedFormat = createFormat( f, c );
    cachedFormat->collection = this;
    cKey.insert( cachedFormat->key(), cachedFormat );
#ifdef DEBUG_COLLECTION
    qDebug( " format of font and col '%s' - worst case", cachedFormat->key().latin1() );
#endif
    return cachedFormat;
}

void QTextFormatCollection::remove( QTextFormat *f )
{
    if ( lastFormat == f )
	lastFormat = 0;
    if ( cres == f )
	cres = 0;
    if ( cachedFormat == f )
	cachedFormat = 0;
    cKey.remove( f->key() );
}

void QTextFormatCollection::setPainter( QPainter *p )
{
    QDictIterator<QTextFormat> it( cKey );
    QTextFormat *f;
    while ( ( f = it.current() ) ) {
	++it;
	f->setPainter( p );
    }
}

void QTextFormatCollection::debug()
{
    qDebug( "------------ QTextFormatCollection: debug --------------- BEGIN" );
    QDictIterator<QTextFormat> it( cKey );
    for ( ; it.current(); ++it ) {
	qDebug( "format '%s' (%p): refcount: %d", it.current()->key().latin1(),
		it.current(), it.current()->ref );
    }
    qDebug( "------------ QTextFormatCollection: debug --------------- END" );
}

void QTextFormatCollection::updateStyles()
{
    QDictIterator<QTextFormat> it( cKey );
    QTextFormat *f;
    while ( ( f = it.current() ) ) {
	++it;
	f->updateStyle();
    }
}

void QTextFormatCollection::updateFontSizes( int base )
{
    QDictIterator<QTextFormat> it( cKey );
    QTextFormat *f;
    while ( ( f = it.current() ) ) {
	++it;
	f->stdPointSize = base;
	f->fn.setPointSize( f->stdPointSize );
	styleSheet()->scaleFont( f->fn, f->logicalFontSize );
	f->update();
    }
    f = defFormat;
    f->stdPointSize = base;
    f->fn.setPointSize( f->stdPointSize );
    styleSheet()->scaleFont( f->fn, f->logicalFontSize );
    f->update();
}

void QTextFormatCollection::updateFontAttributes( const QFont &f, const QFont &old )
{
    QDictIterator<QTextFormat> it( cKey );
    QTextFormat *fm;
    while ( ( fm = it.current() ) ) {
	++it;
	if ( fm->fn.family() == old.family() &&
	     fm->fn.weight() == old.weight() &&
	     fm->fn.italic() == old.italic() &&
	     fm->fn.underline() == old.underline() ) {
	    fm->fn.setFamily( f.family() );
	    fm->fn.setWeight( f.weight() );
	    fm->fn.setItalic( f.italic() );
	    fm->fn.setUnderline( f.underline() );
	    fm->update();
	}
    }
    fm = defFormat;
    if ( fm->fn.family() == old.family() &&
	 fm->fn.weight() == old.weight() &&
	 fm->fn.italic() == old.italic() &&
	 fm->fn.underline() == old.underline() ) {
	fm->fn.setFamily( f.family() );
	fm->fn.setWeight( f.weight() );
	fm->fn.setItalic( f.italic() );
	fm->fn.setUnderline( f.underline() );
	fm->update();
    }
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void QTextFormat::copyFormat( const QTextFormat & nf, int flags )
{
    if ( flags & QTextFormat::Bold )
	fn.setBold( nf.fn.bold() );
    if ( flags & QTextFormat::Italic )
	fn.setItalic( nf.fn.italic() );
    if ( flags & QTextFormat::Underline )
	fn.setUnderline( nf.fn.underline() );
    if ( flags & QTextFormat::Family )
	fn.setFamily( nf.fn.family() );
    if ( flags & QTextFormat::Size )
	fn.setPointSize( nf.fn.pointSize() );
    if ( flags & QTextFormat::Color )
	col = nf.col;
    if ( flags & QTextFormat::Misspelled )
	missp = nf.missp;
    if ( flags & QTextFormat::VAlign )
	ha = nf.ha;
    update();
}

void QTextFormat::setBold( bool b )
{
    if ( b == fn.bold() )
	return;
    fn.setBold( b );
    update();
}

void QTextFormat::setMisspelled( bool b )
{
    if ( b == (bool)missp )
	return;
    missp = b;
    update();
}

void QTextFormat::setVAlign( VerticalAlignment a )
{
    if ( a == ha )
	return;
    ha = a;
    update();
}

void QTextFormat::setItalic( bool b )
{
    if ( b == fn.italic() )
	return;
    fn.setItalic( b );
    update();
}

void QTextFormat::setUnderline( bool b )
{
    if ( b == fn.underline() )
	return;
    fn.setUnderline( b );
    update();
}

void QTextFormat::setFamily( const QString &f )
{
    if ( f == fn.family() )
	return;
    fn.setFamily( f );
    update();
}

void QTextFormat::setPointSize( int s )
{
    if ( s == fn.pointSize() )
	return;
    fn.setPointSize( s );
    update();
}

void QTextFormat::setFont( const QFont &f )
{
    if ( f == fn && !k.isEmpty() )
	return;
    fn = f;
    update();
}

void QTextFormat::setColor( const QColor &c )
{
    if ( c == col )
	return;
    col = c;
    update();
}

void QTextFormat::setPainter( QPainter *p )
{
    painter = p;
    update();
}

static int makeLogicFontSize( int s )
{
    int defSize = QApplication::font().pointSize();
    if ( s < defSize - 4 )
	return 1;
    if ( s < defSize )
	return 2;
    if ( s < defSize + 4 )
	return 3;
    if ( s < defSize + 8 )
	return 4;
    if ( s < defSize + 12 )
	return 5;
    if (s < defSize + 16 )
	return 6;
    return 7;
}

static QTextFormat *defaultFormat = 0;

QString QTextFormat::makeFormatChangeTags( QTextFormat *f ) const
{
    if ( !defaultFormat )
	defaultFormat = new QTextFormat( QApplication::font(),
					     QApplication::palette().color( QPalette::Active, QColorGroup::Text ) );

    QString tag;

    if ( f ) {
	if ( f->font() != defaultFormat->font() ||
	     f->color().rgb() != defaultFormat->color().rgb() )
	    tag += "</font>";
	if ( f->font() != defaultFormat->font() ) {
	    if ( f->font().underline() && f->font().underline() != defaultFormat->font().underline() )
		tag += "</u>";
	    if ( f->font().italic() && f->font().italic() != defaultFormat->font().italic() )
		tag += "</i>";
	    if ( f->font().bold() && f->font().bold() != defaultFormat->font().bold() )
		tag += "</b>";
	}
	if ( f->isAnchor() && !f->anchorHref().isEmpty() )
	    tag += "</a>";
    }

    if ( isAnchor() ) {
	if ( !anchor_href.isEmpty() )
	    tag += "<a href=\"" + anchor_href + "\">";
	else
	    tag += "<a name=\"" + anchor_name + "\"></a>";
    }

    if ( font() != defaultFormat->font() ) {
	if ( font().bold() && font().bold() != defaultFormat->font().bold() )
	    tag += "<b>";
	if ( font().italic() && font().italic() != defaultFormat->font().italic() )
	    tag += "<i>";
	if ( font().underline() && font().underline() != defaultFormat->font().underline() )
	    tag += "<u>";
    }
    if ( font() != defaultFormat->font() ||
	 color().rgb() != defaultFormat->color().rgb() ) {
	tag += "<font ";
	if ( font().family() != defaultFormat->font().family() )
	    tag +="face=\"" + fn.family() + "\" ";
	if ( font().pointSize() != defaultFormat->font().pointSize() )
	    tag +="size=\"" + QString::number( makeLogicFontSize( fn.pointSize() ) ) + "\" ";
	if ( color().rgb() != defaultFormat->color().rgb() )
	    tag +="color=\"" + col.name() + "\" ";
	tag += ">";
    }

    return tag;
}

QString QTextFormat::makeFormatEndTags() const
{
    if ( !defaultFormat )
	defaultFormat = new QTextFormat( QApplication::font(),
					     QApplication::palette().color( QPalette::Active, QColorGroup::Text ) );

    QString tag;
    if ( font() != defaultFormat->font() ||
	 color().rgb() != defaultFormat->color().rgb() )
	tag += "</font>";
    if ( font() != defaultFormat->font() ) {
	if ( font().underline() && font().underline() != defaultFormat->font().underline() )
	    tag += "</u>";
	if ( font().italic() && font().italic() != defaultFormat->font().italic() )
	    tag += "</i>";
	if ( font().bold() && font().bold() != defaultFormat->font().bold() )
	    tag += "</b>";
    }
    if ( isAnchor() && anchorHref().isEmpty() )
	tag += "</a>";
    return tag;
}

QTextFormat QTextFormat::makeTextFormat( const QStyleSheetItem *style, const QMap<QString,QString>& attr ) const
{
    QTextFormat format(*this);
    bool changed = FALSE;
    if ( style ) {
	format.style = style->name();
	if ( style->name() == "font") {
	    if ( attr.contains("color") ) {
		QString s = attr["color"];
		if ( !s.isEmpty() ) {
		    format.col.setNamedColor( s );
		    format.linkColor = FALSE;
		}
	    }
	    if ( attr.contains("size") ) {
		QString a = attr["size"];
		int n = a.toInt();
		if ( a[0] == '+' || a[0] == '-' )
		    n += format.logicalFontSize;
		format.logicalFontSize = n;
		format.fn.setPointSize( format.stdPointSize );
		style->styleSheet()->scaleFont( format.fn, format.logicalFontSize );
	    }
	    if ( attr.contains("face") ) {
		QString a = attr["face"];
		if ( a.contains(',') )
		    a = a.left( a.find(',') );
		format.fn.setFamily( a );
	    }
	} else {

	    if ( style->isAnchor() ) {
		format.anchor_href = attr["href"];
		format.anchor_name = attr["name"];
		changed = TRUE;
	    }

	    if ( style->fontWeight() != QStyleSheetItem::Undefined )
		format.fn.setWeight( style->fontWeight() );
	    if ( style->fontSize() != QStyleSheetItem::Undefined )
		format.fn.setPointSize( style->fontSize() );
	    else if ( style->logicalFontSize() != QStyleSheetItem::Undefined ) {
		format.logicalFontSize = style->logicalFontSize();
		format.fn.setPointSize( format.stdPointSize );
		style->styleSheet()->scaleFont( format.fn, format.logicalFontSize );
	    }
	    else if ( style->logicalFontSizeStep() ) {
		format.logicalFontSize += style->logicalFontSizeStep();
		format.fn.setPointSize( format.stdPointSize );
		style->styleSheet()->scaleFont( format.fn, format.logicalFontSize );
	    }
	    if ( !style->fontFamily().isEmpty() )
		format.fn.setFamily( style->fontFamily() );
	    if ( style->color().isValid() )
		format.col = style->color();
	    if ( style->definesFontItalic() )
		format.fn.setItalic( style->fontItalic() );
	    if ( style->definesFontUnderline() )
		format.fn.setUnderline( style->fontUnderline() );
	}
    }

    if ( fn != format.fn || changed || col != format.col ) // slight performance improvement
	format.generateKey();
    format.update();
    return format;
}

QTextCustomItem::QTextCustomItem( QTextDocument *p )
      :  xpos(0), ypos(-1), width(-1), height(0), parent( p ), parag(0)
{
}

QTextCustomItem::~QTextCustomItem()
{
}

struct QPixmapInt
{
    QPixmapInt() : ref( 0 ) {}
    QPixmap pm;
    int	    ref;
};

static QMap<QString, QPixmapInt> *pixmap_map = 0;

QTextImage::QTextImage( QTextDocument *p, const QMap<QString, QString> &attr, const QString& context,
			QMimeSourceFactory &factory )
    : QTextCustomItem( p )
{
#if defined(PARSER_DEBUG)
    qDebug( debug_indent + "new QTextImage (pappi: %p)", p );
#endif

    width = height = 0;
    if ( attr.contains("width") )
	width = attr["width"].toInt();
    if ( attr.contains("height") )
	height = attr["height"].toInt();

    reg = 0;
    QString imageName = attr["src"];

    if (!imageName)
	imageName = attr["source"];

#if defined(PARSER_DEBUG)
    qDebug( debug_indent + "    .." + imageName );
#endif

    if ( !imageName.isEmpty() ) {
	imgId = QString( "%1,%2,%3,%4" ).arg( imageName ).arg( width ).arg( height ).arg( (ulong)&factory );
	if ( !pixmap_map )
	    pixmap_map = new QMap<QString, QPixmapInt>;
	if ( pixmap_map->contains( imgId ) ) {
	    QPixmapInt& pmi = pixmap_map->operator[](imgId);
	    pm = pmi.pm;
	    pmi.ref++;
	    width = pm.width();
	    height = pm.height();
	} else {
	    QImage img;
	    const QMimeSource* m =
		factory.data( imageName, context );
	    if ( !m ) {
		qWarning("QTextImage: no mimesource for %s", imageName.latin1() );
	    }
	    else {
		if ( !QImageDrag::decode( m, img ) ) {
		    qWarning("QTextImage: cannot decode %s", imageName.latin1() );
		}
	    }

	    if ( !img.isNull() ) {
		if ( width == 0 ) {
		    width = img.width();
		    if ( height != 0 ) {
			width = img.width() * height / img.height();
		    }
		}
		if ( height == 0 ) {
		    height = img.height();
		    if ( width != img.width() ) {
			height = img.height() * width / img.width();
		    }
		}

		if ( img.width() != width || img.height() != height ){
		    img = img.smoothScale(width, height);
		    width = img.width();
		    height = img.height();
		}
		pm.convertFromImage( img );
	    }
	    if ( !pm.isNull() ) {
		QPixmapInt& pmi = pixmap_map->operator[](imgId);
		pmi.pm = pm;
		pmi.ref++;
	    }
	}
	if ( pm.mask() ) {
	    QRegion mask( *pm.mask() );
	    QRegion all( 0, 0, pm.width(), pm.height() );
	    reg = new QRegion( all.subtract( mask ) );
	}
    }

    if ( pm.isNull() && (width*height)==0 )
	width = height = 50;

    place = PlaceInline;
    if ( attr["align"] == "left" )
	place = PlaceLeft;
    else if ( attr["align"] == "right" )
	place = PlaceRight;

    tmpwidth = width;
    tmpheight = height;

    attributes = attr;
}

QTextImage::~QTextImage()
{
    if ( pixmap_map && pixmap_map->contains( imgId ) ) {
	QPixmapInt& pmi = pixmap_map->operator[](imgId);
	pmi.ref--;
	if ( !pmi.ref ) {
	    pixmap_map->remove( imgId );
	    if ( pixmap_map->isEmpty() ) {
		delete pixmap_map;
		pixmap_map = 0;
	    }
	}
    }
}

QString QTextImage::richText() const
{
    QString s;
    s += "<img ";
    QMap<QString, QString>::ConstIterator it = attributes.begin();
    for ( ; it != attributes.end(); ++it )
	s += it.key() + "=" + *it + " ";
    s += ">";
    return s;
}

void QTextImage::adjustToPainter( QPainter* p )
{
    width = tmpwidth;
    height = tmpheight;
    if ( !is_printer( p ) )
	return;
    QPaintDeviceMetrics metrics(p->device());
    width = int( width * scale_factor( metrics.logicalDpiX() ) );
    height = int( height * scale_factor( metrics.logicalDpiY() ) );
}

void QTextImage::draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg )
{
    if ( placement() != PlaceInline ) {
	x = xpos;
	y = ypos;
    }

    if ( pm.isNull() ) {
	p->fillRect( x , y, width, height,  cg.dark() );
	return;
    }

    if ( is_printer( p ) ) {
#ifndef QT_NO_TRANSFORMATIONS
	p->saveWorldMatrix();
	QPaintDeviceMetrics metrics( p->device() );
	p->translate( x, y );
	p->scale( scale_factor( metrics.logicalDpiY() ),
		  scale_factor( metrics.logicalDpiY() ) );
	p->drawPixmap( 0, 0, pm );
	p->restoreWorldMatrix();
#else
	p->drawPixmap( x, y, pm );
#endif
	return;
    }

    if ( placement() != PlaceInline && !QRect( xpos, ypos, width, height ).intersects( QRect( cx, cy, cw, ch ) ) )
	return;
    if ( placement() == PlaceInline )
	p->drawPixmap( x , y, pm );
    else
	p->drawPixmap( cx , cy, pm, cx - x, cy - y, cw, ch );
}

void QTextHorizontalLine::adjustToPainter( QPainter* p )
{
    if ( !is_printer( p ) )
	return;
    QPaintDeviceMetrics metrics(p->device());
    height = int( tmpheight * scale_factor( metrics.logicalDpiY() ) );
}


QTextHorizontalLine::QTextHorizontalLine( QTextDocument *p )
    : QTextCustomItem( p )
{
    height = tmpheight = 8;
}

QTextHorizontalLine::~QTextHorizontalLine()
{
}

QString QTextHorizontalLine::richText() const
{
    return "<hr>";
}

void QTextHorizontalLine::draw( QPainter* p, int x, int y, int , int , int , int , const QColorGroup& cg )
{
    QRect r( x, y, width, height);
    if ( is_printer( p ) || ( p && p->device() && p->device()->devType() == QInternal::Printer ) ) {
	QPen oldPen = p->pen();
	p->setPen( QPen( cg.text(), height/8 ) );
	p->drawLine( r.left()-1, y + height / 2, r.right() + 1, y + height / 2 );
	p->setPen( oldPen );
    } else {
	qDrawShadeLine( p, r.left() - 1, y + height / 2, r.right() + 1, y + height / 2, cg, TRUE, height / 8 );
    }
}


/*****************************************************************/
// Small set of utility functions to make the parser a bit simpler
//

bool QTextDocument::hasPrefix(const QString& doc, int pos, QChar c)
{
    if ( pos >= (int)doc.length() )
	return FALSE;
    return ( doc.unicode() )[ pos ].lower() == c.lower();
}

bool QTextDocument::hasPrefix( const QString& doc, int pos, const QString& s )
{
    if ( pos + s.length() >= doc.length() )
	return FALSE;
    for ( int i = 0; i < (int)s.length(); i++ ) {
	if ( ( doc.unicode() )[ pos + i ].lower() != s[ i ].lower() )
	    return FALSE;
    }
    return TRUE;
}

static bool qt_is_cell_in_use( QPtrList<QTextTableCell>& cells, int row, int col )
{
    for ( QTextTableCell* c = cells.first(); c; c = cells.next() ) {
	if ( row >= c->row() && row < c->row() + c->rowspan()
	     && col >= c->column() && col < c->column() + c->colspan() )
	    return TRUE;
    }
    return FALSE;
}

QTextCustomItem* QTextDocument::parseTable( const QMap<QString, QString> &attr, const QTextFormat &fmt,
					    const QString &doc, int& pos, QTextParag *curpar )
{

    QTextTable* table = new QTextTable( this, attr );
    int row = -1;
    int col = -1;

    QString rowbgcolor;
    QString rowalign;
    QString tablebgcolor = attr["bgcolor"];

    QPtrList<QTextTableCell> multicells;

    QString tagname;
    (void) eatSpace(doc, pos);
    while ( pos < int(doc.length() )) {
	int beforePos = pos;
	if (hasPrefix(doc, pos, QChar('<')) ){
	    if (hasPrefix(doc, pos+1, QChar('/'))) {
		tagname = parseCloseTag( doc, pos );
		if ( tagname == "table" ) {
		    pos = beforePos;
#if defined(PARSER_DEBUG)
		    debug_indent.remove( debug_indent.length() - 3, 2 );
#endif
		    return table;
		}
	    } else {
		QMap<QString, QString> attr2;
		bool emptyTag = FALSE;
		tagname = parseOpenTag( doc, pos, attr2, emptyTag );
		if ( tagname == "tr" ) {
		    rowbgcolor = attr2["bgcolor"];
		    rowalign = attr2["align"];
		    row++;
		    col = -1;
		}
		else if ( tagname == "td" || tagname == "th" ) {
		    col++;
		    while ( qt_is_cell_in_use( multicells, row, col ) ) {
			col++;
		    }

		    if ( row >= 0 && col >= 0 ) {
			const QStyleSheetItem* s = sheet_->item(tagname);
			if ( !attr2.contains("bgcolor") ) {
			    if (!rowbgcolor.isEmpty() )
				attr2["bgcolor"] = rowbgcolor;
			    else if (!tablebgcolor.isEmpty() )
				attr2["bgcolor"] = tablebgcolor;
			}
			if ( !attr2.contains("align") ) {
			    if (!rowalign.isEmpty() )
				attr2["align"] = rowalign;
			}

			// extract the cell contents
			int end = pos;
			while ( end < (int) doc.length()
				&& !hasPrefix( doc, end, "</td")
				&& !hasPrefix( doc, end, "<td")
				&& !hasPrefix( doc, end, "</th")
				&& !hasPrefix( doc, end, "<th")
				&& !hasPrefix( doc, end, "<td")
				&& !hasPrefix( doc, end, "</tr")
				&& !hasPrefix( doc, end, "<tr")
				&& !hasPrefix( doc, end, "</table") ) {
			    if ( hasPrefix( doc, end, "<table" ) ) { // nested table
				int nested = 1;
				++end;
				while ( end < (int)doc.length() && nested != 0 ) {
				    if ( hasPrefix( doc, end, "</table" ) )
					nested--;
				    if ( hasPrefix( doc, end, "<table" ) )
					nested++;
				    end++;
				}
			    }
			    end++;
			}
			QTextTableCell* cell  = new QTextTableCell( table, row, col,
								    attr2, s, fmt.makeTextFormat( s, attr2 ),
								    contxt, *factory_, sheet_, doc.mid( pos, end - pos ) );
			cell->richText()->parParag = curpar;
			if ( cell->colspan() > 1 || cell->rowspan() > 1 )
			    multicells.append( cell );
			col += cell->colspan()-1;
			pos = end;
		    }
		}
	    }

	} else {
	    ++pos;
	}
    }
#if defined(PARSER_DEBUG)
    debug_indent.remove( debug_indent.length() - 3, 2 );
#endif
    return table;
}

bool QTextDocument::eatSpace(const QString& doc, int& pos, bool includeNbsp )
{
    int old_pos = pos;
    while (pos < int(doc.length()) && (doc.unicode())[pos].isSpace() && ( includeNbsp || (doc.unicode())[pos] != QChar::nbsp ) )
	pos++;
    return old_pos < pos;
}

bool QTextDocument::eat(const QString& doc, int& pos, QChar c)
{
    bool ok = pos < int(doc.length()) && ((doc.unicode())[pos] == c);
    if ( ok )
	pos++;
    return ok;
}
/*****************************************************************/



static QMap<QCString, QChar> *html_map = 0;
static void qt_cleanup_html_map()
{
    delete html_map;
    html_map = 0;
}

static QMap<QCString, QChar> *htmlMap()
{
    if ( !html_map ) {
	html_map = new QMap<QCString, QChar>;
	qAddPostRoutine( qt_cleanup_html_map );
	html_map->insert( "lt", '<');
	html_map->insert( "gt", '>');
	html_map->insert( "amp", '&');
	html_map->insert( "nbsp", 0x00a0U);
	html_map->insert( "bull", 0x2022U);
	html_map->insert( "aring", '\xe5');
	html_map->insert( "oslash", '\xf8');
	html_map->insert( "ouml", '\xf6');
	html_map->insert( "auml", '\xe4');
	html_map->insert( "uuml", '\xfc');
	html_map->insert( "Ouml", '\xd6');
	html_map->insert( "Auml", '\xc4');
	html_map->insert( "Uuml", '\xdc');
	html_map->insert( "szlig", '\xdf');
	html_map->insert( "copy", '\xa9');
	html_map->insert( "deg", '\xb0');
	html_map->insert( "micro", '\xb5');
	html_map->insert( "plusmn", '\xb1');
	html_map->insert( "middot", '*');
	html_map->insert( "quot", '\"');
	html_map->insert( "commat", '@');
	html_map->insert( "num", '#');
	html_map->insert( "dollar", '$');
	html_map->insert( "ldquo", '`');
	html_map->insert( "rdquo", '\'');
	html_map->insert( "sol", '/' );
	html_map->insert( "bsol", '\\');
	html_map->insert( "lowbar", '_');
	html_map->insert( "shy", '\xad');
    }
    return html_map;
}

QChar QTextDocument::parseHTMLSpecialChar(const QString& doc, int& pos)
{
    QCString s;
    pos++;
    int recoverpos = pos;
    while ( pos < int(doc.length()) && (doc.unicode())[pos] != ';' && !(doc.unicode())[pos].isSpace() && pos < recoverpos + 6) {
	s += (doc.unicode())[pos];
	pos++;
    }
    if ((doc.unicode())[pos] != ';' && !(doc.unicode())[pos].isSpace() ) {
	pos = recoverpos;
	return '&';
    }
    pos++;

    if ( s.length() > 1 && s[0] == '#') {
	int num = s.mid(1).toInt();
	if ( num == 151 ) // ### hack for designer manual
	    return '-';
	return num;
    }

    QMap<QCString, QChar>::Iterator it = htmlMap()->find(s);
    if ( it != htmlMap()->end() ) {
	return *it;
    }

    pos = recoverpos;
    return '&';
}

QString QTextDocument::parseWord(const QString& doc, int& pos, bool lower)
{
    QString s;

    if ((doc.unicode())[pos] == '"') {
	pos++;
	while ( pos < int(doc.length()) && (doc.unicode())[pos] != '"' ) {
	    s += (doc.unicode())[pos];
	    pos++;
	}
	eat(doc, pos, '"');
    } else {
	static QString term = QString::fromLatin1("/>");
	while( pos < int(doc.length()) &&
	       ((doc.unicode())[pos] != '>' && !hasPrefix( doc, pos, term))
	       && (doc.unicode())[pos] != '<'
	       && (doc.unicode())[pos] != '='
	       && !(doc.unicode())[pos].isSpace())
	{
	    if ( (doc.unicode())[pos] == '&')
		s += parseHTMLSpecialChar( doc, pos );
	    else {
		s += (doc.unicode())[pos];
		pos++;
	    }
	}
	if (lower)
	    s = s.lower();
    }
    return s;
}

QChar QTextDocument::parseChar(const QString& doc, int& pos, QStyleSheetItem::WhiteSpaceMode wsm )
{
    if ( pos >=  int(doc.length() ) )
	return QChar::null;

    QChar c = (doc.unicode())[pos++];

    if (c == '<' )
	return QChar::null;

    if ( c.isSpace() && c != QChar::nbsp ) {
	if ( wsm == QStyleSheetItem::WhiteSpacePre ) {
	    if ( c == ' ' )
		return QChar::nbsp;
	    else
		return c;
	} else { // non-pre mode: collapse whitespace except nbsp
	    while ( pos< int(doc.length() ) &&
		    (doc.unicode())[pos].isSpace()  && (doc.unicode())[pos] != QChar::nbsp )
		pos++;
	    if ( wsm == QStyleSheetItem::WhiteSpaceNoWrap )
		return QChar::nbsp;
	    else
		return ' ';
	}
    }
    else if ( c == '&' )
	return parseHTMLSpecialChar( doc, --pos );
    else
	return c;
}

QString QTextDocument::parseOpenTag(const QString& doc, int& pos,
				  QMap<QString, QString> &attr, bool& emptyTag)
{
    emptyTag = FALSE;
    pos++;
    if ( hasPrefix(doc, pos, '!') ) {
	if ( hasPrefix( doc, pos+1, "--")) {
	    pos += 3;
	    // eat comments
	    QString pref = QString::fromLatin1("-->");
	    while ( !hasPrefix(doc, pos, pref ) && pos < int(doc.length()) )
		pos++;
	    if ( hasPrefix(doc, pos, pref ) ) {
		pos += 3;
		eatSpace(doc, pos, TRUE);
	    }
	    emptyTag = TRUE;
	    return QString::null;
	}
	else {
	    // eat strange internal tags
	    while ( !hasPrefix(doc, pos, '>') && pos < int(doc.length()) )
		pos++;
	    if ( hasPrefix(doc, pos, '>') ) {
		pos++;
		eatSpace(doc, pos, TRUE);
	    }
	    return QString::null;
	}
    }

    QString tag = parseWord(doc, pos );
    eatSpace(doc, pos, TRUE);
    static QString term = QString::fromLatin1("/>");
    static QString s_TRUE = QString::fromLatin1("TRUE");

    while ((doc.unicode())[pos] != '>' && ! (emptyTag = hasPrefix(doc, pos, term) )) {
	QString key = parseWord(doc, pos );
	eatSpace(doc, pos, TRUE);
	if ( key.isEmpty()) {
	    // error recovery
	    while ( pos < int(doc.length()) && (doc.unicode())[pos] != '>' )
		pos++;
	    break;
	}
	QString value;
	if (hasPrefix(doc, pos, '=') ){
	    pos++;
	    eatSpace(doc, pos);
	    value = parseWord(doc, pos, FALSE);
	}
	else
	    value = s_TRUE;
	attr.insert(key, value );
	eatSpace(doc, pos, TRUE);
    }

    if (emptyTag) {
	eat(doc, pos, '/');
	eat(doc, pos, '>');
    }
    else
	eat(doc, pos, '>');

    return tag;
}

QString QTextDocument::parseCloseTag( const QString& doc, int& pos )
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos );
    eatSpace(doc, pos, TRUE);
    eat(doc, pos, '>');
    return tag;
}

QTextFlow::QTextFlow()
{
    width = height = pagesize = 0;
    leftItems.setAutoDelete( FALSE );
    rightItems.setAutoDelete( FALSE );
}

QTextFlow::~QTextFlow()
{
}

void QTextFlow::clear()
{
    leftItems.clear();
    rightItems.clear();
}

void QTextFlow::setWidth( int w )
{
    height = 0;
    width = w;
}

int QTextFlow::adjustLMargin( int yp, int, int margin, int space )
{
    for ( QTextCustomItem* item = leftItems.first(); item; item = leftItems.next() ) {
	if ( item->ypos == -1 )
	    continue;
	if ( yp >= item->ypos && yp < item->ypos + item->height )
	    margin = QMAX( margin, item->xpos + item->width + space );
    }
    return margin;
}

int QTextFlow::adjustRMargin( int yp, int, int margin, int space )
{
    for ( QTextCustomItem* item = rightItems.first(); item; item = rightItems.next() ) {
	if ( item->ypos == -1 )
	    continue;
	if ( yp >= item->ypos && yp < item->ypos + item->height )
	    margin = QMAX( margin, width - item->xpos - space );
    }
    return margin;
}

void QTextFlow::adjustFlow( int &yp, int , int h, QTextParag *, bool pages )
{
    if ( pages && pagesize > 0 ) { // check pages
	int ty = yp;
	int yinpage = ty % pagesize;
	if ( yinpage < 2 )
	    yp += 2 - yinpage;
	else
	    if ( yinpage + h > pagesize - 2 )
	    yp += ( pagesize - yinpage ) + 2;
    }

    if ( yp + h > height )
    {
	height = yp + h;
	//qDebug("QTextFlow::adjustFlow now height=%d",height);
    }
}

void QTextFlow::unregisterFloatingItem( QTextCustomItem* item )
{
    leftItems.removeRef( item );
    rightItems.removeRef( item );
}

void QTextFlow::registerFloatingItem( QTextCustomItem* item, bool right )
{
    if ( right ) {
	if ( !rightItems.contains( item ) )
	    rightItems.append( item );
    } else if ( !leftItems.contains( item ) ) {
	leftItems.append( item );
    }
}

void QTextFlow::drawFloatingItems( QPainter* p, int cx, int cy, int cw, int ch, const QColorGroup& cg )
{
    QTextCustomItem *item;
    for ( item = leftItems.first(); item; item = leftItems.next() ) {
	if ( item->xpos == -1 || item->ypos == -1 )
	    continue;
	item->draw( p, item->xpos, item->ypos, cx, cy, cw, ch, cg );
    }

    for ( item = rightItems.first(); item; item = rightItems.next() ) {
	if ( item->xpos == -1 || item->ypos == -1 )
	    continue;
	item->draw( p, item->xpos, item->ypos, cx, cy, cw, ch, cg );
    }
}

void QTextFlow::updateHeight( QTextCustomItem *i )
{
    qDebug("QTextFlow::updateHeight item=%p currenth=%d item's ypos=%d + item's height=%d", i, height, i->ypos, i->height);
    height = QMAX( height, i->ypos + i->height );
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextTable::QTextTable( QTextDocument *p, const QMap<QString, QString> & attr  )
    : QTextCustomItem( p ), painter( 0 )
{
    cells.setAutoDelete( FALSE );
#if defined(PARSER_DEBUG)
    debug_indent += "\t";
    qDebug( debug_indent + "new QTextTable (%p)", this );
    debug_indent += "\t";
#endif
    cellspacing = 2;
    if ( attr.contains("cellspacing") )
	cellspacing = attr["cellspacing"].toInt();
    cellpadding = 1;
    if ( attr.contains("cellpadding") )
	cellpadding = attr["cellpadding"].toInt();
    border = 0;
    innerborder = us_ib = 1;
    if ( attr.contains("border" ) ) {
	QString s( attr["border"] );
	if ( s == "TRUE" )
	    border = 1;
	else
	    border = attr["border"].toInt();
    }
    us_b = border;

    if ( border )
	cellspacing += 2;
    us_cs = cellspacing;
    outerborder = cellspacing + border;
    us_ob = outerborder;
    layout = new QGridLayout( 1, 1, cellspacing );

    fixwidth = 0;
    stretch = 0;
    if ( attr.contains("width") ) {
	bool b;
	QString s( attr["width"] );
	int w = s.toInt( &b );
	if ( b ) {
	    fixwidth = w;
	} else {
	    s = s.stripWhiteSpace();
	    if ( s.length() > 1 && s[ (int)s.length()-1 ] == '%' )
		stretch = s.left( s.length()-1).toInt();
	}
    }

    place = PlaceInline;
    if ( attr["align"] == "left" )
	place = PlaceLeft;
    else if ( attr["align"] == "right" )
	place = PlaceRight;
    cachewidth = 0;
    attributes = attr;
}

QTextTable::~QTextTable()
{
    delete layout;
}

QString QTextTable::richText() const
{
    QString s;
    s = "<table ";
    QMap<QString, QString>::ConstIterator it = attributes.begin();
    for ( ; it != attributes.end(); ++it )
	s += it.key() + "=" + *it + " ";
    s += ">\n";

    int lastRow = -1;
    bool needEnd = FALSE;
    QPtrListIterator<QTextTableCell> it2( cells );
    while ( it2.current() ) {
	QTextTableCell *cell = it2.current();
	++it2;
	if ( lastRow != cell->row() ) {
	    if ( lastRow != -1 )
		s += "</tr>\n";
	    s += "<tr>";
	    lastRow = cell->row();
	    needEnd = TRUE;
	}
	s += "<td ";
	it = cell->attributes.begin();
	for ( ; it != cell->attributes.end(); ++it )
	    s += it.key() + "=" + *it + " ";
	s += ">";
	s += cell->richText()->richText();
	s += "</td>";
    }
    if ( needEnd )
	s += "</tr>\n";
    s += "</table>\n";
    return s;
}

void QTextTable::adjustToPainter( QPainter* p)
{
    painter = p;
    if ( is_printer( p ) ) {
	QPaintDeviceMetrics metrics(p->device());
	double xscale = QMAX( scale_factor( metrics.logicalDpiX() ),
			      scale_factor( metrics.logicalDpiY() ) );
	xscale = QMAX( xscale, 1 );
	cellspacing = int(us_cs * xscale);
	border = int(us_b * xscale);
	innerborder = int(us_ib * xscale);
	outerborder = int(us_ob * xscale);
    }
    for ( QTextTableCell* cell = cells.first(); cell; cell = cells.next() )
	cell->adjustToPainter();

    width = 0;
}

void QTextTable::verticalBreak( int  yt, QTextFlow* flow )
{
    if ( flow->pageSize() <= 0 )
	return;
    int shift = 0;
    for (QTextTableCell* cell = cells.first(); cell; cell = cells.next() ) {
	QRect r = cell->geometry();
	r.moveBy(0, shift );
	cell->setGeometry( r );
	if ( cell->column() == 0 ) {
	    int y = yt + outerborder + cell->geometry().y();
	    int oldy = y;
	    flow->adjustFlow( y, width, cell->geometry().height() + 2*cellspacing, 0 );
	    shift += y - oldy;
	    r = cell->geometry();
	    r.moveBy(0, y - oldy );
	    cell->setGeometry( r );
	}
    }
    height += shift;
}

void QTextTable::draw(QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg )
{
    if ( placement() != PlaceInline ) {
	x = xpos;
	y = ypos;
    }

    lastX = x;
    lastY = y;

    painter = p;

    for (QTextTableCell* cell = cells.first(); cell; cell = cells.next() ) {
	if ( cx < 0 && cy < 0 ||
	     QRect( cx, cy, cw, ch ).intersects( QRect( x + outerborder + cell->geometry().x(),
							y + outerborder + cell->geometry().y(),
							cell->geometry().width(), cell->geometry().height() ) ) ) {
	    cell->draw( x+outerborder, y+outerborder, cx, cy, cw, ch, cg );
	    if ( border ) {
		QRect r( x+outerborder+cell->geometry().x() - us_ib,
			 y+outerborder+cell->geometry().y() - us_ib,
			 cell->geometry().width() + 2 * us_ib,
			 cell->geometry().height() + 2 * us_ib );
		int s = cellspacing;
		if ( is_printer( p ) || ( p && p->device() && p->device()->devType() == QInternal::Printer ) ) {
		    qDrawPlainRect( p, r, cg.text(), us_ib );
		} else {
		    p->fillRect( r.left()-s, r.top(), s, r.height(), cg.button() );
		    p->fillRect( r.right(), r.top(), s, r.height(), cg.button() );
		    p->fillRect( r.left()-s, r.top()-s, r.width()+2*s, s, cg.button() );
		    p->fillRect( r.left()-s, r.bottom(), r.width()+2*s, s, cg.button() );
		    qDrawShadePanel( p, r, cg, TRUE, us_ib );
		}
	    }
	}
    }
    if ( border ) {
	QRect r ( x, y, width, height );
	if ( is_printer( p ) || ( p && p->device() && p->device()->devType() == QInternal::Printer ) ) {
	    qDrawPlainRect( p, QRect(QMAX( 0, r.x()+1 ), QMAX( 0, r.y()+1 ), QMAX( r.width()-2, 0 ), QMAX( 0, r.height()-2 ) ), cg.text(), us_b );
	} else {
	    int s = border;
	    p->fillRect( r.left(), r.top(), s, r.height(), cg.button() );
	    p->fillRect( r.right()-s, r.top(), s, r.height(), cg.button() );
	    p->fillRect( r.left(), r.top(), r.width(), s, cg.button() );
	    p->fillRect( r.left(), r.bottom()-s, r.width(), s, cg.button() );
	    qDrawShadePanel( p, r, cg, FALSE, us_b );
	}
    }

#if defined(DEBUG_TABLE_RENDERING)
    p->save();
    p->setPen( Qt::red );
    p->drawRect( x, y, width, height );
    p->restore();
#endif
}

void QTextTable::resize( QPainter* p, int nwidth )
{
    if ( fixwidth && cachewidth != 0 )
	return;
    if ( nwidth == cachewidth )
	return;
    cachewidth = nwidth;
    int w = nwidth;
    painter = p;
    if ( is_printer( painter ) ) {
	adjustToPainter( painter );
    } else {
	painter = 0;
    }
    format( w );
    if ( nwidth >= 32000 )
	nwidth = w;
    if ( stretch )
	nwidth = nwidth * stretch / 100;

    width = nwidth + 2*outerborder;
    layout->invalidate();
    int shw = layout->sizeHint().width() + 2*outerborder;
    int mw = layout->minimumSize().width() + 2*outerborder;
    if ( stretch )
	width = QMAX( mw, nwidth );
    else
	width = QMAX( mw, QMIN( nwidth, shw ) );

    if ( fixwidth )
	width = fixwidth;

    layout->invalidate();
    mw = layout->minimumSize().width() + 2*outerborder;
    width = QMAX( width, mw );

    int h = layout->heightForWidth( width-2*outerborder );
    layout->setGeometry( QRect(0, 0, width-2*outerborder, h)  );
    height = layout->geometry().height()+2*outerborder;
};

void QTextTable::format( int &w )
{
    for ( int i = 0; i < (int)cells.count(); ++i ) {
	QTextTableCell *cell = cells.at( i );
	cell->richText()->doLayout( painter, QWIDGETSIZE_MAX );
	cell->cached_sizehint = cell->richText()->widthUsed() + 2 * ( innerborder + 4 );
	if ( cell->cached_sizehint > 32000 ) // nested table in paragraph
	    cell->cached_sizehint = cell->minimumSize().width();
	cell->richText()->doLayout( painter, cell->cached_sizehint );
	cell->cached_width = -1;
    }
    w = widthHint();
    layout->invalidate();
    layout->activate();
    width = minimumWidth();
}

void QTextTable::addCell( QTextTableCell* cell )
{
    cells.append( cell );
    layout->addMultiCell( cell, cell->row(), cell->row() + cell->rowspan()-1,
			  cell->column(), cell->column() + cell->colspan()-1 );
}

void QTextTable::enter( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd )
{
    currCell.remove( c );
    if ( !atEnd ) {
	next( c, doc, parag, idx, ox, oy );
    } else {
	currCell.insert( c, cells.count() );
	prev( c, doc, parag, idx, ox, oy );
    }
}

void QTextTable::enterAt( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, const QPoint &pos )
{
    currCell.remove( c );
    int lastCell = -1;
    int lastY = -1;
    for ( int i = 0; i < (int)cells.count(); ++i ) {
	QTextTableCell *cell = cells.at( i );
	if ( !cell )
	    continue;
	if ( cell->geometry().x() - innerborder <= pos.x() &&
	     cell->geometry().x() + cell->geometry().width() + innerborder >= pos.x() ) {
	    if ( cell->geometry().y() > lastY ) {
		lastCell = i;
		lastY = cell->geometry().y();
	    }
	    if ( cell->geometry().y() - innerborder <= pos.y() &&
		 cell->geometry().y() + cell->geometry().height() + innerborder >= pos.y() ) {
		currCell.insert( c, i );
		break;
	    }
	}
    }

    if ( currCell.find( c ) == currCell.end() ) {
	if ( lastY != -1 )
	    currCell.insert( c, lastCell );
    }

    QTextTableCell *cell = cells.at( *currCell.find( c ) );
    if ( !cell )
	return;
    doc = cell->richText();
    parag = doc->firstParag();
    idx = 0;
    ox += cell->geometry().x() + outerborder + parent->x();
    oy += cell->geometry().y() + outerborder;
}

void QTextTable::next( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy )
{
    int cc = *currCell.find( c );
    if ( cc > (int)cells.count() - 1 || cc < 0 )
	cc = -1;
    currCell.remove( c );
    currCell.insert( c, ++cc );
    if ( cc >= (int)cells.count() ) {
	currCell.insert( c, 0 );
	QTextCustomItem::next( c, doc, parag, idx, ox, oy );
	QTextTableCell *cell = cells.at( 0 );
	if ( !cell )
	    return;
	doc = cell->richText();
	idx = -1;
	return;
    }

    QTextTableCell *cell = cells.at( *currCell.find( c ) );
    if ( !cell )
	return;
    doc = cell->richText();
    parag = doc->firstParag();
    idx = 0;
    ox += cell->geometry().x() + outerborder + parent->x();
    oy += cell->geometry().y() + outerborder;
}

void QTextTable::prev( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy )
{
    int cc = *currCell.find( c );
    if ( cc > (int)cells.count() - 1 || cc < 0 )
	cc = cells.count();
    currCell.remove( c );
    currCell.insert( c, --cc );
    if ( cc < 0 ) {
	currCell.insert( c, 0 );
	QTextCustomItem::prev( c, doc, parag, idx, ox, oy );
	QTextTableCell *cell = cells.at( 0 );
	if ( !cell )
	    return;
	doc = cell->richText();
	idx = -1;
	return;
    }

    QTextTableCell *cell = cells.at( *currCell.find( c ) );
    if ( !cell )
	return;
    doc = cell->richText();
    parag = doc->firstParag();
    idx = parag->length() - 1;
    ox += cell->geometry().x() + outerborder + parent->x();
    oy += cell->geometry().y() + outerborder;
}

void QTextTable::down( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy )
{
    QTextTableCell *cell = cells.at( *currCell.find( c ) );
    if ( cell->row_ == layout->numRows() - 1 ) {
	currCell.insert( c, 0 );
	QTextCustomItem::down( c, doc, parag, idx, ox, oy );
	QTextTableCell *cell = cells.at( 0 );
	if ( !cell )
	    return;
	doc = cell->richText();
	idx = -1;
	return;
    }

    int oldRow = cell->row_;
    int oldCol = cell->col_;
    int cc = *currCell.find( c );
    for ( int i = cc; i < (int)cells.count(); ++i ) {
	cell = cells.at( i );
	if ( cell->row_ > oldRow && cell->col_ == oldCol ) {
	    currCell.insert( c, i );
	    break;
	}
    }
    doc = cell->richText();
    if ( !cell )
	return;
    parag = doc->firstParag();
    idx = 0;
    ox += cell->geometry().x() + outerborder + parent->x(),
    oy += cell->geometry().y() + outerborder;
}

void QTextTable::up( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy )
{
    QTextTableCell *cell = cells.at( *currCell.find( c ) );
    if ( cell->row_ == 0 ) {
	currCell.insert( c, 0 );
	QTextCustomItem::up( c, doc, parag, idx, ox, oy );
	QTextTableCell *cell = cells.at( 0 );
	if ( !cell )
	    return;
	doc = cell->richText();
	idx = -1;
	return;
    }

    int oldRow = cell->row_;
    int oldCol = cell->col_;
    int cc = *currCell.find( c );
    for ( int i = cc; i >= 0; --i ) {
	cell = cells.at( i );
	if ( cell->row_ < oldRow && cell->col_ == oldCol ) {
	    currCell.insert( c, i );
	    break;
	}
    }
    doc = cell->richText();
    if ( !cell )
	return;
    parag = doc->lastParag();
    idx = parag->length() - 1;
    ox += cell->geometry().x() + outerborder + parent->x();
    oy += cell->geometry().y() + outerborder;
}

QTextTableCell::QTextTableCell( QTextTable* table,
				int row, int column,
				const QMap<QString, QString> &attr,
				const QStyleSheetItem* /*style*/, // ### use them
				const QTextFormat& /*fmt*/, const QString& context,
				QMimeSourceFactory &factory, QStyleSheet *sheet,
				const QString& doc)
{
#if defined(PARSER_DEBUG)
    qDebug( debug_indent + "new QTextTableCell1 (pappi: %p)", table );
    qDebug( debug_indent + doc );
#endif
    cached_width = -1;
    cached_sizehint = -1;

    maxw = QWIDGETSIZE_MAX;
    minw = 0;

    parent = table;
    row_ = row;
    col_ = column;
    stretch_ = 0;
    richtext = new QTextDocument( table->parent );
    richtext->setTableCell( this );
    QString align = *attr.find( "align" );
    if ( !align.isEmpty() ) {
	if ( align.lower() == "left" )
	    richtext->setAlignment( Qt::AlignLeft );
	else if ( align.lower() == "center" )
	    richtext->setAlignment( Qt::AlignHCenter );
	else if ( align.lower() == "right" )
	    richtext->setAlignment( Qt::AlignRight );
    }
    richtext->setFormatter( table->parent->formatter() );
    richtext->setUseFormatCollection( table->parent->useFormatCollection() );
    richtext->setMimeSourceFactory( &factory );
    richtext->setStyleSheet( sheet );
    richtext->setDefaultFont( table->parent->formatCollection()->defaultFormat()->font() );
    richtext->setRichText( doc, context );
    rowspan_ = 1;
    colspan_ = 1;
    if ( attr.contains("colspan") )
	colspan_ = attr["colspan"].toInt();
    if ( attr.contains("rowspan") )
	rowspan_ = attr["rowspan"].toInt();

    background = 0;
    if ( attr.contains("bgcolor") ) {
	background = new QBrush(QColor( attr["bgcolor"] ));
    }

    hasFixedWidth = FALSE;
    if ( attr.contains("width") ) {
	bool b;
	QString s( attr["width"] );
	int w = s.toInt( &b );
	if ( b ) {
	    maxw = w;
	    minw = maxw;
	    hasFixedWidth = TRUE;
	} else {
	    s = s.stripWhiteSpace();
	    if ( s.length() > 1 && s[ (int)s.length()-1 ] == '%' )
		stretch_ = s.left( s.length()-1).toInt();
	}
    }

    attributes = attr;

    parent->addCell( this );
}

QTextTableCell::QTextTableCell( QTextTable* table, int row, int column )
{
#if defined(PARSER_DEBUG)
    qDebug( debug_indent + "new QTextTableCell2( pappi: %p", table );
#endif
    maxw = QWIDGETSIZE_MAX;
    minw = 0;
    cached_width = -1;
    cached_sizehint = -1;

    parent = table;
    row_ = row;
    col_ = column;
    stretch_ = 0;
    richtext = new QTextDocument( table->parent );
    richtext->setTableCell( this );
    richtext->setFormatter( table->parent->formatter() );
    richtext->setUseFormatCollection( table->parent->useFormatCollection() );
    richtext->setDefaultFont( table->parent->formatCollection()->defaultFormat()->font() );
    richtext->setRichText( "<html></html>", QString::null );
    rowspan_ = 1;
    colspan_ = 1;
    background = 0;
    hasFixedWidth = FALSE;
    parent->addCell( this );
}


QTextTableCell::~QTextTableCell()
{
    delete background;
    background = 0;
    delete richtext;
    richtext = 0;
}

QSize QTextTableCell::sizeHint() const
{
    if ( cached_sizehint != -1 )
	return QSize( cached_sizehint, 0 );
    QTextTableCell *that = (QTextTableCell*)this;
    if ( stretch_ )
	return QSize( ( that->cached_sizehint = QWIDGETSIZE_MAX ), 0 );
    return QSize( ( that->cached_sizehint = richtext->widthUsed() + 2 * ( table()->innerborder + 4 ) ), 0 );
}

QSize QTextTableCell::minimumSize() const
{
    if ( stretch_ )
	return QSize( QMAX( minw, parent->width * stretch_ / 100 - 2*parent->cellspacing), 0 );
    return QSize(QMAX( richtext->minimumWidth(), minw ),0);
}

QSize QTextTableCell::maximumSize() const
{
    return QSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
}

QSizePolicy::ExpandData QTextTableCell::expanding() const
{
    return QSizePolicy::BothDirections;
}

bool QTextTableCell::isEmpty() const
{
    return FALSE;
}
void QTextTableCell::setGeometry( const QRect& r )
{
    if ( r.width() != cached_width )
	richtext->doLayout( painter(), r.width() );
    cached_width = r.width();
    richtext->setWidth( r.width() );
    richtext->flow()->height = r.height();
    geom = r;
}

QRect QTextTableCell::geometry() const
{
    return geom;
}

bool QTextTableCell::hasHeightForWidth() const
{
    return TRUE;
}

int QTextTableCell::heightForWidth( int w ) const
{
    w = QMAX( minw, w );

    if ( cached_width != w ) {
	QTextTableCell* that = (QTextTableCell*) this;
	that->richtext->doLayout( painter(), w );
	that->cached_width = w;
    }
    return richtext->height() + 2 * parent->innerborder;
}

void QTextTableCell::adjustToPainter()
{
    if ( !is_printer( painter() ) )
	return;
    richtext->formatCollection()->setPainter( painter() );
    QTextParag *parag = richtext->firstParag();
    while ( parag ) {
	parag->setPainter( painter() );
	parag = parag->next();
    }
}

QPainter* QTextTableCell::painter() const
{
    return parent->painter;
}

void QTextTableCell::draw( int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg )
{
    if ( cached_width != geom.width() ) {
	richtext->doLayout( painter(), geom.width() );
	cached_width = geom.width();
    }
    QColorGroup g( cg );
    if ( background )
	g.setBrush( QColorGroup::Base, *background );
    else if ( richtext->paper() )
	g.setBrush( QColorGroup::Base, *richtext->paper() );

    painter()->save();
    painter()->translate( x + geom.x(), y + geom.y() );
    if ( background )
	painter()->fillRect( 0, 0, geom.width(), geom.height(), *background );
    else if ( richtext->paper() )
	painter()->fillRect( 0, 0, geom.width(), geom.height(), *richtext->paper() );

    QRegion r;
    QTextCursor *c = 0;
    if ( richtext->parent()->tmpCursor )
	c = richtext->parent()->tmpCursor;
    if ( cx >= 0 && cy >= 0 )
	richtext->draw( painter(), cx - ( x + geom.x() ), cy - ( y + geom.y() ), cw, ch, g, FALSE, (c != 0), c );
    else
	richtext->draw( painter(), -1, -1, -1, -1, g, FALSE, (c != 0), c );

#if defined(DEBUG_TABLE_RENDERING)
    painter()->save();
    painter()->setPen( Qt::blue );
    painter()->drawRect( 0, 0, geom.width(), geom.height() );
    painter()->restore();
#endif

    painter()->restore();
}


///// Some of this is in qrichtext_p.cpp in qt-rsync at the moment

QTextFormat::QTextFormat()
    : fm( QFontMetrics( fn ) ), linkColor( TRUE ), logicalFontSize( 3 ), stdPointSize( qApp->font().pointSize() ),
      painter( 0 ), different( NoFlags )
{
    ref = 0;
    missp = FALSE;
    ha = AlignNormal;
    collection = 0;
//#ifdef DEBUG_COLLECTION
//    qDebug("QTextFormat simple ctor, no addRef ! %p",this);
//#endif
}
 
QTextFormat::QTextFormat( const QStyleSheetItem *style )
    : fm( QFontMetrics( fn ) ), linkColor( TRUE ), logicalFontSize( 3 ), stdPointSize( qApp->font().pointSize() ),
      painter( 0 ), different( NoFlags )
{
#ifdef DEBUG_COLLECTION
    qDebug("QTextFormat::QTextFormat( const QStyleSheetItem *style )");
#endif
    ref = 0;
    this->style = style->name();
    missp = FALSE;
    ha = AlignNormal;
    collection = 0;
    fn = QFont( style->fontFamily(),
                style->fontSize(),
                style->fontWeight(),
                style->fontItalic() );
    fn.setUnderline( style->fontUnderline() );
    col = style->color();
    fm = QFontMetrics( fn );
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.height();
    asc = fm.ascent();
    dsc = fm.descent();
    missp = FALSE;
    ha = AlignNormal;
    memset( widths, 0, 256 );
    generateKey();
    addRef();
    updateStyleFlags();
}
 
QTextFormat::QTextFormat( const QFont &f, const QColor &c, QTextFormatCollection * coll )
    : fn( f ), col( c ), fm( QFontMetrics( f ) ), linkColor( TRUE ),
      logicalFontSize( 3 ), stdPointSize( f.pointSize() ), painter( 0 ),
      different( NoFlags )
{
#ifdef DEBUG_COLLECTION
    qDebug("QTextFormat with font & color & coll (%p), addRef. %p",coll,this);
#endif
    ref = 0;
    collection = coll;
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.height();
    asc = fm.ascent();
    dsc = fm.descent();
    missp = FALSE;
    ha = AlignNormal;
    memset( widths, 0, 256 );
    generateKey();
    addRef();
    updateStyleFlags();
}
 
QTextFormat::QTextFormat( const QTextFormat &f )
    : fm( f.fm )
{
#ifdef DEBUG_COLLECTION
    qDebug("QTextFormat::QTextFormat %p copy ctor (copying %p). Will addRef.",this,&f);
#endif
    ref = 0;
    collection = 0;
    fn = f.fn;
    col = f.col;
    painter = f.painter;
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    memset( widths, 0, 256 );
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    stdPointSize = f.stdPointSize;
    logicalFontSize = f.logicalFontSize;
    missp = f.missp;
    ha = f.ha;
    k = f.k;
    anchor_name = f.anchor_name;
    anchor_href = f.anchor_href;
    linkColor = f.linkColor;
    style = f.style;
    different = f.different;
    addRef();
}
 
QTextFormat& QTextFormat::operator=( const QTextFormat &f )
{
#ifdef DEBUG_COLLECTION
    qDebug("QTextFormat::operator= %p (copying %p). Will addRef",this,&f);
#endif
    ref = 0;
    collection = f.collection;
    fn = f.fn;
    col = f.col;
    fm = f.fm;
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    memset( widths, 0, 256 );
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    stdPointSize = f.stdPointSize;
    logicalFontSize = f.logicalFontSize;
    missp = f.missp;
    ha = f.ha;
    k = f.k;
    anchor_name = f.anchor_name;
    anchor_href = f.anchor_href;
    linkColor = f.linkColor;
    style = f.style;
    different = f.different;
    addRef();
    return *this;
}
 
void QTextFormat::update()
{
    fm = QFontMetrics( fn );
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.height();
    asc = fm.ascent();
    dsc = fm.descent();
    memset( widths, 0, 256 );
    generateKey();
    updateStyleFlags();
}

void QTextFormat::addRef()
{
    ref++;
#ifdef DEBUG_COLLECTION
    if ( collection )
        qDebug( "  add ref of '%s' to %d (%p) (coll %p)", k.latin1(), ref, this, collection );
#endif
}
 
void QTextFormat::removeRef()
{
    ref--;
    if ( !collection )
        return;
#ifdef DEBUG_COLLECTION
    qDebug( "  remove ref of '%s' to %d (%p) (coll %p)", k.latin1(), ref, this, collection );
#endif
    if ( ref == 0 )
        collection->remove( this );
}

void QTextParag::setTabArray( int *a )
{
    if ( tArray )
        delete [] tArray;
    tArray = a;
}
 
void QTextParag::setTabStops( int tw )
{
    if ( doc )
        doc->setTabStops( tw );
    else
        tabStopWidth = tw;
}

