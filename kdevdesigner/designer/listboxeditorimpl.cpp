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

#include "listboxeditorimpl.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "pixmapchooser.h"
#include "command.h"
#include "listboxdnd.h"
#include "listboxrename.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

ListBoxEditor::ListBoxEditor( QWidget *parent, QWidget *editWidget, FormWindow *fw )
    : ListBoxEditorBase( parent, 0, TRUE ), formwindow( fw )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    listbox = (QListBox*)editWidget;

    itemText->setText( "" );
    itemText->setEnabled( FALSE );
    itemPixmap->setText( "" );
    itemChoosePixmap->setEnabled( FALSE );
    itemDeletePixmap->setEnabled( FALSE );

    QListBoxItem *i = 0;
    for ( i = listbox->firstItem(); i; i = i->next() ) {
	if ( i->pixmap() )
	    (void)new QListBoxPixmap( preview, *i->pixmap(), i->text() );
	else
	    (void)new QListBoxText( preview, i->text() );
    }

    if ( preview->firstItem() )
	preview->setCurrentItem( preview->firstItem() );

    ListBoxDnd *editorDnd = new ListBoxDnd( preview );
    editorDnd->setDragMode( ListBoxDnd::Internal | ListBoxDnd::Move );
    QObject::connect( editorDnd, SIGNAL( dropped( QListBoxItem * ) ),
		      editorDnd, SLOT( confirmDrop( QListBoxItem * ) ) );

    ListBoxRename *editorRename = new ListBoxRename( preview );

    QObjectList *l = parent->queryList( "QLineEdit", "itemText" );
    QObjectListIt it( *l );
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
	QObject::connect( editorRename,
			  SIGNAL( itemTextChanged( const QString & ) ),
			  obj,
			  SLOT( setText( const QString & ) ) );
    }
    delete l;
    
}

void ListBoxEditor::insertNewItem()
{
    QListBoxItem *i = new QListBoxText( preview, tr("New Item") );
    preview->setCurrentItem( i );
    preview->setSelected( i, TRUE );
    itemText->setFocus();
    itemText->selectAll();
}

void ListBoxEditor::deleteCurrentItem()
{
    delete preview->item( preview->currentItem() );
    if ( preview->currentItem() != -1 )
	preview->setSelected( preview->currentItem(), TRUE );
}

void ListBoxEditor::currentItemChanged( QListBoxItem *i )
{
    itemText->blockSignals( TRUE );
    itemText->setText( "" );
    itemPixmap->setText( "" );
    itemText->blockSignals( FALSE );

    if ( !i ) {
	itemText->setEnabled( FALSE );
	itemChoosePixmap->setEnabled( FALSE );
	itemDeletePixmap->setEnabled( FALSE );
	return;
    }

    itemText->blockSignals( TRUE );
    itemText->setEnabled( TRUE );
    itemChoosePixmap->setEnabled( TRUE );
    itemDeletePixmap->setEnabled( i->pixmap() && !i->pixmap()->isNull() );

    itemText->setText( i->text() );
    if ( i->pixmap() )
	itemPixmap->setPixmap( *i->pixmap() );
    itemText->blockSignals( FALSE );
}

void ListBoxEditor::currentTextChanged( const QString &txt )
{
    if ( preview->currentItem() == -1 )
	return;

    preview->blockSignals( TRUE );
    if ( preview->item( preview->currentItem() )->pixmap() )
	preview->changeItem( *preview->item( preview->currentItem() )->pixmap(), txt, preview->currentItem() );
    else
	preview->changeItem( txt, preview->currentItem() );
    preview->blockSignals( FALSE );
}

void ListBoxEditor::okClicked()
{
    applyClicked();
    accept();
}

void ListBoxEditor::cancelClicked()
{
    reject();
}

void ListBoxEditor::applyClicked()
{
    QListBoxItem *i = 0;
    QValueList<PopulateListBoxCommand::Item> items;
    for ( i = preview->firstItem(); i; i = i->next() ) {
	PopulateListBoxCommand::Item item;
	if ( i->pixmap() )
	    item.pix = *i->pixmap();
	item.text = i->text();
	items.append( item );
    }

    PopulateListBoxCommand *cmd = new PopulateListBoxCommand( tr( "Edit the Items of '%1'" ).arg( listbox->name() ),
							      formwindow, listbox, items );
    cmd->execute();
    formwindow->commandHistory()->addCommand( cmd );
}

void ListBoxEditor::choosePixmap()
{
    if ( preview->currentItem() == -1 )
	return;

    QPixmap pix;
    if ( preview->item( preview->currentItem() )->pixmap() )
    	pix = qChoosePixmap( this, formwindow, *preview->item( preview->currentItem() )->pixmap() );
    else
    	pix = qChoosePixmap( this, formwindow, QPixmap() );

    if ( pix.isNull() )
	return;

    QString txt = preview->item( preview->currentItem() )->text();
    preview->changeItem( pix, txt, preview->currentItem() );
    itemDeletePixmap->setEnabled( TRUE );
}

void ListBoxEditor::moveItemUp()
{
    if ( preview->currentItem() < 1 )
	return;

    QListBoxItem *i = preview->item( preview->currentItem() );
    bool hasPix = (i->pixmap() != 0);
    QPixmap pix;
    if ( hasPix )
	pix = *i->pixmap();
    QString txt = i->text();

    QListBoxItem *p = i->prev();
    if ( p->pixmap() )
	preview->changeItem( *p->pixmap(), p->text(), preview->currentItem() );
    else
	preview->changeItem( p->text(), preview->currentItem() );

    if ( hasPix )
	preview->changeItem( pix, txt, preview->currentItem() - 1 );
    else
	preview->changeItem( txt, preview->currentItem() - 1 );
}

void ListBoxEditor::moveItemDown()
{
    if ( preview->currentItem() == -1 || preview->currentItem() > (int)preview->count() - 2 )
	return;

    QListBoxItem *i = preview->item( preview->currentItem() );
    bool hasPix = (i->pixmap() != 0);
    QPixmap pix;
    if ( hasPix )
	pix = *i->pixmap();
    QString txt = i->text();

    QListBoxItem *n = i->next();
    if ( n->pixmap() )
	preview->changeItem( *n->pixmap(), n->text(), preview->currentItem() );
    else
	preview->changeItem( n->text(), preview->currentItem() );

    if ( hasPix )
	preview->changeItem( pix, txt, preview->currentItem() + 1 );
    else
	preview->changeItem( txt, preview->currentItem() + 1 );
}

void ListBoxEditor::deletePixmap()
{
    if ( preview->currentItem() == -1 )
	return;

    QListBoxItem *i = preview->item( preview->currentItem() );
    preview->changeItem( i->text(), preview->currentItem() );
    itemDeletePixmap->setEnabled( FALSE );
}
