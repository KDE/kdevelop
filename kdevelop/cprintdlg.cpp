/***************************************************************************
                          cprintdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Feb 4 1999                                           
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


#include "cprintdlg.h"
#include "cfileprintdlg.h"
#include <qpixmap.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <iostream.h>
#include <string.h>
#include <kapp.h>

CPrintDlg::CPrintDlg(QWidget* parent,const char* name) : QDialog(parent, name, true){
  init();
  slotProgramActivated(0);
  slotPrintToFileClicked(false);
  slotPrettyPrintClicked(false);
}

CPrintDlg::~CPrintDlg(){
}


void CPrintDlg::init(){
  QWidget *mainwidget = new QWidget(this,"printdialog");
  mainwidget->resize (600,480);
  QWidget *printwidget = new QWidget(this,"printing");
  printwidget->resize(600,430);

  qtarch_ButtonGroup_35 = new QButtonGroup( printwidget, "ButtonGroup_35" );
  qtarch_ButtonGroup_35->setGeometry( 20, 280, 140, 60 );
  qtarch_ButtonGroup_35->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_35->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_35->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_35->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_35->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_35->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_35->setFrameStyle( 49 );
  qtarch_ButtonGroup_35->setTitle(i18n( "Outputmedia" ));
  qtarch_ButtonGroup_35->setAlignment( 1 );
  
  QButtonGroup* qtarch_ButtonGroup_8;
  qtarch_ButtonGroup_8 = new QButtonGroup( printwidget, "ButtonGroup_8" );
  qtarch_ButtonGroup_8->setGeometry( 320, 90, 260, 330 );
  qtarch_ButtonGroup_8->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_8->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_8->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_8->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_8->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_8->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_8->setFrameStyle( 49 );
  qtarch_ButtonGroup_8->setTitle(i18n( "Outprinting" ));
  qtarch_ButtonGroup_8->setAlignment( 1 );
  
  qtarch_ButtonGroup_9 = new QButtonGroup( printwidget, "ButtonGroup_9" );
  qtarch_ButtonGroup_9->setGeometry( 330, 290, 240, 120 );
  qtarch_ButtonGroup_9->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_9->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_9->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_9->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_9->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_9->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_9->setFrameStyle( 49 );
  qtarch_ButtonGroup_9->setTitle(i18n( "Pretty-print" ));
  qtarch_ButtonGroup_9->setAlignment( 1 );
  
  qtarch_ButtonGroup_34 = new QButtonGroup( printwidget, "ButtonGroup_34" );
  qtarch_ButtonGroup_34->setGeometry( 350, 340, 200, 60 );
  qtarch_ButtonGroup_34->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_34->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_34->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_34->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_34->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_34->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_34->setFrameStyle( 49 );
  qtarch_ButtonGroup_34->setTitle(i18n( "Pretty-print mode" ));
  qtarch_ButtonGroup_34->setAlignment( 1 );
  
  qtarch_ButtonGroup_11 = new QButtonGroup( printwidget, "ButtonGroup_11" );
  qtarch_ButtonGroup_11->setGeometry( 330, 220, 240, 60 );
  qtarch_ButtonGroup_11->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_11->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_11->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_11->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_11->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_11->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_11->setFrameStyle( 49 );
  qtarch_ButtonGroup_11->setTitle(i18n( "Pages" ));
  qtarch_ButtonGroup_11->setAlignment( 1 );
  
  QButtonGroup* qtarch_ButtonGroup_10;
  qtarch_ButtonGroup_10 = new QButtonGroup( printwidget, "ButtonGroup_10" );
  qtarch_ButtonGroup_10->setGeometry( 330, 110, 240, 100 );
  qtarch_ButtonGroup_10->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_10->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_10->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_10->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_10->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_10->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_10->setFrameStyle( 49 );
  qtarch_ButtonGroup_10->setTitle(i18n( "Page printing" ));
  qtarch_ButtonGroup_10->setAlignment( 1 );
  
  QButtonGroup* qtarch_ButtonGroup_7;
  qtarch_ButtonGroup_7 = new QButtonGroup( printwidget, "ButtonGroup_7" );
  qtarch_ButtonGroup_7->setGeometry( 170, 200, 140, 60 );
  qtarch_ButtonGroup_7->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_7->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_7->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_7->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_7->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_7->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_7->setFrameStyle( 49 );
  qtarch_ButtonGroup_7->setTitle(i18n( "Copy" ));
  qtarch_ButtonGroup_7->setAlignment( 1 );
  
  QButtonGroup* qtarch_ButtonGroup_5;
  qtarch_ButtonGroup_5 = new QButtonGroup( printwidget, "ButtonGroup_5" );
  qtarch_ButtonGroup_5->setGeometry( 130, 10, 210, 60 );
  qtarch_ButtonGroup_5->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_5->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_5->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_5->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_5->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_5->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_5->setFrameStyle( 49 );
  qtarch_ButtonGroup_5->setTitle(i18n( "Printer" ));
  qtarch_ButtonGroup_5->setAlignment( 1 );
  
  QButtonGroup* qtarch_ButtonGroup_6;
  qtarch_ButtonGroup_6 = new QButtonGroup( printwidget, "ButtonGroup_6" );
  qtarch_ButtonGroup_6->setGeometry( 20, 90, 290, 90 );
  qtarch_ButtonGroup_6->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_6->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_6->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_6->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_6->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_6->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_6->setFrameStyle( 49 );
  qtarch_ButtonGroup_6->setTitle(i18n( "Outputlocation" ));
  qtarch_ButtonGroup_6->setAlignment( 1 );
  
  QButtonGroup* qtarch_ButtonGroup_4;
  qtarch_ButtonGroup_4 = new QButtonGroup( printwidget, "ButtonGroup_4" );
  qtarch_ButtonGroup_4->setGeometry( 20, 360, 290, 60 );
  qtarch_ButtonGroup_4->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_4->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_4->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_4->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_4->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_4->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_4->setFrameStyle( 49 );
  qtarch_ButtonGroup_4->setTitle(i18n( "Default printsettings" ));
  qtarch_ButtonGroup_4->setAlignment( 1 );
  
  //QButtonGroup* qtarch_ButtonGroup_3;
  qtarch_ButtonGroup_3 = new QButtonGroup( printwidget, "ButtonGroup_3" );
  qtarch_ButtonGroup_3->setGeometry( 170, 280, 140, 60 );
  qtarch_ButtonGroup_3->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_3->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_3->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_3->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_3->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_3->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_3->setFrameStyle( 49 );
  qtarch_ButtonGroup_3->setTitle(i18n( "Outputformat" ));
  qtarch_ButtonGroup_3->setAlignment( 1 );
  
  QButtonGroup* qtarch_ButtonGroup_2;
  qtarch_ButtonGroup_2 = new QButtonGroup( printwidget, "ButtonGroup_2" );
  qtarch_ButtonGroup_2->setGeometry( 20, 10, 100, 60 );
  qtarch_ButtonGroup_2->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_2->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_2->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_2->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_2->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_2->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_2->setFrameStyle( 49 );
  qtarch_ButtonGroup_2->setTitle(i18n( "Program" ));
  qtarch_ButtonGroup_2->setAlignment( 1 );
  
  paperFormatGroup = new QButtonGroup( printwidget, "PaperFormatGroup" );
  paperFormatGroup->setGeometry( 20, 200, 140, 60 );
  paperFormatGroup->setMinimumSize( 0, 0 );
  paperFormatGroup->setMaximumSize( 32767, 32767 );
  paperFormatGroup->setFocusPolicy( QWidget::NoFocus );
  paperFormatGroup->setBackgroundMode( QWidget::PaletteBackground );
  paperFormatGroup->setFontPropagation( QWidget::NoChildren );
  paperFormatGroup->setPalettePropagation( QWidget::NoChildren );
  paperFormatGroup->setFrameStyle( 49 );
  paperFormatGroup->setTitle(i18n( "Paper format" ));
  paperFormatGroup->setAlignment( 1 );
  
  printingConfButton = new QPushButton( printwidget, "printingConfButton" );
  printingConfButton->setGeometry( 470, 30, 110, 30 );
  printingConfButton->setMinimumSize( 0, 0 );
  printingConfButton->setMaximumSize( 32767, 32767 );
  printingConfButton->setFocusPolicy( QWidget::TabFocus );
  printingConfButton->setBackgroundMode( QWidget::PaletteBackground );
  printingConfButton->setFontPropagation( QWidget::NoChildren );
  printingConfButton->setPalettePropagation( QWidget::NoChildren );
  printingConfButton->setText(i18n( "Options..." ));
  printingConfButton->setAutoRepeat( FALSE );
  printingConfButton->setAutoResize( FALSE );
  connect (printingConfButton,SIGNAL(clicked()),SLOT(slotPrintingConfClicked()));  

  printToFileLine = new QLineEdit( printwidget, "printToFileLine" );
  printToFileLine->setGeometry( 30, 140, 230, 30 );
  printToFileLine->setMinimumSize( 0, 0 );
  printToFileLine->setMaximumSize( 32767, 32767 );
  printToFileLine->setFocusPolicy( QWidget::StrongFocus );
  printToFileLine->setBackgroundMode( QWidget::PaletteBase );
  printToFileLine->setFontPropagation( QWidget::NoChildren );
  printToFileLine->setPalettePropagation( QWidget::NoChildren );
  printToFileLine->setText( "" );
  printToFileLine->setMaxLength( 32767 );
  printToFileLine->setEchoMode( QLineEdit::Normal );
  printToFileLine->setFrame( TRUE );
  
  paperCombBox = new QComboBox( FALSE, printwidget, "paperCombBox" );
  paperCombBox->setGeometry( 30, 220, 120, 30 );
  paperCombBox->setMinimumSize( 0, 0 );
  paperCombBox->setMaximumSize( 32767, 32767 );
  paperCombBox->setFocusPolicy( QWidget::StrongFocus );
  paperCombBox->setBackgroundMode( QWidget::PaletteBackground );
  paperCombBox->setFontPropagation( QWidget::NoChildren );
  paperCombBox->setPalettePropagation( QWidget::NoChildren );
  paperCombBox->setSizeLimit( 10 );
  paperCombBox->setAutoResize( FALSE );
  paperCombBox->insertItem(i18n("portrait"));
  paperCombBox->insertItem(i18n("landscape"));
  
  programCombBox = new QComboBox( FALSE, printwidget, "programCombBox" );
  programCombBox->setGeometry( 30, 30, 80, 30 );
  programCombBox->setMinimumSize( 0, 0 );
  programCombBox->setMaximumSize( 32767, 32767 );
  programCombBox->setFocusPolicy( QWidget::StrongFocus );
  programCombBox->setBackgroundMode( QWidget::PaletteBackground );
  programCombBox->setFontPropagation( QWidget::NoChildren );
  programCombBox->setPalettePropagation( QWidget::NoChildren );
  programCombBox->setSizeLimit( 10 );
  programCombBox->setAutoResize( FALSE );
  programCombBox->insertItem( "a2ps" );
  programCombBox->insertItem( "enscript" );
  connect(programCombBox,SIGNAL(activated (int)),SLOT(slotProgramActivated(int)));
  
  QLabel* qtarch_Label_1;
  qtarch_Label_1 = new QLabel( printwidget, "Label_1" );
  qtarch_Label_1->setGeometry( 140, 30, 70, 30 );
  qtarch_Label_1->setMinimumSize( 0, 0 );
  qtarch_Label_1->setMaximumSize( 32767, 32767 );
  qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_1->setText(i18n( "printername" ));
  qtarch_Label_1->setAlignment( 289 );
  qtarch_Label_1->setMargin( -1 );
  
  printerLine = new QLineEdit( printwidget, "printerLine" );
  printerLine->setGeometry( 210, 30, 120, 30 );
  printerLine->setMinimumSize( 0, 0 );
  printerLine->setMaximumSize( 32767, 32767 );
  printerLine->setFocusPolicy( QWidget::StrongFocus );
  printerLine->setBackgroundMode( QWidget::PaletteBase );
  printerLine->setFontPropagation( QWidget::NoChildren );
  printerLine->setPalettePropagation( QWidget::NoChildren );
  printerLine->setText( "lpr" );
  printerLine->setMaxLength( 32767 );
  printerLine->setEchoMode( QLineEdit::Normal );
  printerLine->setFrame( TRUE );
  
  printToFileDlg = new QPushButton( printwidget, "PrintToFileDlg" );
  printToFileDlg->setGeometry( 270, 140, 30, 30 );
  printToFileDlg->setMinimumSize( 0, 0 );
  printToFileDlg->setMaximumSize( 32767, 32767 );
  printToFileDlg->setFocusPolicy( QWidget::TabFocus );
  printToFileDlg->setBackgroundMode( QWidget::PaletteBackground );
  printToFileDlg->setFontPropagation( QWidget::NoChildren );
  printToFileDlg->setPalettePropagation( QWidget::NoChildren );
  printToFileDlg->setText( "..." );
  printToFileDlg->setAutoRepeat( FALSE );
  printToFileDlg->setAutoResize( FALSE );
  connect (printToFileDlg,SIGNAL(clicked()),SLOT(slotPrintToFileDlgClicked()));
  
  formatCombBox = new QComboBox( FALSE, printwidget, "formatCombBox" );
  formatCombBox->setGeometry( 180, 300, 120, 30 );
  formatCombBox->setMinimumSize( 0, 0 );
  formatCombBox->setMaximumSize( 32767, 32767 );
  formatCombBox->setFocusPolicy( QWidget::StrongFocus );
  formatCombBox->setBackgroundMode( QWidget::PaletteBackground );
  formatCombBox->setFontPropagation( QWidget::NoChildren );
  formatCombBox->setPalettePropagation( QWidget::NoChildren );
  formatCombBox->setSizeLimit( 10 );
  formatCombBox->setAutoResize( FALSE );
  formatCombBox->insertItem( "postscript" );
  formatCombBox->insertItem( "html" );
  
  defaultCombBox = new QComboBox( TRUE, printwidget, "defaultCombBox" );
  defaultCombBox->setGeometry( 30, 380, 270, 30 );
  defaultCombBox->setMinimumSize( 0, 0 );
  defaultCombBox->setMaximumSize( 32767, 32767 );
  defaultCombBox->setFocusPolicy( QWidget::StrongFocus );
  defaultCombBox->setBackgroundMode( QWidget::PaletteBackground );
  defaultCombBox->setFontPropagation( QWidget::NoChildren );
  defaultCombBox->setPalettePropagation( QWidget::NoChildren );
  defaultCombBox->setInsertionPolicy( QComboBox::AtBottom );
  defaultCombBox->setSizeLimit( 10 );
  defaultCombBox->setAutoResize( FALSE );
  defaultCombBox->insertItem(i18n("settings"));
  
  copySpinBox = new QSpinBox( printwidget, "copySpinBox" );
  copySpinBox->setGeometry( 180, 220, 120, 30 );
  copySpinBox->setMinimumSize( 0, 0 );
  copySpinBox->setMaximumSize( 32767, 32767 );
  copySpinBox->setFocusPolicy( QWidget::StrongFocus );
  copySpinBox->setBackgroundMode( QWidget::PaletteBackground );
  copySpinBox->setFontPropagation( QWidget::NoChildren );
  copySpinBox->setPalettePropagation( QWidget::NoChildren );
  copySpinBox->setFrameStyle( 50 );
  copySpinBox->setLineWidth( 2 );
  copySpinBox->setRange( 1, 99 );
  copySpinBox->setSteps( 1, 0 );
  copySpinBox->setPrefix( "" );
  copySpinBox->setSuffix( "" );
  copySpinBox->setSpecialValueText( "" );
  copySpinBox->setWrapping( FALSE );
  
  prettyPrintCheckBox = new QCheckBox( printwidget, "prettyPrintCheckBox" );
  prettyPrintCheckBox->setGeometry( 350, 310, 110, 30 );
  prettyPrintCheckBox->setMinimumSize( 0, 0 );
  prettyPrintCheckBox->setMaximumSize( 32767, 32767 );
  prettyPrintCheckBox->setFocusPolicy( QWidget::TabFocus );
  prettyPrintCheckBox->setBackgroundMode( QWidget::PaletteBackground );
  prettyPrintCheckBox->setFontPropagation( QWidget::NoChildren );
  prettyPrintCheckBox->setPalettePropagation( QWidget::NoChildren );
  prettyPrintCheckBox->setText(i18n( "pretty-print" ));
  prettyPrintCheckBox->setAutoRepeat( FALSE );
  prettyPrintCheckBox->setAutoResize( FALSE );
  connect (prettyPrintCheckBox,SIGNAL(toggled(bool)),SLOT(slotPrettyPrintClicked(bool)));
  
  prettyColorCheckBox = new QCheckBox( printwidget, "prettyColorCheckBox" );
  prettyColorCheckBox->setGeometry( 480, 310, 70, 30 );
  prettyColorCheckBox->setMinimumSize( 0, 0 );
  prettyColorCheckBox->setMaximumSize( 32767, 32767 );
  prettyColorCheckBox->setFocusPolicy( QWidget::TabFocus );
  prettyColorCheckBox->setBackgroundMode( QWidget::PaletteBackground );
  prettyColorCheckBox->setFontPropagation( QWidget::NoChildren );
  prettyColorCheckBox->setPalettePropagation( QWidget::NoChildren );
  prettyColorCheckBox->setText(i18n( "color" ));
  prettyColorCheckBox->setAutoRepeat( FALSE );
  prettyColorCheckBox->setAutoResize( FALSE );
  
  pageSide = new QComboBox( TRUE, printwidget, "pageSide" );
  pageSide->setGeometry( 340, 240, 220, 30 );
  pageSide->setMinimumSize( 0, 0 );
  pageSide->setMaximumSize( 32767, 32767 );
  pageSide->setFocusPolicy( QWidget::StrongFocus );
  pageSide->setBackgroundMode( QWidget::PaletteBackground );
  pageSide->setFontPropagation( QWidget::NoChildren );
  pageSide->setPalettePropagation( QWidget::NoChildren );
  pageSide->setInsertionPolicy( QComboBox::NoInsertion );
  pageSide->setSizeLimit( 10 );
  pageSide->setAutoResize( FALSE );
  pageSide->insertItem(i18n("odd"));
  pageSide->insertItem(i18n("even"));
  pageSide->insertItem(i18n("first half"));
  pageSide->insertItem(i18n("second half"));
  
  sidePerPage = new QComboBox( FALSE, printwidget, "sidePerPage" );
  sidePerPage->setGeometry( 340, 130, 220, 30 );
  sidePerPage->setMinimumSize( 0, 0 );
  sidePerPage->setMaximumSize( 32767, 32767 );
  sidePerPage->setFocusPolicy( QWidget::StrongFocus );
  sidePerPage->setBackgroundMode( QWidget::PaletteBackground );
  sidePerPage->setFontPropagation( QWidget::NoChildren );
  sidePerPage->setPalettePropagation( QWidget::NoChildren );
  sidePerPage->setSizeLimit( 10 );
  sidePerPage->setAutoResize( FALSE );
  sidePerPage->insertItem(i18n( "one side of paper" ));
  sidePerPage->insertItem(i18n( "two side of paper" ));

  pagePerSide = new QComboBox( FALSE, printwidget, "pagePerSide" );
  pagePerSide->setGeometry( 340, 170, 220, 30 );
  pagePerSide->setMinimumSize( 0, 0 );
  pagePerSide->setMaximumSize( 32767, 32767 );
  pagePerSide->setFocusPolicy( QWidget::StrongFocus );
  pagePerSide->setBackgroundMode( QWidget::PaletteBackground );
  pagePerSide->setFontPropagation( QWidget::NoChildren );
  pagePerSide->setPalettePropagation( QWidget::NoChildren );
  pagePerSide->setSizeLimit( 10 );
  pagePerSide->setAutoResize( FALSE );
  pagePerSide->insertItem(i18n( "one page per side" ));
  pagePerSide->insertItem(i18n( "two pages per side" ));
  
  filesConfButton = new QPushButton( printwidget, "filesConfButton" );
  filesConfButton->setGeometry( 350, 30, 110, 30 );
  filesConfButton->setMinimumSize( 0, 0 );
  filesConfButton->setMaximumSize( 32767, 32767 );
  filesConfButton->setFocusPolicy( QWidget::TabFocus );
  filesConfButton->setBackgroundMode( QWidget::PaletteBackground );
  filesConfButton->setFontPropagation( QWidget::NoChildren );
  filesConfButton->setPalettePropagation( QWidget::NoChildren );
  filesConfButton->setText(i18n( "Files..." ));
  filesConfButton->setAutoRepeat( FALSE );
  filesConfButton->setAutoResize( FALSE );
  connect (filesConfButton, SIGNAL(clicked()),SLOT(slotFilesConfClicked()));
  
  prettyCombBox = new QComboBox( FALSE, printwidget, "prettyCombBox" );
  prettyCombBox->setGeometry( 360, 360, 180, 30 );
  prettyCombBox->setMinimumSize( 0, 0 );
  prettyCombBox->setMaximumSize( 32767, 32767 );
  prettyCombBox->setFocusPolicy( QWidget::StrongFocus );
  prettyCombBox->setBackgroundMode( QWidget::PaletteBackground );
  prettyCombBox->setFontPropagation( QWidget::NoChildren );
  prettyCombBox->setPalettePropagation( QWidget::NoChildren );
  prettyCombBox->setSizeLimit( 40 );
  prettyCombBox->setAutoResize( FALSE );
  prettyCombBox->insertItem( "Ada95" );
  prettyCombBox->insertItem( "AWK" );
  prettyCombBox->insertItem( "C" );
  prettyCombBox->insertItem( "ChangeLog" );
  prettyCombBox->insertItem( "C++" );
  prettyCombBox->insertItem( "normal diff" );
  prettyCombBox->insertItem( "unified diff" );
  prettyCombBox->insertItem( "Delphi" );
  prettyCombBox->insertItem( "LISP" );
  prettyCombBox->insertItem( "Fortran77" );
  prettyCombBox->insertItem( "Haskell" );
  prettyCombBox->insertItem( "HTML" );
  prettyCombBox->insertItem( "Java" );
  prettyCombBox->insertItem( "JavaScript" );
  prettyCombBox->insertItem( "Mail & News" );
  prettyCombBox->insertItem( "Makefile" );
  prettyCombBox->insertItem( "nroff" );
  prettyCombBox->insertItem( "Objective-C" );
  prettyCombBox->insertItem( "Pascal" );
  prettyCombBox->insertItem( "Perl" );
  prettyCombBox->insertItem( "PostScript" );
  prettyCombBox->insertItem( "Scheme" );
  prettyCombBox->insertItem( "Bourne shell" );
  prettyCombBox->insertItem( "SQL" );
  prettyCombBox->insertItem( "States" );
  prettyCombBox->insertItem( "Synopsys dc shell" );
  prettyCombBox->insertItem( "Tcl" );
  prettyCombBox->insertItem( "Verilog" );
  prettyCombBox->insertItem( "VHDLt" );
  
  mediaCombBox = new QComboBox( FALSE, printwidget, "mediaCombBox" );
  mediaCombBox->setGeometry( 30, 300, 120, 30 );
  mediaCombBox->setMinimumSize( 0, 0 );
  mediaCombBox->setMaximumSize( 32767, 32767 );
  mediaCombBox->setFocusPolicy( QWidget::StrongFocus );
  mediaCombBox->setBackgroundMode( QWidget::PaletteBackground );
  mediaCombBox->setFontPropagation( QWidget::NoChildren );
  mediaCombBox->setPalettePropagation( QWidget::NoChildren );
  mediaCombBox->setSizeLimit( 10 );
  mediaCombBox->setAutoResize( FALSE );
  mediaCombBox->insertItem( "a4" );
  mediaCombBox->insertItem( "a3" );
  mediaCombBox->insertItem( "a5" );
  mediaCombBox->insertItem( "a6" );
  mediaCombBox->insertItem( "letter" );
  
  printToFileButton = new QCheckBox( printwidget, "printToFileButton" );
  printToFileButton->setGeometry( 30, 110, 270, 30 );
  printToFileButton->setMinimumSize( 0, 0 );
  printToFileButton->setMaximumSize( 32767, 32767 );
  printToFileButton->setFocusPolicy( QWidget::TabFocus );
  printToFileButton->setBackgroundMode( QWidget::PaletteBackground );
  printToFileButton->setFontPropagation( QWidget::NoChildren );
  printToFileButton->setPalettePropagation( QWidget::NoChildren );
  printToFileButton->setText(i18n( "print to file" ));
  printToFileButton->setAutoRepeat( FALSE );
  printToFileButton->setAutoResize( FALSE );
  connect (printToFileButton,SIGNAL(toggled(bool)),SLOT(slotPrintToFileClicked(bool)));
  
  qtarch_ButtonGroup_8->insert( prettyPrintCheckBox );
  qtarch_ButtonGroup_8->insert( prettyColorCheckBox );
  qtarch_ButtonGroup_9->insert( prettyPrintCheckBox );
  qtarch_ButtonGroup_9->insert( prettyColorCheckBox );
  qtarch_ButtonGroup_6->insert( printToFileDlg );
  qtarch_ButtonGroup_6->insert( printToFileButton );

  okButton = new QPushButton( mainwidget, "okButton" );
  okButton->setText(("Ok"));
  okButton->setGeometry( 20, 440, 100, 30 );
  cancelButton = new QPushButton( mainwidget, "cancelButton" );
  cancelButton->setText(i18n("Cancel"));
  cancelButton->setGeometry( 140, 440, 100, 30 );
  connect(cancelButton,SIGNAL(clicked()),SLOT(slotCancelClicked()));

  mainwidget->show();
}

void CPrintDlg::slotProgramActivated(int i) {
  if (i==1) 
    {
      mediaCombBox->setEnabled(true);
      prettyPrintCheckBox->setEnabled(true);
      slotPrettyPrintClicked(false);
      formatCombBox->setEnabled(true);
      pageSide->setEnabled(true);
      qtarch_ButtonGroup_35->setEnabled(true);
      qtarch_ButtonGroup_9->setEnabled(true);
      qtarch_ButtonGroup_11->setEnabled(true);
      qtarch_ButtonGroup_3->setEnabled(true);
      int j =defaultCombBox->count();
      int state=0;
      for (int a=0;a<j;a++) {
	if (!(strcmp(defaultCombBox->text(a),i18n("unix manual output")))) {
	  defaultCombBox->removeItem(a);
	  a--;
	  j--;
	}
	if (!(strcmp(defaultCombBox->text(a),i18n("lineprinter")))) {
	  state++;
	}
      }
      if (state == 0) {
      defaultCombBox->insertItem(i18n("lineprinter"));
      }
    }
  else 
    { 
      mediaCombBox->setEnabled(false);
      prettyPrintCheckBox->setEnabled(false);
      slotPrettyPrintClicked(false);
      formatCombBox->setEnabled(false);
      pageSide->setEnabled(false);
      qtarch_ButtonGroup_35->setEnabled(false);
      qtarch_ButtonGroup_9->setEnabled(false);
      qtarch_ButtonGroup_11->setEnabled(false);
      qtarch_ButtonGroup_3->setEnabled(false);
      int j =defaultCombBox->count();
      int state=0;
      for (int a=0;a<j;a++) {
	if (!(strcmp(defaultCombBox->text(a),i18n("lineprinter")))) {
	  defaultCombBox->removeItem(a);
	  a--;
	  j--;
	}
	if (!(strcmp(defaultCombBox->text(a),i18n("unix manual output")))) {
	  state++;
	}
      }
      if (state == 0) {
      defaultCombBox->insertItem(i18n("unix manual output"));
      }
    }
}

void CPrintDlg::slotPrettyPrintClicked(bool status) {
  if (status) 
    {
      prettyCombBox->setEnabled(true);
      prettyColorCheckBox->setEnabled(true);
      qtarch_ButtonGroup_34->setEnabled(true);
    }
  else
    {
      prettyCombBox->setEnabled(false);
      prettyColorCheckBox->setEnabled(false);
      prettyPrintCheckBox->setChecked(false);
      prettyColorCheckBox->setChecked(false);
      qtarch_ButtonGroup_34->setEnabled(false);
    }
}

void CPrintDlg::slotPrintToFileClicked(bool status) {
  if (status) 
    {
      printToFileDlg->setEnabled(true);
      printToFileLine->setEnabled(true);
    }
  else
    {
      printToFileDlg->setEnabled(false);
      printToFileLine->setEnabled(false);
    }
}

void CPrintDlg::slotCancelClicked() {
  reject();
}

void CPrintDlg::slotFilesConfClicked() {
 CFilePrintDlg *fileconf = new CFilePrintDlg(this, "filedialog");
 fileconf->exec(); 
}

void CPrintDlg::slotPrintingConfClicked() {
  int prog=programCombBox->currentItem();
  if (prog==0) {
    printconf = new CConfigPrintDlg(this, "confdialog",1);
  }
  else
    if (prog==1) {
      printconf = new CConfigPrintDlg(this, "confdialog",2);
    }
  printconf->resize(610,510);
  printconf->setCancelButton();
  printconf->setDefaultButton();
  printconf->setApplyButton(i18n("Preview"));
  printconf->exec(); 
}

void CPrintDlg::slotPrintToFileDlgClicked() {
  printToFileLine->setText(KFileDialog::getOpenFileName());
}
