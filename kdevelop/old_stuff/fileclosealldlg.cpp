/***************************************************************************
     
    begin                : Wed Jan 5 2000
    copyright            : (C) 2000 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "fileclosealldlg.h"
#include <qheader.h>

#include <qlabel.h>

FileCloseAllDlg::FileCloseAllDlg(QWidget* parent,const char* name,QStringList* files):  QDialog( parent, name, TRUE, 0 ){
	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( this, "Label_1" );
	qtarch_Label_1->setGeometry( 10, 10, 440, 40 );
	qtarch_Label_1->setMinimumSize( 0, 0 );
	qtarch_Label_1->setMaximumSize( 32767, 32767 );
	qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_1->setText( "There are modified files. Select files to be saved:" );
	qtarch_Label_1->setAlignment( 289 );
	qtarch_Label_1->setMargin( -1 );

	sellect_all_button = new QPushButton( this, "sellect_all_button" );
	sellect_all_button->setGeometry( 10, 250, 100, 30 );
	sellect_all_button->setMinimumSize( 0, 0 );
	sellect_all_button->setMaximumSize( 32767, 32767 );
	sellect_all_button->setFocusPolicy( QWidget::TabFocus );
	sellect_all_button->setBackgroundMode( QWidget::PaletteBackground );
	sellect_all_button->setFontPropagation( QWidget::NoChildren );
	sellect_all_button->setPalettePropagation( QWidget::NoChildren );
	sellect_all_button->setText( "Select all" );
	sellect_all_button->setAutoRepeat( FALSE );
	sellect_all_button->setAutoResize( FALSE );

	sellect_none_button = new QPushButton( this, "sellect_none_button" );
	sellect_none_button->setGeometry( 120, 250, 100, 30 );
	sellect_none_button->setMinimumSize( 0, 0 );
	sellect_none_button->setMaximumSize( 32767, 32767 );
	sellect_none_button->setFocusPolicy( QWidget::TabFocus );
	sellect_none_button->setBackgroundMode( QWidget::PaletteBackground );
	sellect_none_button->setFontPropagation( QWidget::NoChildren );
	sellect_none_button->setPalettePropagation( QWidget::NoChildren );
	sellect_none_button->setText( "Select none" );
	sellect_none_button->setAutoRepeat( FALSE );
	sellect_none_button->setAutoResize( FALSE );

	ok_button = new QPushButton( this, "PushButton_3" );
	ok_button->setGeometry( 230, 250, 100, 30 );
	ok_button->setMinimumSize( 0, 0 );
	ok_button->setMaximumSize( 32767, 32767 );
	ok_button->setFocusPolicy( QWidget::TabFocus );
	ok_button->setBackgroundMode( QWidget::PaletteBackground );
	ok_button->setFontPropagation( QWidget::NoChildren );
	ok_button->setPalettePropagation( QWidget::NoChildren );
	ok_button->setText( "OK" );
	ok_button->setAutoRepeat( FALSE );
	ok_button->setAutoResize( FALSE );

	cancel_button = new QPushButton( this, "cancel_button" );
	cancel_button->setGeometry( 350, 250, 100, 30 );
	cancel_button->setMinimumSize( 0, 0 );
	cancel_button->setMaximumSize( 32767, 32767 );
	cancel_button->setFocusPolicy( QWidget::TabFocus );
	cancel_button->setBackgroundMode( QWidget::PaletteBackground );
	cancel_button->setFontPropagation( QWidget::NoChildren );
	cancel_button->setPalettePropagation( QWidget::NoChildren );
	cancel_button->setText( "Cancel" );
	cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );

	files_listview = new QListView( this, "files_listview" );
	files_listview->setGeometry( 10, 50, 440, 190 );
	files_listview->addColumn( "clmn0" );
	files_listview->header()->hide();

	
	files_listview->setRootIsDecorated( FALSE );
	files_listview->addColumn( "Sample items", -1 );
	resize( 460,290 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );


	QValueList<QString>::Iterator it;
	QCheckListItem* item;
	for( it = files->begin(); it != files->end(); ++it ){
	    item =  new QCheckListItem(files_listview,*it,QCheckListItem::CheckBox);
	    item->setOn(true);
	}
	
	connect(ok_button,SIGNAL(clicked()),SLOT(slotOK()));
	connect(sellect_all_button,SIGNAL(clicked()),SLOT(slotSelectAll()));
	connect(sellect_none_button,SIGNAL(clicked()),SLOT(slotSelectNone()));
	connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
}
void FileCloseAllDlg::slotSelectAll(){
    QCheckListItem* item = static_cast<QCheckListItem*>(files_listview->firstChild());
    item->setOn(true);
    for(;item != 0;item = static_cast<QCheckListItem*>(item->nextSibling())){
	item->setOn(true);
    }
    
    
}
void FileCloseAllDlg::slotSelectNone(){
    QCheckListItem* item = static_cast<QCheckListItem*>(files_listview->firstChild());
    item->setOn(true);
    for(;item != 0;item = static_cast<QCheckListItem*>(item->nextSibling())){
	item->setOn(false);
    }
}
void FileCloseAllDlg::slotOK(){
    accept();
}

void FileCloseAllDlg::getSelectedFiles(QStringList* list){
  list->clear();
  QCheckListItem* item = static_cast<QCheckListItem*>(files_listview->firstChild());
  if(item->isOn()){
    list->append(item->text(0));
  }
  for(;item != 0;item = static_cast<QCheckListItem*>(item->nextSibling())){
    if(item->isOn()){
      list->append(item->text(0));
    }
  }
}
#include "fileclosealldlg.moc"
