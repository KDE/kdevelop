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
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

/**
 * find/replace adapted from KOffice KoText
 * Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
 * Copyright (C) 2001, S.R.Haque <srhaque@iee.org>
 * Copyright (C) 2001, David Faure <david@mandrakesoft.com>
 */

#include "qeditor_view.h"
#include "qeditor_part.h"
#include "qeditor.h"
#include "paragdata.h"
#include "qeditorcodecompletion.h"
#include "linenumberwidget.h"
#include "markerwidget.h"
#include "levelwidget.h"
#include "gotolinedialog.h"
#include "koFind.h"
#include "koReplace.h"

#include <qregexp.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <private/qrichtext_p.h>

#include <kdebug.h>
#include <kapplication.h>


QEditorView::QEditorView( QEditorPart* document, QWidget* parent, const char* name )
    : KTextEditor::View( document, parent, name ),
      m_document( document )
{
    m_findDialog = new KoFindDialog( this, "FindDialog_0", long(KoFindDialog::FromCursor) );
    m_replaceDialog = new KoReplaceDialog( this, "ReplaceDialog_0",
                                           long(KoReplaceDialog::PromptOnReplace |
                                           KoReplaceDialog::FromCursor) );
    m_currentParag = 0;
    m_find = 0;
    m_replace = 0;
    m_options = 0;
    m_offset = 0;

    QHBoxLayout* lay = new QHBoxLayout( this );

    m_editor = new QEditor( this );
    m_lineNumberWidget = new LineNumberWidget( m_editor, this );

    m_markerWidget = new MarkerWidget( m_editor, this );
    connect( document, SIGNAL(marksChanged()),
             m_markerWidget, SLOT(doRepaint()) );

    m_levelWidget = new LevelWidget( m_editor, this );
    connect( m_levelWidget, SIGNAL(expandBlock(QTextParag*)),
	     this, SLOT(expandBlock(QTextParag*)) );
    connect( m_levelWidget, SIGNAL(collapseBlock(QTextParag*)),
	     this, SLOT(collapseBlock(QTextParag*)) );

    lay->addWidget( m_markerWidget );
    lay->addWidget( m_lineNumberWidget );
    lay->addWidget( m_levelWidget );
    lay->addWidget( m_editor );

    setFocusProxy( m_editor );
    connect( m_editor, SIGNAL(cursorPositionChanged(int, int)),
	     this, SIGNAL(cursorPositionChanged()) );

    m_pCodeCompletion = new QEditorCodeCompletion( this );
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

    configChanged();
}

QEditorView::~QEditorView()
{
    delete( m_pCodeCompletion );
    m_pCodeCompletion = 0;
}

void QEditorView::configChanged()
{
    setMarkerWidgetVisible( m_document->showMarkers() );
    setLineNumberWidgetVisible( m_document->showLineNumber() );
    setLevelWidgetVisible( m_document->showCodeFoldingMarkers() );
    m_editor->configChanged();
}


bool QEditorView::isMarkerWidgetVisible() const
{
    return m_markerWidget->isVisible();
}

void QEditorView::setMarkerWidgetVisible( bool b )
{
    if( b ){
	m_markerWidget->show();
    } else {
	m_markerWidget->hide();
    }
}

bool QEditorView::isLineNumberWidgetVisible() const
{
    return m_lineNumberWidget->isVisible();
}

void QEditorView::setLineNumberWidgetVisible( bool b )
{
    if( b ){
	m_lineNumberWidget->show();
    } else {
	m_lineNumberWidget->hide();
    }
}

bool QEditorView::isLevelWidgetVisible() const
{
    return m_levelWidget->isVisible();
}

void QEditorView::setLevelWidgetVisible( bool b )
{
    if( b ){
	m_levelWidget->show();
    } else {
	m_levelWidget->hide();
    }
}

int QEditorView::tabStop() const
{
    return m_editor->tabStop();
}

void QEditorView::setTabStop( int tabStop )
{
    m_editor->setTabStop( tabStop );
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
    kdDebug(9032) << "TODO: implement QEditorView::setCursorPosition" << endl;

    m_editor->setCursorPosition( line, col );
    m_editor->ensureCursorVisible();

    return true;
}

