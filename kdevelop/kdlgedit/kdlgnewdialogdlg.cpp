/***************************************************************************
                          kdlgnewdialogdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Mon Apr 12 1999                                           
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
#include <kfiledialog.h>
#include "../cproject.h"
#include "kdlgnewdialogdlg.h"

KDlgNewDialogDlg::KDlgNewDialogDlg(QWidget *parent, const char *name,CProject* prj ) : QDialog(parent,name,true) {
  project = prj;

  QButtonGroup* qtarch_ButtonGroup_3;
  qtarch_ButtonGroup_3 = new QButtonGroup( this, "ButtonGroup_3" );
	qtarch_ButtonGroup_3->setGeometry( 250, 210, 290, 160 );
	qtarch_ButtonGroup_3->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_3->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_3->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_3->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_3->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_3->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_3->setFrameStyle( 49 );
	qtarch_ButtonGroup_3->setTitle( "Location:" );
	qtarch_ButtonGroup_3->setAlignment( 1 );

	custom_prob_gbox = new QButtonGroup( this, "custom_prob_gbox" );
	custom_prob_gbox->setGeometry( 10, 210, 220, 160 );
	custom_prob_gbox->setMinimumSize( 0, 0 );
	custom_prob_gbox->setMaximumSize( 32767, 32767 );
	custom_prob_gbox->setFocusPolicy( QWidget::NoFocus );
	custom_prob_gbox->setBackgroundMode( QWidget::PaletteBackground );
	custom_prob_gbox->setFontPropagation( QWidget::NoChildren );
	custom_prob_gbox->setPalettePropagation( QWidget::NoChildren );
	custom_prob_gbox->setFrameStyle( 49 );
	custom_prob_gbox->setTitle( "Custom Properties:" );
	custom_prob_gbox->setAlignment( 1 );

	QButtonGroup* qtarch_ButtonGroup_2;
	qtarch_ButtonGroup_2 = new QButtonGroup( this, "ButtonGroup_2" );
	qtarch_ButtonGroup_2->setGeometry( 250, 10, 290, 190 );
	qtarch_ButtonGroup_2->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_2->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_2->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_2->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_2->setFrameStyle( 49 );
	qtarch_ButtonGroup_2->setTitle( "Files:" );
	qtarch_ButtonGroup_2->setAlignment( 1 );

	QButtonGroup* qtarch_ButtonGroup_1;
	qtarch_ButtonGroup_1 = new QButtonGroup( this, "ButtonGroup_1" );
	qtarch_ButtonGroup_1->setGeometry( 10, 10, 220, 190 );
	qtarch_ButtonGroup_1->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_1->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_1->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_1->setFrameStyle( 49 );
	qtarch_ButtonGroup_1->setTitle( "Dialog Class" );
	qtarch_ButtonGroup_1->setAlignment( 1 );

	qdialog_radio_button = new QRadioButton( this, "qdialog_radio_button" );
	qdialog_radio_button->setGeometry( 110, 30, 100, 30 );
	qdialog_radio_button->setMinimumSize( 0, 0 );
	qdialog_radio_button->setMaximumSize( 32767, 32767 );
	qdialog_radio_button->setFocusPolicy( QWidget::TabFocus );
	qdialog_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	qdialog_radio_button->setFontPropagation( QWidget::NoChildren );
	qdialog_radio_button->setPalettePropagation( QWidget::NoChildren );
	qdialog_radio_button->setText( "QDialog" );
	qdialog_radio_button->setAutoRepeat( FALSE );
	qdialog_radio_button->setAutoResize( FALSE );
	qdialog_radio_button->setChecked( TRUE );

	qwidget_radio_button = new QRadioButton( this, "qwidget_radio_button" );
	qwidget_radio_button->setGeometry( 20, 30, 80, 30 );
	qwidget_radio_button->setMinimumSize( 0, 0 );
	qwidget_radio_button->setMaximumSize( 32767, 32767 );
	qwidget_radio_button->setFocusPolicy( QWidget::TabFocus );
	qwidget_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	qwidget_radio_button->setFontPropagation( QWidget::NoChildren );
	qwidget_radio_button->setPalettePropagation( QWidget::NoChildren );
	qwidget_radio_button->setText( "QWidget" );
	qwidget_radio_button->setAutoRepeat( FALSE );
	qwidget_radio_button->setAutoResize( FALSE );

	qtabdialog_radio_button = new QRadioButton( this, "qtabdialog_radio_button" );
	qtabdialog_radio_button->setGeometry( 110, 70, 100, 30 );
	qtabdialog_radio_button->setMinimumSize( 0, 0 );
	qtabdialog_radio_button->setMaximumSize( 32767, 32767 );
	qtabdialog_radio_button->setFocusPolicy( QWidget::TabFocus );
	qtabdialog_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	qtabdialog_radio_button->setFontPropagation( QWidget::NoChildren );
	qtabdialog_radio_button->setPalettePropagation( QWidget::NoChildren );
	qtabdialog_radio_button->setText( "QTabDialog" );
	qtabdialog_radio_button->setAutoRepeat( FALSE );
	qtabdialog_radio_button->setAutoResize( FALSE );

	qframe_radio_button = new QRadioButton( this, "qframe_radio_button" );
	qframe_radio_button->setGeometry( 20, 70, 70, 30 );
	qframe_radio_button->setMinimumSize( 0, 0 );
	qframe_radio_button->setMaximumSize( 32767, 32767 );
	qframe_radio_button->setFocusPolicy( QWidget::TabFocus );
	qframe_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	qframe_radio_button->setFontPropagation( QWidget::NoChildren );
	qframe_radio_button->setPalettePropagation( QWidget::NoChildren );
	qframe_radio_button->setText( "QFrame" );
	qframe_radio_button->setAutoRepeat( FALSE );
	qframe_radio_button->setAutoResize( FALSE );

	ok_button = new QPushButton( this, "ok_button" );
	ok_button->setGeometry( 150, 400, 100, 30 );
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
	cancel_button->setGeometry( 300, 400, 100, 30 );
	cancel_button->setMinimumSize( 0, 0 );
	cancel_button->setMaximumSize( 32767, 32767 );
	cancel_button->setFocusPolicy( QWidget::TabFocus );
       cancel_button->setBackgroundMode( QWidget::PaletteBackground );
	cancel_button->setFontPropagation( QWidget::NoChildren );
	cancel_button->setPalettePropagation( QWidget::NoChildren );
	cancel_button->setText( "Cancel" );
       cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );

	data_edit = new QLineEdit( this, "data_edit" );
	data_edit->setGeometry( 390, 150, 140, 30 );
	data_edit->setMinimumSize( 0, 0 );
	data_edit->setMaximumSize( 32767, 32767 );
	data_edit->setFocusPolicy( QWidget::StrongFocus );
	data_edit->setBackgroundMode( QWidget::PaletteBase );
	data_edit->setFontPropagation( QWidget::NoChildren );
	data_edit->setPalettePropagation( QWidget::NoChildren );
	data_edit->setText( "" );
	data_edit->setMaxLength( 32767 );
	data_edit->setEchoMode( QLineEdit::Normal );
	data_edit->setFrame( TRUE );


	custom_header_edit = new QLineEdit( this, "custom_header_edit" );
	custom_header_edit->setGeometry( 20, 330, 200, 30 );
	custom_header_edit->setMinimumSize( 0, 0 );
	custom_header_edit->setMaximumSize( 32767, 32767 );
	custom_header_edit->setFocusPolicy( QWidget::StrongFocus );
	custom_header_edit->setBackgroundMode( QWidget::PaletteBase );
	custom_header_edit->setFontPropagation( QWidget::NoChildren );
	custom_header_edit->setPalettePropagation( QWidget::NoChildren );
	custom_header_edit->setText( "" );
	custom_header_edit->setMaxLength( 32767 );
	custom_header_edit->setEchoMode( QLineEdit::Normal );
	custom_header_edit->setFrame( TRUE );

	cpp_edit = new QLineEdit( this, "cpp_edit" );
	cpp_edit->setGeometry( 390, 110, 140, 30 );
	cpp_edit->setMinimumSize( 0, 0 );
	cpp_edit->setMaximumSize( 32767, 32767 );
	cpp_edit->setFocusPolicy( QWidget::StrongFocus );
	cpp_edit->setBackgroundMode( QWidget::PaletteBase );
	cpp_edit->setFontPropagation( QWidget::NoChildren );
	cpp_edit->setPalettePropagation( QWidget::NoChildren );
	cpp_edit->setText( "" );
	cpp_edit->setMaxLength( 32767 );
	cpp_edit->setEchoMode( QLineEdit::Normal );
	cpp_edit->setFrame( TRUE );

	loc_button = new QPushButton( this, "loc_button" );
	loc_button->setGeometry( 500, 290, 30, 30 );
	loc_button->setMinimumSize( 0, 0 );
	loc_button->setMaximumSize( 32767, 32767 );
	loc_button->setFocusPolicy( QWidget::TabFocus );
	loc_button->setBackgroundMode( QWidget::PaletteBackground );
	loc_button->setFontPropagation( QWidget::NoChildren );
	loc_button->setPalettePropagation( QWidget::NoChildren );
	loc_button->setText( "..." );
	loc_button->setAutoRepeat( FALSE );
	loc_button->setAutoResize( FALSE );

	header_edit = new QLineEdit( this, "header_edit" );
	header_edit->setGeometry( 390, 70, 140, 30 );
	header_edit->setMinimumSize( 0, 0 );
	header_edit->setMaximumSize( 32767, 32767 );
	header_edit->setFocusPolicy( QWidget::StrongFocus );
	header_edit->setBackgroundMode( QWidget::PaletteBase );
	header_edit->setFontPropagation( QWidget::NoChildren );
	header_edit->setPalettePropagation( QWidget::NoChildren );
	header_edit->setText( "" );
	header_edit->setMaxLength( 32767 );
	header_edit->setEchoMode( QLineEdit::Normal );
	header_edit->setFrame( TRUE );

	custom_header_label = new QLabel( this, "custom_header_label" );
	custom_header_label->setGeometry( 20, 300, 100, 30 );
	custom_header_label->setMinimumSize( 0, 0 );
	custom_header_label->setMaximumSize( 32767, 32767 );
	custom_header_label->setFocusPolicy( QWidget::NoFocus );
	custom_header_label->setBackgroundMode( QWidget::PaletteBackground );
	custom_header_label->setFontPropagation( QWidget::NoChildren );
	custom_header_label->setPalettePropagation( QWidget::NoChildren );
	custom_header_label->setText( "Custom Header:" );
	custom_header_label->setAlignment( 289 );
	custom_header_label->setMargin( -1 );

	custom_radio_button = new QRadioButton( this, "custom_radio_button" );
	custom_radio_button->setGeometry( 20, 110, 100, 30 );
	custom_radio_button->setMinimumSize( 0, 0 );
	custom_radio_button->setMaximumSize( 32767, 32767 );
	custom_radio_button->setFocusPolicy( QWidget::TabFocus );
	custom_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	custom_radio_button->setFontPropagation( QWidget::NoChildren );
	custom_radio_button->setPalettePropagation( QWidget::NoChildren );
	custom_radio_button->setText( "Custom" );
	custom_radio_button->setAutoRepeat( FALSE );
	custom_radio_button->setAutoResize( FALSE );

	custom_class_label = new QLabel( this, "custom_class_label" );
	custom_class_label->setGeometry( 20, 230, 100, 30 );
	custom_class_label->setMinimumSize( 0, 0 );
	custom_class_label->setMaximumSize( 32767, 32767 );
	custom_class_label->setFocusPolicy( QWidget::NoFocus );
	custom_class_label->setBackgroundMode( QWidget::PaletteBackground );
	custom_class_label->setFontPropagation( QWidget::NoChildren );
	custom_class_label->setPalettePropagation( QWidget::NoChildren );
	custom_class_label->setText( "Custom Class:" );
	custom_class_label->setAlignment( 289 );
	custom_class_label->setMargin( -1 );

	QLabel* qtarch_Label_3;
	qtarch_Label_3 = new QLabel( this, "Label_3" );
	qtarch_Label_3->setGeometry( 260, 70, 130, 30 );
	qtarch_Label_3->setMinimumSize( 0, 0 );
	qtarch_Label_3->setMaximumSize( 32767, 32767 );
	qtarch_Label_3->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_3->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_3->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_3->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_3->setText( "Header File:" );
	qtarch_Label_3->setAlignment( 289 );
	qtarch_Label_3->setMargin( -1 );

	custom_class_edit = new QLineEdit( this, "custom_class_edit" );
	custom_class_edit->setGeometry( 20, 260, 200, 30 );
	custom_class_edit->setMinimumSize( 0, 0 );
	custom_class_edit->setMaximumSize( 32767, 32767 );
	custom_class_edit->setFocusPolicy( QWidget::StrongFocus );
	custom_class_edit->setBackgroundMode( QWidget::PaletteBase );
	custom_class_edit->setFontPropagation( QWidget::NoChildren );
	custom_class_edit->setPalettePropagation( QWidget::NoChildren );
	custom_class_edit->setText( "" );
	custom_class_edit->setMaxLength( 32767 );
	custom_class_edit->setEchoMode( QLineEdit::Normal );
	custom_class_edit->setFrame( TRUE );

	QLabel* qtarch_Label_4;
	qtarch_Label_4 = new QLabel( this, "Label_4" );
	qtarch_Label_4->setGeometry( 260, 110, 130, 30 );
	qtarch_Label_4->setMinimumSize( 0, 0 );
	qtarch_Label_4->setMaximumSize( 32767, 32767 );
	qtarch_Label_4->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_4->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_4->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_4->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_4->setText( "C++ File:" );
	qtarch_Label_4->setAlignment( 289 );
	qtarch_Label_4->setMargin( -1 );

	QLabel* qtarch_Label_5;
	qtarch_Label_5 = new QLabel( this, "Label_5" );
	qtarch_Label_5->setGeometry( 260, 150, 130, 30 );
	qtarch_Label_5->setMinimumSize( 0, 0 );
	qtarch_Label_5->setMaximumSize( 32767, 32767 );
	qtarch_Label_5->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_5->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_5->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_5->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_5->setText( "Data File:" );
	qtarch_Label_5->setAlignment( 289 );
	qtarch_Label_5->setMargin( -1 );

	loc_edit = new QLineEdit( this, "loc_edit" );
	loc_edit->setGeometry( 260, 290, 230, 30 );
	loc_edit->setMinimumSize( 0, 0 );
	loc_edit->setMaximumSize( 32767, 32767 );
	loc_edit->setFocusPolicy( QWidget::StrongFocus );
	loc_edit->setBackgroundMode( QWidget::PaletteBase );
	loc_edit->setFontPropagation( QWidget::NoChildren );
	loc_edit->setPalettePropagation( QWidget::NoChildren );
	loc_edit->setText( "" );
	loc_edit->setMaxLength( 32767 );
	loc_edit->setEchoMode( QLineEdit::Normal );
	loc_edit->setFrame( TRUE );

	QLabel* qtarch_Label_6;
	qtarch_Label_6 = new QLabel( this, "Label_6" );
	qtarch_Label_6->setGeometry( 260, 260, 100, 30 );
	qtarch_Label_6->setMinimumSize( 0, 0 );
	qtarch_Label_6->setMaximumSize( 32767, 32767 );
	qtarch_Label_6->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_6->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_6->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_6->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_6->setText( "Directory:" );
	qtarch_Label_6->setAlignment( 289 );
	qtarch_Label_6->setMargin( -1 );

	QLabel* qtarch_Label_7;
	qtarch_Label_7 = new QLabel( this, "Label_7" );
	qtarch_Label_7->setGeometry( 260, 30, 100, 30 );
	qtarch_Label_7->setMinimumSize( 0, 0 );
	qtarch_Label_7->setMaximumSize( 32767, 32767 );
	qtarch_Label_7->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_7->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_7->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_7->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_7->setText( "Classname:" );
	qtarch_Label_7->setAlignment( 289 );
	qtarch_Label_7->setMargin( -1 );

	classname_edit = new QLineEdit( this, "classname_edit" );
	classname_edit->setGeometry( 390, 30, 140, 30 );
	classname_edit->setMinimumSize( 0, 0 );
	classname_edit->setMaximumSize( 32767, 32767 );
	classname_edit->setFocusPolicy( QWidget::StrongFocus );
	classname_edit->setBackgroundMode( QWidget::PaletteBase );
	classname_edit->setFontPropagation( QWidget::NoChildren );
	classname_edit->setPalettePropagation( QWidget::NoChildren );
	classname_edit->setText( "" );
       classname_edit->setMaxLength( 32767 );
	classname_edit->setEchoMode( QLineEdit::Normal );
	classname_edit->setFrame( TRUE );

	qtarch_ButtonGroup_3->insert( loc_button );



	qtarch_ButtonGroup_1->insert( qdialog_radio_button );
	qtarch_ButtonGroup_1->insert( qwidget_radio_button );
	qtarch_ButtonGroup_1->insert( qtabdialog_radio_button );
	qtarch_ButtonGroup_1->insert( qframe_radio_button );
	qtarch_ButtonGroup_1->insert( custom_radio_button );

	resize( 550,450 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
	
	//set custom disabled
	setCustomPropsEnabled(false);

	// other settings
	loc_edit->setText(prj->getProjectDir() + prj->getSubDir());
	setCaption("New Dialog...");

	header_modified = false;
	source_modified = false;
	data_modified = false;
	
	// connections
	connect(classname_edit,SIGNAL(textChanged(const char*)),SLOT(slotClassEditChanged(const char*)));
	connect(header_edit,SIGNAL(textChanged(const char*)),SLOT(slotHeaderEditChanged(const char*)));
	connect(cpp_edit,SIGNAL(textChanged(const char*)),SLOT(slotSourceEditChanged(const char*)));
	connect(data_edit,SIGNAL(textChanged(const char*)),SLOT(slotDataEditChanged(const char*)));
	
	connect(ok_button,SIGNAL(clicked()),SLOT(slotOKClicked()));
	connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
	connect(loc_button,SIGNAL(clicked()),SLOT(slotLocButtonClicked()));
	
	connect(qdialog_radio_button,SIGNAL(clicked()),SLOT(slotRadioButtonClicked()));
	connect(qwidget_radio_button,SIGNAL(clicked()),SLOT(slotRadioButtonClicked()));
	connect(qtabdialog_radio_button,SIGNAL(clicked()),SLOT(slotRadioButtonClicked()));
	connect(qframe_radio_button,SIGNAL(clicked()),SLOT(slotRadioButtonClicked()));
	connect(custom_radio_button,SIGNAL(clicked()),SLOT(slotRadioButtonClicked()));
}
KDlgNewDialogDlg::~KDlgNewDialogDlg(){
}

void KDlgNewDialogDlg::slotOKClicked(){
  accept();
}
void KDlgNewDialogDlg::slotLocButtonClicked(){
  QString str=  KDirDialog::getDirectory(loc_edit->text(),this,"test");
  if(!str.isEmpty()){
    loc_edit->setText(str);
  }
}
void KDlgNewDialogDlg::slotRadioButtonClicked(){
  if(custom_radio_button->isChecked()){
    setCustomPropsEnabled(true);
  }
  else{
    setCustomPropsEnabled(false);
  }
}

void KDlgNewDialogDlg::setCustomPropsEnabled(bool show){
  if(show){
    custom_header_label->setEnabled(true);
    custom_class_label->setEnabled(true);
    custom_header_edit->setEnabled(true);
    custom_class_edit->setEnabled(true);
    custom_prob_gbox->setEnabled(true);
  }
  else{
    custom_header_label->setEnabled(false);
    custom_class_label->setEnabled(false);
    custom_header_edit->setEnabled(false);
    custom_class_edit->setEnabled(false);
    custom_prob_gbox->setEnabled(false);
  }
}
void  KDlgNewDialogDlg::slotClassEditChanged(const char* text){
  QString str = text;
  if(!header_modified){
    header_edit->setText(str.lower() + ".h");
  }
  if(!source_modified){
    cpp_edit->setText(str.lower() +".cpp");
  }
  if(!data_modified){
    data_edit->setText(str.lower() +"data.cpp");
  }
  
}
void KDlgNewDialogDlg::slotHeaderEditChanged(const char*){
  if(header_edit->hasFocus()){
    header_modified = true;
  }
}
void KDlgNewDialogDlg::slotSourceEditChanged(const char*){
  if(cpp_edit->hasFocus()){
    source_modified = true;
  }
}
void KDlgNewDialogDlg::slotDataEditChanged(const char*){
  if(data_edit->hasFocus()){
    data_modified = true;
  }
}
QString KDlgNewDialogDlg::getBaseClass(){
  if(qwidget_radio_button->isChecked()) return "QWidget";
  if(qframe_radio_button->isChecked()) return "QFrame";
  if(qdialog_radio_button->isChecked()) return "QDialog";
  if(qtabdialog_radio_button->isChecked()) return "QTabDialog";
  if(custom_radio_button->isChecked()) return custom_class_edit->text();
  return "QDialog";
}
