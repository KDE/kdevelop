/***************************************************************************
                          qeditorcodecompletion.cpp -  description
                             -------------------


  begin      : Sun Nov 18 20:00 CET 2001
  copyright  : (C) 2001 Joseph Wenninger <jowenn@kde.org>
               (C) 2002 John Firebaugh <jfirebaugh@kde.org>
               (C) 2002 Roberto Raggi <roberto@kdevelop.org>

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
#include "qeditor_settings.h"
#include "qeditorcodecompletion.moc"

// #include "qeditorcodecompletion_arghint.h"
#include "qeditor_arghint.h"
#include "qeditor_part.h"
#include "qeditor_view.h"
#include "qeditor.h"

#include <q3whatsthis.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qsizegrip.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QEvent>
#include <QKeyEvent>
#include <Q3Frame>
#include <Q3ValueList>
#include <private/qrichtext_p.h>

#include <kdebug.h>
//default size for codecompletionlistbox, value may not be ideal, change later
QSize CCListBox::m_size = QSize(300,200);

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

class CompletionItem : public Q3ListBoxItem
{
public:
    CompletionItem( Q3ListBox *lb, const KTextEditor::CompletionEntry& entry )
	: Q3ListBoxItem( lb ), parag( 0 ), lastState( FALSE ), m_entry( entry )
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

    int height( const Q3ListBox * ) const {
	if ( !parag )
	    ( (CompletionItem*)this )->setupParag();
	return parag->rect().height();
    }

    int width( const Q3ListBox * ) const {
	if ( !parag )
	    ( (CompletionItem*)this )->setupParag();
	return parag->rect().width() - 2;
    }

    QString text() const { return Q3ListBoxItem::text() + m_entry.postfix; }

private:
    void setupParag() {
	if ( !parag ) {
	    QTextFormatter *formatter;
	    formatter = new QTextFormatterBreakWords;
	    formatter->setWrapEnabled( FALSE );
	    parag = new QTextParagraph( 0 );
	    parag->pseudoDocument()->pFormatter = formatter;
	    parag->insert( 0, " " + m_entry.type + ( m_entry.type.isEmpty() ? " " : "\t" ) + m_entry.prefix + " "+
			   Q3ListBoxItem::text() + m_entry.postfix );
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
                                  m_entry.prefix.length() + 1 + Q3ListBoxItem::text().length() - 1,
                                  f2 );
            } else {
                parag->setFormat( m_entry.type.length() + 2,
                                  m_entry.prefix.length() + 1 + Q3ListBoxItem::text().length(),
                                  f2 );
            }

	    if ( !m_entry.postfix.isEmpty() )
		parag->setFormat( m_entry.type.length() + 2 + m_entry.prefix.length() + 1 + Q3ListBoxItem::text().length(),
				  m_entry.postfix.length(), f3 );

	    f1->removeRef();
	    f2->removeRef();
	    f3->removeRef();
	    parag->format();
	}
    }

public:
    QTextParagraph *parag;
    bool lastState;
    KTextEditor::CompletionEntry m_entry;
};


QEditorCodeCompletion::QEditorCodeCompletion( QEditorView* view )
    : QObject( view, "QEditor Code Completion" )
    , m_view( view )
    , m_commentLabel( 0 )
{
    m_completionPopup = new Q3VBox( 0, 0, Qt::WType_Popup );
    m_completionPopup->setFrameStyle( Q3Frame::Box | Q3Frame::Plain );
    m_completionPopup->setLineWidth( 1 );

    m_completionListBox = new CCListBox( m_completionPopup );
    m_completionListBox->setFrameStyle( Q3Frame::NoFrame );
    m_completionListBox->installEventFilter( this );
    m_completionListBox->setHScrollBarMode( Q3ScrollView::AlwaysOn );
    m_completionListBox->setVScrollBarMode( Q3ScrollView::AlwaysOn );
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
    Q3ValueList<KTextEditor::CompletionEntry> complList, int offset, bool casesensitive )
{
    kdDebug(9032) << "showCompletionBox " << endl;

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
        if( (ke->key() == Qt::Key_Left)  || (ke->key() == Qt::Key_Right) ||
            (ke->key() == Qt::Key_Up)    || (ke->key() == Qt::Key_Down ) ||
            (ke->key() == Qt::Key_Home ) || (ke->key() == Qt::Key_End)   ||
            (ke->key() == Qt::Key_PageUp) || (ke->key() == Qt::Key_PageDown )) {
            QTimer::singleShot(0,this,SLOT(showComment()));
            return false;
        }
        if( ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return ||
            (QEditorSettings::self()->completeWordWithSpace() && (ke->key() == Qt::Key_Space || ke->key() == Qt::Key_Tab)) ) {
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

            if( QEditorSettings::self()->completeWordWithSpace() ){
                if( ke->key() == Qt::Key_Space )
                    m_view->insertText( " " );
                else if( ke->key() == Qt::Key_Tab )
                    m_view->insertText( "\t" );
            }

            // HACK: move cursor. This needs to be handled in a clean way
            // by the doc/view.
            //m_view->setCursorPositionReal( m_lineCursor, m_view->cursorColumnReal() + add.length() );

            complete( item->m_entry );
            m_view->setFocus();
            return false;
        }

        if( ke->key() == Qt::Key_Escape ) {
            abortCompletion();
            m_view->setFocus();
            return false;
        }

        // redirect the event to the editor
        QApplication::sendEvent( m_view->editor(), e );

        QString currentLine = m_view->currentTextLine();
        int len = m_view->cursorColumnReal() - m_colCursor;
        QString currentComplText = currentLine.mid( m_colCursor, len );
	
        if( m_colCursor + m_offset > m_view->cursorColumnReal() ||
	    (m_completionListBox->count() == 1 && m_completionListBox->currentText() == currentComplText) ) {
            // the cursor is too far left
            kdDebug(9032) << "Aborting Codecompletion after sendEvent" << endl;
            kdDebug(9032) << m_view->cursorColumnReal() << endl;
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

    kdDebug(9032) << "Column: " << m_colCursor << endl;
    kdDebug(9032) << "Line: " << currentLine << endl;
    kdDebug(9032) << "CurrentColumn: " << m_view->cursorColumnReal() << endl;
    kdDebug(9032) << "Len: " << len << endl;
    kdDebug(9032) << "Text: " << currentComplText << endl;
    kdDebug(9032) << "Count: " << m_complList.count() << endl;

    Q3ValueList<KTextEditor::CompletionEntry>::Iterator it;
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
        QTextStringChar *chr = cursor->paragraph()->at( cursor->index() );
        int x = cursor->paragraph()->rect().x() + chr->x;
        int y, dummy;
        int h = cursor->paragraph()->lineHeightOfChar( cursor->index(), &dummy, &y );
        y += cursor->paragraph()->rect().y();

        m_completionPopup->resize( m_completionListBox->sizeHint()+ QSize(2,2));

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
        kdDebug(9032) << "Insert function text: " << *it << endl;

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
    QDesktopWidget *desktop = QApplication::desktop();
    QRect screen = desktop->screenGeometry( desktop->screenNumber(m_commentLabel) );
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

