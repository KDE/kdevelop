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
#include <qbuttongroup.h>
#include <kdebug.h>
#include <kapp.h>
#include <kmessagebox.h>
#include <qdatetime.h>
#include "../cproject.h"
#include <qwhatsthis.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qgrid.h>
#include <qlayout.h>
#include <kbuttonbox.h>
#include <kglobal.h>

// This order is dependent on the construction order :(
enum {
  ID_currentButton                = 0,
  ID_allInProjectButton           = 1,
  ID_selfChoosenFilesButton       = 2,
  ID_selfChoosenFilesPushButton   = 3,
  ID_headerFilesButton            = 4,
  ID_cppFilesButton               = 5,
  ID_changedFilesButton           = 6
};

CFilePrintDlg::CFilePrintDlg(QWidget* parent,const char* name) : QDialog(parent, name, true){
  init();
  loadSettings();
  slotSelfChoosenFilesActive(ID_currentButton);
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
}

void CFilePrintDlg::init() {
  QGridLayout *grid1 = new QGridLayout(this,4,2,15,7);
  qtarch_ButtonGroup_139 = new QButtonGroup( this, "ButtonGroup_139" );
  qtarch_ButtonGroup_139->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_139->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_139->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_139->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_139->setFrameStyle( 49 );
  qtarch_ButtonGroup_139->setTitle(i18n("File selection") );
  qtarch_ButtonGroup_139->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_139,0,0);

  QGridLayout *grid2 = new QGridLayout(qtarch_ButtonGroup_139,4,3,15,7);
  currentButton = new QRadioButton( qtarch_ButtonGroup_139, "currentButton" );
  currentButton->setFocusPolicy( QWidget::TabFocus );
  currentButton->setBackgroundMode( QWidget::PaletteBackground );
  currentButton->setFontPropagation( QWidget::NoChildren );
  currentButton->setPalettePropagation( QWidget::NoChildren );
  currentButton->setText( i18n("current") );
  currentButton->setAutoRepeat( FALSE );
  currentButton->setAutoResize( FALSE );
  grid2->addWidget(currentButton,0,0);

	
  allInProjectButton = new QRadioButton(qtarch_ButtonGroup_139, "allInProjectButton" );

  allInProjectButton->setFocusPolicy( QWidget::TabFocus );
  allInProjectButton->setBackgroundMode( QWidget::PaletteBackground );
  allInProjectButton->setFontPropagation( QWidget::NoChildren );
  allInProjectButton->setPalettePropagation( QWidget::NoChildren );
  allInProjectButton->setText( i18n("all in project") );
  allInProjectButton->setAutoRepeat( FALSE );
  allInProjectButton->setAutoResize( FALSE );
  grid2->addWidget(allInProjectButton,1,0);


  selfChoosenFilesButton = new QRadioButton(qtarch_ButtonGroup_139 , "selfChoosenFilesButton" );

  selfChoosenFilesButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesButton->setText( i18n("self choosen files") );
  selfChoosenFilesButton->setAutoRepeat( FALSE );
  selfChoosenFilesButton->setAutoResize( FALSE );
  grid2->addWidget(selfChoosenFilesButton,2,0);

  qtarch_Label_79 = new QLabel( qtarch_ButtonGroup_139, "Label_79" );

  qtarch_Label_79->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_79->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_79->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_79->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_79->setText( i18n("file") );
  qtarch_Label_79->setAlignment( 289 );
  qtarch_Label_79->setMargin( -1 );
  grid2->addWidget( qtarch_Label_79,3,0);

  selfChoosenFileLine = new QLineEdit( qtarch_ButtonGroup_139, "selfChoosenFileLine" );

  selfChoosenFileLine->setFocusPolicy( QWidget::StrongFocus );
  selfChoosenFileLine->setBackgroundMode( QWidget::PaletteBase );
  selfChoosenFileLine->setFontPropagation( QWidget::NoChildren );
  selfChoosenFileLine->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFileLine->setText( "" );
  selfChoosenFileLine->setMaxLength( 32767 );
  selfChoosenFileLine->setEchoMode( QLineEdit::Normal );
  selfChoosenFileLine->setFrame( TRUE );
  grid2->addWidget(selfChoosenFileLine,3,1);


  selfChoosenFilesPushButton = new QPushButton(qtarch_ButtonGroup_139 , "selfChoosenFilesPushButton" );
  selfChoosenFilesPushButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesPushButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesPushButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesPushButton->setPalettePropagation( QWidget::NoChildren );
	QPixmap pix(SmallIcon("fileopen"));
  selfChoosenFilesPushButton->setPixmap(pix);
  selfChoosenFilesPushButton->setAutoRepeat( FALSE );
  selfChoosenFilesPushButton->setAutoResize( FALSE );
  grid2->addWidget(selfChoosenFilesPushButton,3,2);


  headerFilesButton = new QRadioButton(qtarch_ButtonGroup_139 , "headerFilesButton" );
  headerFilesButton->setFocusPolicy( QWidget::TabFocus );
  headerFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  headerFilesButton->setFontPropagation( QWidget::NoChildren );
  headerFilesButton->setPalettePropagation( QWidget::NoChildren );
  headerFilesButton->setText( i18n("all headers") );
  headerFilesButton->setAutoRepeat( FALSE );
  headerFilesButton->setAutoResize( FALSE );
  grid2->addWidget(headerFilesButton,0,1);

  cppFilesButton = new QRadioButton( qtarch_ButtonGroup_139, "cppFilesButton" );
  cppFilesButton->setFocusPolicy( QWidget::TabFocus );
  cppFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  cppFilesButton->setFontPropagation( QWidget::NoChildren );
  cppFilesButton->setPalettePropagation( QWidget::NoChildren );
  cppFilesButton->setText( i18n("all cpp files") );
  cppFilesButton->setAutoRepeat( FALSE );
  cppFilesButton->setAutoResize( FALSE );
  grid2->addWidget(cppFilesButton,1,1);

  changedFilesButton = new QRadioButton( qtarch_ButtonGroup_139, "changedFilesButton" );
  changedFilesButton->setFocusPolicy( QWidget::TabFocus );
  changedFilesButton->setBackgroundMode( QWidget::PaletteBackground );
  changedFilesButton->setFontPropagation( QWidget::NoChildren );
  changedFilesButton->setPalettePropagation( QWidget::NoChildren );
  changedFilesButton->setText( i18n("changed files") );
  changedFilesButton->setAutoRepeat( FALSE );
  changedFilesButton->setAutoResize( FALSE );
  grid2->addWidget(changedFilesButton,2,1);

  qtarch_ButtonGroup_140 = new QButtonGroup( this, "ButtonGroup_140" );
  qtarch_ButtonGroup_140->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_140->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_140->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_140->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_140->setFrameStyle( 49 );
  qtarch_ButtonGroup_140->setTitle( i18n("Changed files") );
  qtarch_ButtonGroup_140->setAlignment( 1 );
  grid1->addMultiCellWidget( qtarch_ButtonGroup_140,1,2,0,0);


  grid2 = new QGridLayout(qtarch_ButtonGroup_140,2,1,15,7);

  qtarch_ButtonGroup_142 = new QButtonGroup(  qtarch_ButtonGroup_140, "ButtonGroup_142" );
  qtarch_ButtonGroup_142->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_142->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_142->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_142->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_142->setFrameStyle( 49 );
  qtarch_ButtonGroup_142->setTitle( i18n("Between") );
  qtarch_ButtonGroup_142->setAlignment( 1 );
  grid2->addWidget(qtarch_ButtonGroup_142,0,0);

  QGridLayout *grid3 = new QGridLayout(qtarch_ButtonGroup_142,2,5,15,7);

  qtarch_Label_77 = new QLabel( qtarch_ButtonGroup_142, "Label_77" );
  qtarch_Label_77->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_77->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_77->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_77->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_77->setText( i18n("date") );
  qtarch_Label_77->setAlignment( 289 );
  qtarch_Label_77->setMargin( -1 );
  grid3->addWidget(qtarch_Label_77,0,0);

  beginDateButton = new QPushButton( qtarch_ButtonGroup_142, "beginDateButton" );
  beginDateButton->setFocusPolicy( QWidget::TabFocus );
  beginDateButton->setBackgroundMode( QWidget::PaletteBackground );
  beginDateButton->setFontPropagation( QWidget::NoChildren );
  beginDateButton->setPalettePropagation( QWidget::NoChildren );
  beginDateButton->setText( "" );
  beginDateButton->setAutoRepeat( FALSE );
  beginDateButton->setAutoResize( FALSE );
  grid3->addMultiCellWidget(beginDateButton,0,0,1,4);

  qtarch_Label_80 = new QLabel( qtarch_ButtonGroup_142, "Label_80" );
  qtarch_Label_80->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_80->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_80->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_80->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_80->setText( i18n("time") );
  qtarch_Label_80->setAlignment( 289 );
  qtarch_Label_80->setMargin( -1 );
  grid3->addWidget(qtarch_Label_80,1,0);

  beginTimeHourLine = new QSpinBox(  qtarch_ButtonGroup_142, "beginTimeHourLine" );
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
  grid3->addWidget(beginTimeHourLine,1,1);


  qtarch_Label_88 = new QLabel(qtarch_ButtonGroup_142, "Label_88" );
  qtarch_Label_88->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_88->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_88->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_88->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_88->setText( ":" );
  qtarch_Label_88->setAlignment( 292 );
  qtarch_Label_88->setMargin( -1 );
  grid3->addWidget( qtarch_Label_88,1,2);



  beginTimeMinuteLine = new QSpinBox( qtarch_ButtonGroup_142, "beginTimeMinuteList" );
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
  grid3->addWidget(beginTimeMinuteLine,1,3);

  qtarch_Label_89 = new QLabel( qtarch_ButtonGroup_142 , "Label_89" );
  qtarch_Label_89->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_89->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_89->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_89->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_89->setText(i18n("o'clock"));
  qtarch_Label_89->setAlignment( 289 );
  qtarch_Label_89->setMargin( -1 );
  grid3->addWidget(qtarch_Label_89,1,4);


  qtarch_ButtonGroup_141 = new QButtonGroup(qtarch_ButtonGroup_140 , "ButtonGroup_141" );
  qtarch_ButtonGroup_141->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_141->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_141->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_141->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_141->setFrameStyle( 49 );
  qtarch_ButtonGroup_141->setTitle( i18n("And") );
  qtarch_ButtonGroup_141->setAlignment( 1 );

  grid2->addWidget(qtarch_ButtonGroup_141,1,0);


  grid3 = new QGridLayout(qtarch_ButtonGroup_141,2,5,15,7);
  qtarch_Label_83 = new QLabel(qtarch_ButtonGroup_141 , "Label_83" );
  qtarch_Label_83->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_83->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_83->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_83->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_83->setText( i18n("date") );
  qtarch_Label_83->setAlignment( 289 );
  qtarch_Label_83->setMargin( -1 );
  grid3->addWidget(qtarch_Label_83,0,0);

  beginTimeButton = new QPushButton(qtarch_ButtonGroup_141 , "beginTimeButton" );
  beginTimeButton->setFocusPolicy( QWidget::TabFocus );
  beginTimeButton->setBackgroundMode( QWidget::PaletteBackground );
  beginTimeButton->setFontPropagation( QWidget::NoChildren );
  beginTimeButton->setPalettePropagation( QWidget::NoChildren );
  beginTimeButton->setText( "" );
  beginTimeButton->setAutoRepeat( FALSE );
  beginTimeButton->setAutoResize( FALSE );
  grid3->addMultiCellWidget(beginTimeButton,0,0,1,4);


  qtarch_Label_84 = new QLabel(qtarch_ButtonGroup_141 , "Label_84" );
  qtarch_Label_84->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_84->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_84->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_84->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_84->setText( i18n("time") );
  qtarch_Label_84->setAlignment( 289 );
  qtarch_Label_84->setMargin( -1 );
  grid3->addWidget(qtarch_Label_84,1,0);

  endTimeHourLine = new QSpinBox( qtarch_ButtonGroup_141, "endTimeHourLine" );
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
  grid3->addWidget(endTimeHourLine,1,1);


  qtarch_Label_91 = new QLabel( qtarch_ButtonGroup_141, "Label_91" );
  qtarch_Label_91->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_91->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_91->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_91->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_91->setText( ":" );
  qtarch_Label_91->setAlignment( 292 );
  qtarch_Label_91->setMargin( -1 );
  grid3->addWidget(qtarch_Label_91,1,2);

  endTimeMinuteLine = new QSpinBox(  qtarch_ButtonGroup_141, "endTimeMinuteLine" );
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
  grid3->addWidget(endTimeMinuteLine,1,3);

  qtarch_Label_90 = new QLabel( qtarch_ButtonGroup_141, "Label_90" );
  qtarch_Label_90->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_90->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_90->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_90->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_90->setText(i18n("o'clock"));
  qtarch_Label_90->setAlignment( 289 );
  qtarch_Label_90->setMargin( -1 );
  grid3->addWidget( qtarch_Label_90,1,4);

  qtarch_ButtonGroup_143 = new QButtonGroup(this , "ButtonGroup_143" );
  qtarch_ButtonGroup_143->setGeometry( 350, 10, 230, 410 );
  qtarch_ButtonGroup_143->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_143->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_143->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_143->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_143->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_143->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_143->setFrameStyle( 49 );
  qtarch_ButtonGroup_143->setTitle( i18n("Self choosen files") );
  qtarch_ButtonGroup_143->setAlignment( 1 );

  grid1->addMultiCellWidget(qtarch_ButtonGroup_143,0,2,1,1);

  grid2 = new QGridLayout(qtarch_ButtonGroup_143,8,3,15,7);

  selfChoosenFilesMultiLine = new QListBox(qtarch_ButtonGroup_143 , "selfChoosenFilesMultiLine" );
  selfChoosenFilesMultiLine->setFocusPolicy( QWidget::StrongFocus );
  selfChoosenFilesMultiLine->setBackgroundMode( QWidget::PaletteBase );
  selfChoosenFilesMultiLine->setFontPropagation( QWidget::SameFont );
  selfChoosenFilesMultiLine->setPalettePropagation( QWidget::SameFont );
  selfChoosenFilesMultiLine->setFrameStyle( 51 );
  selfChoosenFilesMultiLine->setLineWidth( 2 );
  selfChoosenFilesMultiLine->setMultiSelection( TRUE );
  grid2->addMultiCellWidget(selfChoosenFilesMultiLine,0,6,0,2);

  selfChoosenFilesAddButton = new QPushButton( qtarch_ButtonGroup_143, "selfChoosenFilesAddButton" );
  selfChoosenFilesAddButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesAddButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesAddButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesAddButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesAddButton->setText( i18n("add") );
  selfChoosenFilesAddButton->setAutoRepeat( FALSE );
  selfChoosenFilesAddButton->setAutoResize( FALSE );
  grid2->addWidget(selfChoosenFilesAddButton,7,0);


  selfChoosenFilesDeleteButton = new QPushButton( qtarch_ButtonGroup_143, "selfChoosenFilesDeleteButton" );
  selfChoosenFilesDeleteButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesDeleteButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesDeleteButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesDeleteButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesDeleteButton->setText( i18n("delete") );
  selfChoosenFilesDeleteButton->setAutoRepeat( FALSE );
  selfChoosenFilesDeleteButton->setAutoResize( FALSE );
  grid2->addWidget(selfChoosenFilesDeleteButton,7,1);



  selfChoosenFilesCleanButton = new QPushButton( qtarch_ButtonGroup_143 , "selfChoosenFilesCleanButton" );
  selfChoosenFilesCleanButton->setFocusPolicy( QWidget::TabFocus );
  selfChoosenFilesCleanButton->setBackgroundMode( QWidget::PaletteBackground );
  selfChoosenFilesCleanButton->setFontPropagation( QWidget::NoChildren );
  selfChoosenFilesCleanButton->setPalettePropagation( QWidget::NoChildren );
  selfChoosenFilesCleanButton->setText( i18n("clear") );
  selfChoosenFilesCleanButton->setAutoRepeat( FALSE );
  selfChoosenFilesCleanButton->setAutoResize( FALSE );

  grid2->addWidget(selfChoosenFilesCleanButton,7,2);


  currentButton->setChecked(true);

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  okButton = bb->addButton( i18n("OK") );
  cancelButton = bb->addButton(i18n("Cancel"));
  bb->layout();
  grid1->addWidget( bb,3,1 );

  QWhatsThis::add(selfChoosenFilesMultiLine,
	i18n("Here are the files which will be printed, if 'self\n"
	     "choosen file' is checked."));

  QWhatsThis::add(selfChoosenFilesCleanButton,
	i18n("Cleans the 'self choosen file list'."));

  QWhatsThis::add(selfChoosenFilesAddButton,
	i18n("Adds the selected file to the 'self choosen file list'."));

  QWhatsThis::add(selfChoosenFilesDeleteButton,
	i18n("Deletes the selected files in the filelist."));

  QString selfChoosenFileLineMsg = i18n("Insert the filename and path to the file you\n"
				"want to add to the self-chosen file list.\n"
				"After insertion, press the 'add' button below\n"
				"the file list box.");
  QWhatsThis::add(qtarch_Label_79, selfChoosenFileLineMsg);
  QWhatsThis::add(selfChoosenFileLine, selfChoosenFileLineMsg);

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

  QString beginTimeButtonMsg = i18n("Sets the end date of the interval. Press the button\n"
			"to pick a date by the calendar. After selection, the\n"
			"chosen end-date will be displayed on the button.");
  QWhatsThis::add(qtarch_Label_83, beginTimeButtonMsg);
  QWhatsThis::add(beginTimeButton, beginTimeButtonMsg);

  QString beginDateButtonMsg = i18n("Sets the beginning date of the interval. Press the button\n"
			"to pick a date by the calendar. After selection, the\n"
			"chosen begin-date will be displayed on the button.");
  QWhatsThis::add(qtarch_Label_77, beginDateButtonMsg);
  QWhatsThis::add(beginDateButton, beginDateButtonMsg);

  QWhatsThis::add(qtarch_Label_89,
	i18n("Sets the minute value of the beginning time."));
  QWhatsThis::add(beginTimeMinuteLine,
	i18n("Sets the minute value of the beginning time."));

  QWhatsThis::add(qtarch_Label_80,
	i18n("Sets the hour value of the beginning time."));
  QWhatsThis::add(beginTimeHourLine,
	i18n("Sets the hour value of the beginning time."));

  QWhatsThis::add(qtarch_Label_84,
	i18n("Sets the hour value of the end time."));
  QWhatsThis::add(endTimeHourLine,
	i18n("Sets the hour value of the end time."));

  QWhatsThis::add(qtarch_Label_90,
	i18n("Sets the minute value of the end time."));
  QWhatsThis::add(endTimeMinuteLine,
	i18n("Sets the minute value of the end time."));

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

  beginDateButton->setText(KGlobal::locale()->formatDate(QDate::currentDate()));

  beginTimeButton->setText(KGlobal::locale()->formatDate(QDate::currentDate()));
  beginTimeHourLine->setValue(QTime::currentTime().hour());
  beginTimeMinuteLine->setValue(QTime::currentTime().minute());
  endTimeHourLine->setValue(QTime::currentTime().hour());
  endTimeMinuteLine->setValue(QTime::currentTime().minute());
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
  if(!text.isEmpty())
    beginDateButton->setText(text);

}


