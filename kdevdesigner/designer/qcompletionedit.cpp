/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcompletionedit.h"
#include <qlistbox.h>
#include <qsizegrip.h>
#include <qapplication.h>
#include <qvbox.h>

QCompletionEdit::QCompletionEdit( QWidget *parent, const char *name )
    : QLineEdit( parent, name ), aAdd( FALSE ), caseSensitive( FALSE )
{
    popup = new QVBox( 0, 0, WType_Popup );
    popup->setFrameStyle( QFrame::Box | QFrame::Plain );
    popup->setLineWidth( 1 );
    popup->hide();

    listbox = new QListBox( popup );
    listbox->setFrameStyle( QFrame::NoFrame );
    listbox->setLineWidth( 1 );
    listbox->installEventFilter( this );
    listbox->setHScrollBarMode( QScrollView::AlwaysOn );
    listbox->setVScrollBarMode( QScrollView::AlwaysOn );
    listbox->setCornerWidget( new QSizeGrip( listbox, "completion sizegrip" ) );
    connect( this, SIGNAL( textChanged( const QString & ) ),
	     this, SLOT( textDidChange( const QString & ) ) );
    popup->setFocusProxy( listbox );
    installEventFilter( this );
}

bool QCompletionEdit::autoAdd() const
{
    return aAdd;
}

QStringList QCompletionEdit::completionList() const
{
    return compList;
}

void QCompletionEdit::setCompletionList( const QStringList &l )
{
    compList = l;
}

void QCompletionEdit::setAutoAdd( bool add )
{
    aAdd = add;
}

void QCompletionEdit::textDidChange( const QString &text )
{
    if ( text.isEmpty() ) {
	popup->close();
	return;
    }
    updateListBox();
    placeListBox();
}

void QCompletionEdit::placeListBox()
{
    if ( listbox->count() == 0 ) {
	popup->close();
	return;
    }

    popup->resize( QMAX( listbox->sizeHint().width() + listbox->verticalScrollBar()->width() + 4, width() ),
		   listbox->sizeHint().height() + listbox->horizontalScrollBar()->height() + 4 );

    QPoint p( mapToGlobal( QPoint( 0, 0 ) ) );
    if ( p.y() + height() + popup->height() <= QApplication::desktop()->height() )
	popup->move( p.x(), p.y() + height() );
    else
	popup->move( p.x(), p.y() - listbox->height() );
    popup->show();
    listbox->setCurrentItem( 0 );
    listbox->setSelected( 0, TRUE );
    setFocus();
}

void QCompletionEdit::updateListBox()
{
    listbox->clear();
    if ( compList.isEmpty() )
	return;
    for ( QStringList::Iterator it = compList.begin(); it != compList.end(); ++it ) {
	if ( caseSensitive && (*it).left( text().length() ) == text() ||
	     !caseSensitive && (*it).left( text().length() ).lower() == text().lower() )
	    listbox->insertItem( *it );
    }
}

bool QCompletionEdit::eventFilter( QObject *o, QEvent *e )
{
    if ( o == popup || o == listbox || o == listbox->viewport() ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Enter || ke->key() == Key_Return || ke->key() == Key_Tab ) {
		if ( ke->key() == Key_Tab && listbox->count() > 1 &&
		     listbox->currentItem() < (int)listbox->count() - 1 ) {
		    listbox->setCurrentItem( listbox->currentItem() + 1 );
		    return TRUE;
		}
		popup->close();
		setFocus();
		blockSignals( TRUE );
		setText( listbox->currentText() );
		blockSignals( FALSE );
		emit chosen( text() );
		return TRUE;
	    } else if ( ke->key() == Key_Left || ke->key() == Key_Right ||
			ke->key() == Key_Up || ke->key() == Key_Down ||
			ke->key() == Key_Home || ke->key() == Key_End ||
			ke->key() == Key_Prior || ke->key() == Key_Next ) {
		return FALSE;
	    } else if ( ke->key() == Key_Escape ) {
		popup->close();
		setFocus();
	    } else if ( ke->key() != Key_Shift && ke->key() != Key_Control &&
			ke->key() != Key_Alt ) {
		updateListBox();
		if ( listbox->count() == 0 || text().length() == 0 ) {
		    popup->close();
		    setFocus();
		}
		QApplication::sendEvent( this, e );
		return TRUE;
	    }
	} else if ( e->type() == QEvent::MouseButtonDblClick ) {
	    popup->close();
	    setFocus();
	    blockSignals( TRUE );
	    setText( listbox->currentText() );
	    blockSignals( FALSE );
	    emit chosen( text() );
	    return TRUE;
	}
    } else if ( o == this ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Up ||
		 ke->key() == Key_Down ||
		 ke->key() == Key_Prior ||
		 ke->key() == Key_Next ||
		 ke->key() == Key_Return ||
		 ke->key() == Key_Enter ||
		 ke->key() == Key_Tab ||
		 ke->key() ==  Key_Escape ) {
		QApplication::sendEvent( listbox, e );
		return TRUE;
	    }
	}
    }
    return QLineEdit::eventFilter( o, e );
}

void QCompletionEdit::addCompletionEntry( const QString &entry )
{
    if ( compList.find( entry ) == compList.end() ) {
	compList << entry;
	compList.sort();
    }
}

void QCompletionEdit::removeCompletionEntry( const QString &entry )
{
    QStringList::Iterator it = compList.find( entry );
    if ( it != compList.end() )
	compList.remove( it );
}

void QCompletionEdit::setCaseSensitive( bool b )
{
    caseSensitive = b;
}

bool QCompletionEdit::isCaseSensitive() const
{
    return caseSensitive;
}

void QCompletionEdit::clear()
{
    QLineEdit::clear();
    compList.clear();
}
