/* $Id$
 *
 *  Copyright (C) 2001 Roberto Raggi (roberto@kdevelop.org)
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
#include "qeditor_settings.h"
#include "qeditor_factory.h"
#include "qeditor.h"
#include "paragdata.h"
#include "qeditorcodecompletion.h"
#include "linenumberwidget.h"
#include "markerwidget.h"
#include "levelwidget.h"
#include "gotolinedialog.h"
#include "koFind.h"
#include "koReplace.h"
#include "qeditor_texthint.h"

#include <qregexp.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <private/qrichtext_p.h>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kdeversion.h>

//#include "kdevkstdaction.h"

QEditorView::QEditorView( QEditorPart* document, QWidget* parent, const char* name )
    : KTextEditor::View( document, parent, name ),
      m_document( document ), m_textHintToolTip( 0 )
{
    setInstance( QEditorPartFactory::instance() );

    QEditorPartFactory::registerView( this );
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
    connect( m_markerWidget, SIGNAL(markChanged(KTextEditor::Mark,KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
             document, SIGNAL(markChanged(KTextEditor::Mark,KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
    connect( m_markerWidget, SIGNAL(marksChanged()), document, SIGNAL(marksChanged()) );

    m_levelWidget = new LevelWidget( m_editor, this );
    connect( m_levelWidget, SIGNAL(expandBlock(QTextParagraph*)),
	     this, SLOT(expandBlock(QTextParagraph*)) );
    connect( m_levelWidget, SIGNAL(collapseBlock(QTextParagraph*)),
	     this, SLOT(collapseBlock(QTextParagraph*)) );

    lay->addWidget( m_markerWidget );
    lay->addWidget( m_lineNumberWidget );
    lay->addWidget( m_levelWidget );
    lay->addWidget( m_editor );

    setFocusProxy( m_editor );
    connect( m_editor, SIGNAL(cursorPositionChanged(int, int)),
	     this, SIGNAL(cursorPositionChanged()) );

    // connections
    connect( m_editor, SIGNAL(textChanged()),
             doc(), SIGNAL(textChanged()) );
    connect( doc(), SIGNAL(newStatus()),
             this, SIGNAL(newStatus()) );
    connect( m_editor, SIGNAL(selectionChanged()),
             doc(), SIGNAL(selectionChanged()) );

    connect( m_editor, SIGNAL(ensureTextIsVisible(QTextParagraph*)),
             this, SLOT(ensureTextIsVisible(QTextParagraph*)) );

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

    // set our XML-UI resource file
    setXMLFile( "qeditor_part.rc" );

    setupActions();

    configChanged();

    //enableTextHints( 500 );
}

QEditorView::~QEditorView()
{
    delete( m_pCodeCompletion );
    m_pCodeCompletion = 0;
    QEditorPartFactory::deregisterView( this );
}

void QEditorView::configChanged()
{
    m_editor->configChanged();

    setMarkerWidgetVisible( QEditorSettings::self()->showMarkers() );
    setLineNumberWidgetVisible( QEditorSettings::self()->showLineNumber() );
    setLevelWidgetVisible( QEditorSettings::self()->showCodeFoldingMarkers() );
}

bool QEditorView::isMarkerWidgetVisible() const
{
    return m_markerWidget->isVisible();
}

void QEditorView::setMarkerWidgetVisible( bool b )
{
    if( b ){
	m_markerWidget->show();
	m_markerWidget->doRepaint();
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
	m_lineNumberWidget->doRepaint();
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
	m_levelWidget->doRepaint();
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
    QTextStringChar *chr = cursor->paragraph()->at( cursor->index() );
    int h = cursor->paragraph()->lineHeightOfChar( cursor->index() );
    int x = cursor->paragraph()->rect().x() + chr->x;
    int y, dummy;
    cursor->paragraph()->lineHeightOfChar( cursor->index(), &dummy, &y );
    y += cursor->paragraph()->rect().y();
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

    QTextParagraph* p = m_editor->document()->paragAt( line );
    if (p)
        ensureTextIsVisible( p );

    m_editor->setCursorPosition( line, col );
    m_editor->ensureCursorVisible();

    return true;
}

bool QEditorView::setCursorPositionReal(unsigned int line, unsigned int col)
{
    QTextParagraph* p = m_editor->document()->paragAt( line );
    if( p )
        ensureTextIsVisible( p );

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
                              const QString& strWrapping,
                              const QString& strDelimiter)
{
    m_pCodeCompletion->showArgHint( functionList, strWrapping, strDelimiter );
}

void QEditorView::showCompletionBox(QValueList<KTextEditor::CompletionEntry> complList,
                                    int offset,
                                    bool casesensitive )
{
    emit aboutToShowCompletionBox();
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
    QTextParagraph * firstParagraph = m_editor->document()->firstParagraph();
    int firstIndex = 0;
    QTextParagraph * startParagraph = firstParagraph;
    int startIndex = 0;
    QTextParagraph * lastParagraph;
    int lastIndex;

    // 'From Cursor' option
    QEditor* edit = m_editor;
    if ( edit && ( m_options & KoFindDialog::FromCursor ) )
    {
        startParagraph = edit->textCursor()->paragraph();
        startIndex = edit->textCursor()->index();
    } // no else here !

    bool forw = ! (m_options & KoFindDialog::FindBackwards);

    // 'Selected Text' option
    if ( edit && ( m_options & KoFindDialog::SelectedText ) )
    {
        QTextCursor c1 = edit->document()->selectionStartCursor( QTextDocument::Standard );
        firstParagraph = c1.paragraph();
        firstIndex = c1.index();
        QTextCursor c2 = edit->document()->selectionEndCursor( QTextDocument::Standard );
        lastParagraph = c2.paragraph();
        lastIndex = c2.index();
    }
    else // Not 'find in selection', need to iterate over the framesets
    {
        lastParagraph = edit->document()->lastParagraph();
        lastIndex = lastParagraph->length()-1;
    }

    bool bProceed = true;
    if (forw) {
        while (bProceed) { // loop until cancelled
            bProceed = find_real( startParagraph, startIndex, lastParagraph, lastIndex );
            if (bProceed) {
                bProceed = find_real( firstParagraph, firstIndex, startParagraph, startIndex );
            }
            if (!m_editor->selectedText()) {
                bProceed = false; // nothing found in the whole selection or file
            }
        }
    }
    else { // backwards
        while (bProceed) { // loop until cancelled
            bProceed = find_real( firstParagraph, firstIndex, startParagraph, startIndex );
            if (bProceed) {
                bProceed = find_real( startParagraph, startIndex, lastParagraph, lastIndex );
            }
            if (!m_editor->selectedText()) {
                bProceed = false; // nothing found in the whole selection or file
            }
        }
    }
}

bool QEditorView::find_real( QTextParagraph* firstParagraph, int firstIndex,
                             QTextParagraph* lastParagraph, int lastIndex )
{
    Q_ASSERT( firstParagraph );
    Q_ASSERT( lastParagraph );

    m_currentParag = firstParagraph;
    m_offset = 0;

    if( firstParagraph == lastParagraph ){
        m_offset = firstIndex;
        return process( firstParagraph->string()->toString().mid( firstIndex, lastIndex-firstIndex ) );
    } else {
        bool forw = ! (m_options & KoFindDialog::FindBackwards);
        bool ret = true;
        if( forw ){
            m_offset = firstIndex;
            QString str = m_currentParag->string()->toString();
            ret = process( str.mid( firstIndex ) );
            if (!ret) return false;
        } else {
            m_currentParag = lastParagraph;
            ret = process( lastParagraph->string()->toString().left( lastIndex + 1 ) );
            if (!ret) return false;
        }

        m_currentParag = forw ? firstParagraph->next() : lastParagraph->prev();
        m_offset = 0;
        QTextParagraph* endParag = forw ? lastParagraph : firstParagraph;
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
            QString s = lastParagraph->string()->toString().left( lastIndex + 1 );
            ret = process( s );
        } else {
            m_offset = firstIndex;
            QString str = m_currentParag->string()->toString();
            str = str.mid( firstIndex );
            ret = process( str );
        }
        return ret;
    }
}

void QEditorView::doFind()
{
    m_findDialog->m_find->setEditURL(KURL( m_editor->selectedText() ));

    if( m_findDialog->exec() ){
        m_options = m_findDialog->options();
		if ( m_find )
		{
			m_find->abort();
			delete m_find;
		}
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
    m_replaceDialog->m_find->setEditURL(KURL( m_editor->selectedText() ));

    if( m_replaceDialog->exec() ){
        m_options = m_replaceDialog->options();
		if ( m_replace )
		{
			m_replace->abort();
			delete m_replace;
		}
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

void QEditorView::ensureTextIsVisible( QTextParagraph* p)
{
    internalEnsureVisibleBlock( p );

    m_editor->refresh();
    doRepaint();

    // some math
    QRect r = m_editor->paragraphRect(p->paragId());
    int y = r.y();
    int h = r.height();
    y = y + h/2;
    int cY = m_editor->contentsY();
    h = m_editor->viewport()->size().height();

    // if the paragraph is in the lower quarter of the viewport, center it
    if (y > (cY + (3*h)/4)) {
	m_editor->center(0, y);
    }
}

void QEditorView::internalEnsureVisibleBlock( QTextParagraph* p )
{
    ParagData* data = (ParagData*) p->extraData();

    if( !data ){
        return;
    }
    int lev = data->level(), parentLevel;

    while( lev > 0 ){
        QTextParagraph* parent = p->prev();

        parentLevel = parent ? ((ParagData*) parent->extraData())->level() : 0;

        while( parentLevel > lev ){
            parent = parent->prev();
            parentLevel = ((ParagData*) parent->extraData())->level();
        }
        if( parentLevel < lev ){
            internalExpandBlock(p);
            lev = parentLevel;
        }

        p = parent;
    }
}

void QEditorView::internalExpandBlock( QTextParagraph* p )
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
}

void QEditorView::internalCollapseBlock( QTextParagraph* p )
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

            if( data->level() == lev ){
                break;
            }

	    // kdDebug(9032) << "hide parag " << p->paragId() << " level = " << data->level() << endl;
            p->hide();

            p = p->next();
        }
    }
}

void QEditorView::expandBlock( QTextParagraph* p )
{
    internalExpandBlock( p );

    m_editor->setCursorPosition( p->paragId(), 0 );
    m_editor->refresh();
    doRepaint();
}

void QEditorView::collapseBlock( QTextParagraph* p )
{
    internalCollapseBlock( p );

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

void QEditorView::setupActions()
{
    // create our actions
    KStdAction::open( doc(), SLOT(fileOpen()), actionCollection() );
    KStdAction::saveAs( doc(), SLOT(fileSaveAs()), actionCollection() );
    KStdAction::save( doc(), SLOT(save()), actionCollection() );

    KAction *action = new KAction( i18n("Reloa&d"), "reload", Key_F5,
                 doc(), SLOT(fileReload()), actionCollection(), "file_reload" );
    action->setToolTip(i18n("Reload"));
    action->setWhatsThis(i18n("<b>Reload</b><p>Reloads the current document from disk."));

    action = KStdAction::undo( doc(), SLOT(undo()), actionCollection() );
    action->setWhatsThis(i18n("Reverts the most recent editing actions."));
    action = KStdAction::redo( doc(), SLOT(redo()), actionCollection() );
    action->setWhatsThis(i18n("Reverts the most recent undo operation."));

    action = KStdAction::cut( this, SLOT(cut()), actionCollection() );
    action->setWhatsThis(i18n("Cuts the selected text and moves it to the clipboard."));
    action = KStdAction::copy( this, SLOT(copy()), actionCollection() );
    action->setWhatsThis(i18n("Copies the selected text to the clipboard."));

    action = KStdAction::pasteText( this, SLOT(paste()), actionCollection() );
    action->setWhatsThis(i18n("Pastes previously copied or cut clipboard contents."));
    action = KStdAction::selectAll( this, SLOT(selectAll()), actionCollection() );
    action->setWhatsThis(i18n("Selects the entire text of the current document."));

    action = KStdAction::gotoLine( this, SLOT(gotoLine()), actionCollection() );
    action->setWhatsThis(i18n("Opens a dialog and lets you choose a line that you want the cursor to move to."));
    action = KStdAction::find( this, SLOT(doFind()), actionCollection() );
    action->setWhatsThis(i18n("Looks up the first occurrence of a piece of text or regular expression."));
    action = KStdAction::replace( this, SLOT(doReplace()), actionCollection() );
    action->setWhatsThis(i18n("Looks up a piece of text or regular expression and replace the result with some given text."));

    action = new KAction( i18n("&Indent"), "indent", CTRL + Key_I,
		 editor(), SLOT(indent()),
                 actionCollection(), "edit_indent" );
    action->setToolTip(i18n("Indent"));
    action->setWhatsThis(i18n("<b>Indent</b><p>Indents a selected block of text."));

    action = new KAction( i18n("Collapse All Blocks"), "collapse all blocks", 0,
		 this, SLOT(collapseAllBlocks()),
                 actionCollection(), "edit_collapse_all_blocks" );
    action->setToolTip(i18n("Collapse all blocks"));
    action->setWhatsThis(i18n("<b>Collapse all blocks</b><p>Collapses all blocks in the current document."));

    action = new KAction( i18n("Expand All Blocks"), "collapse all blocks", 0,
		 this, SLOT(expandAllBlocks()),
                 actionCollection(), "edit_expand_all_blocks" );
    action->setToolTip(i18n("Expand all blocks"));
    action->setWhatsThis(i18n("<b>Expand all blocks</b><p>Expands all blocks in the current document."));

    action = new KAction( i18n("Start Macro"), "start macro", CTRL + Key_ParenLeft,
		 editor(), SLOT(startMacro()),
                 actionCollection(), "tools_start_macro" );
    action->setToolTip(i18n("Start macro"));
    action->setWhatsThis(i18n("<b>Start macro</b><p>Starts recording a macro based on the editor input."));

    action = new KAction( i18n("Stop Macro"), "stop macro", CTRL + Key_ParenRight,
		 editor(), SLOT(stopMacro()),
                 actionCollection(), "tools_stop_macro" );
    action->setToolTip(i18n("Stop macro"));
    action->setWhatsThis(i18n("<b>Stop macro</b><p>Stops recording a macro."));

    action = new KAction( i18n("Execute Macro"), "execute macro", CTRL + Key_E,
		 editor(), SLOT(executeMacro()),
                 actionCollection(), "tools_execute_macro" );
    action->setToolTip(i18n("Execute macro"));
    action->setWhatsThis(i18n("<b>Execute macro</b><p>Executes previously recorded macro."));

    action = new KAction( i18n("&Configure Editor..."), "configure editor", 0,
		 doc(), SLOT(configDialog()),
                 actionCollection(), "set_confdlg" );
    action->setToolTip(i18n("Configure editor"));
    action->setWhatsThis(i18n("<b>Configure editor</b><p>Opens an editor configuration dialog."));
//                 actionCollection(), "settings_configure_editor" );
}

void QEditorView::expandAllBlocks()
{
    QTextParagraph* p = m_editor->document()->firstParagraph();
    while( p ){
	ParagData* data = (ParagData*) p->extraData();
	if( data && data->isBlockStart() ){
	    internalExpandBlock( p );
	}
	p = p->next();
    }

    m_editor->refresh();
    doRepaint();
}

void QEditorView::collapseAllBlocks()
{
    QTextParagraph* p = m_editor->document()->firstParagraph();
    while( p ){
	ParagData* data = (ParagData*) p->extraData();
	if( data && data->isBlockStart() ){
	    internalCollapseBlock( p );
	}
	p = p->next();
    }

    m_editor->refresh();
    doRepaint();
}

QString QEditorView::computeTextHint( int line, int column )
{
    QString s;
    emit needTextHint( line, column, s );
    return s;
}

void QEditorView::enableTextHints( int timeout )
{
    if( !m_textHintToolTip )
        m_textHintToolTip = new QEditorTextHint( this );

#if KDE_VERSION > 305
    m_textHintToolTip->setWakeUpDelay( timeout );
#else
    /// @todo set delay
#endif
}

void QEditorView::disableTextHints()
{
    if( m_textHintToolTip ){
        delete m_textHintToolTip;
	m_textHintToolTip = 0;
    }
}


#include "qeditor_view.moc"
