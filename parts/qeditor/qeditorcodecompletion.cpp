/***************************************************************************
                          qeditorcodecompletion.cpp -  description
                             -------------------


  begin      : Sun Nov 18 20:00 CET 2001
  copyright  : (C) 2001 Joseph Wenninger <jowenn@kde.org>
               (C) 2002 John Firebaugh <jfirebaugh@kde.org>
               (C) 2002 Roberto Raggi <raggi@cli.di.unipi.it>

  taken from KDEVELOP:
  begin   : Sam Jul 14 18:20:00 CEST 2001
  copyright : (C) 2001 by Victor Röder <Victor_Roeder@GMX.de>
 ***************************************************************************/

/******** Partly based on the ArgHintWidget of Qt3 by Trolltech AS *********/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qeditorcodecompletion.h"
#include "qeditorcodecompletion.moc"

// #include "qeditorcodecompletion_arghint.h"
#include "qeditor_arghint.h"
#include "qeditor_part.h"
#include "qeditor_view.h"
#include "qeditor.h"

#include <qwhatsthis.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qsizegrip.h>
#include <qapp.h>
#include <private/qrichtext_p.h>

#include <kdebug.h>

#define kdDebug() kdDebug(9032)


static QColor getColor( const QString &type )
{
    // #### should be configurable
    if ( type == "function" || type == "slot" )
	return Qt::blue;
    if ( type == "variable" )
	return Qt::darkRed;
    if ( type == "property" )
	return Qt::darkGreen;
    if ( type == "type" )
	return Qt::darkBlue;
    return Qt::black;
}

class CompletionItem : public QListBoxItem
{
public:
    CompletionItem( QListBox *lb, const KTextEditor::CompletionEntry& entry )
	: QListBoxItem( lb ), parag( 0 ), lastState( FALSE ), m_entry( entry )
        {
            m_entry.type = "";  // #### at the moment cppcodecompletion don't fill type in the right way (robe)
            setText( m_entry.text );
        }

    ~CompletionItem() { delete parag; }

    void paint( QPainter *painter ) {
	if ( lastState != isSelected() ) {
	    delete parag;
	    parag = 0;
	}
	lastState = isSelected();
	if ( !parag )
	    setupParag();
	parag->paint( *painter, listBox()->colorGroup() );
    }

    int height( const QListBox * ) const {
	if ( !parag )
	    ( (CompletionItem*)this )->setupParag();
	return parag->rect().height();
    }

    int width( const QListBox * ) const {
	if ( !parag )
	    ( (CompletionItem*)this )->setupParag();
	return parag->rect().width() - 2;
    }

    QString text() const { return QListBoxItem::text() + m_entry.postfix; }

private:
    void setupParag() {
	if ( !parag ) {
	    QTextFormatter *formatter;
	    formatter = new QTextFormatterBreakWords;
	    formatter->setWrapEnabled( FALSE );
	    parag = new QTextParag( 0 );
	    parag->pseudoDocument()->pFormatter = formatter;
	    parag->insert( 0, " " + m_entry.type + ( m_entry.type.isEmpty() ? " " : "\t" ) + m_entry.prefix + " "+
			   QListBoxItem::text() + m_entry.postfix );
	    bool selCol = isSelected() && listBox()->colorGroup().highlightedText() != listBox()->colorGroup().text();
	    QColor sc = listBox()->colorGroup().highlightedText();
	    QTextFormat *f1 = parag->formatCollection()->format( listBox()->font(), selCol ? sc : getColor( m_entry.type ) );
	    QTextFormat *f3 = parag->formatCollection()->format( listBox()->font(), isSelected() ?
								 listBox()->colorGroup().highlightedText() :
								 listBox()->colorGroup().text() );
	    QFont f( listBox()->font() );
	    f.setBold( TRUE );
	    QTextFormat *f2 =
		parag->formatCollection()->format( f, isSelected() ? listBox()->colorGroup().highlightedText() :
                                                   listBox()->colorGroup().text() );

	    parag->setFormat( 1, m_entry.type.length() + 1, f1 );
            if( m_entry.text.endsWith("(") ){
                parag->setFormat( m_entry.type.length() + 2,
                                  m_entry.prefix.length() + 1 + QListBoxItem::text().length() - 1,
                                  f2 );
            } else {
                parag->setFormat( m_entry.type.length() + 2,
                                  m_entry.prefix.length() + 1 + QListBoxItem::text().length(),
                                  f2 );
            }

	    if ( !m_entry.postfix.isEmpty() )
		parag->setFormat( m_entry.type.length() + 2 + m_entry.prefix.length() + 1 + QListBoxItem::text().length(),
				  m_entry.postfix.length(), f3 );

	    f1->removeRef();
	    f2->removeRef();
	    f3->removeRef();
	    parag->format();
	}
    }

public:
    QTextParag *parag;
    bool lastState;
    KTextEditor::CompletionEntry m_entry;
};


