/* $Id$
 *
 *  This file is part of Klint
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

/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qeditor.h"

#include "qsourcecolorizer.h"
#include "cpp_colorizer.h"
#include "java_colorizer.h"
#include "python_colorizer.h"
#include "xml_colorizer.h"
#include "qmake_colorizer.h"

#if defined(HAVE_PERL_MODE)
#  include "perl_colorizer.h"
#endif

#include "simple_indent.h"
#include "cindent.h"

#include "parenmatcher.h"
#include "paragdata.h"

#include <private/qrichtext_p.h>
#include <qregexp.h>
#include <qmap.h>
#include <qpopupmenu.h>

#include <kdebug.h>
#include <klocale.h>

using namespace std;

// from trolltech's editor -- START
static int backspace_indentation( const QString &s, int tabwidth )
{
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


static int backspace_indentForLine( QTextParag* parag, int tabwidth )
{
    int line_ind = backspace_indentation( parag->string()->toString(), tabwidth );
    line_ind = line_ind > 0 ? line_ind-1 : 0;
    int ind = 0;
    QTextParag* p = parag->prev();
    while( p ){
	QString raw_text = p->string()->toString();
	QString line = raw_text.stripWhiteSpace();
	if( !line.isEmpty() ){
	    int new_ind = backspace_indentation( raw_text, tabwidth );
	    if( new_ind < line_ind ){
		ind = new_ind;
		break;
	    }
	}
	p = p->prev();
    }
    return ind;
}

QEditor::QEditor( QWidget* parent, const char* name )
    : KTextEdit( parent, name )
{
    setWordWrap( NoWrap );
    setHScrollBarMode( QScrollView::AlwaysOn );
    setVScrollBarMode( QScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );

    parenMatcher = new ParenMatcher();

    m_tabIndent = TRUE;
    m_backspaceIndent = TRUE;
    m_currentLine = -1;
    m_tabStop = 8;
    m_applicationMenu = 0;

    document()->addSelection( ParenMatcher::Match );
    document()->addSelection( ParenMatcher::Mismatch );
    document()->setSelectionColor( ParenMatcher::Match, QColor( 204, 232, 195 ) );
    document()->setSelectionColor( ParenMatcher::Mismatch, Qt::magenta );
    document()->setInvertSelectionText( ParenMatcher::Match, FALSE );
    document()->setInvertSelectionText( ParenMatcher::Mismatch, FALSE );

    document()->addSelection( 1000 );
    document()->setSelectionColor( 1000, QColor( 204, 232, 195 ) );

    connect( this, SIGNAL(cursorPositionChanged(QTextCursor*) ),
	     this, SLOT(doMatch(QTextCursor*)) );
    connect( this, SIGNAL(cursorPositionChanged(int, int) ),
	     this, SLOT(slotCursorPositionChanged(int, int)) );
}

QEditor::~QEditor()
{
    if( parenMatcher ){
	delete( parenMatcher );
    }
}

QPopupMenu* QEditor::createPopupMenu( const QPoint& pt )
{
    QPopupMenu* menu = KTextEdit::createPopupMenu( pt );
    if( m_applicationMenu ){
	    menu->insertSeparator();
	    menu->insertItem( i18n("&Application"), m_applicationMenu );
	}
    return menu;
}


int QEditor::tabStop() const
{
    return m_tabStop;
}

void QEditor::setTabStop( int tabStop )
{
    m_tabStop = tabStop;
}

void QEditor::keyPressEvent( QKeyEvent* e )
{
    if( e->key() == Key_Tab ){
	if( tabIndentEnabled() ){
	    int parag, index;
	    getCursorPosition( &parag, &index );
	    QString s = textLine( parag ).mid( index ).stripWhiteSpace();
	    if( s.isEmpty() ){
		insert( "\t" );
	    } else {
		indent();
	    }
	} else {
	    insert( "\t" );
	}
	e->accept();
    } else if( m_electricKeys.contains( e->ascii() ) ){
	insert( e->text(), FALSE );
        indent();
	e->accept();
    } else if( e->ascii() == '{' || e->ascii() == '}' ||
               e->ascii() == ':' || e->ascii() == '#' ){
	insert( e->text(), FALSE );
	e->accept();
    } else if( e->key() == Key_Backspace ){
	if( backspaceIndentEnabled() ){
	    backspaceIndent( e );
	} else {
	    KTextEdit::keyPressEvent( e );
	}
    } else {
	KTextEdit::keyPressEvent( e );
    }
}

void QEditor::doMatch( QTextCursor* c )
{
    if( parenMatcher->match(c) ){
	repaintChanged();
    }
}

void QEditor::doGotoLine( int line )
{
    setCursorPosition( line, 0 );
    QTextParag *p = document()->paragAt( line );
    if ( !p )
	return;
    QTextCursor c( document() );
    c.setParag( p );
    c.setIndex( 0 );
    document()->removeSelection( 1000 );
    document()->setSelectionStart( 1000, &c );
    c.gotoLineEnd();
    document()->setSelectionEnd( 1000, &c );
    viewport()->repaint( FALSE );
}

QTextCursor* QEditor::textCursor() const
{
    return KTextEdit::textCursor();
}

QTextDocument* QEditor::document() const
{
    return KTextEdit::document();
}

void QEditor::drawCursor( bool visible )
{
    KTextEdit::drawCursor( visible );
}

void QEditor::configChanged()
{
    updateStyles();
    internalRefresh();
}

void QEditor::zoomIn()
{
    KTextEdit::zoomIn();
    updateStyles();
}

void QEditor::zoomOut()
{
    KTextEdit::zoomOut();
    updateStyles();
}

void QEditor::updateStyles()
{
    kdDebug() << "QEditor::updateStyles()" << endl;
    int tabwidth = tabStop();
    QSourceColorizer* colorizer = dynamic_cast<QSourceColorizer*>( document()->preProcessor() );
    if( colorizer ){
	setTabStopWidth( colorizer->format(0)->width('x') * tabwidth );
	document()->setTabStops( colorizer->format(0)->width('x') * tabwidth );
	setFont( colorizer->format( 0 )->font() );
    }
    KTextEdit::updateStyles();
}

void QEditor::backspaceIndent( QKeyEvent* e )
{
    QTextCursor* c = textCursor();
    QTextParag* p = c->parag();
    QString raw_text = p->string()->toString();
    QString line = raw_text.stripWhiteSpace();

    if( raw_text.left(c->index()).stripWhiteSpace().isEmpty()
	&& c->index() > 0 && !hasSelectedText() ){
	    drawCursor( FALSE );
	    int oi = backspace_indentation( raw_text, tabStop() );
	    int ni = backspace_indentForLine( p, tabStop() );

	    indentLine( p, tabStop(), oi, ni );

	    int idx = c->index();
	    if ( idx >= oi )
		idx += ni - oi;
	    else
		idx = ni;
	    c->setIndex( idx );
	    repaintChanged();
	    drawCursor( TRUE );
	    e->accept();
	} else {
	    // doKeyboardAction( KTextEdit::ActionBackspace );
	    KTextEdit::keyPressEvent( e );
	}
    }

bool QEditor::replace( const QString &text, const QString &replace,
		       bool cs, bool wo, bool forward, bool startAtCursor,
		       bool replaceAll )
{
    // from trolltech's editor sources -- START
    bool ok = FALSE;
    if ( startAtCursor ) {
	ok = find( text, cs, wo, forward );
    } else {
	int dummy = 0;
	ok =  find( text, cs, wo, forward, &dummy, &dummy );
    }

    if ( ok ) {
	removeSelectedText();
	insert( replace, FALSE, FALSE );
    }

    if ( !replaceAll || !ok ) {
	if ( ok )
	    setSelection( textCursor()->parag()->paragId(),
			  textCursor()->index() - replace.length(),
			  textCursor()->parag()->paragId(),
			  textCursor()->index() );
	return ok;
    }

    bool ok2 = TRUE;
    while ( ok2 ) {
	ok2 = find( text, cs, wo, forward );
	if ( ok2 ) {
	    removeSelectedText();
	    insert( replace, FALSE, FALSE );
	}
    }

    return TRUE;
    // from trolltech's editor sources -- END
}

void QEditor::setDocument( QTextDocument* doc )
{
    KTextEdit::setDocument( doc );
}

void QEditor::repaintChanged()
{
    KTextEdit::repaintChanged();
}

QString QEditor::textLine( uint line ) const
{
    if ( line < 0 || line >= lines() )
	return QString::null;

    QString str = document()->paragAt( line )->string()->toString();
    str.truncate( str.length() - 1 );
    return str;
}

void QEditor::setLanguage( const QString& l )
{
    kdDebug() << "QEditor::setLanguage(" << l << ")" << endl;
    m_language = l;
    if( m_language == "c++" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new CppColorizer(this) );
	document()->setIndent( new CIndent() );
    } else if( m_language == "java" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new JavaColorizer(this) );
	document()->setIndent( new CIndent() );
#if defined(HAVE_PERL_MODE)
    } else if( m_language == "perl" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new PerlColorizer(this) );
	document()->setIndent( new CIndent() );
#endif
    } else if( m_language == "python" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new PythonColorizer(this) );
	document()->setIndent( new SimpleIndent(this) );
    } else if( m_language == "xml" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new XMLColorizer(this) );
	document()->setIndent( new SimpleIndent(this) );
    } else if( m_language == "qmake" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new QMakeColorizer(this) );
	document()->setIndent( new SimpleIndent(this) );
    } else {
        setElectricKeys( QString::null );
	document()->setPreProcessor( 0 );
	document()->setIndent( new SimpleIndent(this) );
    }

    configChanged();
    sync();
}

QString QEditor::language() const
{
    return m_language;
}

void QEditor::setText( const QString& text )
{
    setTextFormat( KTextEdit::PlainText );
    QString s = text;
    // tabify( s );
    KTextEdit::setText( s );
    setTextFormat( KTextEdit::AutoText );
}

void QEditor::slotCursorPositionChanged( int line, int )
{
#if 0
    if( line != m_currentLine ){
	if( m_currentLine != -1 ){
	    clearParagraphBackground( m_currentLine );
	}
	m_currentLine = line;
	setParagraphBackgroundColor( m_currentLine,
				     palette().active().mid() );
    }
#endif
}

int QEditor::level( int line) const
{
    ParagData* data = (ParagData*) document()->paragAt( line )->extraData();
    if( data ){
	return data->level();
    }
    return 0;
}


void QEditor::setLevel( int line, int lev )
{
    ParagData* data = (ParagData*) document()->paragAt( line )->extraData();
    if( data ){
        return data->setLevel( lev );
    }
}

QSourceColorizer* QEditor::colorizer() const
{
    return dynamic_cast<QSourceColorizer*>( document()->preProcessor() );
}

void QEditor::internalRefresh()
{
    QTextParag* p = document()->firstParag();
    while( p ){
	if( p->endState() == -1 ){
	    break;
	}
	p->setEndState( -1 );
        p->invalidate( 0 );
	p = p->next();
    }
    sync();
    viewport()->repaint( true );
    ensureCursorVisible();
}