void CFilePrintDlg::slotEndDateDlgClicked() {
  QString text="";
  CDatepikerDlg datepik;
  datepik.setCaption(i18n("EndDate"));
  datepik.exec();
  datepik.getDateString(text);
  datepik.getDate(endDate);
  if(!text.isEmpty())
    beginTimeButton->setText(text);
}
void CFilePrintDlg::slotSelfChoosenFilesActive(int number)
{
  // ignore this button
  if (number==ID_selfChoosenFilesPushButton)
    return;

  if (number==ID_currentButton)
  {
    if (currentButton->isChecked()) {
      slotSelfChoosenFilesActive(ID_changedFilesButton);
    }
    else if (cppFilesButton->isChecked()) {
      slotSelfChoosenFilesActive(ID_cppFilesButton);
    }
    else if (headerFilesButton->isChecked()) {
      slotSelfChoosenFilesActive(ID_headerFilesButton);
    }
    else if (changedFilesButton->isChecked()) {
      slotSelfChoosenFilesActive(ID_changedFilesButton);
    }
    else if (allInProjectButton->isChecked()) {
      slotSelfChoosenFilesActive(ID_allInProjectButton);
    }
    else slotSelfChoosenFilesActive(ID_selfChoosenFilesButton);
  }
  else if (number==ID_selfChoosenFilesButton)
  {
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
  else if (number==ID_changedFilesButton)
  {
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
  else
  {
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
  selfChoosenFileLine->setText(KFileDialog::getOpenFileName());
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
  settings = KGlobal::config();
  settings->setGroup("FileDialog");

  int i=ID_currentButton;
  if (currentButton->isChecked()) {
    i=ID_currentButton;
  }
  else if (cppFilesButton->isChecked()) {
    i=ID_cppFilesButton;
  }
  else if (headerFilesButton->isChecked()) {
    i=ID_headerFilesButton;
  }
  else if (changedFilesButton->isChecked()) {
    i=ID_changedFilesButton;
  }
  else if (allInProjectButton->isChecked()) {
    i=ID_allInProjectButton;
  }
  else i=ID_selfChoosenFilesButton;
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
      KMessageBox::error(0,i18n("You need to select at least one file for printing. "
                                "This can be done by entering the filename on the left"
                                " at the file entry field."),
                            i18n("No File to print !"));
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
    kdDebug() << "Start output" << endl;
    kdDebug() << sources << endl;
    kdDebug() << "End Output" << endl;
  }
  settings->sync();
  reject();
}

void CFilePrintDlg::loadSettings() {
  selfChoosenFilesMultiLine->clear();
  fileList.clear();
  settings = KGlobal::config();
  settings->setGroup("FileDialog");
  qtarch_ButtonGroup_139->setButton(settings->readNumEntry("FileSelection"));
  selfChoosenFileLine->setText(settings->readEntry("FileLine"));
  settings->readListEntry("FileList",fileList);
  selfChoosenFilesMultiLine->insertStrList(&fileList);
}

#include "cfileprintdlg.moc"
