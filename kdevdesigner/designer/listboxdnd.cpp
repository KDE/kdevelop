/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#include "listboxdnd.h"
#include <qwidget.h>
#include <qheader.h>
#include <qpainter.h>
#include <qdragobject.h>
#include <qvaluelist.h>

// The Dragobject Declaration ---------------------------------------
class ListBoxItemDrag : public QStoredDrag
{
public:
    ListBoxItemDrag( ListBoxItemList & items, bool sendPtr = FALSE, QListBox * parent = 0, const char * name = 0 );
    ~ListBoxItemDrag() {};
    static bool canDecode( QDragMoveEvent * event );
    static bool decode( QDropEvent * event, QListBox * parent, QListBoxItem * insertPoint );
    enum ItemType { ListBoxText = 1, ListBoxPixmap = 2 };
};
// ------------------------------------------------------------------

ListBoxDnd::ListBoxDnd( QListBox * eventSource, const char * name )
    : ListDnd( eventSource, name ) { }

void ListBoxDnd::confirmDrop( QListBoxItem * )
{
    dropConfirmed = TRUE;
}

bool ListBoxDnd::dropEvent( QDropEvent * event )
{
    if ( dragInside ) {
    
	if ( dMode & NullDrop ) { // combined with Move, a NullDrop will delete an item
	    event->accept();
	    emit dropped( 0 ); // a NullDrop
	    return TRUE;
	}
	
	QPoint pos = event->pos();
	QListBoxItem * after = itemAt( pos );

	if ( ListBoxItemDrag::decode( event, (QListBox *) src, after ) ) {
	    event->accept();
	    QListBox * src = (QListBox *) this->src;
	    QListBoxItem * item = ( after ? after->next() : src->firstItem() );
	    src->setCurrentItem( item );
	    emit dropped( item ); // ###FIX: Supports only one item!
	}
    }

    line->hide();
    dragInside = FALSE;

    return TRUE;
}

bool ListBoxDnd::mouseMoveEvent( QMouseEvent * event )
{
    if ( event->state() & LeftButton ) {
	if ( ( event->pos() - mousePressPos ).manhattanLength() > 3 ) {

	    ListBoxItemList list;
	    buildList( list );
	    ListBoxItemDrag * dragobject = new ListBoxItemDrag( list, (dMode & Internal), (QListBox *) src );

	    // Emit signal for all dragged items
	    QListBoxItem * i = list.first();
	    while ( i ) {
		emit dragged( i );
		i = list.next();
	    }

	    if ( dMode & Move ) {
		removeList( list ); // "hide" items
	    }

	    dragobject->dragCopy();

	    if ( dMode & Move ) {
		if ( dropConfirmed ) {
		    // ###FIX: memleak ? in internal mode, only pointers are transfered...
		    //list.setAutoDelete( TRUE );
		    list.clear();
		    dropConfirmed = FALSE;
		}
		insertList( list ); // "show" items
	    }
	}
    }
    return FALSE;
}

int ListBoxDnd::buildList( ListBoxItemList & list )
{
    QListBoxItem * i = ((QListBox *)src)->firstItem();
    while ( i ) {
	if ( i->isSelected() ) {
	    ((QListBox *)src)->setSelected( i, FALSE );
	    list.append( i );
	}
	i = i->next();
    }
    return list.count();
}

void ListBoxDnd::insertList( ListBoxItemList & list )
{
    QListBoxItem * i = list.first();
    while ( i ) {
	((QListBox *)src)->insertItem( i, i->prev() );
	i = list.next();
    }
}

void ListBoxDnd::removeList( ListBoxItemList & list )
{
    QListBoxItem * i = list.first();
    while ( i ) {
	((QListBox *)src)->takeItem( i ); // remove item from QListBox
	i = list.next();
    }
}

void ListBoxDnd::updateLine( const QPoint & dragPos )
{
    QListBox * src = (QListBox *) this->src;
    QListBoxItem *item = itemAt( dragPos );

    int ypos = item ? 
	( src->itemRect( item ).bottom() - ( line->height() / 2 ) ) : 
	( src->itemRect( ((QListBox *)src)->firstItem() ).top() );

    line->resize( src->viewport()->width(), line->height() );
    line->move( 0, ypos );
}

