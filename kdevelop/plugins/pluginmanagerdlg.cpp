/***************************************************************************
                          pluginmanagerdlg.cpp  -  description
                             -------------------
    begin                : Thu Sep 23 1999
    copyright            : (C) 1999 by Sandy Meier
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

#include "pluginmanagerdlg.h"

PluginManagerDlg::PluginManagerDlg(QWidget *parent, const char *name ) : QDialog(parent,name,true) {
    	ok_button = new QPushButton( this, "ok_button" );
	ok_button->setGeometry( 450, 380, 100, 30 );
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
	cancel_button->setGeometry( 570, 380, 100, 30 );
	cancel_button->setMinimumSize( 0, 0 );
	cancel_button->setMaximumSize( 32767, 32767 );
	cancel_button->setFocusPolicy( QWidget::TabFocus );
	cancel_button->setBackgroundMode( QWidget::PaletteBackground );
	cancel_button->setFontPropagation( QWidget::NoChildren );
	cancel_button->setPalettePropagation( QWidget::NoChildren );
	cancel_button->setText( "Cancel" );
	cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );

	plugin_listbox = new CheckListBox( this, "plugin_listbox" );
	plugin_listbox->setGeometry( 10, 10, 290, 360 );
	

	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( this, "Label_1" );
	qtarch_Label_1->setGeometry( 310, 10, 70, 30 );
	qtarch_Label_1->setMinimumSize( 0, 0 );
	qtarch_Label_1->setMaximumSize( 32767, 32767 );
	qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_1->setText( "Author:" );
	qtarch_Label_1->setAlignment( 289 );
	qtarch_Label_1->setMargin( -1 );

	QLabel* qtarch_Label_2;
	qtarch_Label_2 = new QLabel( this, "Label_2" );
	qtarch_Label_2->setGeometry( 310, 100, 90, 30 );
	qtarch_Label_2->setMinimumSize( 0, 0 );
	qtarch_Label_2->setMaximumSize( 32767, 32767 );
	qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_2->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_2->setText( "Copyright:" );
	qtarch_Label_2->setAlignment( 289 );
	qtarch_Label_2->setMargin( -1 );

	QLabel* qtarch_Label_4;
	qtarch_Label_4 = new QLabel( this, "Label_4" );
	qtarch_Label_4->setGeometry( 310, 130, 80, 30 );
	qtarch_Label_4->setMinimumSize( 0, 0 );
	qtarch_Label_4->setMaximumSize( 32767, 32767 );
	qtarch_Label_4->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_4->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_4->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_4->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_4->setText( "Version:" );
	qtarch_Label_4->setAlignment( 289 );
	qtarch_Label_4->setMargin( -1 );

	author_label = new QLabel( this, "author_label" );
	author_label->setGeometry( 390, 10, 280, 30 );
	author_label->setMinimumSize( 0, 0 );
	author_label->setMaximumSize( 32767, 32767 );
	author_label->setFocusPolicy( QWidget::NoFocus );
	author_label->setBackgroundMode( QWidget::PaletteBackground );
	author_label->setFontPropagation( QWidget::NoChildren );
	author_label->setPalettePropagation( QWidget::NoChildren );
	//	author_label->setText( "Sandy Meier" );
	author_label->setAlignment( 289 );
	author_label->setMargin( -1 );

	version_label = new QLabel( this, "version_label" );
	version_label->setGeometry( 390, 130, 280, 30 );
	version_label->setMinimumSize( 0, 0 );
	version_label->setMaximumSize( 32767, 32767 );
	version_label->setFocusPolicy( QWidget::NoFocus );
	version_label->setBackgroundMode( QWidget::PaletteBackground );
	version_label->setFontPropagation( QWidget::NoChildren );
	version_label->setPalettePropagation( QWidget::NoChildren );
	//	version_label->setText( "v2.0" );
	version_label->setAlignment( 289 );
	version_label->setMargin( -1 );

	copyright_label = new QLabel( this, "copyright_label" );
	copyright_label->setGeometry( 390, 100, 280, 30 );
	copyright_label->setMinimumSize( 0, 0 );
	copyright_label->setMaximumSize( 32767, 32767 );
	copyright_label->setFocusPolicy( QWidget::NoFocus );
	copyright_label->setBackgroundMode( QWidget::PaletteBackground );
	copyright_label->setFontPropagation( QWidget::NoChildren );
	copyright_label->setPalettePropagation( QWidget::NoChildren );
	//	copyright_label->setText( "(c) 1999, GPL license" );
	copyright_label->setAlignment( 289 );
	copyright_label->setMargin( -1 );

	QLabel* qtarch_Label_11;
	qtarch_Label_11 = new QLabel( this, "Label_11" );
	qtarch_Label_11->setGeometry( 310, 70, 80, 30 );
	qtarch_Label_11->setMinimumSize( 0, 0 );
	qtarch_Label_11->setMaximumSize( 32767, 32767 );
	qtarch_Label_11->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_11->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_11->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_11->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_11->setText( "Homepage:" );
	qtarch_Label_11->setAlignment( 289 );
	qtarch_Label_11->setMargin( -1 );

	homepage_label = new QLabel( this, "homepage_label" );
	homepage_label->setGeometry( 390, 70, 280, 30 );
	homepage_label->setMinimumSize( 0, 0 );
	homepage_label->setMaximumSize( 32767, 32767 );
	homepage_label->setFocusPolicy( QWidget::NoFocus );
	homepage_label->setBackgroundMode( QWidget::PaletteBackground );
	homepage_label->setFontPropagation( QWidget::NoChildren );
	homepage_label->setPalettePropagation( QWidget::NoChildren );
	//	homepage_label->setText( "www.kdevelop.org" );
	homepage_label->setAlignment( 289 );
	homepage_label->setMargin( -1 );

	QLabel* qtarch_Label_13;
	qtarch_Label_13 = new QLabel( this, "Label_13" );
	qtarch_Label_13->setGeometry( 310, 40, 70, 30 );
	qtarch_Label_13->setMinimumSize( 0, 0 );
	qtarch_Label_13->setMaximumSize( 32767, 32767 );
	qtarch_Label_13->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_13->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_13->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_13->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_13->setText( "Email:" );
	qtarch_Label_13->setAlignment( 289 );
	qtarch_Label_13->setMargin( -1 );

	email_label = new QLabel( this, "email_label" );
	email_label->setGeometry( 390, 40, 280, 30 );
	email_label->setMinimumSize( 0, 0 );
	email_label->setMaximumSize( 32767, 32767 );
	email_label->setFocusPolicy( QWidget::NoFocus );
	email_label->setBackgroundMode( QWidget::PaletteBackground );
	email_label->setFontPropagation( QWidget::NoChildren );
	email_label->setPalettePropagation( QWidget::NoChildren );
	//	email_label->setText( "smeier@rz.uni-potsdam.de" );
	email_label->setAlignment( 289 );
	email_label->setMargin( -1 );

	QLabel* qtarch_Label_15;
	qtarch_Label_15 = new QLabel( this, "Label_15" );
	qtarch_Label_15->setGeometry( 310, 160, 100, 30 );
	qtarch_Label_15->setMinimumSize( 0, 0 );
	qtarch_Label_15->setMaximumSize( 32767, 32767 );
	qtarch_Label_15->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_15->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_15->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_15->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_15->setText( "Size:" );
	qtarch_Label_15->setAlignment( 289 );
	qtarch_Label_15->setMargin( -1 );

	size_label = new QLabel( this, "size_label" );
	size_label->setGeometry( 390, 160, 280, 30 );
	size_label->setMinimumSize( 0, 0 );
	size_label->setMaximumSize( 32767, 32767 );
	size_label->setFocusPolicy( QWidget::NoFocus );
	size_label->setBackgroundMode( QWidget::PaletteBackground );
	size_label->setFontPropagation( QWidget::NoChildren );
	size_label->setPalettePropagation( QWidget::NoChildren );
	//	size_label->setText( "343.333 bytes" );
	size_label->setAlignment( 289 );
	size_label->setMargin( -1 );

	desc_multilineedit = new QMultiLineEdit( this, "desc_multilineedit" );
	desc_multilineedit->setGeometry( 310, 230, 360, 140 );
	desc_multilineedit->setMinimumSize( 0, 0 );
	desc_multilineedit->setMaximumSize( 32767, 32767 );
	desc_multilineedit->setFocusPolicy( QWidget::StrongFocus );
	desc_multilineedit->setBackgroundMode( QWidget::PaletteBase );
	desc_multilineedit->setFontPropagation( QWidget::SameFont );
	desc_multilineedit->setPalettePropagation( QWidget::SameFont );
	desc_multilineedit->insertLine( "" );
	desc_multilineedit->setReadOnly( TRUE );
	desc_multilineedit->setOverwriteMode( FALSE );

	QLabel* qtarch_Label_17;
	qtarch_Label_17 = new QLabel( this, "Label_17" );
	qtarch_Label_17->setGeometry( 310, 190, 130, 30 );
	qtarch_Label_17->setMinimumSize( 0, 0 );
	qtarch_Label_17->setMaximumSize( 32767, 32767 );
	qtarch_Label_17->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_17->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_17->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_17->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_17->setText( "Description:" );
	qtarch_Label_17->setAlignment( 289 );
	qtarch_Label_17->setMargin( -1 );

	resize( 680,420 );
	setMinimumSize( 680, 420 );
	setMaximumSize( 680, 420 );

	setCaption( "Plugin Manager" );
	CheckListBoxItem* item;
	
	//item = plugin_listbox->insertItem( "Project Statistic (plugin example)", true );
	
	//	item = plugin_listbox->insertItem( "File Info (plugin example)", true );
	
	item = plugin_listbox->insertItem( "item3", false );
	
	item = plugin_listbox->insertItem( "item4", true );
}
PluginManagerDlg::~PluginManagerDlg(){
}
