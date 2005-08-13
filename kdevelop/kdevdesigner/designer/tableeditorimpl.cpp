/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include "tableeditorimpl.h"
#ifndef QT_NO_TABLE
#include <qtable.h>
#endif
#include "formwindow.h"
#include <qlabel.h>
#include <qcombobox.h>
#include <qheader.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include "pixmapchooser.h"
#include "command.h"
#include <qvaluelist.h>
#include <qtabwidget.h>
#include <qdatatable.h>
#include "project.h"
#include "metadatabase.h"
#include "mainwindow.h"

#include <klocale.h>

TableEditor::TableEditor( QWidget* parent,  QWidget *editWidget, FormWindow *fw, const char* name, bool modal, WFlags fl )
    : TableEditorBase( parent, name, modal, fl ),
#ifndef QT_NO_TABLE
    editTable( (QTable*)editWidget ),
#endif
    formWindow( fw )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
#ifndef QT_NO_TABLE
    labelColumnPixmap->setText( "" );
    labelRowPixmap->setText( "" );

#ifndef QT_NO_SQL
    if ( !::qt_cast<QDataTable*>(editTable) )
#endif
    {
	labelFields->hide();
	comboFields->hide();
	labelTable->hide();
	labelTableValue->hide();
    }
#ifndef QT_NO_SQL
    if ( ::qt_cast<QDataTable*>(editTable) ) {
	// ## why does this behave weird?
	//	TabWidget->removePage( rows_tab );
	//	rows_tab->hide();
	// ## do this in the meantime...
	TabWidget->setTabEnabled( rows_tab, FALSE );
    }

    if ( formWindow->project() && ::qt_cast<QDataTable*>(editTable) ) {
	QStringList lst = MetaDataBase::fakeProperty( editTable, "database" ).toStringList();
	if ( lst.count() == 2 && !lst[ 0 ].isEmpty() && !lst[ 1 ].isEmpty() ) {
	    QStringList fields;
	    fields << "<no field>";
	    fields += formWindow->project()->databaseFieldList( lst[ 0 ], lst[ 1 ] );
	    comboFields->insertStringList( fields );
	}
	if ( !lst[ 1 ].isEmpty() )
	    labelTableValue->setText( lst[ 1 ] );
    }
#endif

    readFromTable();
#endif
}

TableEditor::~TableEditor()
{
}

void TableEditor::columnDownClicked()
{
    if ( listColumns->currentItem() == -1 ||
	 listColumns->currentItem() == (int)listColumns->count() - 1 ||
	 listColumns->count() < 2 )
	return;
    saveFieldMap();
    int index = listColumns->currentItem() + 1;
    QListBoxItem *i = listColumns->item( listColumns->currentItem() );
    listColumns->takeItem( i );
    listColumns->insertItem( i, index );
    listColumns->setCurrentItem( i );
    readColumns();
    restoreFieldMap();
    currentColumnChanged( i );
}

void TableEditor::columnTextChanged( const QString &s )
{
    if ( listColumns->currentItem() == -1 )
	return;
    listColumns->blockSignals( TRUE );
    listColumns->changeItem( s, listColumns->currentItem() );
    listColumns->blockSignals( FALSE );
#ifndef QT_NO_TABLE
    if ( table->horizontalHeader()->iconSet( listColumns->currentItem() ) )
	table->horizontalHeader()->setLabel( listColumns->currentItem(),
					     *table->horizontalHeader()->iconSet( listColumns->currentItem() ), s );
    else
	table->horizontalHeader()->setLabel( listColumns->currentItem(), s );
#endif
}

void TableEditor::columnUpClicked()
{
    if ( listColumns->currentItem() <= 0 ||
	 listColumns->count() < 2 )
	return;
    saveFieldMap();
    int index = listColumns->currentItem() - 1;
    QListBoxItem *i = listColumns->item( listColumns->currentItem() );
    listColumns->takeItem( i );
    listColumns->insertItem( i, index );
    listColumns->setCurrentItem( i );
    readColumns();
    restoreFieldMap();
    currentColumnChanged( i );
}

