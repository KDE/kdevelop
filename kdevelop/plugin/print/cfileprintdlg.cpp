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

#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kiconloader.h>

#include "cproject.h"
#include "cfileprintdlg.h"


CFilePrintDlg::CFilePrintDlg(QWidget* parent,const char* name) : QDialog(parent, name, true){
  init();
  loadSettings();
  slotSelfChoosenFilesActive(0);
  endDate = QDate::currentDate();
  endTime.setHMS(endTimeHourLine->value(),endTimeMinuteLine->value(),0);
  beginDate = QDate::currentDate();
  beginTime.setHMS(beginTimeHourLine->value(),beginTimeMinuteLine->value(),0);
}

CFilePrintDlg::~CFilePrintDlg(){
  selfChoosenFilesMultiLine->clear();
  delete(selfChoosenFilesMultiLine);
  delete (cancelButton);
  delete (okButton);
  delete (qtarch_Label_91);
  delete (qtarch_Label_90);
  delete (endTimeMinuteLine);
  delete (endTimeHourLine);
  delete (qtarch_Label_89);
  delete (qtarch_Label_88);
  delete (beginTimeHourLine);
  delete (beginTimeMinuteLine);
  delete (beginDateButton);
  delete (beginTimeButton);
  delete (cppFilesButton);
  delete (headerFilesButton);
  delete (qtarch_Label_84);
  delete (qtarch_Label_83);
  delete (qtarch_Label_80);
  delete (changedFilesButton);
  delete (selfChoosenFilesButton);
  delete (allInProjectButton);
  delete (currentButton);
  delete (qtarch_Label_79);
  delete (selfChoosenFilesPushButton);
  delete (selfChoosenFileLine);
  delete (qtarch_Label_77);
  delete (selfChoosenFilesAddButton);
  delete (selfChoosenFilesDeleteButton);
  delete (selfChoosenFilesCleanButton);
  delete (qtarch_ButtonGroup_139);
  delete (qtarch_ButtonGroup_142);
  delete (qtarch_ButtonGroup_141);
  delete (qtarch_ButtonGroup_140);
  delete (qtarch_ButtonGroup_143);
  delete (filewidget);
  delete (mainwidget);
}

