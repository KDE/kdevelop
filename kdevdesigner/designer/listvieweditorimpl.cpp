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

#include "formwindow.h"
#include "mainwindow.h"
#include "listvieweditorimpl.h"
#include "pixmapchooser.h"
#include "command.h"
#include "listviewdnd.h"
#include "listboxdnd.h"
#include "listboxrename.h"

#include <q3listview.h>
#include <q3header.h>
#include <q3listbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <q3ptrstack.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>

#include <klocale.h>

ListViewEditor::ListViewEditor( QWidget *parent, Q3ListView *lv, FormWindow *fw )
    : ListViewEditorBase( parent, 0, TRUE ), listview( lv ), formwindow( fw )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    itemText->setEnabled( FALSE );
    itemChoosePixmap->setEnabled( FALSE );
    itemDeletePixmap->setEnabled( FALSE );
    itemColumn->setEnabled( FALSE );

    setupColumns();
    PopulateListViewCommand::transferItems( listview, itemsPreview );
    setupItems();

    itemsPreview->setShowSortIndicator( listview->showSortIndicator() );
    itemsPreview->setAllColumnsShowFocus( listview->allColumnsShowFocus() );
    itemsPreview->setRootIsDecorated( listview->rootIsDecorated() );

    if ( itemsPreview->firstChild() ) {
	itemsPreview->setCurrentItem( itemsPreview->firstChild() );
	itemsPreview->setSelected( itemsPreview->firstChild(), TRUE );
    }

    // Clamp on drag and drop to QListView
    ListViewDnd *itemsDnd = new ListViewDnd( itemsPreview );
    itemsDnd->setDragMode( ListViewDnd::Internal | ListViewDnd::Move );
    QObject::connect( itemsDnd, SIGNAL( dropped( Q3ListViewItem * ) ),
		      itemsDnd, SLOT( confirmDrop( Q3ListViewItem * ) ) );

    // Enable rename for all QListViewItems
    Q3ListViewItemIterator it = ((Q3ListView *)itemsPreview)->firstChild();
    for ( ; *it; it++ )
	(*it)->setRenameEnabled( 0, TRUE );

    // Connect listview signal to signal-relay
    QObject::connect( itemsPreview,
		      SIGNAL( itemRenamed( Q3ListViewItem*, int, const QString & ) ),
		      this,
		      SLOT( emitItemRenamed(Q3ListViewItem*, int, const QString&) ) );

    // Connect signal-relay to QLineEdit "itemText"
    QObjectList *l = parent->queryList( "QLineEdit", "itemText" );
    QObject *obj;
    QObjectListIt itemsLineEditIt( *l );
    while ( (obj = itemsLineEditIt.current()) != 0 ) {
        ++itemsLineEditIt;
	QObject::connect( this,
			  SIGNAL( itemRenamed( const QString & ) ),
			  obj,
			  SLOT( setText( const QString & ) ) );
    }
    delete l;

    // Clamp on drag and drop to QListBox
    ListBoxDnd *columnsDnd = new ListBoxDnd( colPreview );
    columnsDnd->setDragMode( ListBoxDnd::Internal | ListBoxDnd::Move );
    QObject::connect( columnsDnd, SIGNAL( dropped( Q3ListBoxItem * ) ),
		      columnsDnd, SLOT( confirmDrop( Q3ListBoxItem * ) ) );

    // Clamp on rename to QListBox
    ListBoxRename *columnsRename = new ListBoxRename( colPreview );
    QObject::connect( columnsRename,
		      SIGNAL( itemTextChanged( const QString & ) ),
		      this,
		      SLOT( columnTextChanged( const QString & ) ) );

    // Find QLineEdit "colText" and connect
    l = parent->queryList( "QLineEdit", "colText" );
    QObjectListIt columnsLineEditIt( *l );
    while ( (obj = columnsLineEditIt.current()) != 0 ) {
        ++columnsLineEditIt;
	QObject::connect( columnsRename,
			  SIGNAL( itemTextChanged( const QString & ) ),
			  obj,
			  SLOT( setText( const QString & ) ) );
    }
    delete l;
}

