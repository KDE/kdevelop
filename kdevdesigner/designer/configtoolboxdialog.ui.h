/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "mainwindow.h"
#include "widgetaction.h"
#include "listviewdnd.h"
#include <q3listview.h>

void ConfigToolboxDialog::init()
{
    listViewTools->setSorting( -1 );
    listViewCommon->setSorting( -1 );

    ListViewDnd *toolsDnd = new ListViewDnd( listViewTools );
    toolsDnd->setDragMode( ListViewDnd::External | ListViewDnd::NullDrop | ListViewDnd::Flat );

    ListViewDnd *commonDnd = new ListViewDnd( listViewCommon );
    commonDnd->setDragMode( ListViewDnd::Both | ListViewDnd::Move | ListViewDnd::Flat );

    QObject::connect( toolsDnd, SIGNAL( dropped( Q3ListViewItem * ) ),
			commonDnd, SLOT( confirmDrop( Q3ListViewItem * ) ) );
    QObject::connect( commonDnd, SIGNAL( dropped( Q3ListViewItem * ) ),
			commonDnd, SLOT( confirmDrop( Q3ListViewItem * ) ) );

    Q3Dict<Q3ListViewItem> groups;
    QAction *a;
    for ( a = MainWindow::self->toolActions.last();
	  a;
	  a = MainWindow::self->toolActions.prev() ) {
	QString grp = ( (WidgetAction*)a )->group();
	Q3ListViewItem *parent = groups.find( grp );
	if ( !parent ) {
	    parent = new Q3ListViewItem( listViewTools );
	    parent->setText( 0, grp );
	    parent->setOpen( TRUE );
	    groups.insert( grp, parent );
	}
	Q3ListViewItem *i = new Q3ListViewItem( parent );
	i->setText( 0, a->text() );
	i->setPixmap( 0, a->iconSet().pixmap() );
    }
    for ( a = MainWindow::self->commonWidgetsPage.last(); a;
    a = MainWindow::self->commonWidgetsPage.prev() ) {
	Q3ListViewItem *i = new Q3ListViewItem( listViewCommon );
	i->setText( 0, a->text() );
	i->setPixmap( 0, a->iconSet().pixmap() );
    }

}


void ConfigToolboxDialog::addTool()
{
    Q3ListView *src = listViewTools;

    bool addKids = FALSE;
    Q3ListViewItem *nextSibling = 0;
    Q3ListViewItem *nextParent = 0;
    Q3ListViewItemIterator it = src->firstChild();
    for ( ; *it; it++ ) {
	// Hit the nextSibling, turn of child processing
	if ( (*it) == nextSibling )
	    addKids = FALSE;

	if ( (*it)->isSelected() ) {
	    if ( (*it)->childCount() == 0 ) {
		// Selected, no children
		Q3ListViewItem *i = new Q3ListViewItem( listViewCommon, listViewCommon->lastItem() );
		i->setText( 0, (*it)->text(0) );
		i->setPixmap( 0, *((*it)->pixmap(0)) );
		listViewCommon->setCurrentItem( i );
		listViewCommon->ensureItemVisible( i );
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
	    Q3ListViewItem *i = new Q3ListViewItem( listViewCommon, listViewCommon->lastItem() );
	    i->setText( 0, (*it)->text(0) );
	    i->setPixmap( 0, *((*it)->pixmap(0)) );
	    listViewCommon->setCurrentItem( i );
	    listViewCommon->ensureItemVisible( i );
	}
    }
}


void ConfigToolboxDialog::removeTool()
{
    Q3ListViewItemIterator it = listViewCommon->firstChild();
    while ( *it ) {
	if ( (*it)->isSelected() )
	    delete (*it);
	else
	    it++;
    }
}


void ConfigToolboxDialog::moveToolUp()
{
    Q3ListViewItem *next = 0;
    Q3ListViewItem *item = listViewCommon->firstChild();
    for ( int i = 0; i < listViewCommon->childCount(); ++i ) {
	next = item->itemBelow();
	if ( item->isSelected() && (i > 0) && !item->itemAbove()->isSelected() )
	    item->itemAbove()->moveItem( item );
	item = next;
    }
}


void ConfigToolboxDialog::moveToolDown()
{
    int count = listViewCommon->childCount();
    Q3ListViewItem *next = 0;
    Q3ListViewItem *item = listViewCommon->lastItem();
    for ( int i = 0; i < count; ++i ) {
	next = item->itemAbove();
	if ( item->isSelected() && (i > 0) && !item->itemBelow()->isSelected() )
	    item->moveItem( item->itemBelow() );
	item = next;
    }

 //   QListViewItem *item = listViewCommon->firstChild();
 //   for ( int i = 0; i < listViewCommon->childCount(); ++i ) {
	//if ( item == listViewCommon->currentItem() ) {
	//    item->moveItem( item->itemBelow() );
	//    currentCommonToolChanged( item );
	//    break;
	//}
	//item = item->itemBelow();
 //   }
}


void ConfigToolboxDialog::currentToolChanged( Q3ListViewItem *i )
{
    bool canAdd = FALSE;
    Q3ListViewItemIterator it = listViewTools->firstChild();
    for ( ; *it; it++ ) {
	if ( (*it)->isSelected() ) {
	    canAdd = TRUE;
	    break;
	}
    }
    buttonAdd->setEnabled( canAdd || ( i && i->isSelected() ) );
}


void ConfigToolboxDialog::currentCommonToolChanged( Q3ListViewItem *i )
{
    buttonUp->setEnabled( (bool) (i && i->itemAbove()) );
    buttonDown->setEnabled( (bool) (i && i->itemBelow()) );

    bool canRemove = FALSE;
    Q3ListViewItemIterator it = listViewCommon->firstChild();
    for ( ; *it; it++ ) {
	if ( (*it)->isSelected() ) {
	    canRemove = TRUE;
	    break;
	}
    }
    buttonRemove->setEnabled( canRemove || ( i && i->isSelected() ) );
}


void ConfigToolboxDialog::ok()
{
    MainWindow::self->commonWidgetsPage.clear();
    Q3ListViewItem *item = listViewCommon->firstChild();
    for ( int j = 0; j < listViewCommon->childCount(); item = item->itemBelow(), ++j ) {
        QAction *a = 0;
	for ( a = MainWindow::self->toolActions.last();
	    a;
	    a = MainWindow::self->toolActions.prev() ) {
	    if ( a->text() == item->text( 0 ) )
		break;
	}
	if ( a )
	    MainWindow::self->commonWidgetsPage.insert( j, a );
    }
}
