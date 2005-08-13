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

#include "listviewdnd.h"
#include <qwidget.h>
#include <qheader.h>
#include <qpainter.h>
#include <qdragobject.h>
#include <qvaluelist.h>

// The Dragobject Declaration ---------------------------------------
class ListViewItemDrag : public QStoredDrag
{
public:
    enum DropRelation { Sibling, Child };
    ListViewItemDrag( ListViewItemList & items, QWidget * parent = 0, const char * name = 0 );
    ~ListViewItemDrag() {};
    static bool canDecode( QDragMoveEvent * event );
    static bool decode( QDropEvent * event, QListView * parent, QListViewItem * insertPoint, DropRelation dr );
};
// ------------------------------------------------------------------

ListViewDnd::ListViewDnd( QListView * eventSource, const char * name )
    : ListDnd( eventSource, name ) { }

void ListViewDnd::confirmDrop( QListViewItem * )
{
    dropConfirmed = TRUE;
}

bool ListViewDnd::dropEvent( QDropEvent * event )
{
    if ( dragInside ) {
    
	if ( dMode & NullDrop ) { // combined with Move, a NullDrop will delete an item
	    event->accept();
	    emit dropped( 0 ); // a NullDrop
	    return TRUE;
	}
	
	QPoint pos = event->pos();

	ListViewItemDrag::DropRelation dr = ListViewItemDrag::Sibling;
	QListViewItem *item = itemAt( pos );
	int dpos = dropDepth( item, pos );

	if ( item ) {
	    if ( dpos > item->depth() && !(dMode & Flat) ) {
		// Child node
		dr = ListViewItemDrag::Child;
	    } else if ( dpos < item->depth() ) {
		// Parent(s) Sibling
		while ( item && (item->depth() > dpos) )
		    item = item->parent();
	    }
	}

	if ( ListViewItemDrag::decode( event, (QListView *) src, item, dr ) ) {
	    event->accept();
	    emit dropped( 0 ); // Use ID instead of item?
	}
    }

    line->hide();
    dragInside = FALSE;

    return TRUE;
}

bool ListViewDnd::mouseMoveEvent( QMouseEvent * event )
{
    if ( event->state() & LeftButton ) {
	if ( ( event->pos() - mousePressPos ).manhattanLength() > 3 ) {
	    ListViewItemList list;

	    if ( dMode & Flat )
		buildFlatList( list );
	    else
		buildTreeList( list );

	    ListViewItemDrag * dragobject = new ListViewItemDrag( list, (QListView *) src );

	    if ( dMode & Move ) {
		disabledItems = list;
		setVisibleItems( FALSE );
	    }

	    dragobject->dragCopy();

	    if ( dMode & Move ) {
		// Did the target accept the drop?
		if ( dropConfirmed ) {
		    // Shouldn't autoDelete handle this?
		    for( list.first(); list.current(); list.next() ) 
			delete list.current();
		    dropConfirmed = FALSE;
		} else {
		    // Reenable disabled items since 
		    // drag'n'drop was aborted
		    setVisibleItems( TRUE );
		}
		disabledItems.clear();
	    }
	}
    }
    return FALSE;
}

int ListViewDnd::buildFlatList( ListViewItemList & list )
{
    bool addKids = FALSE;
    QListViewItem *nextSibling = 0;
    QListViewItem *nextParent = 0;
    QListViewItemIterator it = ((QListView *)src)->firstChild();
    for ( ; *it; it++ ) {
	// Hit the nextSibling, turn of child processing
	if ( (*it) == nextSibling )
	    addKids = FALSE;

	if ( (*it)->isSelected() ) {
	    if ( (*it)->childCount() == 0 ) {
		// Selected, no children
		list.append( *it );
	    } else if ( !addKids ) {
		// Children processing not set, so set it
		// Also find the item were we shall quit
		// processing children...if any such item
		addKids = TRUE;
		nextSibling = (*it)->nextSibling();
		nextParent = (*it)->parent();
		while ( nextParent && !nextSibling ) {
		    nextSibling = nextParent->nextSibling();
		    nextParent = nextParent->parent();
		}
	    }
	} else if ( ((*it)->childCount() == 0) && addKids ) {
	    // Leaf node, and we _do_ process children
	    list.append( *it );
	}
    }
    return list.count();
}