void CFilePrintDlg::init() {
  mainwidget = new QWidget(this,"filedialog");
  mainwidget->resize (600,480);
  filewidget = new QWidget(this,"files");
  filewidget->resize(600,430);

  qtarch_ButtonGroup_139 = new QButtonGroup( this, "ButtonGroup_139" );
  qtarch_ButtonGroup_139->setGeometry( 20, 10, 320, 160 );
  qtarch_ButtonGroup_139->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_139->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_139->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_139->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_139->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_139->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_139->setFrameStyle( 49 );
  qtarch_ButtonGroup_139->setTitle(i18n("File selection") );
  qtarch_ButtonGroup_139->setAlignment( 1 );
  
  currentButton = new QRadioButton( this, "currentButton" );
  currentButton->setGeometry( 30, 30, 140, 30 );
  currentButton->setMinimumSize( 0, 0 );
  currentButton->setMaximumSize( 32767, 32767 );
  currentButton->setFocusPolicy( QWidget::TabFocus );
  currentButton->setBackgroundMode( QWidget::PaletteBackground );
  currentButton->setFontPropagation( QWidget::NoChildren );
  currentButton->setPalettePropagation( QWidget::NoChildren );
  currentButton->setText( i18n("current") );
  currentButton->setAutoRepeat( FALSE );
  currentButton->setAutoResize( FALSE );
	
  allInProjectButton = new QRadioButton( this, "allInProjectButton" );
  allInProjectButton->setGeometry( 30, 60, 140, 30 );
  allInProjectButton->setMinimumSize( 0, 0 );
  allInProjectButton->setMaximumSize( 32767, 32767 );
  allInProjectButton->setFocusPolicy( QWidget::TabFocus );
  allInProjectButton->setBackgroundMode( QWidget::PaletteBackground );
  allInProjectButton->setFontPropagation( QWidget::NoChildren );
  allInProjectButton->setPalettePropagation( QWidget::NoChildren );
  allInProjectButton->setText( i18n("all in project") );
  allInProjectButton->setAutoRepeat( FALSE );
  allInProjectButton->setAutoResize( FALSE );

  selfChoosenFilesButton = new QRadioButton( this, "selfChoosenFilesButton" );
  selfChoosenFilesButton->setGeometry( 30, 90, 290, 30 );
  selfChoosenFilesButton->setMinimumSize( 0, 0 );
  selfChoosenFilesButton->setMaximumSize( 32767, 32767 );
  selfChoosenFilesButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesButton->setText( i18n("self chosen files") );
  selfChoosenFilesButton->setAutoRepeat( FALSE );
  selfChoosenFilesButton->setAutoResize( FALSE );

  qtarch_Label_79 = new QLabel( this, "Label_79" );
  qtarch_Label_79->setGeometry( 50, 130, 30, 30 );
  qtarch_Label_79->setMinimumSize( 0, 0 );
  qtarch_Label_79->setMaximumSize( 32767, 32767 );
  qtarch_Label_79->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_79->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_79->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_79->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_79->setText( i18n("file") );
  qtarch_Label_79->setAlignment( 289 );
  qtarch_Label_79->setMargin( -1 );

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
  selfChoosenFilesPushButton->setPixmap(BarIcon("open"));
  selfChoosenFilesPushButton->setAutoRepeat( FALSE );
  selfChoosenFilesPushButton->setAutoResize( FALSE );
  
  headerFilesButton = new QRadioButton( this, "headerFilesButton" );
  headerFilesButton->setGeometry( 190, 30, 140, 30 );
  headerFilesButton->setMinimumSize( 0, 0 );
  headerFilesButton->setMaximumSize( 32767, 32767 );
  headerFilesButton->setFocusPolicy( QWidget::TabFocus );
  headerFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  headerFilesButton->setFontPropagation( QWidget::NoChildren );
  headerFilesButton->setPalettePropagation( QWidget::NoChildren );
  headerFilesButton->setText( i18n("all headers") );
  headerFilesButton->setAutoRepeat( FALSE );
  headerFilesButton->setAutoResize( FALSE );

  cppFilesButton = new QRadioButton( this, "cppFilesButton" );
  cppFilesButton->setGeometry( 190, 60, 140, 30 );
  cppFilesButton->setMinimumSize( 0, 0 );
  cppFilesButton->setMaximumSize( 32767, 32767 );
  cppFilesButton->setFocusPolicy( QWidget::TabFocus );
  cppFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  cppFilesButton->setFontPropagation( QWidget::NoChildren );
  cppFilesButton->setPalettePropagation( QWidget::NoChildren );
  cppFilesButton->setText( i18n("all cpp files") );
  cppFilesButton->setAutoRepeat( FALSE );
  cppFilesButton->setAutoResize( FALSE );

  changedFilesButton = new QRadioButton( this, "changedFilesButton" );
  changedFilesButton->setGeometry( 190, 90, 140, 30 );
  changedFilesButton->setMinimumSize( 0, 0 );
  changedFilesButton->setMaximumSize( 32767, 32767 );
  changedFilesButton->setFocusPolicy( QWidget::TabFocus );
  changedFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  changedFilesButton->setFontPropagation( QWidget::NoChildren );
  changedFilesButton->setPalettePropagation( QWidget::NoChildren );
  changedFilesButton->setText( i18n("changed files") );
  changedFilesButton->setAutoRepeat( FALSE );
  changedFilesButton->setAutoResize( FALSE );
  
  qtarch_ButtonGroup_140 = new QButtonGroup( this, "ButtonGroup_140" );
  qtarch_ButtonGroup_140->setGeometry( 20, 180, 320, 240 );
  qtarch_ButtonGroup_140->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_140->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_140->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_140->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_140->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_140->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_140->setFrameStyle( 49 );
  qtarch_ButtonGroup_140->setTitle( i18n("Changed files") );
  qtarch_ButtonGroup_140->setAlignment( 1 );

  qtarch_ButtonGroup_142 = new QButtonGroup( this, "ButtonGroup_142" );
  qtarch_ButtonGroup_142->setGeometry( 30, 200, 300, 100 );
  qtarch_ButtonGroup_142->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_142->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_142->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_142->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_142->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_142->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_142->setFrameStyle( 49 );
  qtarch_ButtonGroup_142->setTitle( i18n("Between") );
  qtarch_ButtonGroup_142->setAlignment( 1 );

  qtarch_Label_77 = new QLabel( this, "Label_77" );
  qtarch_Label_77->setGeometry( 40, 220, 70, 30 );
  qtarch_Label_77->setMinimumSize( 0, 0 );
  qtarch_Label_77->setMaximumSize( 32767, 32767 );
  qtarch_Label_77->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_77->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_77->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_77->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_77->setText( i18n("date") );
  qtarch_Label_77->setAlignment( 289 );
  qtarch_Label_77->setMargin( -1 );

  beginDateButton = new QPushButton( this, "beginDateButton" );
  beginDateButton->setGeometry( 120, 220, 200, 30 );
  beginDateButton->setMinimumSize( 0, 0 );
  beginDateButton->setMaximumSize( 32767, 32767 );
  beginDateButton->setFocusPolicy( QWidget::TabFocus );
  beginDateButton->setBackgroundMode( QWidget::PaletteBackground );
  beginDateButton->setFontPropagation( QWidget::NoChildren );
  beginDateButton->setPalettePropagation( QWidget::NoChildren );
  beginDateButton->setText( "" );
  beginDateButton->setAutoRepeat( FALSE );
  beginDateButton->setAutoResize( FALSE );
  
  qtarch_Label_80 = new QLabel( this, "Label_80" );
  qtarch_Label_80->setGeometry( 40, 260, 70, 30 );
  qtarch_Label_80->setMinimumSize( 0, 0 );
  qtarch_Label_80->setMaximumSize( 32767, 32767 );
  qtarch_Label_80->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_80->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_80->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_80->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_80->setText( i18n("time") );
  qtarch_Label_80->setAlignment( 289 );
  qtarch_Label_80->setMargin( -1 );

  beginTimeHourLine = new QSpinBox( this, "beginTimeHourLine" );
  beginTimeHourLine->setGeometry( 120, 260, 60, 30 );
  beginTimeHourLine->setMinimumSize( 0, 0 );
  beginTimeHourLine->setMaximumSize( 32767, 32767 );
  beginTimeHourLine->setFocusPolicy( QWidget::StrongFocus );
  beginTimeHourLine->setBackgroundMode( QWidget::PaletteBackground );
  beginTimeHourLine->setFontPropagation( QWidget::NoChildren );
  beginTimeHourLine->setPalettePropagation( QWidget::NoChildren );
  beginTimeHourLine->setFrameStyle( 50 );
  beginTimeHourLine->setLineWidth( 2 );
  beginTimeHourLine->setRange( 0, 24 );
  beginTimeHourLine->setSteps( 1, 0 );
  beginTimeHourLine->setPrefix( "" );
  beginTimeHourLine->setSuffix( "" );
  beginTimeHourLine->setSpecialValueText( "" );
  beginTimeHourLine->setWrapping( FALSE );
  
  qtarch_Label_88 = new QLabel( this, "Label_88" );
  qtarch_Label_88->setGeometry( 180, 260, 20, 30 );
  qtarch_Label_88->setMinimumSize( 0, 0 );
  qtarch_Label_88->setMaximumSize( 32767, 32767 );
  qtarch_Label_88->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_88->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_88->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_88->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_88->setText( ":" );
  qtarch_Label_88->setAlignment( 292 );
  qtarch_Label_88->setMargin( -1 );
  
  beginTimeMinuteLine = new QSpinBox( this, "beginTimeMinuteList" );
  beginTimeMinuteLine->setGeometry( 200, 260, 60, 30 );
  beginTimeMinuteLine->setMinimumSize( 0, 0 );
  beginTimeMinuteLine->setMaximumSize( 32767, 32767 );
  beginTimeMinuteLine->setFocusPolicy( QWidget::StrongFocus );
  beginTimeMinuteLine->setBackgroundMode( QWidget::PaletteBackground );
  beginTimeMinuteLine->setFontPropagation( QWidget::NoChildren );
  beginTimeMinuteLine->setPalettePropagation( QWidget::NoChildren );
  beginTimeMinuteLine->setFrameStyle( 50 );
  beginTimeMinuteLine->setLineWidth( 2 );
  beginTimeMinuteLine->setRange( 0, 60 );
  beginTimeMinuteLine->setSteps( 1, 0 );
  beginTimeMinuteLine->setPrefix( "" );
  beginTimeMinuteLine->setSuffix( "" );
  beginTimeMinuteLine->setSpecialValueText( "" );
  beginTimeMinuteLine->setWrapping( FALSE );

  qtarch_Label_89 = new QLabel( this, "Label_89" );
  qtarch_Label_89->setGeometry( 270, 260, 50, 30 );
  qtarch_Label_89->setMinimumSize( 0, 0 );
  qtarch_Label_89->setMaximumSize( 32767, 32767 );
  qtarch_Label_89->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_89->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_89->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_89->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_89->setText(i18n("o´clock"));
  qtarch_Label_89->setAlignment( 289 );
  qtarch_Label_89->setMargin( -1 );
  
  qtarch_ButtonGroup_141 = new QButtonGroup( this, "ButtonGroup_141" );
  qtarch_ButtonGroup_141->setGeometry( 30, 310, 300, 100 );
  qtarch_ButtonGroup_141->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_141->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_141->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_141->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_141->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_141->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_141->setFrameStyle( 49 );
  qtarch_ButtonGroup_141->setTitle( i18n("And") );
  qtarch_ButtonGroup_141->setAlignment( 1 );

  qtarch_Label_83 = new QLabel( this, "Label_83" );
  qtarch_Label_83->setGeometry( 40, 330, 70, 30 );
  qtarch_Label_83->setMinimumSize( 0, 0 );
  qtarch_Label_83->setMaximumSize( 32767, 32767 );
  qtarch_Label_83->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_83->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_83->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_83->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_83->setText( i18n("date") );
  qtarch_Label_83->setAlignment( 289 );
  qtarch_Label_83->setMargin( -1 );

  beginTimeButton = new QPushButton( this, "beginTimeButton" );
  beginTimeButton->setGeometry( 120, 330, 200, 30 );
  beginTimeButton->setMinimumSize( 0, 0 );
  beginTimeButton->setMaximumSize( 32767, 32767 );
  beginTimeButton->setFocusPolicy( QWidget::TabFocus );
  beginTimeButton->setBackgroundMode( QWidget::PaletteBackground );
  beginTimeButton->setFontPropagation( QWidget::NoChildren );
  beginTimeButton->setPalettePropagation( QWidget::NoChildren );
  beginTimeButton->setText( "" );
  beginTimeButton->setAutoRepeat( FALSE );
  beginTimeButton->setAutoResize( FALSE );

  qtarch_Label_84 = new QLabel( this, "Label_84" );
  qtarch_Label_84->setGeometry( 40, 370, 70, 30 );
  qtarch_Label_84->setMinimumSize( 0, 0 );
  qtarch_Label_84->setMaximumSize( 32767, 32767 );
  qtarch_Label_84->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_84->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_84->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_84->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_84->setText( i18n("time") );
  qtarch_Label_84->setAlignment( 289 );
  qtarch_Label_84->setMargin( -1 );

  endTimeHourLine = new QSpinBox( this, "endTimeHourLine" );
  endTimeHourLine->setGeometry( 120, 370, 60, 30 );
  endTimeHourLine->setMinimumSize( 0, 0 );
  endTimeHourLine->setMaximumSize( 32767, 32767 );
  endTimeHourLine->setFocusPolicy( QWidget::StrongFocus );
  endTimeHourLine->setBackgroundMode( QWidget::PaletteBackground );
  endTimeHourLine->setFontPropagation( QWidget::NoChildren );
  endTimeHourLine->setPalettePropagation( QWidget::NoChildren );
  endTimeHourLine->setFrameStyle( 50 );
  endTimeHourLine->setLineWidth( 2 );
  endTimeHourLine->setRange( 0, 24 );
  endTimeHourLine->setSteps( 1, 0 );
  endTimeHourLine->setPrefix( "" );
  endTimeHourLine->setSuffix( "" );
  endTimeHourLine->setSpecialValueText( "" );
  endTimeHourLine->setWrapping( FALSE );
  
  qtarch_Label_91 = new QLabel( this, "Label_91" );
  qtarch_Label_91->setGeometry( 180, 370, 20, 30 );
  qtarch_Label_91->setMinimumSize( 0, 0 );
  qtarch_Label_91->setMaximumSize( 32767, 32767 );
  qtarch_Label_91->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_91->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_91->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_91->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_91->setText( ":" );
  qtarch_Label_91->setAlignment( 292 );
  qtarch_Label_91->setMargin( -1 );
  
  endTimeMinuteLine = new QSpinBox( this, "endTimeMinuteLine" );
  endTimeMinuteLine->setGeometry( 200, 370, 60, 30 );
  endTimeMinuteLine->setMinimumSize( 0, 0 );
  endTimeMinuteLine->setMaximumSize( 32767, 32767 );
  endTimeMinuteLine->setFocusPolicy( QWidget::StrongFocus );
  endTimeMinuteLine->setBackgroundMode( QWidget::PaletteBackground );
  endTimeMinuteLine->setFontPropagation( QWidget::NoChildren );
  endTimeMinuteLine->setPalettePropagation( QWidget::NoChildren );
  endTimeMinuteLine->setFrameStyle( 50 );
  endTimeMinuteLine->setLineWidth( 2 );
  endTimeMinuteLine->setRange( 0, 60 );
  endTimeMinuteLine->setSteps( 1, 0 );
  endTimeMinuteLine->setPrefix( "" );
  endTimeMinuteLine->setSuffix( "" );
  endTimeMinuteLine->setSpecialValueText( "" );
  endTimeMinuteLine->setWrapping( FALSE );

  qtarch_Label_90 = new QLabel( this, "Label_90" );
  qtarch_Label_90->setGeometry( 270, 370, 50, 30 );
  qtarch_Label_90->setMinimumSize( 0, 0 );
  qtarch_Label_90->setMaximumSize( 32767, 32767 );
  qtarch_Label_90->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_90->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_90->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_90->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_90->setText(i18n("o´clock"));
  qtarch_Label_90->setAlignment( 289 );
  qtarch_Label_90->setMargin( -1 );

  qtarch_ButtonGroup_143 = new QButtonGroup( this, "ButtonGroup_143" );
  qtarch_ButtonGroup_143->setGeometry( 350, 10, 230, 410 );
  qtarch_ButtonGroup_143->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_143->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_143->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_143->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_143->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_143->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_143->setFrameStyle( 49 );
  qtarch_ButtonGroup_143->setTitle( i18n("Self chosen files") );
  qtarch_ButtonGroup_143->setAlignment( 1 );

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

  selfChoosenFilesAddButton = new QPushButton( this, "selfChoosenFilesAddButton" );
  selfChoosenFilesAddButton->setGeometry( 360, 380, 70, 30 );
  selfChoosenFilesAddButton->setMinimumSize( 0, 0 );
  selfChoosenFilesAddButton->setMaximumSize( 32767, 32767 );
  selfChoosenFilesAddButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesAddButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesAddButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesAddButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesAddButton->setText( i18n("add") );
  selfChoosenFilesAddButton->setAutoRepeat( FALSE );
  selfChoosenFilesAddButton->setAutoResize( FALSE );

  selfChoosenFilesDeleteButton = new QPushButton( this, "selfChoosenFilesDeleteButton" );
  selfChoosenFilesDeleteButton->setGeometry( 430, 380, 70, 30 );
  selfChoosenFilesDeleteButton->setMinimumSize( 0, 0 );
  selfChoosenFilesDeleteButton->setMaximumSize( 32767, 32767 );
  selfChoosenFilesDeleteButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesDeleteButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesDeleteButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesDeleteButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesDeleteButton->setText( i18n("delete") );
  selfChoosenFilesDeleteButton->setAutoRepeat( FALSE );
  selfChoosenFilesDeleteButton->setAutoResize( FALSE );

  selfChoosenFilesCleanButton = new QPushButton( this, "selfChoosenFilesCleanButton" );
  selfChoosenFilesCleanButton->setGeometry( 500, 380, 70, 30 );
  selfChoosenFilesCleanButton->setMinimumSize( 0, 0 );
  selfChoosenFilesCleanButton->setMaximumSize( 32767, 32767 );
  selfChoosenFilesCleanButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesCleanButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesCleanButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesCleanButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesCleanButton->setText( i18n("clear") );
  selfChoosenFilesCleanButton->setAutoRepeat( FALSE );
  selfChoosenFilesCleanButton->setAutoResize( FALSE );

  qtarch_ButtonGroup_143->insert( selfChoosenFilesCleanButton );
  qtarch_ButtonGroup_143->insert( selfChoosenFilesDeleteButton );
  qtarch_ButtonGroup_143->insert( selfChoosenFilesAddButton );
  qtarch_ButtonGroup_140->insert( beginTimeButton );
  qtarch_ButtonGroup_140->insert( beginDateButton );
  qtarch_ButtonGroup_141->insert( beginTimeButton );
  qtarch_ButtonGroup_142->insert( beginDateButton );
  qtarch_ButtonGroup_139->insert( currentButton,6 );
  qtarch_ButtonGroup_139->insert( allInProjectButton,4 );
  qtarch_ButtonGroup_139->insert( selfChoosenFilesButton,5 );
  qtarch_ButtonGroup_139->insert( changedFilesButton,3 );
  qtarch_ButtonGroup_139->insert( headerFilesButton,2 );
  qtarch_ButtonGroup_139->insert( cppFilesButton,1 );
  currentButton->setChecked(true);

  okButton = new QPushButton( mainwidget, "okButton" );
  okButton->setText(i18n("OK"));
  okButton->setGeometry( 370, 440, 100, 30 );

  cancelButton = new QPushButton( mainwidget, "cancelButton" );
  cancelButton->setText(i18n("Cancel"));
  cancelButton->setGeometry( 480, 440, 100, 30 );

  QWhatsThis::add(selfChoosenFilesMultiLine,
	i18n("Here are the files which will be printed, if 'self\n"
	     "chosen file' is checked."));

  QWhatsThis::add(selfChoosenFilesCleanButton,
	i18n("Cleans the 'self chosen files' list."));

  QWhatsThis::add(selfChoosenFilesAddButton,
	i18n("Adds the selected files to the list."));

  QWhatsThis::add(selfChoosenFilesDeleteButton,
	i18n("Deletes the selected files in the file list."));

  QString text;
  text = i18n("Insert the filename and path to the file you\n"
              "want to add to the self chosen file list.\n"
              "After insertion, press the 'add' button below\n"
              "the file list box.");
  QWhatsThis::add(qtarch_Label_79, text);
  QWhatsThis::add(selfChoosenFileLine, text);

  QWhatsThis::add(allInProjectButton,
	i18n("Selects all registered project files\n"
			"for printing, including the pixmap and\n"
			"html documentation files"));

  QWhatsThis::add(selfChoosenFilesButton,
	i18n("Allows to set up a list of self-chosen files\n"
				"for printing. Select your files to print below\n"
				"and press the 'add' button on the file list."));

  QWhatsThis::add(changedFilesButton,
	i18n("Lets you select files to print by their date of\n"
			"the last modification. Set the modification time\n"
			"range below by choosing the beginning and end date."));

  QWhatsThis::add(headerFilesButton,
	i18n("Selects all registered header files for printing."));

  QWhatsThis::add(cppFilesButton,
	i18n("Selects all registered source files for printing."));

  
  text = i18n("Sets the end date of the interval. Press the button\n"
              "to pick a date by the calendar. After selection, the\n"
              "chosen end-date will be displayed on the button.");
  QWhatsThis::add(qtarch_Label_83, text);
  QWhatsThis::add(beginTimeButton, text);
  
  text = i18n("Sets the beginning date of the interval. Press the button\n"
              "to pick a date by the calendar. After selection, the\n"
              "chosen begin-date will be displayed on the button.");
  QWhatsThis::add(qtarch_Label_77, text);
  QWhatsThis::add(beginDateButton, text);

  text = i18n("Sets the minute value of the beginning time.");
  QWhatsThis::add(qtarch_Label_89, text);
  QWhatsThis::add(beginTimeMinuteLine, text);

  text = i18n("Sets the hour value of the beginning time.");
  QWhatsThis::add(qtarch_Label_80, text);
  QWhatsThis::add(beginTimeHourLine, text);

  text = i18n("Sets the hour value of the end time.");
  QWhatsThis::add(qtarch_Label_84, text);
  QWhatsThis::add(endTimeHourLine, text);

  text = i18n("Sets the minute value of the end time.");
  QWhatsThis::add(qtarch_Label_90, text);
  QWhatsThis::add(endTimeMinuteLine, text);

  QWhatsThis::add(selfChoosenFilesPushButton,
	i18n("Pressing this button lets you select the file\n"
				"by a filedialog."));

  QWhatsThis::add(currentButton,
	i18n("Selects the current file of the visible\n"
			"editor window for printing."));



  connect(cancelButton,SIGNAL(clicked()),SLOT(slotFileCancelClicked()));
  connect(okButton,SIGNAL(clicked()),SLOT(slotOkClicked()));
  connect (selfChoosenFilesCleanButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileCleanClicked()));
  connect(selfChoosenFilesPushButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileButtonClicked()));
  connect (selfChoosenFilesAddButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileAddClicked()));
  connect (selfChoosenFilesDeleteButton,SIGNAL(clicked()),SLOT(slotSelfChoosenFileDeleteClicked()));
  connect(qtarch_ButtonGroup_139,SIGNAL(clicked(int)),SLOT(slotSelfChoosenFilesActive(int)));
  connect(beginDateButton,SIGNAL(clicked()),SLOT(slotBeginDateDlgClicked()));
  connect(beginTimeButton,SIGNAL(clicked()),SLOT(slotEndDateDlgClicked()));
  
  beginDateButton->setText((QDate::currentDate()).toString());
  beginTimeButton->setText((QDate::currentDate()).toString());
  beginTimeHourLine->setValue(QTime::currentTime().hour());
  beginTimeMinuteLine->setValue(QTime::currentTime().minute());
  endTimeHourLine->setValue(QTime::currentTime().hour());
  endTimeMinuteLine->setValue(QTime::currentTime().minute());
  mainwidget->show();
}

