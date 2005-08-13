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
#include <qlistview.h>

void ConfigToolboxDialog::init()
{
    listViewTools->setSorting( -1 );
    listViewCommon->setSorting( -1 );

    ListViewDnd *toolsDnd = new ListViewDnd( listViewTools );
    toolsDnd->setDragMode( ListViewDnd::External | ListViewDnd::NullDrop | ListViewDnd::Flat );

    ListViewDnd *commonDnd = new ListViewDnd( listViewCommon );
    commonDnd->setDragMode( ListViewDnd::Both | ListViewDnd::Move | ListViewDnd::Flat );

    QObject::connect( toolsDnd, SIGNAL( dropped( QListViewItem * ) ),
			commonDnd, SLOT( confirmDrop( QListViewItem * ) ) );
    QObject::connect( commonDnd, SIGNAL( dropped( QListViewItem * ) ),
			commonDnd, SLOT( confirmDrop( QListViewItem * ) ) );

    QDict<QListViewItem> groups;
    QAction *a;
    for ( a = MainWindow::self->toolActions.last();
	  a;
	  a = MainWindow::self->toolActions.prev() ) {
	QString grp = ( (WidgetAction*)a )->group();
	QListViewItem *parent = groups.find( grp );
	if ( !parent ) {
	    parent = new QListViewItem( listViewTools );
	    parent->setText( 0, grp );
	    parent->setOpen( TRUE );
	    groups.insert( grp, parent );
	}
	QListViewItem *i = new QListViewItem( parent );
	i->setText( 0, a->text() );
	i->setPixmap( 0, a->iconSet().pixmap() );
    }
    for ( a = MainWindow::self->commonWidgetsPage.last(); a;
    a = MainWindow::self->commonWidgetsPage.prev() ) {
	QListViewItem *i = new QListViewItem( listViewCommon );
	i->setText( 0, a->text() );
	i->setPixmap( 0, a->iconSet().pixmap() );
    }

}


void ConfigToolboxDialog::addTool()
{
    QListView *src = listViewTools;

    bool addKids = FALSE;
    QListViewItem *nextSibling = 0;
    QListViewItem *nextParent = 0;
    QListViewItemIterator it = src->firstChild();
    for ( ; *it; it++ ) {
	// Hit the nextSibling, turn of child processing
	if ( (*it) == nextSibling )
	    addKids = FALSE;

	if ( (*it)->isSelected() ) {
	    if ( (*it)->childCount() == 0 ) {
		// Selected, no children
		QListViewItem *i = new QListViewItem( listViewCommon, listViewCommon->lastItem() );
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
	    QListViewItem *i = new QListViewItem( listViewCommon, listViewCommon->lastItem() );
	    i->setText( 0, (*it)->text(0) );
	    i->setPixmap( 0, *((*it)->pixmap(0)) );
	    listViewCommon->setCurrentItem( i );
	    listViewCommon->ensureItemVisible( i );
	}
    }
}


void ConfigToolboxDialog::removeTool()
{
    QListViewItemIterator it = listViewCommon->firstChild();
    while ( *it ) {
	if ( (*it)->isSelected() )
	    delete (*it);
	else
	    it++;
    }
}


void ConfigToolboxDialog::moveToolUp()
{
    QListViewItem *next = 0;
    QListViewItem *item = listViewCommon->firstChild();
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
    QListViewItem *next = 0;
    QListViewItem *item = listViewCommon->lastItem();
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


void ConfigToolboxDialog::currentToolChanged( QListViewItem *i )
{
    bool canAdd = FALSE;
    QListViewItemIterator it = listViewTools->firstChild();
    for ( ; *it; it++ ) {
	if ( (*it)->isSelected() ) {
	    canAdd = TRUE;
	    break;
	}
    }
    buttonAdd->setEnabled( canAdd || ( i && i->isSelected() ) );
}


void ConfigToolboxDialog::currentCommonToolChanged( QListViewItem *i )
{
    buttonUp->setEnabled( (bool) (i && i->itemAbove()) );
    buttonDown->setEnabled( (bool) (i && i->itemBelow()) );

    bool canRemove = FALSE;
    QListViewItemIterator it = listViewCommon->firstChild();
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
    QListViewItem *item = listViewCommon->firstChild();
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
