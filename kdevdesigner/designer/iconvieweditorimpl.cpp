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

#include "iconvieweditorimpl.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "pixmapchooser.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qiconview.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

IconViewEditor::IconViewEditor( QWidget *parent, QWidget *editWidget, FormWindow *fw )
    : IconViewEditorBase( parent, 0, TRUE ), formwindow( fw )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    iconview = (QIconView*)editWidget;

    itemText->setText( "" );
    itemText->setEnabled( FALSE );
    itemPixmap->setText( "" );
    itemChoosePixmap->setEnabled( FALSE );
    itemDeletePixmap->setEnabled( FALSE );

    QIconViewItem *i = 0;
    for ( i = iconview->firstItem(); i; i = i->nextItem() ) {
	(void)new QIconViewItem( preview, i->text(), *i->pixmap()  );
    }

    if ( preview->firstItem() )
	preview->setCurrentItem( preview->firstItem() );
}

void IconViewEditor::insertNewItem()
{
    QIconViewItem *i = new QIconViewItem( preview, tr( "New Item" ) );
    preview->setCurrentItem( i );
    preview->setSelected( i, TRUE );
    itemText->setFocus();
    itemText->selectAll();
}

void IconViewEditor::deleteCurrentItem()
{
    delete preview->currentItem();
    if ( preview->currentItem() )
	preview->setSelected( preview->currentItem(), TRUE );
}

void IconViewEditor::currentItemChanged( QIconViewItem *i )
{
    itemText->blockSignals( TRUE );
    itemText->setText( "" );
    itemPixmap->setText( "" );
    itemText->blockSignals( FALSE );

    if ( !i ) {
	itemText->setEnabled( FALSE );
	itemChoosePixmap->setEnabled( FALSE );
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

void IconViewEditor::currentTextChanged( const QString &txt )
{
    if ( !preview->currentItem() )
	return;

    preview->currentItem()->setText( txt );
}

void IconViewEditor::okClicked()
{
    applyClicked();
    accept();
}

void IconViewEditor::cancelClicked()
{
    reject();
}

void IconViewEditor::applyClicked()
{
    QIconViewItem *i = 0;
    QValueList<PopulateIconViewCommand::Item> items;
    for ( i = preview->firstItem(); i; i = i->nextItem() ) {
	PopulateIconViewCommand::Item item;
	if ( i->pixmap() )
	    item.pix = *i->pixmap();
	item.text = i->text();
	items.append( item );
    }

    PopulateIconViewCommand *cmd = new PopulateIconViewCommand( tr( "Edit the Items of '%1'" ).arg( iconview->name() ),
								formwindow, iconview, items );
    cmd->execute();
    formwindow->commandHistory()->addCommand( cmd );
}

void IconViewEditor::choosePixmap()
{
    if ( !preview->currentItem() )
	return;

    QPixmap pix;
    if ( preview->currentItem()->pixmap() )
    	pix = qChoosePixmap( this, formwindow, *preview->currentItem()->pixmap() );
    else
    	pix = qChoosePixmap( this, formwindow, QPixmap() );

    if ( pix.isNull() )
	return;

    preview->currentItem()->setPixmap( pix );
    itemPixmap->setPixmap( pix );
    itemDeletePixmap->setEnabled( TRUE );
}

void IconViewEditor::deletePixmap()
{
    if ( !preview->currentItem() )
	return;

    preview->currentItem()->setPixmap( QPixmap() );
    itemPixmap->setText( "" );
    itemDeletePixmap->setEnabled( FALSE );
}