int ListViewDnd::buildTreeList( ListViewItemList & list )
{
    QListViewItemIterator it = ((QListView *)src)->firstChild();
    for ( ; *it; it++ ) {
	if ( (*it)->isSelected() )
	    list.append( *it );
    }
    return list.count();
}

void ListViewDnd::setVisibleItems( bool b )
{
    if ( disabledItems.isEmpty() ) 
	return;
    
    disabledItems.first();
    do {
        disabledItems.current()->setVisible( b );
    } while ( disabledItems.next() );
}

void ListViewDnd::updateLine( const QPoint & dragPos )
{
    QListViewItem * item = itemAt(dragPos);
    QListView * src = (QListView *) this->src;

    int ypos = item ? 
	( src->itemRect( item ).bottom() - ( line->height() / 2 ) ) : 
	( src->itemRect( src->firstChild() ).top() );

    int xpos = dropDepth( item, dragPos ) * src->treeStepSize();
    line->resize( src->viewport()->width() - xpos, line->height() );
    line->move( xpos, ypos );
}

QListViewItem * ListViewDnd::itemAt( QPoint pos )
{
    QListView * src = (QListView *) this->src;
    int headerHeight = (int)(src->header()->height());
    pos.ry() -= headerHeight;
    QListViewItem * result = src->itemAt( pos );

    if ( result && ( pos.ry() < (src->itemPos(result) + result->height()/2) ) )
	result = result->itemAbove();

    // Wind back if has parent, and we're in flat mode
    while ( result && result->parent() && (dMode & Flat) )
	result = result->parent();

    // Wind back if has parent, and we're in flat mode
    while ( result && !result->isVisible() && result->parent() )
	result = result->parent();

    if ( !result && src->firstChild() && (pos.y() > src->itemRect(src->firstChild()).bottom()) ) {
	result = src->lastItem();
	if ( !result->isVisible() )
	    // Handle special case where last item is actually hidden
	    result = result->itemAbove();
    }

    return result;
}

int ListViewDnd::dropDepth( QListViewItem * item, QPoint pos )
{
    if ( !item || (dMode & Flat) )
	return 0;

    int result     = 0;
    int itemDepth  = item->depth();
    int indentSize = ((QListView *)src)->treeStepSize();
    int itemLeft   = indentSize * itemDepth;
    int childMargin  = indentSize*2;
    if ( pos.x() > itemLeft + childMargin ) {
	result = itemDepth + 1;
    } else if ( pos.x() < itemLeft ) {
	result = pos.x() / indentSize;
    } else {
	result = itemDepth;
    }
    return result;
}

bool ListViewDnd::canDecode( QDragEnterEvent * event )
{
    return ListViewItemDrag::canDecode( event );
}

// ------------------------------------------------------------------
// The Dragobject Implementation ------------------------------------
// ------------------------------------------------------------------

QDataStream & operator<< ( QDataStream & stream, const QListViewItem & item );
QDataStream & operator>> ( QDataStream & stream, QListViewItem & item );

ListViewItemDrag::ListViewItemDrag( ListViewItemList & items, QWidget * parent, const char * name )
    : QStoredDrag( "qt/listviewitem", parent, name )
{
    // ### FIX!
    QByteArray data( sizeof( Q_INT32 ) + sizeof( QListViewItem ) * items.count() );
    QDataStream stream( data, IO_WriteOnly );

    stream << items.count();

    QListViewItem *i = items.first();
    while ( i ) {
        stream << *i;
	i = items.next();
    }

    setEncodedData( data );
}

bool ListViewItemDrag::canDecode( QDragMoveEvent * event )
{
    return event->provides( "qt/listviewitem" );
}