void TableEditor::currentColumnChanged( QListBoxItem *i )
{
    if ( !i )
	return;
    editColumnText->blockSignals( TRUE );
    editColumnText->setText( i->text() );
    if ( i->pixmap() )
	labelColumnPixmap->setPixmap( *i->pixmap() );
    else
	labelColumnPixmap->setText( "" );
    editColumnText->blockSignals( FALSE );

#ifndef QT_NO_SQL
    if ( ::qt_cast<QDataTable*>(editTable) ) {
	QString s = *fieldMap.find( listColumns->index( i ) );
	if ( s.isEmpty() )
	    comboFields->setCurrentItem( 0 );
	else if ( comboFields->listBox()->findItem( s ) )
	    comboFields->setCurrentItem( comboFields->listBox()->index( comboFields->listBox()->findItem( s ) ) );
	else
	    comboFields->lineEdit()->setText( s );
    }
#endif
}

void TableEditor::currentFieldChanged( const QString &s )
{
    if ( listColumns->currentItem() == -1 )
	return;
    fieldMap.remove( listColumns->currentItem() );
    fieldMap.insert( listColumns->currentItem(), s );
    editColumnText->blockSignals( TRUE ); //## necessary
    QString newColText = s.mid(0,1).upper() + s.mid(1);
    editColumnText->setText( newColText );
    columnTextChanged( newColText );
    editColumnText->blockSignals( FALSE );
}

void TableEditor::currentRowChanged( QListBoxItem *i )
{
    if ( !i )
	return;
    editRowText->blockSignals( TRUE );
    editRowText->setText( i->text() );
    if ( i->pixmap() )
	labelRowPixmap->setPixmap( *i->pixmap() );
    else
	labelRowPixmap->setText( "" );
    editRowText->blockSignals( FALSE );
}

void TableEditor::deleteColumnClicked()
{
    if ( listColumns->currentItem() == -1 )
	return;
#ifndef QT_NO_TABLE
    table->setNumCols( table->numCols() - 1 );
    delete listColumns->item( listColumns->currentItem() );
    readColumns();
    if ( listColumns->firstItem() ) {
	listColumns->setCurrentItem( listColumns->firstItem() );
	listColumns->setSelected( listColumns->firstItem(), TRUE );
    }
#endif
}

void TableEditor::deleteRowClicked()
{
    if ( listRows->currentItem() == -1 )
	return;
#ifndef QT_NO_TABLE
    table->setNumRows( table->numRows() - 1 );
    delete listRows->item( listRows->currentItem() );
    readRows();
    if ( listRows->firstItem() ) {
	listRows->setCurrentItem( listRows->firstItem() );
	listRows->setSelected( listRows->firstItem(), TRUE );
    }
#endif
}

void TableEditor::newColumnClicked()
{
#ifndef QT_NO_TABLE
    table->setNumCols( table->numCols() + 1 );
    QMap<QString, bool> m;
    for ( int i = 0; i < table->numCols() - 1; ++i )
	m.insert( table->horizontalHeader()->label( i ), TRUE );
    int n = table->numCols() - 1;
    QString t = QString::number( n );
    while ( m.find( t ) != m.end() )
	t = QString::number( ++n );
    table->horizontalHeader()->setLabel( table->numCols() - 1, t );
    listColumns->insertItem( t );
    QListBoxItem *item = listColumns->item( listColumns->count() - 1 );
    listColumns->setCurrentItem( item );
    listColumns->setSelected( item, TRUE );
#ifndef QT_NO_SQL
    if ( ::qt_cast<QDataTable*>(editTable) ) {
	comboFields->setFocus();
    } else
#endif
    {
	editColumnText->setFocus();
	editColumnText->selectAll();
    }
#endif
}

void TableEditor::newRowClicked()
{
#ifndef QT_NO_TABLE
    table->setNumRows( table->numRows() + 1 );
    QMap<QString, bool> m;
    for ( int i = 0; i < table->numRows() - 1; ++i )
	m.insert( table->verticalHeader()->label( i ), TRUE );
    int n = table->numRows() - 1;
    QString t = QString::number( n );
    while ( m.find( t ) != m.end() )
	t = QString::number( ++n );
    table->verticalHeader()->setLabel( table->numRows() - 1, t );
    listRows->insertItem( t );
    QListBoxItem *item = listRows->item( listRows->count() - 1 );
    listRows->setCurrentItem( item );
    listRows->setSelected( item, TRUE );
#endif
}