void CFilePrintDlg::slotFileCancelClicked() {
  reject();
}

void CFilePrintDlg::slotBeginDateDlgClicked() {
  QString text="";
  CDatepikerDlg datepik;
  datepik.setCaption(i18n("BeginDate"));
  datepik.exec();
  datepik.getDateString(text);
  datepik.getDate(beginDate);
  beginDateButton->setText(text);
}


void CFilePrintDlg::slotEndDateDlgClicked() {
  QString text="";
  CDatepikerDlg datepik;
  datepik.setCaption(i18n("EndDate"));
  datepik.exec();
  datepik.getDateString(text);
  datepik.getDate(endDate);
  beginTimeButton->setText(text);
}
void CFilePrintDlg::slotSelfChoosenFilesActive(int number) {
  if (number==0) {
    if (currentButton->isChecked()) {
      slotSelfChoosenFilesActive(6);
    }
    else if (cppFilesButton->isChecked()) {
      slotSelfChoosenFilesActive(1);
    }
    else if (headerFilesButton->isChecked()) {
      slotSelfChoosenFilesActive(2);
    }
    else if (changedFilesButton->isChecked()) {
      slotSelfChoosenFilesActive(3);
    }
    else if (allInProjectButton->isChecked()) {
      slotSelfChoosenFilesActive(4);
    }
    else slotSelfChoosenFilesActive(5);
  }
  else if (number==5) {
    selfChoosenFilesPushButton->setEnabled(true);
    selfChoosenFileLine->setEnabled(true);
    selfChoosenFilesAddButton->setEnabled(true);
    selfChoosenFilesDeleteButton->setEnabled(true);
    selfChoosenFilesCleanButton->setEnabled(true);
    selfChoosenFilesMultiLine->setEnabled(true);
    qtarch_ButtonGroup_143->setEnabled(true);
    qtarch_Label_79->setEnabled(true);
    qtarch_Label_88->setEnabled(false);
    qtarch_Label_89->setEnabled(false);
    qtarch_Label_90->setEnabled(false);
    qtarch_Label_91->setEnabled(false);
    qtarch_Label_77->setEnabled(false);
    qtarch_Label_80->setEnabled(false);
    qtarch_Label_83->setEnabled(false);
    qtarch_Label_84->setEnabled(false);
    qtarch_ButtonGroup_140->setEnabled(false);
    qtarch_ButtonGroup_141->setEnabled(false);
    qtarch_ButtonGroup_142->setEnabled(false);
    beginDateButton->setEnabled(false);
    endTimeHourLine->setEnabled(false);
    endTimeMinuteLine->setEnabled(false);
    beginTimeButton->setEnabled(false);
    beginTimeHourLine->setEnabled(false);
    beginTimeMinuteLine->setEnabled(false);
  }
  else if (number==3) {
    selfChoosenFilesPushButton->setEnabled(false);
    selfChoosenFileLine->setEnabled(false);
    selfChoosenFilesAddButton->setEnabled(false);
    selfChoosenFilesDeleteButton->setEnabled(false);
    selfChoosenFilesCleanButton->setEnabled(false);
    selfChoosenFilesMultiLine->setEnabled(false);
    qtarch_ButtonGroup_143->setEnabled(false);
    qtarch_Label_79->setEnabled(false);
    qtarch_Label_88->setEnabled(true);
    qtarch_Label_89->setEnabled(true);
    qtarch_Label_90->setEnabled(true);
    qtarch_Label_91->setEnabled(true);
    qtarch_Label_77->setEnabled(true);
    qtarch_Label_80->setEnabled(true);
    qtarch_Label_83->setEnabled(true);
    qtarch_Label_84->setEnabled(true);
    qtarch_ButtonGroup_140->setEnabled(true);
    qtarch_ButtonGroup_141->setEnabled(true);
    qtarch_ButtonGroup_142->setEnabled(true);
    beginDateButton->setEnabled(true);
    endTimeHourLine->setEnabled(true);
    endTimeMinuteLine->setEnabled(true);
    beginTimeButton->setEnabled(true);
    beginTimeHourLine->setEnabled(true);
    beginTimeMinuteLine->setEnabled(true);
  }
  else {
    selfChoosenFilesPushButton->setEnabled(false);
    selfChoosenFileLine->setEnabled(false);
    selfChoosenFilesAddButton->setEnabled(false);
    selfChoosenFilesDeleteButton->setEnabled(false);
    selfChoosenFilesCleanButton->setEnabled(false);
    selfChoosenFilesMultiLine->setEnabled(false);
    qtarch_ButtonGroup_143->setEnabled(false);
    qtarch_Label_79->setEnabled(false);
    qtarch_Label_88->setEnabled(false);
    qtarch_Label_89->setEnabled(false);
    qtarch_Label_90->setEnabled(false);
    qtarch_Label_91->setEnabled(false);     
    qtarch_Label_77->setEnabled(false);
    qtarch_Label_80->setEnabled(false);
    qtarch_Label_83->setEnabled(false);
    qtarch_Label_84->setEnabled(false);
    qtarch_ButtonGroup_140->setEnabled(false);
    qtarch_ButtonGroup_141->setEnabled(false);
    qtarch_ButtonGroup_142->setEnabled(false);
    beginDateButton->setEnabled(false);
    endTimeHourLine->setEnabled(false);
    endTimeMinuteLine->setEnabled(false);
    beginTimeButton->setEnabled(false);
    beginTimeHourLine->setEnabled(false);
    beginTimeMinuteLine->setEnabled(false);
  }
}