void ListViewEditor::applyClicked()
{
    setupItems();
    PopulateListViewCommand *cmd = new PopulateListViewCommand( i18n( "Edit the Items and Columns of '%1'" ).arg( listview->name() ),
								formwindow, listview, itemsPreview );
    cmd->execute();
    formwindow->commandHistory()->addCommand( cmd );
}

void ListViewEditor::okClicked()
{
    applyClicked();
    accept();
}

void ListViewEditor::columnClickable( bool b )
{
    Column *c = findColumn( colPreview->item( colPreview->currentItem() ) );
    if ( !c )
	return;
    c->clickable = b;
}

void ListViewEditor::columnDownClicked()
{
    if ( colPreview->currentItem() == -1 ||
	 colPreview->currentItem() > (int)colPreview->count() - 2 )
	return;

    colPreview->clearSelection();
    Q3ListBoxItem *i = colPreview->item( colPreview->currentItem() );
    Q3ListBoxItem *below = i->next();

    colPreview->takeItem( i );
    colPreview->insertItem( i, below );

    colPreview->setCurrentItem( i );
    colPreview->setSelected( i, TRUE );
}

void ListViewEditor::columnPixmapChosen()
{
    Column *c = findColumn( colPreview->item( colPreview->currentItem() ) );
    if ( !c )
	return;

    QPixmap pix;
    if ( colPixmap->pixmap() )
	pix = qChoosePixmap( this, formwindow, *colPixmap->pixmap() );
    else
	pix = qChoosePixmap( this, formwindow, QPixmap() );

    if ( pix.isNull() )
	return;

    c->pixmap = pix;
    colPreview->blockSignals( TRUE );
    if ( !c->pixmap.isNull() )
	colPreview->changeItem( c->pixmap, c->text, colPreview->index( c->item ) );
    else
	colPreview->changeItem( c->text, colPreview->index( c->item ) );
    c->item = colPreview->item( colPreview->currentItem() );
    colPixmap->setPixmap( c->pixmap );
    colPreview->blockSignals( FALSE );
    colDeletePixmap->setEnabled( TRUE );
}

void ListViewEditor::columnPixmapDeleted()
{
    Column *c = findColumn( colPreview->item( colPreview->currentItem() ) );
    if ( !c )
	return;

    c->pixmap = QPixmap();
    colPreview->blockSignals( TRUE );
    if ( !c->pixmap.isNull() )
	colPreview->changeItem( c->pixmap, c->text, colPreview->index( c->item ) );
    else
	colPreview->changeItem( c->text, colPreview->index( c->item ) );
    c->item = colPreview->item( colPreview->currentItem() );
    colPixmap->setText( "" );
    colPreview->blockSignals( FALSE );
    colDeletePixmap->setEnabled( FALSE );
}

void ListViewEditor::columnResizable( bool b )
{
    Column *c = findColumn( colPreview->item( colPreview->currentItem() ) );
    if ( !c )
	return;
    c->resizable = b;
}

void ListViewEditor::columnTextChanged( const QString &txt )
{
    Column *c = findColumn( colPreview->item( colPreview->currentItem() ) );
    if ( !c )
	return;

    c->text = txt;
    colPreview->blockSignals( TRUE );
    if ( !c->pixmap.isNull() )
	colPreview->changeItem( c->pixmap, c->text, colPreview->index( c->item ) );
    else
	colPreview->changeItem( c->text, colPreview->index( c->item ) );
    c->item = colPreview->item( colPreview->currentItem() );
    colPreview->blockSignals( FALSE );
}

void ListViewEditor::columnUpClicked()
{
    if ( colPreview->currentItem() <= 0 )
	return;

    colPreview->clearSelection();
    Q3ListBoxItem *i = colPreview->item( colPreview->currentItem() );
    Q3ListBoxItem *above = i->prev();

    colPreview->takeItem( above );
    colPreview->insertItem( above, i );

    colPreview->setCurrentItem( i );
    colPreview->setSelected( i, TRUE );
}

