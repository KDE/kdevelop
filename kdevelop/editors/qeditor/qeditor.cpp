/*
 *  This file is part of Klint
 *  Copyright (C) 2002 Roberto Raggi (roberto@kdevelop.org)
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
#include "qeditor_settings.h"

#include "qsourcecolorizer.h"
#include "cpp_colorizer.h"
#include "java_colorizer.h"
#include "js_colorizer.h"
#include "jsp_colorizer.h"
#include "python_colorizer.h"
#include "xml_colorizer.h"
#include "qmake_colorizer.h"
#include "cs_colorizer.h"
#include "ocaml_colorizer.h"
#include "pascal_colorizer.h"
#include "ada_colorizer.h"
#include "sql_colorizer.h"

#if defined(HAVE_PERL_MODE)
#  include "perl_colorizer.h"
#endif

#include "qeditor_indenter.h"
#include "simple_indent.h"
#include "python_indent.h"
#include "cindent.h"
#include "pascal_indent.h"
#include "ada_indent.h"

#include "parenmatcher.h"
#include "paragdata.h"

#include <private/qrichtext_p.h>
#include <qregexp.h>
#include <qmap.h>
#include <qpopupmenu.h>

#include <kdebug.h>
#include <klocale.h>

using namespace std;

int QEditor::backspace_indentation( const QString &s )
{
    int tabwidth = tabStop();
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


int QEditor::backspace_indentForLine( int line )
{
//unused    int tabwidth = tabStop();
    int line_ind = backspace_indentation( text(line) );
    line_ind = line_ind > 0 ? line_ind-1 : 0;
    int ind = 0;

    --line;
    while( line>=0 ){
        QString raw_text = text( line );
	QString lineText = raw_text.stripWhiteSpace();
	if( !lineText.isEmpty() ){
	    int new_ind = backspace_indentation( raw_text );
	    if( new_ind < line_ind ){
		ind = new_ind;
		break;
	    }
	}
	--line;
    }
    return ind;
}

struct QEditorKey{
    int key;
    int ascii;
    int state;
    QString text;
    bool autorep;
    ushort count;
};

QEditor::QEditor( QWidget* parent, const char* name )
    : KTextEdit( parent, name )
{
    document()->setUseFormatCollection( FALSE );

    parenMatcher = new ParenMatcher();

    m_tabIndent = TRUE;
    m_backspaceIndent = TRUE;
    m_currentLine = -1;
    m_tabStop = 8;
    m_applicationMenu = 0;
    m_recording = FALSE;
    m_keys.setAutoDelete( TRUE );

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
//    connect( this, SIGNAL(cursorPositionChanged(int, int) ),
//	     this, SLOT(slotCursorPositionChanged(int, int)) );
}

QEditor::~QEditor()
{
    m_keys.clear();
    delete( parenMatcher );
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
    if( m_tabStop == 0 )
        m_tabStop = 8;
}

void QEditor::keyPressEvent( QKeyEvent* e )
{
    //kdDebug(9032) << "QEditor::keyPressEvent()" << endl;
    if( e->key() == Key_Tab ){
	if( tabIndentEnabled() ){
	    int line, col;
	    getCursorPosition( &line, &col );
	    QString s = text( line );
	    if( hasSelectedText() || s.stripWhiteSpace().isEmpty() || !s.mid( col ).stripWhiteSpace().isEmpty() )
		indent();
	    else
		insert( "\t" );
	} else
	    insert( "\t" );
	e->accept();
    } else if( m_electricKeys.contains( e->ascii() ) ){
	insert( e->text(), FALSE );
        indent();
	e->accept();
    } else if( e->ascii() == '{' || e->ascii() == '}' ||
               e->ascii() == ':' || e->ascii() == '#' ){
	insert( e->text(), FALSE );
	e->accept();
    } else if (e->state() == Qt::ControlButton) {
	bool bRemove = false;
        switch (e->key()) {
        case Qt::Key_Backspace:
	    bRemove = true;
	    if (!hasSelectedText()) {
		removeSelection();
	    }
	case Qt::Key_Left: {
	    QTextCursor* cur = textCursor();
	    if (cur->index() < 1) {
		moveCursor( MoveBackward, bRemove ); break;
	    }
	    QChar c(cur->paragraph()->at(cur->index()-1)->c);
	    bool firstMove = true; // make sure we do move
	    if (c.isSpace()) {
		while (cur->index() > 0 && (cur->paragraph()->at(cur->index()-1)->c.isSpace() || firstMove)) {
		    moveCursor( MoveBackward, bRemove ); firstMove = false; cur = textCursor();
		}
	    }
	    else if (isDelimiter(c)) {
		while (cur->index() > 0 && (isDelimiter(cur->paragraph()->at(cur->index()-1)->c) || firstMove)) {
		    moveCursor( MoveBackward, bRemove ); firstMove = false; cur = textCursor();
		}
	    }
	    else if (!isDelimiter(c)) {
		while (cur->index() > 0 && (!isDelimiter(cur->paragraph()->at(cur->index()-1)->c) || firstMove)) {
		    moveCursor( MoveBackward, bRemove ); firstMove = false; cur = textCursor();
		}
	    }
	    }
	    break;
        case Qt::Key_Delete:
	    bRemove = true;
	    if (!hasSelectedText()) {
		removeSelection();
	    }
	case Qt::Key_Right: {
	    QTextCursor* cur = textCursor();
	    if (cur->atParagEnd()) {
		moveCursor( MoveForward, bRemove ); break;
	    }
	    QChar c(cur->paragraph()->at(cur->index())->c);
	    bool firstMove = true; // make sure we do move
	    if (c.isSpace()) {
		while (!cur->atParagEnd() && (cur->paragraph()->at(cur->index())->c.isSpace() || firstMove)) {
		    moveCursor( MoveForward, bRemove ); firstMove = false; cur = textCursor();
		}
	    }
	    else if (!isDelimiter(c)) {
		while (!cur->atParagEnd() && (!isDelimiter(cur->paragraph()->at(cur->index())->c) || firstMove)) {
		    moveCursor( MoveForward, bRemove ); firstMove = false; cur = textCursor();
		}
	    }
	    else if (isDelimiter(c)) {
		while (!cur->atParagEnd() && (isDelimiter(cur->paragraph()->at(cur->index())->c) || firstMove)) {
		    moveCursor( MoveForward, bRemove ); firstMove = false; cur = textCursor();
		}
	    }
	    }
	    break;
        default:
	    KTextEdit::keyPressEvent( e );
	    break;
        }
	if (bRemove) {
	    removeSelectedText();
        }
    }
    else if( e->key() == Key_Backspace ){
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
    QTextParagraph *p = document()->paragAt( line );
    if ( !p )
	return;
    QTextCursor c( document() );
    emit ensureTextIsVisible( p );
    c.setParagraph( p );
    c.setIndex( 0 );
    document()->removeSelection( 1000 );
    document()->setSelectionStart( 1000, c );
    c.gotoLineEnd();
    document()->setSelectionEnd( 1000, c );
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

    if( QEditorSettings::self()->wordWrap() ){
	setWordWrap( QEditor::WidgetWidth );
	setHScrollBarMode( QScrollView::AlwaysOff );
	setVScrollBarMode( QScrollView::AlwaysOn );
    } else {
	setWordWrap( QEditor::NoWrap );
	setHScrollBarMode( QScrollView::AlwaysOn );
	setVScrollBarMode( QScrollView::AlwaysOn );
    }

    refresh();
}

void QEditor::zoomIn()
{
    KTextEdit::zoomIn();
    updateStyles();
    refresh();
}

void QEditor::zoomOut()
{
    KTextEdit::zoomOut();
    updateStyles();
    refresh();
}

void QEditor::updateStyles()
{
    //kdDebug(9032) << "QEditor::updateStyles()" << endl;
    int tabwidth = tabStop();
    QSourceColorizer* colorizer = dynamic_cast<QSourceColorizer*>( document()->preProcessor() );
    if( colorizer ){
	setFont( colorizer->format( 0 )->font() );
	setTabStopWidth( colorizer->format(0)->width('x') * tabwidth );
	document()->setTabStops( colorizer->format(0)->width('x') * tabwidth );
    }
    KTextEdit::updateStyles();
}

void QEditor::backspaceIndent( QKeyEvent* e )
{
    QTextCursor* c = textCursor();
    QTextParagraph* p = c->paragraph();
    QString raw_text = text( p->paragId() );
    QString line = raw_text.stripWhiteSpace();

    if( raw_text.left(c->index()).stripWhiteSpace().isEmpty()
	&& c->index() > 0 && !hasSelectedText() ){
	    drawCursor( FALSE );
	    int oi = backspace_indentation( raw_text );
	    int ni = backspace_indentForLine( p->paragId() );

	    if( indenter() )
	      indenter()->indentLine( p, oi, ni );

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
	    setSelection( textCursor()->paragraph()->paragId(),
			  textCursor()->index() - replace.length(),
			  textCursor()->paragraph()->paragId(),
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
    return text( line );
}

void QEditor::setLanguage( const QString& l )
{
    kdDebug(9032) << "QEditor::setLanguage(" << l << ")" << endl;
    m_language = l;
    if( m_language == "c++" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new CppColorizer(this) );
	document()->setIndent( new CIndent(this) );
    } else if( m_language == "java" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new JavaColorizer(this) );
	document()->setIndent( new CIndent(this) );
    } else if( m_language == "javascript" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new JSColorizer(this) );
	document()->setIndent( new CIndent(this) );
    } else if( m_language == "jsp" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new JspColorizer(this) );
	document()->setIndent( new SimpleIndent(this) );
    } else if( m_language == "csharp" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new CSharpColorizer(this) );
	document()->setIndent( new CIndent(this) );
#if defined(HAVE_PERL_MODE)
    } else if( m_language == "perl" ){
        setElectricKeys( "{}" );
	document()->setPreProcessor( new PerlColorizer(this) );
	document()->setIndent( new CIndent(this) );
#endif
    } else if( m_language == "python" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new PythonColorizer(this) );
	document()->setIndent( new PythonIndent(this) );
    } else if( m_language == "xml" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new XMLColorizer(this) );
	document()->setIndent( new SimpleIndent(this) );
    } else if( m_language == "qmake" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new QMakeColorizer(this) );
	document()->setIndent( new SimpleIndent(this) );
    } else if( m_language == "ocaml" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new OCamlColorizer(this) );
	document()->setIndent( new SimpleIndent(this) );
    } else if( m_language == "pascal" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new PascalColorizer(this) );
	document()->setIndent( new PascalIndent(this) );
    } else if( m_language == "ada" ){
        setElectricKeys( QString::null );
	document()->setPreProcessor( new AdaColorizer(this) );
	document()->setIndent( new AdaIndent(this) );
    } else if( m_language == "sql" ){
	setElectricKeys( QString::null );
	document()->setPreProcessor( new SqlColorizer(this) );
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

void QEditor::slotCursorPositionChanged( int line, int column )
{
    Q_UNUSED( line );
    Q_UNUSED( column );
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

void QEditor::refresh()
{
    document()->invalidate();
    QTextParagraph* p = document()->firstParagraph();
    while( p ){
        p->format();
	p = p->next();
    }
    removeSelection( ParenMatcher::Match );
    removeSelection( ParenMatcher::Mismatch );
    ensureCursorVisible();
    repaintContents( false );
}

bool QEditor::event( QEvent* e )
{
    if( isRecording() && e->type() == QEvent::KeyPress ){
        QKeyEvent* ke = (QKeyEvent*) e;
        //kdDebug(9032) << "recording" << endl;
        QEditorKey* k = new QEditorKey;
        k->key = ke->key();
        k->ascii = ke->ascii();
        k->state = ke->state();
        k->text = ke->text();
        k->autorep = ke->isAutoRepeat();
        k->count = ke->count();

        m_keys.append( k );
    }
    return QTextEdit::event( e );
}

void QEditor::startMacro()
{
    m_keys.clear();
    setIsRecording( TRUE );
}

void QEditor::stopMacro()
{
    setIsRecording( FALSE );
}

void QEditor::executeMacro()
{
    QPtrListIterator<QEditorKey> it( m_keys );
    while( it.current() ){
        //kdDebug(9032) << "send key" << endl;
        QEditorKey* k = it.current();
        ++it;

        QKeyEvent e( QEvent::KeyPress,
                     k->key,
                     k->ascii,
                     k->state,
                     k->text,
                     k->autorep,
                     k->count );
        QApplication::sendEvent( this, &e );
    }
}

QEditorIndenter* QEditor::indenter() const
{
  return dynamic_cast<QEditorIndenter*>( document()->indent() );
}

void QEditor::indent()
{
    KTextEdit::indent();
    if( !hasSelectedText() && text( textCursor()->paragraph()->paragId() ).stripWhiteSpace().isEmpty() )
	moveCursor( MoveLineEnd, false );
}

void QEditor::contentsMouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() != Qt::LeftButton ) {
        e->ignore();
        return;
    }

    // let the base class do things we maybe don't know
    KTextEdit::contentsMouseDoubleClickEvent(e);

    // now do our own method of marking text:
    int para = 0;
    int index = charAt( e->pos(), &para );
    setCursorPosition(para, index);

    QTextCursor* cur = textCursor();
    QTextCursor c1 = *cur;
    QTextCursor c2 = *cur;
    if (c1.paragraph()->at(c1.index())->c.isSpace()) return;

    // find start of text to select..
    while (c1.index() > 0 && !isDelimiter(c1.paragraph()->at(c1.index()-1)->c)) {
        c1.gotoLeft();
    }
    // find end of text to select..
    while ( !isDelimiter(c2.paragraph()->at(c2.index())->c) && !c2.atParagEnd() ) {
        c2.gotoRight();
        cur->gotoRight();
    }

    document()->setSelectionStart( QTextDocument::Standard, c1 );
    document()->setSelectionEnd( QTextDocument::Standard, c2 );

    repaintChanged();
}

bool QEditor::isDelimiter(const QChar& c)
{
    if (c == '_') return false;
    return !(c.isLetterOrNumber());
}

#include "qeditor.moc"
