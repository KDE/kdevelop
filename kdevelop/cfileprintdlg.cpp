/***************************************************************************
                          cfileprintdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Feb 12 1999                                           
    copyright            : (C) 1999 by Stefan Heidrich                         
    email                : sheidric@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "cfileprintdlg.h"
#include <qlabel.h>
#include <qbttngrp.h>
#include <iostream.h>

CFilePrintDlg::CFilePrintDlg(QWidget* parent,const char* name) : QDialog(parent, name, true){
  init();
  slotSelfChoosenFilesActive(1);
  selfChoosenFilesMultiLine->setEnabled(false);
}

CFilePrintDlg::~CFilePrintDlg(){
}

void CFilePrintDlg::init() {
  QWidget *mainwidget = new QWidget(this,"filedialog");
  mainwidget->resize (600,480);
  QWidget *filewidget = new QWidget(this,"files");
  filewidget->resize(600,430);
	qtarch_ButtonGroup_143 = new QButtonGroup( this, "ButtonGroup_143" );
	qtarch_ButtonGroup_143->setGeometry( 350, 10, 230, 410 );
	qtarch_ButtonGroup_143->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_143->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_143->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_143->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_143->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_143->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_143->setFrameStyle( 49 );
	qtarch_ButtonGroup_143->setTitle( "Self choosen files" );
	qtarch_ButtonGroup_143->setAlignment( 1 );

	qtarch_ButtonGroup_140 = new QButtonGroup( this, "ButtonGroup_140" );
	qtarch_ButtonGroup_140->setGeometry( 20, 180, 320, 240 );
	qtarch_ButtonGroup_140->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_140->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_140->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_140->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_140->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_140->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_140->setFrameStyle( 49 );
	qtarch_ButtonGroup_140->setTitle( "Changed files" );
	qtarch_ButtonGroup_140->setAlignment( 1 );

	qtarch_ButtonGroup_141 = new QButtonGroup( this, "ButtonGroup_141" );
	qtarch_ButtonGroup_141->setGeometry( 30, 310, 300, 100 );
	qtarch_ButtonGroup_141->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_141->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_141->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_141->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_141->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_141->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_141->setFrameStyle( 49 );
	qtarch_ButtonGroup_141->setTitle( "Time" );
	qtarch_ButtonGroup_141->setAlignment( 1 );

	qtarch_ButtonGroup_142 = new QButtonGroup( this, "ButtonGroup_142" );
	qtarch_ButtonGroup_142->setGeometry( 30, 200, 300, 100 );
	qtarch_ButtonGroup_142->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_142->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_142->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_142->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_142->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_142->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_142->setFrameStyle( 49 );
	qtarch_ButtonGroup_142->setTitle( "Date" );
	qtarch_ButtonGroup_142->setAlignment( 1 );

	QButtonGroup* qtarch_ButtonGroup_139;
	qtarch_ButtonGroup_139 = new QButtonGroup( this, "ButtonGroup_139" );
	qtarch_ButtonGroup_139->setGeometry( 20, 10, 320, 160 );
	qtarch_ButtonGroup_139->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_139->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_139->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_139->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_139->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_139->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_139->setFrameStyle( 49 );
	qtarch_ButtonGroup_139->setTitle( "File selection" );
	qtarch_ButtonGroup_139->setAlignment( 1 );

	selfChoosenFilesMultiLine = new QListBox( this, "selfChoosenFilesMultiLine" );
	selfChoosenFilesMultiLine->setGeometry( 360, 30, 210, 340 );
	selfChoosenFilesMultiLine->setMinimumSize( 0, 0 );
	selfChoosenFilesMultiLine->setMaximumSize( 32767, 32767 );
	selfChoosenFilesMultiLine->setFocusPolicy( QWidget::StrongFocus );
	selfChoosenFilesMultiLine->setBackgroundMode( QWidget::PaletteBase );
	selfChoosenFilesMultiLine->setFontPropagation( QWidget::SameFont );
	selfChoosenFilesMultiLine->setPalettePropagation( QWidget::SameFont );
	selfChoosenFilesMultiLine->setFrameStyle( 51 );
	selfChoosenFilesMultiLine->setLineWidth( 2 );
	selfChoosenFilesMultiLine->setMultiSelection( TRUE );

	selfChoosenFilesCleanButton = new QPushButton( this, "selfChoosenFilesCleanButton" );
	selfChoosenFilesCleanButton->setGeometry( 500, 380, 70, 30 );
	selfChoosenFilesCleanButton->setMinimumSize( 0, 0 );
	selfChoosenFilesCleanButton->setMaximumSize( 32767, 32767 );
	selfChoosenFilesCleanButton->setFocusPolicy( QWidget::TabFocus );
	selfChoosenFilesCleanButton->setBackgroundMode( QWidget::PaletteBackground );
	selfChoosenFilesCleanButton->setFontPropagation( QWidget::NoChildren );
	selfChoosenFilesCleanButton->setPalettePropagation( QWidget::NoChildren );
	selfChoosenFilesCleanButton->setText( "clean" );
	selfChoosenFilesCleanButton->setAutoRepeat( FALSE );
	selfChoosenFilesCleanButton->setAutoResize( FALSE );
	connect (selfChoosenFilesCleanButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileCleanClicked()));

	selfChoosenFilesDeleteButton = new QPushButton( this, "selfChoosenFilesDeleteButton" );
	selfChoosenFilesDeleteButton->setGeometry( 430, 380, 70, 30 );
	selfChoosenFilesDeleteButton->setMinimumSize( 0, 0 );
	selfChoosenFilesDeleteButton->setMaximumSize( 32767, 32767 );
	selfChoosenFilesDeleteButton->setFocusPolicy( QWidget::TabFocus );
	selfChoosenFilesDeleteButton->setBackgroundMode( QWidget::PaletteBackground );
	selfChoosenFilesDeleteButton->setFontPropagation( QWidget::NoChildren );
	selfChoosenFilesDeleteButton->setPalettePropagation( QWidget::NoChildren );
	selfChoosenFilesDeleteButton->setText( "delete" );
	selfChoosenFilesDeleteButton->setAutoRepeat( FALSE );
	selfChoosenFilesDeleteButton->setAutoResize( FALSE );
	connect (selfChoosenFilesDeleteButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileDeleteClicked()));

	selfChoosenFilesAddButton = new QPushButton( this, "selfChoosenFilesAddButton" );
	selfChoosenFilesAddButton->setGeometry( 360, 380, 70, 30 );
	selfChoosenFilesAddButton->setMinimumSize( 0, 0 );
	selfChoosenFilesAddButton->setMaximumSize( 32767, 32767 );
	selfChoosenFilesAddButton->setFocusPolicy( QWidget::TabFocus );
	selfChoosenFilesAddButton->setBackgroundMode( QWidget::PaletteBackground );
	selfChoosenFilesAddButton->setFontPropagation( QWidget::NoChildren );
	selfChoosenFilesAddButton->setPalettePropagation( QWidget::NoChildren );
	selfChoosenFilesAddButton->setText( "add" );
	selfChoosenFilesAddButton->setAutoRepeat( FALSE );
	selfChoosenFilesAddButton->setAutoResize( FALSE );
	connect (selfChoosenFilesAddButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileAddClicked()));

	qtarch_Label_77 = new QLabel( this, "Label_77" );
	qtarch_Label_77->setGeometry( 40, 260, 70, 30 );
	qtarch_Label_77->setMinimumSize( 0, 0 );
	qtarch_Label_77->setMaximumSize( 32767, 32767 );
	qtarch_Label_77->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_77->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_77->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_77->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_77->setText( "between" );
	qtarch_Label_77->setAlignment( 289 );
	qtarch_Label_77->setMargin( -1 );

	qtarch_Label_78 = new QLabel( this, "Label_78" );
	qtarch_Label_78->setGeometry( 40, 220, 70, 30 );
	qtarch_Label_78->setMinimumSize( 0, 0 );
	qtarch_Label_78->setMaximumSize( 32767, 32767 );
	qtarch_Label_78->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_78->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_78->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_78->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_78->setText( "since" );
	qtarch_Label_78->setAlignment( 289 );
	qtarch_Label_78->setMargin( -1 );

	selfChoosenFileLine = new QLineEdit( this, "selfChoosenFileLine" );
	selfChoosenFileLine->setGeometry( 100, 130, 190, 30 );
	selfChoosenFileLine->setMinimumSize( 0, 0 );
	selfChoosenFileLine->setMaximumSize( 32767, 32767 );
	selfChoosenFileLine->setFocusPolicy( QWidget::StrongFocus );
	selfChoosenFileLine->setBackgroundMode( QWidget::PaletteBase );
	selfChoosenFileLine->setFontPropagation( QWidget::NoChildren );
	selfChoosenFileLine->setPalettePropagation( QWidget::NoChildren );
	selfChoosenFileLine->setText( "" );
	selfChoosenFileLine->setMaxLength( 32767 );
	selfChoosenFileLine->setEchoMode( QLineEdit::Normal );
	selfChoosenFileLine->setFrame( TRUE );

	selfChoosenFilesPushButton = new QPushButton( this, "selfChoosenFilesPushButton" );
	selfChoosenFilesPushButton->setGeometry( 300, 130, 30, 30 );
	selfChoosenFilesPushButton->setMinimumSize( 0, 0 );
	selfChoosenFilesPushButton->setMaximumSize( 32767, 32767 );
	selfChoosenFilesPushButton->setFocusPolicy( QWidget::TabFocus );
	selfChoosenFilesPushButton->setBackgroundMode( QWidget::PaletteBackground );
	selfChoosenFilesPushButton->setFontPropagation( QWidget::NoChildren );
	selfChoosenFilesPushButton->setPalettePropagation( QWidget::NoChildren );
	selfChoosenFilesPushButton->setText( "..." );
	selfChoosenFilesPushButton->setAutoRepeat( FALSE );
	selfChoosenFilesPushButton->setAutoResize( FALSE );
	connect(selfChoosenFilesPushButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileButtonClicked()));

	qtarch_Label_79 = new QLabel( this, "Label_79" );
	qtarch_Label_79->setGeometry( 50, 130, 30, 30 );
	qtarch_Label_79->setMinimumSize( 0, 0 );
	qtarch_Label_79->setMaximumSize( 32767, 32767 );
	qtarch_Label_79->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_79->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_79->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_79->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_79->setText( "file" );
	qtarch_Label_79->setAlignment( 289 );
	qtarch_Label_79->setMargin( -1 );

	currentButton = new QRadioButton( this, "currentButton" );
	currentButton->setGeometry( 30, 30, 140, 30 );
	currentButton->setMinimumSize( 0, 0 );
	currentButton->setMaximumSize( 32767, 32767 );
	currentButton->setFocusPolicy( QWidget::TabFocus );
	currentButton->setBackgroundMode( QWidget::PaletteBackground );
	currentButton->setFontPropagation( QWidget::NoChildren );
	currentButton->setPalettePropagation( QWidget::NoChildren );
	currentButton->setText( "current" );
	currentButton->setAutoRepeat( FALSE );
	currentButton->setAutoResize( FALSE );

	allInBufferButton = new QRadioButton( this, "allInBufferButton" );
	allInBufferButton->setGeometry( 180, 30, 140, 30 );
	allInBufferButton->setMinimumSize( 0, 0 );
	allInBufferButton->setMaximumSize( 32767, 32767 );
	allInBufferButton->setFocusPolicy( QWidget::TabFocus );
	allInBufferButton->setBackgroundMode( QWidget::PaletteBackground );
	allInBufferButton->setFontPropagation( QWidget::NoChildren );
	allInBufferButton->setPalettePropagation( QWidget::NoChildren );
	allInBufferButton->setText( "all in buffer" );
	allInBufferButton->setAutoRepeat( FALSE );
	allInBufferButton->setAutoResize( FALSE );

	allInProjectButton = new QRadioButton( this, "allInProjectButton" );
	allInProjectButton->setGeometry( 30, 60, 140, 30 );
	allInProjectButton->setMinimumSize( 0, 0 );
	allInProjectButton->setMaximumSize( 32767, 32767 );
	allInProjectButton->setFocusPolicy( QWidget::TabFocus );
	allInProjectButton->setBackgroundMode( QWidget::PaletteBackground );
	allInProjectButton->setFontPropagation( QWidget::NoChildren );
	allInProjectButton->setPalettePropagation( QWidget::NoChildren );
	allInProjectButton->setText( "all in project" );
	allInProjectButton->setAutoRepeat( FALSE );
	allInProjectButton->setAutoResize( FALSE );

	selfChoosenFilesButton = new QRadioButton( this, "selfChoosenFilesButton" );
	selfChoosenFilesButton->setGeometry( 30, 100, 290, 30 );
	selfChoosenFilesButton->setMinimumSize( 0, 0 );
	selfChoosenFilesButton->setMaximumSize( 32767, 32767 );
	selfChoosenFilesButton->setFocusPolicy( QWidget::TabFocus );
	selfChoosenFilesButton->setBackgroundMode( QWidget::PaletteBackground );
	selfChoosenFilesButton->setFontPropagation( QWidget::NoChildren );
	selfChoosenFilesButton->setPalettePropagation( QWidget::NoChildren );
	selfChoosenFilesButton->setText( "self choosen files" );
	selfChoosenFilesButton->setAutoRepeat( FALSE );
	selfChoosenFilesButton->setAutoResize( FALSE );

	changedFilesButton = new QRadioButton( this, "changedFilesButton" );
	changedFilesButton->setGeometry( 180, 60, 140, 30 );
	changedFilesButton->setMinimumSize( 0, 0 );
	changedFilesButton->setMaximumSize( 32767, 32767 );
	changedFilesButton->setFocusPolicy( QWidget::TabFocus );
	changedFilesButton->setBackgroundMode( QWidget::PaletteBackground );
	changedFilesButton->setFontPropagation( QWidget::NoChildren );
	changedFilesButton->setPalettePropagation( QWidget::NoChildren );
	changedFilesButton->setText( "changed files" );
	changedFilesButton->setAutoRepeat( FALSE );
	changedFilesButton->setAutoResize( FALSE );

	dateBetweenLine = new QLineEdit( this, "dateBetweenLine" );
	dateBetweenLine->setGeometry( 120, 260, 80, 30 );
	dateBetweenLine->setMinimumSize( 0, 0 );
	dateBetweenLine->setMaximumSize( 32767, 32767 );
	dateBetweenLine->setFocusPolicy( QWidget::StrongFocus );
	dateBetweenLine->setBackgroundMode( QWidget::PaletteBase );
	dateBetweenLine->setFontPropagation( QWidget::NoChildren );
	dateBetweenLine->setPalettePropagation( QWidget::NoChildren );
	dateBetweenLine->setText( "" );
	dateBetweenLine->setMaxLength( 32767 );
	dateBetweenLine->setEchoMode( QLineEdit::Normal );
	dateBetweenLine->setFrame( TRUE );

	qtarch_Label_80 = new QLabel( this, "Label_80" );
	qtarch_Label_80->setGeometry( 210, 260, 30, 30 );
	qtarch_Label_80->setMinimumSize( 0, 0 );
	qtarch_Label_80->setMaximumSize( 32767, 32767 );
	qtarch_Label_80->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_80->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_80->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_80->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_80->setText( "and" );
	qtarch_Label_80->setAlignment( 289 );
	qtarch_Label_80->setMargin( -1 );

	qtarch_Label_81 = new QLabel( this, "Label_81" );
	qtarch_Label_81->setGeometry( 40, 330, 70, 30 );
	qtarch_Label_81->setMinimumSize( 0, 0 );
	qtarch_Label_81->setMaximumSize( 32767, 32767 );
	qtarch_Label_81->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_81->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_81->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_81->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_81->setText( "since" );
	qtarch_Label_81->setAlignment( 289 );
	qtarch_Label_81->setMargin( -1 );

	dateAndLine = new QLineEdit( this, "dateAndLine" );
	dateAndLine->setGeometry( 240, 260, 80, 30 );
	dateAndLine->setMinimumSize( 0, 0 );
	dateAndLine->setMaximumSize( 32767, 32767 );
	dateAndLine->setFocusPolicy( QWidget::StrongFocus );
	dateAndLine->setBackgroundMode( QWidget::PaletteBase );
	dateAndLine->setFontPropagation( QWidget::NoChildren );
	dateAndLine->setPalettePropagation( QWidget::NoChildren );
	dateAndLine->setText( "" );
	dateAndLine->setMaxLength( 32767 );
	dateAndLine->setEchoMode( QLineEdit::Normal );
	dateAndLine->setFrame( TRUE );

	qtarch_Label_83 = new QLabel( this, "Label_83" );
	qtarch_Label_83->setGeometry( 40, 370, 70, 30 );
	qtarch_Label_83->setMinimumSize( 0, 0 );
	qtarch_Label_83->setMaximumSize( 32767, 32767 );
	qtarch_Label_83->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_83->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_83->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_83->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_83->setText( "between" );
	qtarch_Label_83->setAlignment( 289 );
	qtarch_Label_83->setMargin( -1 );

	timeAndLine = new QLineEdit( this, "timeAndLine" );
	timeAndLine->setGeometry( 240, 370, 80, 30 );
	timeAndLine->setMinimumSize( 0, 0 );
	timeAndLine->setMaximumSize( 32767, 32767 );
	timeAndLine->setFocusPolicy( QWidget::StrongFocus );
	timeAndLine->setBackgroundMode( QWidget::PaletteBase );
	timeAndLine->setFontPropagation( QWidget::NoChildren );
	timeAndLine->setPalettePropagation( QWidget::NoChildren );
	timeAndLine->setText( "" );
	timeAndLine->setMaxLength( 32767 );
	timeAndLine->setEchoMode( QLineEdit::Normal );
	timeAndLine->setFrame( TRUE );

	timeBetweenLine = new QLineEdit( this, "timeBetweenLine" );
	timeBetweenLine->setGeometry( 120, 370, 80, 30 );
	timeBetweenLine->setMinimumSize( 0, 0 );
	timeBetweenLine->setMaximumSize( 32767, 32767 );
	timeBetweenLine->setFocusPolicy( QWidget::StrongFocus );
	timeBetweenLine->setBackgroundMode( QWidget::PaletteBase );
	timeBetweenLine->setFontPropagation( QWidget::NoChildren );
	timeBetweenLine->setPalettePropagation( QWidget::NoChildren );
	timeBetweenLine->setText( "" );
	timeBetweenLine->setMaxLength( 32767 );
	timeBetweenLine->setEchoMode( QLineEdit::Normal );
	timeBetweenLine->setFrame( TRUE );

	timeSinceLine = new QLineEdit( this, "timeSinceLine" );
	timeSinceLine->setGeometry( 120, 330, 200, 30 );
	timeSinceLine->setMinimumSize( 0, 0 );
	timeSinceLine->setMaximumSize( 32767, 32767 );
	timeSinceLine->setFocusPolicy( QWidget::StrongFocus );
	timeSinceLine->setBackgroundMode( QWidget::PaletteBase );
	timeSinceLine->setFontPropagation( QWidget::NoChildren );
	timeSinceLine->setPalettePropagation( QWidget::NoChildren );
	timeSinceLine->setText( "" );
	timeSinceLine->setMaxLength( 32767 );
	timeSinceLine->setEchoMode( QLineEdit::Normal );
	timeSinceLine->setFrame( TRUE );

	qtarch_Label_84 = new QLabel( this, "Label_84" );
	qtarch_Label_84->setGeometry( 210, 370, 30, 30 );
	qtarch_Label_84->setMinimumSize( 0, 0 );
	qtarch_Label_84->setMaximumSize( 32767, 32767 );
	qtarch_Label_84->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_84->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_84->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_84->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_84->setText( "and" );
	qtarch_Label_84->setAlignment( 289 );
	qtarch_Label_84->setMargin( -1 );

	dateSinceLine = new QLabel( this, "dateSinceLine" );
	dateSinceLine->setGeometry( 120, 220, 200, 30 );
	dateSinceLine->setMinimumSize( 0, 0 );
	dateSinceLine->setMaximumSize( 32767, 32767 );
	{
		QColorGroup normal( QColor( QRgb(0) ), QColor( QRgb(16777215) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QColorGroup disabled( QColor( QRgb(8421504) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
		QColorGroup active( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(8421504) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		dateSinceLine->setPalette( palette );
	}
	dateSinceLine->setFocusPolicy( QWidget::NoFocus );
	dateSinceLine->setBackgroundMode( QWidget::PaletteBackground );
	dateSinceLine->setFontPropagation( QWidget::NoChildren );
	dateSinceLine->setPalettePropagation( QWidget::NoChildren );
	dateSinceLine->setFrameStyle( 17 );
	dateSinceLine->setText( "" );
	dateSinceLine->setAlignment( 292 );
	dateSinceLine->setMargin( -1 );

	qtarch_ButtonGroup_143->insert( selfChoosenFilesCleanButton );
	qtarch_ButtonGroup_143->insert( selfChoosenFilesDeleteButton );
	qtarch_ButtonGroup_143->insert( selfChoosenFilesAddButton );
	qtarch_ButtonGroup_139->insert( selfChoosenFilesPushButton );
	qtarch_ButtonGroup_139->insert( currentButton ,1);
	qtarch_ButtonGroup_139->insert( allInBufferButton ,2);
	qtarch_ButtonGroup_139->insert( allInProjectButton ,3);
	qtarch_ButtonGroup_139->insert( selfChoosenFilesButton ,5);
	qtarch_ButtonGroup_139->insert( changedFilesButton ,4);
      	qtarch_ButtonGroup_139->setButton(1);

	okButton = new QPushButton( mainwidget, "okButton" );
	okButton->setText(("Ok"));
	okButton->setGeometry( 20, 440, 100, 30 );
	cancelButton = new QPushButton( mainwidget, "cancelButton" );
	cancelButton->setText(("Cancel"));
	cancelButton->setGeometry( 140, 440, 100, 30 );
	connect(cancelButton,SIGNAL(clicked()),SLOT(slotFileCancelClicked()));

	connect(qtarch_ButtonGroup_139,SIGNAL(clicked(int)),SLOT(slotSelfChoosenFilesActive(int)));
	
	mainwidget->show();
}

void CFilePrintDlg::slotFileCancelClicked() {
  reject();
}

void CFilePrintDlg::slotSelfChoosenFilesActive(int number) {
  if (number==5) {
    selfChoosenFilesPushButton->setEnabled(true);
    selfChoosenFileLine->setEnabled(true);
    selfChoosenFilesAddButton->setEnabled(true);
    selfChoosenFilesDeleteButton->setEnabled(true);
    selfChoosenFilesCleanButton->setEnabled(true);
    selfChoosenFilesMultiLine->setEnabled(true);
    qtarch_ButtonGroup_143->setEnabled(true);
    qtarch_Label_79->setEnabled(true);
    
    qtarch_Label_77->setEnabled(false);
    qtarch_Label_78->setEnabled(false);
    qtarch_Label_80->setEnabled(false);
    qtarch_Label_81->setEnabled(false);
    qtarch_Label_83->setEnabled(false);
    qtarch_Label_84->setEnabled(false);
    qtarch_ButtonGroup_140->setEnabled(false);
    qtarch_ButtonGroup_141->setEnabled(false);
    qtarch_ButtonGroup_142->setEnabled(false);
    dateBetweenLine->setEnabled(false);
    dateAndLine->setEnabled(false);
    dateSinceLine->setEnabled(false);
    timeBetweenLine->setEnabled(false);
    timeAndLine->setEnabled(false);
    timeSinceLine->setEnabled(false);
  }
  else
    if (number==1) {
      selfChoosenFilesPushButton->setEnabled(false);
      selfChoosenFileLine->setEnabled(false);
      selfChoosenFilesAddButton->setEnabled(false);
      selfChoosenFilesDeleteButton->setEnabled(false);
      selfChoosenFilesCleanButton->setEnabled(false);
      selfChoosenFilesMultiLine->setEnabled(false);
      qtarch_ButtonGroup_143->setEnabled(false);
      qtarch_Label_79->setEnabled(false);
      
      qtarch_Label_77->setEnabled(false);
      qtarch_Label_78->setEnabled(false);
      qtarch_Label_80->setEnabled(false);
      qtarch_Label_81->setEnabled(false);
      qtarch_Label_83->setEnabled(false);
      qtarch_Label_84->setEnabled(false);
      qtarch_ButtonGroup_140->setEnabled(false);
      qtarch_ButtonGroup_141->setEnabled(false);
      qtarch_ButtonGroup_142->setEnabled(false);
      dateBetweenLine->setEnabled(false);
      dateAndLine->setEnabled(false);
      dateSinceLine->setEnabled(false);
      timeBetweenLine->setEnabled(false);
      timeAndLine->setEnabled(false);
      timeSinceLine->setEnabled(false);
    }
    else
      if (number==2) {
	selfChoosenFilesPushButton->setEnabled(false);
	selfChoosenFileLine->setEnabled(false);
	selfChoosenFilesAddButton->setEnabled(false);
	selfChoosenFilesDeleteButton->setEnabled(false);
	selfChoosenFilesCleanButton->setEnabled(false);
	selfChoosenFilesMultiLine->setEnabled(false);
	qtarch_ButtonGroup_143->setEnabled(false);
	qtarch_Label_79->setEnabled(false);
	
	qtarch_Label_77->setEnabled(false);
	qtarch_Label_78->setEnabled(false);
	qtarch_Label_80->setEnabled(false);
	qtarch_Label_81->setEnabled(false);
	qtarch_Label_83->setEnabled(false);
	qtarch_Label_84->setEnabled(false);
	qtarch_ButtonGroup_140->setEnabled(false);
	qtarch_ButtonGroup_141->setEnabled(false);
	qtarch_ButtonGroup_142->setEnabled(false);
	dateBetweenLine->setEnabled(false);
	dateAndLine->setEnabled(false);
	dateSinceLine->setEnabled(false);
	timeBetweenLine->setEnabled(false);
	timeAndLine->setEnabled(false);
	timeSinceLine->setEnabled(false);
      }
      else 
	if (number==3) {
	  selfChoosenFilesPushButton->setEnabled(false);
	  selfChoosenFileLine->setEnabled(false);
	  selfChoosenFilesAddButton->setEnabled(false);
	  selfChoosenFilesDeleteButton->setEnabled(false);
	  selfChoosenFilesCleanButton->setEnabled(false);
	  selfChoosenFilesMultiLine->setEnabled(false);
	  qtarch_ButtonGroup_143->setEnabled(false);
	  qtarch_Label_79->setEnabled(false);
	  
	  qtarch_Label_77->setEnabled(false);
	  qtarch_Label_78->setEnabled(false);
	  qtarch_Label_80->setEnabled(false);
	  qtarch_Label_81->setEnabled(false);
	  qtarch_Label_83->setEnabled(false);
	  qtarch_Label_84->setEnabled(false);
	  qtarch_ButtonGroup_140->setEnabled(false);
	  qtarch_ButtonGroup_141->setEnabled(false);
	  qtarch_ButtonGroup_142->setEnabled(false);
	  dateBetweenLine->setEnabled(false);
	  dateAndLine->setEnabled(false);
	  dateSinceLine->setEnabled(false);
	  timeBetweenLine->setEnabled(false);
	  timeAndLine->setEnabled(false);
	  timeSinceLine->setEnabled(false);
	}
	else
	  if (number==4) {
	    selfChoosenFilesPushButton->setEnabled(false);
	    selfChoosenFileLine->setEnabled(false);
	    selfChoosenFilesAddButton->setEnabled(false);
	    selfChoosenFilesDeleteButton->setEnabled(false);
	    selfChoosenFilesCleanButton->setEnabled(false);
	    selfChoosenFilesMultiLine->setEnabled(false);
	    qtarch_ButtonGroup_143->setEnabled(false);
	    qtarch_Label_79->setEnabled(false);

	    qtarch_Label_77->setEnabled(true);
	    qtarch_Label_78->setEnabled(true);
	    qtarch_Label_80->setEnabled(true);
	    qtarch_Label_81->setEnabled(true);
	    qtarch_Label_83->setEnabled(true);
	    qtarch_Label_84->setEnabled(true);
	    qtarch_ButtonGroup_140->setEnabled(true);
	    qtarch_ButtonGroup_141->setEnabled(true);
	    qtarch_ButtonGroup_142->setEnabled(true);
	    dateBetweenLine->setEnabled(true);
	    dateAndLine->setEnabled(true);
	    dateSinceLine->setEnabled(true);
	    timeBetweenLine->setEnabled(true);
	    timeAndLine->setEnabled(true);
	    timeSinceLine->setEnabled(true);
	  }
}

void CFilePrintDlg::slotSelfChoosenFileButtonClicked() {
  selfChoosenFileLine->setText(KFileDialog::getOpenFileName());
}

void CFilePrintDlg::slotSelfChoosenFileAddClicked() {
  selfChoosenFilesMultiLine->insertItem(selfChoosenFileLine->text());
}

void CFilePrintDlg::slotSelfChoosenFileCleanClicked() {
  selfChoosenFilesMultiLine->clear();
}

void CFilePrintDlg::slotSelfChoosenFileDeleteClicked() {
  unsigned int count = selfChoosenFilesMultiLine->count();
  for (unsigned int i=count;i>0;i--) {
    if (selfChoosenFilesMultiLine->isSelected(i-1)) {
      selfChoosenFilesMultiLine->removeItem(i-1);      
    }
  }
}