QEditorCodeCompletion::QEditorCodeCompletion( QEditorView* view )
    : QObject( view, "QEditor Code Completion" )
    , m_view( view )
    , m_commentLabel( 0 )
{
    m_completionPopup = new QVBox( 0, 0, WType_Popup );
    m_completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
    m_completionPopup->setLineWidth( 1 );

    m_completionListBox = new QListBox( m_completionPopup );
    m_completionListBox->setFrameStyle( QFrame::NoFrame );
    m_completionListBox->installEventFilter( this );
    m_completionListBox->setHScrollBarMode( QScrollView::AlwaysOn );
    m_completionListBox->setVScrollBarMode( QScrollView::AlwaysOn );
    m_completionListBox->setCornerWidget( new QSizeGrip(m_completionListBox) );

    m_completionPopup->installEventFilter( this );
    m_completionPopup->setFocusProxy( m_completionListBox );

    m_pArgHint = new QEditorArgHint( m_view );
    m_view->editor()->installEventFilter( m_pArgHint );
    connect( m_pArgHint, SIGNAL(argHintHidden()),
             this, SIGNAL(argHintHidden()) );

    connect( m_view, SIGNAL(cursorPositionChanged()),
             this, SLOT(slotCursorPosChanged()) );
}

void QEditorCodeCompletion::showCompletionBox(
    QValueList<KTextEditor::CompletionEntry> complList, int offset, bool casesensitive )
{
    kdDebug() << "showCompletionBox " << endl;

    m_caseSensitive = casesensitive;
    m_complList = complList;
    m_offset = offset;
    m_view->cursorPositionReal( &m_lineCursor, &m_colCursor );
    m_colCursor -= offset;

    updateBox( true );
}

bool QEditorCodeCompletion::eventFilter( QObject *o, QEvent *e )
{
    if ( o != m_completionPopup &&
         o != m_completionListBox &&
         o != m_completionListBox->viewport() )
        return false;

    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if( (ke->key() == Key_Left)  || (ke->key() == Key_Right) ||
            (ke->key() == Key_Up)    || (ke->key() == Key_Down ) ||
            (ke->key() == Key_Home ) || (ke->key() == Key_End)   ||
            (ke->key() == Key_Prior) || (ke->key() == Key_Next )) {
            QTimer::singleShot(0,this,SLOT(showComment()));
            return false;
        }
        if( ke->key() == Key_Enter || ke->key() == Key_Return ) {
            CompletionItem* item = static_cast<CompletionItem*>(
                m_completionListBox->item(m_completionListBox->currentItem()));

            if( item == 0 )
                return false;

            QString text = item->m_entry.text;
            QString currentLine = m_view->currentTextLine();
            int len = m_view->cursorColumnReal() - m_colCursor;
            QString currentComplText = currentLine.mid(m_colCursor,len);
            QString add = text.mid(currentComplText.length());
            if( item->m_entry.postfix == "()" )
                add += "(";

            emit filterInsertString(&(item->m_entry),&add);
            m_view->insertText(add);
            // HACK: move cursor. This needs to be handled in a clean way
            // by the doc/view.
            m_view->setCursorPositionReal( m_lineCursor, m_view->cursorColumnReal() + add.length() );

            complete( item->m_entry );
            m_view->setFocus();
            return false;
        }

        if( ke->key() == Key_Escape ) {
            abortCompletion();
            m_view->setFocus();
            return false;
        }

        // redirect the event to the editor
        QApplication::sendEvent( m_view->editor(), e );
        if( m_colCursor + m_offset > m_view->cursorColumnReal() ) {
            // the cursor is too far left
            kdDebug() << "Aborting Codecompletion after sendEvent" << endl;
            kdDebug() << m_view->cursorColumnReal() << endl;
            abortCompletion();
            m_view->setFocus();
            return true;
        }
        updateBox();
        return true;
    }

    if( e->type() == QEvent::FocusOut )
        abortCompletion();
    return false;
}

void QEditorCodeCompletion::abortCompletion()
{
    m_completionPopup->hide();
    delete m_commentLabel;
    m_commentLabel = 0;
    emit completionAborted();
}

void QEditorCodeCompletion::complete( KTextEditor::CompletionEntry entry )
{
    m_completionPopup->hide();
    delete m_commentLabel;
    m_commentLabel = 0;
    emit completionDone( entry );
    emit completionDone();
}