void CFilePrintDlg::slotSelfChoosenFileButtonClicked() {
  selfChoosenFileLine->setText(KFileDialog::getOpenURL().path());
}

void CFilePrintDlg::slotSelfChoosenFileAddClicked() {
  selfChoosenFilesMultiLine->insertItem(selfChoosenFileLine->text());
  selfChoosenFileLine->clear();
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

void CFilePrintDlg::slotOkClicked() {
  beginTime.setHMS(beginTimeHourLine->value(),beginTimeMinuteLine->value(),0);
  endTime.setHMS(endTimeHourLine->value(),endTimeMinuteLine->value(),0);
  settings = kapp->config();
  settings->setGroup("FileDialog");
  int i=6;
  if (currentButton->isChecked()) {
    i=6;
  }
  else if (cppFilesButton->isChecked()) {
    i=1;
  }
  else if (headerFilesButton->isChecked()) {
    i=2;
  }
  else if (changedFilesButton->isChecked()) {
    i=3;
  }
  else if (allInProjectButton->isChecked()) {
    i=4;
  }
  else i=5;
  settings->writeEntry("FileSelection",i);
  settings->writeEntry("FileLine",selfChoosenFileLine->text());
  unsigned int count = selfChoosenFilesMultiLine->count();
  fileList.clear();
  for (unsigned int i=count;i>0;i--) {
    fileList.append(selfChoosenFilesMultiLine->text(i-1));      
  }
  settings->writeEntry("FileList",fileList);

  settings->setGroup("LastSettings");
  if (currentButton->isChecked()) {
    settings->writeEntry("FileSettings","current");
  }
  else if (cppFilesButton->isChecked()) {
    settings->writeEntry("FileSettings","cppFiles");
  }
  else if (allInProjectButton->isChecked()) {
    settings->writeEntry("FileSettings","allFiles");
  }
  else if (headerFilesButton->isChecked()) {
    settings->writeEntry("FileSettings","headerFiles");
  }
  else if (selfChoosenFilesButton->isChecked()) {
    QString str = "";
    QString sources = "";
    int counter = selfChoosenFilesMultiLine->count();
    if (counter != 0) {
      for (int i=0;i<counter;i++) {
	sources = (QString) selfChoosenFilesMultiLine->text(i) + (QString) " " + sources;
      }
    }
    else if (strcmp(selfChoosenFileLine->text(),"")) {
      sources = (QString) selfChoosenFileLine->text();
    }
    else {
      QMessageBox::information(0,i18n("No File to print !"),i18n("You need to select at least one file for printing. "
      "This can be done by entering the filename on the left at the file entry field."));
      return;
    }
    settings->writeEntry("FileSettings",sources);
  }
  else {
    QStrList filelist;
    QString sources = "";
    QString str = "";
    QString prj_str = "";
    QString directory = "";
    QDateTime beginDateTime (beginDate, beginTime);
    QDateTime endDateTime (endDate, endTime);
    QFileInfo fileInfo;
    settings->setGroup("Files");
    prj_str = settings->readEntry("project_file");
    CProject project (prj_str);
    project.readProject();
    prj_str.truncate(prj_str.findRev("/")); 
    directory = prj_str;
    project.getAllFiles(filelist);
    for(str= filelist.first();str !=0;str = filelist.next()){
      fileInfo.setFile(directory + "/" + str);
      if (beginDateTime != endDateTime) {
	if ((beginDateTime < fileInfo.lastModified()) && (fileInfo.lastModified() < endDateTime)) {
	  sources =  prj_str + "/" + str + " " + sources ;
	}
      }
    }
    settings->setGroup("LastSettings");
    settings->writeEntry("FileSettings",sources);
  }
  settings->sync();
  reject();
}

void CFilePrintDlg::loadSettings() {
  selfChoosenFilesMultiLine->clear();
  fileList.clear();
  settings = kapp->config();
  settings->setGroup("FileDialog");
  qtarch_ButtonGroup_139->setButton(settings->readNumEntry("FileSelection"));
  selfChoosenFileLine->setText(settings->readEntry("FileLine"));
  settings->readListEntry("FileList",fileList);
  selfChoosenFilesMultiLine->insertStrList(&fileList);
}