void TableEditor::okClicked()
{
    applyClicked();
    accept();
}

void TableEditor::rowDownClicked()
{
    if ( listRows->currentItem() == -1 ||
	 listRows->currentItem() == (int)listRows->count() - 1 ||
	 listRows->count() < 2 )
	return;
    int index = listRows->currentItem() + 1;
    QListBoxItem *i = listRows->item( listRows->currentItem() );
    listRows->takeItem( i );
    listRows->insertItem( i, index );
    listRows->setCurrentItem( i );
    readRows();
}

void TableEditor::rowTextChanged( const QString &s )
{
    if ( listRows->currentItem() == -1 )
	return;
#ifndef QT_NO_TABLE
    listRows->changeItem( s, listRows->currentItem() );
    if ( table->verticalHeader()->iconSet( listRows->currentItem() ) )
	table->verticalHeader()->setLabel( listRows->currentItem(),
					     *table->verticalHeader()->iconSet( listRows->currentItem() ), s );
    else
	table->verticalHeader()->setLabel( listRows->currentItem(), s );
#endif
}

void TableEditor::rowUpClicked()
{
    if ( listRows->currentItem() <= 0 ||
	 listRows->count() < 2 )
	return;
    int index = listRows->currentItem() - 1;
    QListBoxItem *i = listRows->item( listRows->currentItem() );
    listRows->takeItem( i );
    listRows->insertItem( i, index );
    listRows->setCurrentItem( i );
    readRows();
}

void TableEditor::applyClicked()
{
    QValueList<PopulateTableCommand::Row> rows;
    QValueList<PopulateTableCommand::Column> cols;

    int i = 0;
#ifndef QT_NO_TABLE
    for ( i = 0; i < table->horizontalHeader()->count(); ++i ) {
	PopulateTableCommand::Column col;
	col.text = table->horizontalHeader()->label( i );
	if ( table->horizontalHeader()->iconSet( i ) )
	    col.pix = table->horizontalHeader()->iconSet( i )->pixmap();
	col.field = *fieldMap.find( i );
	cols.append( col );
    }
    for ( i = 0; i < table->verticalHeader()->count(); ++i ) {
	PopulateTableCommand::Row row;
	row.text = table->verticalHeader()->label( i );
	if ( table->verticalHeader()->iconSet( i ) )
	    row.pix = table->verticalHeader()->iconSet( i )->pixmap();
	rows.append( row );
    }
    PopulateTableCommand *cmd = new PopulateTableCommand( i18n( "Edit the Rows and Columns of '%1' " ).arg( editTable->name() ),
							  formWindow, editTable, rows, cols );
    cmd->execute();
    formWindow->commandHistory()->addCommand( cmd );
#endif
}

void TableEditor::chooseRowPixmapClicked()
{
    if ( listRows->currentItem() == -1 )
	return;
    QPixmap pix;
    if ( listRows->item( listRows->currentItem() )->pixmap() )
	pix = qChoosePixmap( this, formWindow, *listRows->item( listRows->currentItem() )->pixmap() );
    else
	pix = qChoosePixmap( this, formWindow, QPixmap() );

    if ( pix.isNull() )
	return;

#ifndef QT_NO_TABLE
    table->verticalHeader()->setLabel( listRows->currentItem(), pix, table->verticalHeader()->label( listRows->currentItem() ) );
    listRows->changeItem( pix, listRows->currentText(), listRows->currentItem() );
#endif
}

void TableEditor::deleteRowPixmapClicked()
{
    if ( listRows->currentItem() == -1 )
	return;
#ifndef QT_NO_TABLE
    table->verticalHeader()->setLabel( listRows->currentItem(), QPixmap(), table->verticalHeader()->label( listRows->currentItem() ) );
    listRows->changeItem( listRows->currentText(), listRows->currentItem() );
#endif
}

void TableEditor::chooseColPixmapClicked()
{
    if ( listColumns->currentItem() == -1 )
	return;
    QPixmap pix;
    if ( listColumns->item( listColumns->currentItem() )->pixmap() )
	pix = qChoosePixmap( this, formWindow, *listColumns->item( listColumns->currentItem() )->pixmap() );
    else
	pix = qChoosePixmap( this, formWindow, QPixmap() );

    if ( pix.isNull() )
	return;
#ifndef QT_NO_TABLE
    table->horizontalHeader()->setLabel( listColumns->currentItem(), pix, table->horizontalHeader()->label( listColumns->currentItem() ) );
    listColumns->changeItem( pix, listColumns->currentText(), listColumns->currentItem() );
#endif
}