void ListViewEditor::currentColumnChanged( Q3ListBoxItem *i )
{
    Column *c = findColumn( i );
    if ( !i || !c ) {
	colText->setEnabled( FALSE );
	colPixmap->setEnabled( FALSE );
	colDeletePixmap->setEnabled( FALSE );
	colText->blockSignals( TRUE );
	colText->setText( "" );
	colText->blockSignals( FALSE );
	colClickable->setEnabled( FALSE );
	colResizable->setEnabled( FALSE );
	return;
    }

    colText->setEnabled( TRUE );
    colPixmap->setEnabled( TRUE );
    colDeletePixmap->setEnabled( i->pixmap() && !i->pixmap()->isNull() );
    colClickable->setEnabled( TRUE );
    colResizable->setEnabled( TRUE );

    colText->blockSignals( TRUE );
    colText->setText( c->text );
    colText->blockSignals( FALSE );
    if ( !c->pixmap.isNull() )
	colPixmap->setPixmap( c->pixmap );
    else
	colPixmap->setText( "" );
    colClickable->setChecked( c->clickable );
    colResizable->setChecked( c->resizable );
}

void ListViewEditor::newColumnClicked()
{
    Column col;
    col.text = i18n( "New Column" );
    col.pixmap = QPixmap();
    col.clickable = TRUE;
    col.resizable = TRUE;
    if ( !col.pixmap.isNull() )
	col.item = new Q3ListBoxPixmap( colPreview, col.pixmap, col.text );
    else
	col.item = new Q3ListBoxText( colPreview, col.text );
    columns.append( col );
    colPreview->setCurrentItem( col.item );
    colPreview->setSelected( col.item, TRUE );
}

void ListViewEditor::deleteColumnClicked()
{
    Q3ListBoxItem *i = colPreview->item( colPreview->currentItem() );
    if ( !i )
	return;

    for ( Q3ValueList<Column>::Iterator it = columns.begin(); it != columns.end(); ++it ) {
	if ( ( *it ).item == i ) {
	    delete (*it).item;
	    columns.remove( it );
	    break;
	}
    }

    if ( colPreview->currentItem() != -1 )
	colPreview->setSelected( colPreview->currentItem(), TRUE );
}

void ListViewEditor::currentItemChanged( Q3ListViewItem *i )
{
    if ( !i ) {
	itemText->setEnabled( FALSE );
	itemChoosePixmap->setEnabled( FALSE );
	itemDeletePixmap->setEnabled( FALSE );
	itemColumn->setEnabled( FALSE );
	return;
    }

    itemText->setEnabled( TRUE );
    itemChoosePixmap->setEnabled( TRUE );
    itemDeletePixmap->setEnabled( i->pixmap( itemColumn->value() ) &&
				  !i->pixmap( itemColumn->value() )->isNull() );
    itemColumn->setEnabled( TRUE );

    displayItem( i, itemColumn->value() );
}

void ListViewEditor::displayItem( Q3ListViewItem *i, int col )
{
    itemText->blockSignals( TRUE );
    itemText->setText( i->text( col ) );
    itemText->blockSignals( FALSE );

    itemPixmap->blockSignals( TRUE );
    if ( i->pixmap( col ) )
	itemPixmap->setPixmap( *i->pixmap( col ) );
    else
	itemPixmap->setText( "" );
    itemPixmap->blockSignals( FALSE );
}

void ListViewEditor::itemColChanged( int col )
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    displayItem( i, col );
    itemDeletePixmap->setEnabled( i->pixmap( col ) && !i->pixmap( col )->isNull() );
}

void ListViewEditor::itemDeleteClicked()
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    delete i;
    if ( itemsPreview->firstChild() ) {
	itemsPreview->setCurrentItem( itemsPreview->firstChild() );
	itemsPreview->setSelected( itemsPreview->firstChild(), TRUE );
    }
}