bool QEditorView::setCursorPositionReal(unsigned int line, unsigned int col)
{
    m_editor->setCursorPosition( line, col );
    m_editor->ensureCursorVisible();
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

    for( uint i=0; i<text.length(); ++i ){
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

void QEditorView::installPopup( QPopupMenu *rmb_Menu )
{
    m_editor->setApplicationMenu( rmb_Menu );
}

void QEditorView::showArgHint(QStringList functionList,
                              const QString& /*strWrapping*/,
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

void QEditorView::indent()
{
    m_editor->indent();
}

void QEditorView::gotoLine()
{
    GotoLineDialog dlg;
    dlg.setEditor( m_editor );
    dlg.exec();
}

void QEditorView::proceed()
{
    // Start point
    QTextParag * firstParag = m_editor->document()->firstParag();
    int firstIndex = 0;

    // 'From Cursor' option
    QEditor* edit = m_editor;
    if ( edit && ( m_options & KoFindDialog::FromCursor ) )
    {
        firstParag = edit->textCursor()->parag();
        firstIndex = edit->textCursor()->index();
    } // no else here !

    // 'Selected Text' option
    if ( edit && ( m_options & KoFindDialog::SelectedText ) )
    {
        if ( !firstParag ) // not set by 'from cursor'
        {
            QTextCursor c1 = edit->document()->selectionStartCursor( QTextDocument::Standard );
            firstParag = c1.parag();
            firstIndex = c1.index();
        }
        QTextCursor c2 = edit->document()->selectionEndCursor( QTextDocument::Standard );
        // Find in the selection
        (void) find_real( firstParag, firstIndex, c2.parag(), c2.index() );
    }
    else // Not 'find in selection', need to iterate over the framesets
    {
        QTextParag * lastParag = edit->document()->lastParag();
        (void) find_real( firstParag, firstIndex, lastParag, lastParag->length()-1 );
    }
}

bool QEditorView::find_real( QTextParag* firstParag, int firstIndex,
                             QTextParag* lastParag, int lastIndex )
{
    m_currentParag = firstParag;
    m_offset = 0;

    if( firstParag == lastParag ){
        m_offset = firstIndex;
        return process( firstParag->string()->toString().mid( firstIndex, lastIndex-firstIndex ) );
    } else {
        bool forw = ! (m_options & KoFindDialog::FindBackwards);
        bool ret = true;
        if( forw ){
            m_offset = firstIndex;
            QString str = m_currentParag->string()->toString();
            str.truncate( str.length() - 1 ); // damn trailing space
            ret = process( str.mid( firstIndex ) );
            if (!ret) return false;
        } else {
            m_currentParag = lastParag;
            ret = process( lastParag->string()->toString().left( lastIndex + 1 ) );
            if (!ret) return false;
        }

        m_currentParag = forw ? firstParag->next() : lastParag->prev();
        m_offset = 0;
        QTextParag* endParag = forw ? lastParag : firstParag;
        while( m_currentParag && m_currentParag != endParag ){
            QString str = m_currentParag->string()->toString();
            str = str.left( str.length() - 1 );
            ret = process( str );

            if (!ret) return false;
            m_currentParag = forw ? m_currentParag->next() : m_currentParag->prev();
        }
        Q_ASSERT( endParag == m_currentParag );
        if ( forw )
        {
            QString s = lastParag->string()->toString().left( lastIndex + 1 );
            ret = process( s );
        } else {
            m_offset = firstIndex;
            QString str = m_currentParag->string()->toString();
            str.truncate( str.length() - 1 ); // damn trailing space
            str = str.mid( firstIndex );
            ret = process( str );
        }
        return ret;
    }
}

void QEditorView::doFind()
{
    if( m_findDialog->exec() ){
        m_options = m_findDialog->options();
        m_find = new KoFind( m_findDialog->pattern(), m_findDialog->options() );
        connect( m_find, SIGNAL(highlight(const QString&,int,int,const QRect&)),
                 this, SLOT(highlight(const QString&,int,int,const QRect&)) );
        proceed();
        delete m_find;
        m_find = 0;
    }
}

void QEditorView::doReplace()
{
    if( m_replaceDialog->exec() ){
        m_options = m_replaceDialog->options();
        m_replace = new KoReplace( m_replaceDialog->pattern(), m_replaceDialog->replacement(),
                                   m_replaceDialog->options() );
        connect( m_replace, SIGNAL(highlight(const QString&,int,int,const QRect&)),
                 this, SLOT(highlight(const QString&,int,int,const QRect&)) );
        connect( m_replace, SIGNAL(replace(const QString&,int,int,int,const QRect&)),
                 this, SLOT(replace(const QString&,int,int,int,const QRect&)) );
        proceed();
        delete m_replace;
        m_replace = 0;
    }
}

bool QEditorView::process( const QString& _text )
{
    if( m_find ){
        return m_find->find( _text, QRect() );
    } else if( m_replace ) {
        QString text( _text );
        return m_replace->replace( text, QRect() );
    }
    return false;
}

void QEditorView::highlight( const QString& /*text*/, int matchingIndex, int matchedLength, const QRect& )
{
    m_editor->setSelection( m_currentParag->paragId(), matchingIndex,
                            m_currentParag->paragId(), matchingIndex + matchedLength );
}

void QEditorView::replace( const QString&, int matchingIndex,
                           int /*matchingLength*/, int matchedLength,
                           const QRect &/*expose*/ )
{
    m_editor->setSelection( m_currentParag->paragId(), matchingIndex,
                            m_currentParag->paragId(), matchingIndex + matchedLength );
    m_editor->removeSelectedText();
    m_editor->insertAt( m_replaceDialog->replacement(),
                        m_currentParag->paragId(),
                        matchingIndex );
}

void QEditorView::expandBlock( QTextParag* p )
{
    ParagData* data = (ParagData*) p->extraData();
    if( !data ){
        return;
    }
    int lev = QMAX( data->level() - 1, 0 );

    data->setOpen( true );

    p = p->next();
    while( p ){
        ParagData* data = (ParagData*) p->extraData();
        if( data ){
            p->show();
            data->setOpen( true );

            if( data->level() == lev ){
                break;
            }
            p = p->next();
        }
    }

    m_editor->setCursorPosition( p->paragId(), 0 );
    m_editor->refresh();
    doRepaint();
}

void QEditorView::collapseBlock( QTextParag* p )
{
    ParagData* data = (ParagData*) p->extraData();
    if( !data ){
        return;
    }

    int lev = QMAX( data->level() - 1, 0 );
    data->setOpen( false );

    p = p->next();
    while( p ){
        ParagData* data = (ParagData*) p->extraData();
        if( data ){
            p->hide();

            if( data->level() == lev ){
                break;
            }
            p = p->next();
        }
    }

    m_editor->setCursorPosition( p->paragId(), 0 );
    m_editor->refresh();
    doRepaint();
}

void QEditorView::doRepaint()
{
    m_markerWidget->doRepaint();
    m_lineNumberWidget->doRepaint();
    m_levelWidget->doRepaint();
}

void QEditorView::selectAll( )
{
    m_editor->selectAll();
}