void TableEditor::deleteColPixmapClicked()
{
    if ( listColumns->currentItem() == -1 )
	return;
#ifndef QT_NO_TABLE
    table->horizontalHeader()->setLabel( listColumns->currentItem(), QPixmap(), table->horizontalHeader()->label( listColumns->currentItem() ) );
    listColumns->changeItem( listColumns->currentText(), listColumns->currentItem() );
#endif
}

void TableEditor::readFromTable()
{
#ifndef QT_NO_TABLE
    QHeader *cols = editTable->horizontalHeader();
    table->setNumCols( cols->count() );
    QMap<QString, QString> columnFields = MetaDataBase::columnFields( editTable );
    for ( int i = 0; i < cols->count(); ++i ) {
	if ( editTable->horizontalHeader()->iconSet( i ) ) {
	    table->horizontalHeader()->setLabel( i, *editTable->horizontalHeader()->iconSet( i ),
						 editTable->horizontalHeader()->label( i ) );
	    listColumns->insertItem( editTable->horizontalHeader()->iconSet( i )->pixmap(),
				     editTable->horizontalHeader()->label( i ) );
	} else {
	    table->horizontalHeader()->setLabel( i, editTable->horizontalHeader()->label( i ) );
	    listColumns->insertItem( editTable->horizontalHeader()->label( i ) );
	}
	QString cf = *columnFields.find( editTable->horizontalHeader()->label( i ) );
	fieldMap.insert( i, cf );
    }

    if ( listColumns->firstItem() ) {
	listColumns->setCurrentItem( listColumns->firstItem() );
	listColumns->setSelected( listColumns->firstItem(), TRUE );
    }

    QHeader *rows = editTable->verticalHeader();
    table->setNumRows( rows->count() );
    for ( int j = 0; j < rows->count(); ++j ) {
	if ( editTable->verticalHeader()->iconSet( j ) ) {
	    table->verticalHeader()->setLabel( j, *editTable->verticalHeader()->iconSet( j ),
					       editTable->verticalHeader()->label( j ) );
	    listRows->insertItem( editTable->verticalHeader()->iconSet( j )->pixmap(),
				  editTable->verticalHeader()->label( j ) );
	} else {
	    table->verticalHeader()->setLabel( j, editTable->verticalHeader()->label( j ) );
	    listRows->insertItem( editTable->verticalHeader()->label( j ) );
	}
    }

    if ( listRows->firstItem() ) {
	listRows->setCurrentItem( listRows->firstItem() );
	listRows->setSelected( listRows->firstItem(), TRUE );
    }
#endif
}

void TableEditor::readColumns()
{
    int j = 0;
#ifndef QT_NO_TABLE
    for ( QListBoxItem *i = listColumns->firstItem(); i; i = i->next(), ++j ) {
	if ( i->pixmap() )
	    table->horizontalHeader()->setLabel( j, *i->pixmap(), i->text() );
	else
	    table->horizontalHeader()->setLabel( j, i->text() );
    }
#endif
}

void TableEditor::readRows()
{
    int j = 0;
#ifndef QT_NO_TABLE
    for ( QListBoxItem *i = listRows->firstItem(); i; i = i->next(), ++j ) {
	if ( i->pixmap() )
	    table->verticalHeader()->setLabel( j, *i->pixmap(), i->text() );
	else
	    table->verticalHeader()->setLabel( j, i->text() );
    }
#endif
}

void TableEditor::saveFieldMap()
{
    tmpFieldMap.clear();
    for ( QMap<int, QString>::Iterator it = fieldMap.begin(); it != fieldMap.end(); ++it )
	tmpFieldMap.insert( listColumns->item( it.key() ), *it );
}

void TableEditor::restoreFieldMap()
{
    fieldMap.clear();
    for ( QMap<QListBoxItem*, QString>::Iterator it = tmpFieldMap.begin(); it != tmpFieldMap.end(); ++it )
	fieldMap.insert( listColumns->index( it.key() ), *it );
}