void ListViewEditor::itemDownClicked()
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    Q3ListViewItemIterator it( i );
    Q3ListViewItem *parent = i->parent();
    it++;
    while ( it.current() ) {
	if ( it.current()->parent() == parent )
	    break;
	it++;
    }

    if ( !it.current() )
	return;
    Q3ListViewItem *other = it.current();

    i->moveItem( other );
}

void ListViewEditor::itemNewClicked()
{
    Q3ListViewItem *item = new Q3ListViewItem( itemsPreview );
    item->setText( 0, i18n( "Item" ) );
    item->setRenameEnabled( 0, TRUE );
    itemsPreview->setCurrentItem( item );
    itemsPreview->setSelected( item, TRUE );
    itemText->setFocus();
    itemText->selectAll();
}

void ListViewEditor::itemNewSubClicked()
{
    Q3ListViewItem *parent = itemsPreview->currentItem();
    Q3ListViewItem *item = 0;
    if ( parent ) {
	item = new Q3ListViewItem( parent );
	parent->setOpen( TRUE );
    } else {
	item = new Q3ListViewItem( itemsPreview );
    }
    item->setText( 0, i18n( "Subitem" ) );
    item->setRenameEnabled( 0, TRUE );
    itemsPreview->setCurrentItem( item );
    itemsPreview->setSelected( item, TRUE );
}

void ListViewEditor::itemPixmapChoosen()
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    QPixmap pix;
    if ( itemPixmap->pixmap() )
	pix = qChoosePixmap( this, formwindow, *itemPixmap->pixmap() );
    else
	pix = qChoosePixmap( this, formwindow, QPixmap() );

    if ( pix.isNull() )
	return;

    i->setPixmap( itemColumn->value(), QPixmap( pix ) );
    itemPixmap->setPixmap( pix );
    itemDeletePixmap->setEnabled( TRUE );
}

void ListViewEditor::itemPixmapDeleted()
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    i->setPixmap( itemColumn->value(), QPixmap() );
    itemPixmap->setText( "" );
    itemDeletePixmap->setEnabled( FALSE );
}

void ListViewEditor::itemTextChanged( const QString &txt )
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;
    i->setText( itemColumn->value(), txt );
}

void ListViewEditor::itemUpClicked()
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    Q3ListViewItemIterator it( i );
    Q3ListViewItem *parent = i->parent();
    --it;
    while ( it.current() ) {
	if ( it.current()->parent() == parent )
	    break;
	--it;
    }

    if ( !it.current() )
	return;
    Q3ListViewItem *other = it.current();

    other->moveItem( i );
}

void ListViewEditor::itemRightClicked()
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    Q3ListViewItemIterator it( i );
    Q3ListViewItem *parent = i->parent();
    parent = parent ? parent->firstChild() : itemsPreview->firstChild();
    if ( !parent )
	return;
    it++;
    while ( it.current() ) {
	if ( it.current()->parent() == parent )
	    break;
	it++;
    }

    if ( !it.current() )
	return;
    Q3ListViewItem *other = it.current();

    for ( int c = 0; c < itemsPreview->columns(); ++c ) {
	QString s = i->text( c );
	i->setText( c, other->text( c ) );
	other->setText( c, s );
	QPixmap pix;
	if ( i->pixmap( c ) )
	    pix = *i->pixmap( c );
	if ( other->pixmap( c ) )
	    i->setPixmap( c, *other->pixmap( c ) );
	else
	    i->setPixmap( c, QPixmap() );
	other->setPixmap( c, pix );
    }

    itemsPreview->setCurrentItem( other );
    itemsPreview->setSelected( other, TRUE );
}