void QEditorCodeCompletion::updateBox( bool newCoordinate )
{
    m_completionListBox->clear();

    QString currentLine = m_view->currentTextLine();
    int len = m_view->cursorColumnReal() - m_colCursor;
    QString currentComplText = currentLine.mid(m_colCursor,len);

    kdDebug() << "Column: " << m_colCursor << endl;
    kdDebug() << "Line: " << currentLine << endl;
    kdDebug() << "CurrentColumn: " << m_view->cursorColumnReal() << endl;
    kdDebug() << "Len: " << len << endl;
    kdDebug() << "Text: " << currentComplText << endl;
    kdDebug() << "Count: " << m_complList.count() << endl;

    QValueList<KTextEditor::CompletionEntry>::Iterator it;
    if( m_caseSensitive ) {
        for( it = m_complList.begin(); it != m_complList.end(); ++it ) {
            if( (*it).text.startsWith(currentComplText) ) {
                new CompletionItem(m_completionListBox,*it);
            }
        }
    } else {
        currentComplText = currentComplText.upper();
        for( it = m_complList.begin(); it != m_complList.end(); ++it ) {
            if( (*it).text.upper().startsWith(currentComplText) ) {
                new CompletionItem(m_completionListBox,*it);
            }
        }
    }

    if( m_completionListBox->count() == 0 ) {
        abortCompletion();
        m_view->setFocus();
        return;
    }

    if( newCoordinate ) {
        QEditor* curEditor = m_view->editor();
        QTextCursor* cursor = curEditor->textCursor();
        QTextStringChar *chr = cursor->parag()->at( cursor->index() );
        int x = cursor->parag()->rect().x() + chr->x;
        int y, dummy;
        int h = cursor->parag()->lineHeightOfChar( cursor->index(), &dummy, &y );
        y += cursor->parag()->rect().y();

        m_completionPopup->resize( m_completionListBox->sizeHint() +
                                   QSize( m_completionListBox->verticalScrollBar()->width() + 4,
                                          m_completionListBox->horizontalScrollBar()->height() + 4 ) );

        QPoint pt = curEditor->contentsToViewport( QPoint(x, y) );
        int yy = curEditor->mapToGlobal( pt ).y() + h + m_completionListBox->height();
        if ( yy < QApplication::desktop()->height() )
            m_completionPopup->move( curEditor->mapToGlobal( curEditor->
                                                             contentsToViewport( QPoint( x, y + h ) ) ) );
        else
            m_completionPopup->move( curEditor->mapToGlobal( curEditor->
                                                             contentsToViewport( QPoint( x, y - m_completionPopup->height() ) ) ) );
    }

    m_completionListBox->setCurrentItem( 0 );
    m_completionListBox->setSelected( 0, true );
    m_completionListBox->setFocus();
    m_completionPopup->show();
    QTimer::singleShot( 0, this, SLOT(showComment()) );
}

void QEditorCodeCompletion::showArgHint ( QStringList functionList,
                                          const QString& strWrapping,
                                          const QString& strDelimiter )
{
    unsigned int line, col;
    m_view->cursorPositionReal( &line, &col );
    m_pArgHint->reset( line, col );

    m_pArgHint->setArgMarkInfos( "()", "," );

    int nNum = 0;
    for( QStringList::Iterator it = functionList.begin(); it != functionList.end(); it++ )
    {
        kdDebug() << "Insert function text: " << *it << endl;

        m_pArgHint->addFunction ( nNum, ( *it ) );

        nNum++;
    }

    m_pArgHint->move(m_view->mapToGlobal(m_view->cursorCoordinates()));
    m_pArgHint->show();
}

void QEditorCodeCompletion::slotCursorPosChanged()
{
    unsigned int line, col;
    m_view->cursorPositionReal( &line, &col );
    m_pArgHint->cursorPositionChanged( m_view, line, col );
}

void QEditorCodeCompletion::showComment()
{
    CompletionItem* item = static_cast<CompletionItem*>(m_completionListBox->item(m_completionListBox->currentItem()));

    if( !item )
        return;
    if( item->m_entry.comment.isEmpty() )
        return;

    delete m_commentLabel;
    m_commentLabel = new QEditorCodeCompletionCommentLabel( 0, item->m_entry.comment );
    m_commentLabel->setFont(QToolTip::font());
    m_commentLabel->setPalette(QToolTip::palette());

    QPoint rightPoint = m_completionPopup->mapToGlobal(QPoint(m_completionPopup->width(),0));
    QPoint leftPoint = m_completionPopup->mapToGlobal(QPoint(0,0));
    QRect screen = QApplication::desktop()->screenGeometry( m_commentLabel->x11Screen() );
    QPoint finalPoint;
    if (rightPoint.x()+m_commentLabel->width() > screen.x() + screen.width())
        finalPoint.setX(leftPoint.x()-m_commentLabel->width());
    else
        finalPoint.setX(rightPoint.x());

    m_completionListBox->ensureCurrentVisible();

    finalPoint.setY(
        m_completionListBox->viewport()->mapToGlobal(m_completionListBox->itemRect(
                                                         m_completionListBox->item(m_completionListBox->currentItem())).topLeft()).y());

    m_commentLabel->move(finalPoint);
    m_commentLabel->show();
}

#undef kdDebug