bool ListViewItemDrag::decode( QDropEvent * event, QListView * parent, QListViewItem * insertPoint, DropRelation dr )
{
    QByteArray data = event->encodedData( "qt/listviewitem" );
    QListViewItem* itemParent = insertPoint ? insertPoint->parent() : 0;

    // Change from sibling (default) to child creation
    if ( insertPoint && dr == Child ) {
	itemParent = insertPoint;
	insertPoint = 0;
    }

    if ( data.size() ) {
	event->accept();
	QDataStream stream( data, IO_ReadOnly );

	int count = 0;
	stream >> count;

	for( int i = 0; i < count; i++ ) {
	    if ( itemParent ) {
		insertPoint = new QListViewItem( itemParent, insertPoint );
		itemParent->setOpen( TRUE );
	    } else { // No parent for insertPoint, use QListView
		insertPoint = new QListViewItem( parent, insertPoint );
	    }
	    stream >> *insertPoint;
	}
	return TRUE;
    }
    return FALSE;
}


QDataStream & operator<< ( QDataStream & stream, const QListViewItem & item )
{
    int columns = item.listView()->columns();
    stream << columns;
 
    Q_UINT8 b = 0;

    int i;
    for ( i = 0; i < columns; i++ ) {
	b = (Q_UINT8) ( item.text( i ) != QString::null ); // does column i have a string ?
	stream << b;
	if ( b ) {
	    stream << item.text( i );
	}
    }
    
    for ( i = 0; i < columns; i++ ) {
	b = (Q_UINT8) ( !!item.pixmap( i ) ); // does column i have a pixmap ?
	stream << b;
	if ( b ) {
	    stream << ( *item.pixmap( i ) );
	}
    }

    stream << (Q_UINT8) item.isOpen();
    stream << (Q_UINT8) item.isSelectable();
    stream << (Q_UINT8) item.isExpandable();
    stream << (Q_UINT8) item.dragEnabled();
    stream << (Q_UINT8) item.dropEnabled();
    stream << (Q_UINT8) item.isVisible();

    for ( i = 0; i < columns; i++ ) {
	stream << (Q_UINT8) item.renameEnabled( i );
    }

    stream << (Q_UINT8) item.multiLinesEnabled();
    stream << item.childCount();

    if ( item.childCount() > 0 ) {
	QListViewItem * child = item.firstChild();
	while ( child ) {
	    stream << ( *child ); // recursive call
	    child = child->nextSibling();
	}
    }

    return stream;
}
    
QDataStream & operator>> ( QDataStream & stream, QListViewItem & item )
{
    Q_INT32 columns;
    stream >> columns;

    Q_UINT8 b = 0;

    QString text;
    int i;
    for ( i = 0; i < columns; i++ ) {
	stream >> b;
	if ( b ) { // column i has string ?
	    stream >> text;
	    item.setText( i, text );
	}
    }

    QPixmap pixmap;
    for ( i = 0; i < columns; i++ ) {
	stream >> b; // column i has pixmap ?
	if ( b ) {
	    stream >> pixmap;
	    item.setPixmap( i, pixmap );
	}
    }

    stream >> b;
    item.setOpen( b );

    stream >> b;
    item.setSelectable( b );

    stream >> b;
    item.setExpandable( b );

    stream >> b;
    item.setDragEnabled( b );

    stream >> b;
    item.setDropEnabled( b );

    stream >> b;
    item.setVisible( b );

    for ( i = 0; i < columns; i++ ) {
	stream >> b;
	item.setRenameEnabled( i, b );
    }

    stream >> b;
    item.setMultiLinesEnabled( b );

    int childCount;
    stream >> childCount;

    QListViewItem *child = 0;
    QListViewItem *prevchild = 0;
    for ( i = 0; i < childCount; i++ ) {
	child = new QListViewItem( &item, prevchild );
	stream >> ( *child );
	item.insertItem( child );
	prevchild = child;
    }

    return stream;
}