void ListViewEditor::itemLeftClicked()
{
    Q3ListViewItem *i = itemsPreview->currentItem();
    if ( !i )
	return;

    Q3ListViewItemIterator it( i );
    Q3ListViewItem *parent = i->parent();
    if ( !parent )
	return;
    parent = parent->parent();
    --it;
    while ( it.current() ) {
	if ( it.current()->parent() == parent )
	    break;
	--it;
    }

    if ( !it.current() )
	return;
    Q3ListViewItem *other = it.current();

    for ( int c = 0; c < itemsPreview->columns(); ++c ) {
	QString s = i->text( c );
	i->setText( c, other->text( c ) );
	other->setText( c, s );
	QPixmap pix;
	if ( i->pixmap( c ) )
	    pix = *i->pixmap( c );
	if ( other->pixmap( c ) )
	    i->setPixmap( c, *other->pixmap( c ) );
	else
	    i->setPixmap( c, QPixmap() );
	other->setPixmap( c, pix );
    }

    itemsPreview->setCurrentItem( other );
    itemsPreview->setSelected( other, TRUE );
}

void ListViewEditor::setupColumns()
{
    Q3Header *h = listview->header();
    for ( int i = 0; i < (int)h->count(); ++i ) {
	Column col;
	col.text = h->label( i );
	col.pixmap = QPixmap();
	if ( h->iconSet( i ) )
	    col.pixmap = h->iconSet( i )->pixmap();
	col.clickable = h->isClickEnabled( i );
	col.resizable = h->isResizeEnabled( i );
	if ( !col.pixmap.isNull() )
	    col.item = new Q3ListBoxPixmap( colPreview, col.pixmap, col.text );
	else
	    col.item = new Q3ListBoxText( colPreview, col.text );
	columns.append( col );
    }

    colText->setEnabled( FALSE );
    colPixmap->setEnabled( FALSE );
    colClickable->setEnabled( FALSE );
    colResizable->setEnabled( FALSE );

    if ( colPreview->firstItem() )
	colPreview->setCurrentItem( colPreview->firstItem() );
    numColumns = colPreview->count();
}

void ListViewEditor::setupItems()
{
    itemColumn->setMinValue( 0 );
    itemColumn->setMaxValue( QMAX( numColumns - 1, 0 ) );
    int i = 0;
    Q3Header *header = itemsPreview->header();
    for ( Q3ListBoxItem *item = colPreview->firstItem(); item; item = item->next() ) {
	Column *col = findColumn( item );
	if ( !col )
	    continue;
	if ( i >= itemsPreview->columns() )
	    itemsPreview->addColumn( col->text );
	header->setLabel( i, col->pixmap, col->text );
	header->setResizeEnabled( col->resizable, i );
	header->setClickEnabled( col->clickable, i );
	++i;
    }
    while ( itemsPreview->columns() > i )
	itemsPreview->removeColumn( i );

    itemColumn->setValue( QMIN( numColumns - 1, itemColumn->value() ) );
}

ListViewEditor::Column *ListViewEditor::findColumn( Q3ListBoxItem *i )
{
    if ( !i )
	return 0;

    for ( Q3ValueList<Column>::Iterator it = columns.begin(); it != columns.end(); ++it ) {
	if ( ( *it ).item == i )
	    return &( *it );
    }

    return 0;
}

void ListViewEditor::initTabPage( const QString &page )
{
    numColumns = colPreview->count();
    if ( page == i18n( "&Items" ) ) {
	setupItems();
	if ( numColumns == 0 ) {
	    itemNew->setEnabled( FALSE );
	    itemNewSub->setEnabled( FALSE );
	    itemText->setEnabled( FALSE );
	    itemChoosePixmap->setEnabled( FALSE );
	    itemDeletePixmap->setEnabled( FALSE );
	    itemColumn->setEnabled( FALSE );
	} else {
	    itemNew->setEnabled( TRUE );
	    itemNewSub->setEnabled( TRUE );
	}
    }
}

void ListViewEditor::emitItemRenamed( Q3ListViewItem *, int, const QString & text )
{
    emit itemRenamed( text ); // Relay signal ( to QLineEdit )
}