QListBoxItem * ListBoxDnd::itemAt( QPoint pos )
{
    QListBox * src = (QListBox *) this->src;
    QListBoxItem * result = src->itemAt( pos );
    QListBoxItem * last = src->item( src->count() - 1 );
    int i = src->index( result );

    if ( result && ( pos.y() < (src->itemRect(result).top() + src->itemHeight(i)/2) ) )
	result = result->prev();
    else if ( !result && pos.y() > src->itemRect( last ).bottom() )
	result = last;

    return result;
}

bool ListBoxDnd::canDecode( QDragEnterEvent * event )
{
    return ListBoxItemDrag::canDecode( event );
}


// ------------------------------------------------------------------
// The Dragobject Implementation ------------------------------------
// ------------------------------------------------------------------

ListBoxItemDrag::ListBoxItemDrag( ListBoxItemList & items, bool sendPtr, QListBox * parent, const char * name )
    : QStoredDrag( "qt/listboxitem", parent, name )
{
    // ### FIX!
    QByteArray data( sizeof( Q_INT32 ) + sizeof( QListBoxItem ) * items.count() );
    QDataStream stream( data, IO_WriteOnly );

    stream << items.count();
    stream << (Q_UINT8) sendPtr; // just transfer item pointer; omit data

    QListBoxItem * i = items.first();

    if ( sendPtr ) {
	
	while ( i ) {
	    
	    stream << (Q_ULONG) i; //###FIX: demands sizeof(ulong) >= sizeof(void*)
	    i = items.next();
	    
	}
	
    } else {
  
	while ( i ) {
	
	    Q_UINT8 b = 0;

	    b = (Q_UINT8) ( i->text() != QString::null ); // does item have text ?
	    stream << b;
	    if ( b ) {
		stream << i->text();
	    }
    
	    b = (Q_UINT8) ( !!i->pixmap() ); // does item have a pixmap ?
	    stream << b;
	    if ( b ) {
		stream << ( *i->pixmap() );
	    }

	    stream << (Q_UINT8) i->isSelectable();
	    
	    i = items.next();
	}
    
    }

    setEncodedData( data );
}

bool ListBoxItemDrag::canDecode( QDragMoveEvent * event )
{
    return event->provides( "qt/listboxitem" );
}

bool ListBoxItemDrag::decode( QDropEvent * event, QListBox * parent, QListBoxItem * after )
{
    QByteArray data = event->encodedData( "qt/listboxitem" );

    if ( data.size() ) {
	event->accept();
	QDataStream stream( data, IO_ReadOnly );

	int count = 0;
	stream >> count;

	Q_UINT8 recievePtr = 0; // data contains just item pointers; no data
	stream >> recievePtr;

	QListBoxItem * item = 0;

	if ( recievePtr ) {
	    
	    for( int i = 0; i < count; i++ ) {

		Q_ULONG p = 0; //###FIX: demands sizeof(ulong) >= sizeof(void*)
		stream >> p;
		item = (QListBoxItem *) p;
		
		parent->insertItem( item, after );
		
	    }
	    
	} else {		

	    for ( int i = 0; i < count; i++ ) {

		Q_UINT8 hasText = 0;
		QString text;
		stream >> hasText;
		if ( hasText ) {
		    stream >> text;
		}
	    
		Q_UINT8 hasPixmap = 0;
		QPixmap pixmap;
		stream >> hasPixmap;
		if ( hasPixmap ) {
		    stream >> pixmap;
		}
	    
		Q_UINT8 isSelectable = 0;
		stream >> isSelectable;

		if ( hasPixmap ) {
		    item = new QListBoxPixmap( parent, pixmap, text, after );
		} else {
		    item = new QListBoxText( parent, text, after );
		}

		item->setSelectable( isSelectable );
	    
	    }
	
	}
	
	return TRUE;
    }
    return FALSE;
}
