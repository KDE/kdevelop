/* $Id$
 *
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

#include "qeditor_view.h"
#include "qeditor.h"
#include "qeditorcodecompletion_iface_impl.h"
#include "linenumberwidget.h"
#include "markerwidget.h"

#include <qlayout.h>
#include <qpopupmenu.h>
#include <private/qrichtext_p.h>

#include <kdebug.h>

QEditorView::QEditorView( KTextEditor::Document* document, QWidget* parent, const char* name )
	: KTextEditor::View( document, parent, name ),
	  m_document( document ),
          m_popupMenu( 0 )
{
	QHBoxLayout* lay = new QHBoxLayout( this );


	m_editor = new QEditor( this );
	m_lineNumberWidget = new LineNumberWidget( m_editor, this );
        m_markerWidget = new MarkerWidget( m_editor, this );

	lay->addWidget( m_lineNumberWidget );
        lay->addWidget( m_markerWidget );
	lay->addWidget( m_editor );

	setFocusProxy( m_editor );
	connect( m_editor, SIGNAL(cursorPositionChanged(int, int)),
			 this, SIGNAL(cursorPositionChanged()) );

	m_pCodeCompletion = new CodeCompletion_Impl( this );
	connect(m_pCodeCompletion,SIGNAL(completionAborted()),
			this,SIGNAL(completionAborted()));
	connect(m_pCodeCompletion,SIGNAL(completionDone()),
			this,SIGNAL(completionDone()));
	connect(m_pCodeCompletion,SIGNAL(argHintHidden()),
			this,SIGNAL(argHintHidden()));
	connect(m_pCodeCompletion,SIGNAL(completionDone(KTextEditor::CompletionEntry)),
			this,SIGNAL(completionDone(KTextEditor::CompletionEntry)));
	connect(m_pCodeCompletion,SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString *)),
			this,SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString *)) );
}

QEditorView::~QEditorView()
{
	delete( m_pCodeCompletion );
	m_pCodeCompletion = 0;
}

KTextEditor::Document* QEditorView::document() const
{
	return m_document;
}

QPoint QEditorView::cursorCoordinates()
{
	QTextCursor *cursor = m_editor->textCursor();
	QTextStringChar *chr = cursor->parag()->at( cursor->index() );
	int h = cursor->parag()->lineHeightOfChar( cursor->index() );
	int x = cursor->parag()->rect().x() + chr->x;
	int y, dummy;
	cursor->parag()->lineHeightOfChar( cursor->index(), &dummy, &y );
	y += cursor->parag()->rect().y();
	return m_editor->contentsToViewport( QPoint( x, y+h ) );
}

void QEditorView::cursorPosition(unsigned int *line, unsigned int *col)
{
	*line = cursorLine();
	*col = cursorColumn();
}

void QEditorView::cursorPositionReal(unsigned int *line, unsigned int *col)
{
	*line = cursorLine();
	*col = cursorColumnReal();
}

bool QEditorView::setCursorPosition(unsigned int line, unsigned int col)
{
#warning "TODO: implement QEditorView::setCursorPosition"
    kdDebug() << "TODO: implement QEditorView::setCursorPosition" << endl;
    m_editor->setCursorPosition( line, col );
	return FALSE;
}

bool QEditorView::setCursorPositionReal(unsigned int line, unsigned int col)
{
	m_editor->setCursorPosition( line, col );
	return true;
}

unsigned int QEditorView::cursorLine()
{
	int line, col;
	m_editor->getCursorPosition( &line, &col );
	return line;
}

unsigned int QEditorView::cursorColumn()
{
	const int tabwidth = 4;
	int line, col;

	m_editor->getCursorPosition( &line, &col );
	QString text = m_editor->text( line ).left( col );
	col = 0;

	for( int i=0; i<text.length(); ++i ){
		if( text[ i ] == QChar('\t') ){
			col += tabwidth - (col % tabwidth);
		} else {
			++col;
		}
	}
	return col;
}

unsigned int QEditorView::cursorColumnReal()
{
	int line, col;
	m_editor->getCursorPosition( &line, &col );
	return col;
}

void QEditorView::copy( ) const
{
	m_editor->copy();
}

void QEditorView::cut( )
{
	m_editor->cut();
}

void QEditorView::paste( )
{
	m_editor->paste();
}

void QEditorView::installPopup( class QPopupMenu *rmb_Menu )
{
	kdDebug() << "QEditorView::installPopup()" << endl;
        m_popupMenu = rmb_Menu;
}

void QEditorView::showArgHint(QStringList functionList,
							  const QString& strWrapping,
							  const QString& strDelimiter)
{
	m_pCodeCompletion->showArgHint( functionList, strDelimiter, strDelimiter );
}

void QEditorView::showCompletionBox(QValueList<KTextEditor::CompletionEntry> complList,
									int offset,
									bool casesensitive )
{
	m_pCodeCompletion->showCompletionBox( complList, offset, casesensitive );
}

QString QEditorView::currentTextLine() const
{
	int line, col;
	m_editor->getCursorPosition( &line, &col );
	return m_editor->text( line );
}

void QEditorView::insertText( const QString& text )
{
	m_editor->insert( text );
}

void QEditorView::setLanguage( const QString& language )
{
	m_editor->setLanguage( language );
}

QString QEditorView::language() const
{
	return m_editor->language();
}

void QEditorView::contextMenuEvent( QContextMenuEvent* e )
{
	e->accept();
}
