/***************************************************************************
                          cconfigena2psdlg.cpp  -  description                       
                             -------------------                                   
    begin                : Thu Feb 22 1999                                        
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

#include "cconfiga2psdlg.h"
#include <kapp.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <iostream.h>
#include "ctoolclass.h"

CConfigA2psDlg::CConfigA2psDlg(QWidget* parent,const char* name) : QDialog(parent, name, true){
  init();
  slotDefaultClicked();
}

CConfigA2psDlg::~CConfigA2psDlg(){
}

void CConfigA2psDlg::init() {
  qtarch_ButtonGroup_177 = new QButtonGroup( this, "ButtonGroup_177" );
  qtarch_ButtonGroup_177->setGeometry( 20, 10, 270, 260 );
  qtarch_ButtonGroup_177->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_177->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_177->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_177->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_177->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_177->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_177->setFrameStyle( 49 );
  qtarch_ButtonGroup_177->setTitle( "Header settings" );
  qtarch_ButtonGroup_177->setAlignment( 1 );
  
  qtarch_ButtonGroup_21 = new QButtonGroup( this, "ButtonGroup_21" );
  qtarch_ButtonGroup_21->setGeometry( 300, 10, 280, 220 );
  qtarch_ButtonGroup_21->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_21->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_21->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_21->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_21->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_21->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_21->setFrameStyle( 49 );
  qtarch_ButtonGroup_21->setTitle( "Textprinting" );
  qtarch_ButtonGroup_21->setAlignment( 1 );
  
  boltFontButton = new QCheckBox( this, "boltFontButton" );
  boltFontButton->setGeometry( 310, 190, 100, 30 );
  boltFontButton->setMinimumSize( 0, 0 );
  boltFontButton->setMaximumSize( 32767, 32767 );
  boltFontButton->setFocusPolicy( QWidget::TabFocus );
  boltFontButton->setBackgroundMode( QWidget::PaletteBackground );
  boltFontButton->setFontPropagation( QWidget::NoChildren );
  boltFontButton->setPalettePropagation( QWidget::NoChildren );
  boltFontButton->setText( "bold font" );
  boltFontButton->setAutoRepeat( FALSE );
  boltFontButton->setAutoResize( FALSE );
  
  printAsISOLatin = new QCheckBox( this, "printAsISOLatin" );
  printAsISOLatin->setGeometry( 310, 150, 250, 30 );
  printAsISOLatin->setMinimumSize( 0, 0 );
  printAsISOLatin->setMaximumSize( 32767, 32767 );
  printAsISOLatin->setFocusPolicy( QWidget::TabFocus );
  printAsISOLatin->setBackgroundMode( QWidget::PaletteBackground );
  printAsISOLatin->setFontPropagation( QWidget::NoChildren );
  printAsISOLatin->setPalettePropagation( QWidget::NoChildren );
  printAsISOLatin->setText( "print non ascii character as ISO Latin 1" );
  printAsISOLatin->setAutoRepeat( FALSE );
  printAsISOLatin->setAutoResize( FALSE );
  
  interpretButton = new QCheckBox( this, "interpretButton" );
  interpretButton->setGeometry( 310, 70, 170, 30 );
  interpretButton->setMinimumSize( 0, 0 );
  interpretButton->setMaximumSize( 32767, 32767 );
  interpretButton->setFocusPolicy( QWidget::TabFocus );
  interpretButton->setBackgroundMode( QWidget::PaletteBackground );
  interpretButton->setFontPropagation( QWidget::NoChildren );
  interpretButton->setPalettePropagation( QWidget::NoChildren );
  interpretButton->setText( "interpret TAB, BS and FF" );
  interpretButton->setAutoRepeat( FALSE );
  interpretButton->setAutoResize( FALSE );
  
  currentDateButton = new QCheckBox( this, "currentDateButton" );
  currentDateButton->setGeometry( 40, 90, 130, 30 );
  currentDateButton->setMinimumSize( 0, 0 );
  currentDateButton->setMaximumSize( 32767, 32767 );
  currentDateButton->setFocusPolicy( QWidget::TabFocus );
  currentDateButton->setBackgroundMode( QWidget::PaletteBackground );
  currentDateButton->setFontPropagation( QWidget::NoChildren );
  currentDateButton->setPalettePropagation( QWidget::NoChildren );
  currentDateButton->setText( "date & time" );
  currentDateButton->setAutoRepeat( FALSE );
  currentDateButton->setAutoResize( FALSE );
  
  qtarch_ButtonGroup_60 = new QButtonGroup( this, "ButtonGroup_60" );
  qtarch_ButtonGroup_60->setGeometry( 30, 160, 250, 100 );
  qtarch_ButtonGroup_60->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_60->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_60->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_60->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_60->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_60->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_60->setFrameStyle( 49 );
  qtarch_ButtonGroup_60->setTitle( "Headertext" );
  qtarch_ButtonGroup_60->setAlignment( 1 );
  
  headertextButton = new QCheckBox( this, "headertextButton" );
  headertextButton->setGeometry( 40, 180, 230, 30 );
  headertextButton->setMinimumSize( 0, 0 );
  headertextButton->setMaximumSize( 32767, 32767 );
  headertextButton->setFocusPolicy( QWidget::TabFocus );
  headertextButton->setBackgroundMode( QWidget::PaletteBackground );
  headertextButton->setFontPropagation( QWidget::NoChildren );
  headertextButton->setPalettePropagation( QWidget::NoChildren );
  headertextButton->setText( "headertext" );
  headertextButton->setAutoRepeat( FALSE );
  headertextButton->setAutoResize( FALSE );
  
  qtarch_Label_20 = new QLabel( this, "Label_20" );
  qtarch_Label_20->setGeometry( 40, 220, 50, 30 );
  qtarch_Label_20->setMinimumSize( 0, 0 );
  qtarch_Label_20->setMaximumSize( 32767, 32767 );
  qtarch_Label_20->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_20->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_20->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_20->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_20->setText( "text" );
  qtarch_Label_20->setAlignment( 289 );
  qtarch_Label_20->setMargin( -1 );
  
  headertextLine = new QLineEdit( this, "headertextLine" );
  headertextLine->setGeometry( 90, 220, 180, 30 );
  headertextLine->setMinimumSize( 0, 0 );
  headertextLine->setMaximumSize( 32767, 32767 );
  headertextLine->setFocusPolicy( QWidget::StrongFocus );
  headertextLine->setBackgroundMode( QWidget::PaletteBase );
  headertextLine->setFontPropagation( QWidget::NoChildren );
  headertextLine->setPalettePropagation( QWidget::NoChildren );
  headertextLine->setText( "" );
  headertextLine->setMaxLength( 32767 );
  headertextLine->setEchoMode( QLineEdit::Normal );
  headertextLine->setFrame( TRUE );
  
  loginButton = new QCheckBox( this, "loginButton" );
  loginButton->setGeometry( 40, 120, 110, 30 );
  loginButton->setMinimumSize( 0, 0 );
  loginButton->setMaximumSize( 32767, 32767 );
  loginButton->setFocusPolicy( QWidget::TabFocus );
  loginButton->setBackgroundMode( QWidget::PaletteBackground );
  loginButton->setFontPropagation( QWidget::NoChildren );
  loginButton->setPalettePropagation( QWidget::NoChildren );
  loginButton->setText( "login" );
  loginButton->setAutoRepeat( FALSE );
  loginButton->setAutoResize( FALSE );
  
  filenameLine = new QCheckBox( this, "filenameLine" );
  filenameLine->setGeometry( 40, 60, 90, 30 );
  filenameLine->setMinimumSize( 0, 0 );
  filenameLine->setMaximumSize( 32767, 32767 );
  filenameLine->setFocusPolicy( QWidget::TabFocus );
  filenameLine->setBackgroundMode( QWidget::PaletteBackground );
  filenameLine->setFontPropagation( QWidget::NoChildren );
  filenameLine->setPalettePropagation( QWidget::NoChildren );
  filenameLine->setText( "filename" );
  filenameLine->setAutoRepeat( FALSE );
  filenameLine->setAutoResize( FALSE );
  
  headerButton = new QCheckBox( this, "headerButton" );
  headerButton->setGeometry( 30, 30, 100, 30 );
  headerButton->setMinimumSize( 0, 0 );
  headerButton->setMaximumSize( 32767, 32767 );
  headerButton->setFocusPolicy( QWidget::TabFocus );
  headerButton->setBackgroundMode( QWidget::PaletteBackground );
  headerButton->setFontPropagation( QWidget::NoChildren );
  headerButton->setPalettePropagation( QWidget::NoChildren );
  headerButton->setText( "header" );
  headerButton->setAutoRepeat( FALSE );
  headerButton->setAutoResize( FALSE );
  
  replaceButton = new QCheckBox( this, "replaceButton" );
  replaceButton->setGeometry( 310, 110, 250, 30 );
  replaceButton->setMinimumSize( 0, 0 );
  replaceButton->setMaximumSize( 32767, 32767 );
  replaceButton->setFocusPolicy( QWidget::TabFocus );
  replaceButton->setBackgroundMode( QWidget::PaletteBackground );
  replaceButton->setFontPropagation( QWidget::NoChildren );
  replaceButton->setPalettePropagation( QWidget::NoChildren );
  replaceButton->setText( "replace non-printing character by space" );
  replaceButton->setAutoRepeat( FALSE );
  replaceButton->setAutoResize( FALSE );
  
  cutLinesButton = new QCheckBox( this, "cutLinesButton" );
  cutLinesButton->setGeometry( 310, 30, 100, 30 );
  cutLinesButton->setMinimumSize( 0, 0 );
  cutLinesButton->setMaximumSize( 32767, 32767 );
  cutLinesButton->setFocusPolicy( QWidget::TabFocus );
  cutLinesButton->setBackgroundMode( QWidget::PaletteBackground );
  cutLinesButton->setFontPropagation( QWidget::NoChildren );
  cutLinesButton->setPalettePropagation( QWidget::NoChildren );
  cutLinesButton->setText( "cut lines" );
  cutLinesButton->setAutoRepeat( FALSE );
  cutLinesButton->setAutoResize( FALSE );
  
  qtarch_ButtonGroup_20 = new QButtonGroup( this, "ButtonGroup_20" );
  qtarch_ButtonGroup_20->setGeometry( 20, 280, 270, 100 );
  qtarch_ButtonGroup_20->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_20->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_20->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_20->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_20->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_20->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_20->setFrameStyle( 49 );
  qtarch_ButtonGroup_20->setTitle( "Font and TAB" );
  qtarch_ButtonGroup_20->setAlignment( 1 );

  qtarch_Label_2 = new QLabel( this, "Label_2" );
  qtarch_Label_2->setGeometry( 30, 300, 100, 30 );
  qtarch_Label_2->setMinimumSize( 0, 0 );
  qtarch_Label_2->setMaximumSize( 32767, 32767 );
  qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_2->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_2->setText( "set TAB size" );
  qtarch_Label_2->setAlignment( 289 );
  qtarch_Label_2->setMargin( -1 );
  
  setTabSize = new QSpinBox( this, "setTabSize" );
  setTabSize->setGeometry( 140, 300, 140, 30 );
  setTabSize->setMinimumSize( 0, 0 );
  setTabSize->setMaximumSize( 32767, 32767 );
  setTabSize->setFocusPolicy( QWidget::StrongFocus );
  setTabSize->setBackgroundMode( QWidget::PaletteBackground );
  setTabSize->setFontPropagation( QWidget::NoChildren );
  setTabSize->setPalettePropagation( QWidget::NoChildren );
  setTabSize->setFrameStyle( 50 );
  setTabSize->setLineWidth( 2 );
  setTabSize->setRange( 2, 99 );
  setTabSize->setSteps( 1, 0 );
  setTabSize->setPrefix( "" );
  setTabSize->setSuffix( "" );
  setTabSize->setSpecialValueText( "" );
  setTabSize->setWrapping( FALSE );
  
  a2psFontSize = new QSpinBox( this, "a2psFontSize" );
  a2psFontSize->setGeometry( 140, 340, 140, 30 );
  a2psFontSize->setMinimumSize( 0, 0 );
  a2psFontSize->setMaximumSize( 32767, 32767 );
  a2psFontSize->setFocusPolicy( QWidget::StrongFocus );
  a2psFontSize->setBackgroundMode( QWidget::PaletteBackground );
  a2psFontSize->setFontPropagation( QWidget::NoChildren );
  a2psFontSize->setPalettePropagation( QWidget::NoChildren );
  a2psFontSize->setFrameStyle( 50 );
  a2psFontSize->setLineWidth( 2 );
  a2psFontSize->setRange( 1, 400 );
  a2psFontSize->setSteps( 1, 0 );
  a2psFontSize->setPrefix( "" );
  a2psFontSize->setSuffix( "" );
  a2psFontSize->setSpecialValueText( "" );
  a2psFontSize->setWrapping( FALSE );
  
  qtarch_Label_3 = new QLabel( this, "Label_3" );
  qtarch_Label_3->setGeometry( 30, 340, 100, 30 );
  qtarch_Label_3->setMinimumSize( 0, 0 );
  qtarch_Label_3->setMaximumSize( 32767, 32767 );
  qtarch_Label_3->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_3->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_3->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_3->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_3->setText( "fontsize for a2ps" );
  qtarch_Label_3->setAlignment( 289 );
  qtarch_Label_3->setMargin( -1 );
  
  qtarch_ButtonGroup_33 = new QButtonGroup( this, "ButtonGroup_33" );
  qtarch_ButtonGroup_33->setGeometry( 300, 240, 280, 140 );
  qtarch_ButtonGroup_33->setMinimumSize( 0, 0 );
  qtarch_ButtonGroup_33->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_33->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_33->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_33->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_33->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_33->setFrameStyle( 49 );
  qtarch_ButtonGroup_33->setTitle( "Numbering & border" );
  qtarch_ButtonGroup_33->setAlignment( 1 );
  
  numberingLineButton = new QCheckBox( this, "numberingLineButton" );
  numberingLineButton->setGeometry( 310, 260, 100, 30 );
  numberingLineButton->setMinimumSize( 0, 0 );
  numberingLineButton->setMaximumSize( 32767, 32767 );
  numberingLineButton->setFocusPolicy( QWidget::TabFocus );
  numberingLineButton->setBackgroundMode( QWidget::PaletteBackground );
  numberingLineButton->setFontPropagation( QWidget::NoChildren );
  numberingLineButton->setPalettePropagation( QWidget::NoChildren );
  numberingLineButton->setText( "numbering lines" );
  numberingLineButton->setAutoRepeat( FALSE );
  numberingLineButton->setAutoResize( FALSE );
  
  bordersButton = new QCheckBox( this, "bordersButton" );
  bordersButton->setGeometry( 450, 260, 100, 30 );
  bordersButton->setMinimumSize( 0, 0 );
  bordersButton->setMaximumSize( 32767, 32767 );
  bordersButton->setFocusPolicy( QWidget::TabFocus );
  bordersButton->setBackgroundMode( QWidget::PaletteBackground );
  bordersButton->setFontPropagation( QWidget::NoChildren );
  bordersButton->setPalettePropagation( QWidget::NoChildren );
  bordersButton->setText( "borders" );
  bordersButton->setAutoRepeat( FALSE );
  bordersButton->setAutoResize( FALSE );
  
  numberingPagesList = new QComboBox( FALSE, this, "numberingPagesList" );
  numberingPagesList->setGeometry( 450, 300, 120, 30 );
  numberingPagesList->setMinimumSize( 0, 0 );
  numberingPagesList->setMaximumSize( 32767, 32767 );
  numberingPagesList->setFocusPolicy( QWidget::StrongFocus );
  numberingPagesList->setBackgroundMode( QWidget::PaletteBackground );
  numberingPagesList->setFontPropagation( QWidget::NoChildren );
  numberingPagesList->setPalettePropagation( QWidget::NoChildren );
  numberingPagesList->setSizeLimit( 10 );
  numberingPagesList->setAutoResize( FALSE );
  numberingPagesList->insertItem( "file single" );
  numberingPagesList->insertItem( "file together" );

  linesPerPage = new QSpinBox( this, "linesPerPage" );
  linesPerPage->setGeometry( 450, 340, 120, 30 );
  linesPerPage->setMinimumSize( 0, 0 );
  linesPerPage->setMaximumSize( 32767, 32767 );
  linesPerPage->setFocusPolicy( QWidget::StrongFocus );
  linesPerPage->setBackgroundMode( QWidget::PaletteBackground );
  linesPerPage->setFontPropagation( QWidget::NoChildren );
  linesPerPage->setPalettePropagation( QWidget::NoChildren );
  linesPerPage->setFrameStyle( 50 );
  linesPerPage->setLineWidth( 2 );
  linesPerPage->setRange( 0, 99 );
  linesPerPage->setSteps( 1, 0 );
  linesPerPage->setPrefix( "" );
  linesPerPage->setSuffix( "" );
  linesPerPage->setSpecialValueText( "" );
  linesPerPage->setWrapping( FALSE );
  
  qtarch_label = new QLabel( this, "label" );
  qtarch_label->setGeometry( 310, 340, 90, 30 );
  qtarch_label->setMinimumSize( 0, 0 );
  qtarch_label->setMaximumSize( 32767, 32767 );
  qtarch_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_label->setFontPropagation( QWidget::NoChildren );
  qtarch_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_label->setText( "lines per page" );
  qtarch_label->setAlignment( 289 );
  qtarch_label->setMargin( -1 );
  
  qtarch_Label_113 = new QLabel( this, "Label_113" );
  qtarch_Label_113->setGeometry( 310, 300, 130, 30 );
  qtarch_Label_113->setMinimumSize( 0, 0 );
  qtarch_Label_113->setMaximumSize( 32767, 32767 );
  qtarch_Label_113->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_113->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_113->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_113->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_113->setText( "numbering pages" );
  qtarch_Label_113->setAlignment( 289 );
  qtarch_Label_113->setMargin( -1 );
  
  previewButton = new QPushButton( this, "previewButton" );
  previewButton->setGeometry( 20, 390, 100, 30 );
  previewButton->setMinimumSize( 0, 0 );
  previewButton->setMaximumSize( 32767, 32767 );
  previewButton->setFocusPolicy( QWidget::TabFocus );
  previewButton->setBackgroundMode( QWidget::PaletteBackground );
  previewButton->setFontPropagation( QWidget::NoChildren );
  previewButton->setPalettePropagation( QWidget::NoChildren );
  previewButton->setText( "Preview" );
  previewButton->setAutoRepeat( FALSE );
  previewButton->setAutoResize( FALSE );
  
  okButton = new QPushButton( this, "okButton" );
  okButton->setGeometry( 260, 390, 100, 30 );
  okButton->setMinimumSize( 0, 0 );
  okButton->setMaximumSize( 32767, 32767 );
  okButton->setFocusPolicy( QWidget::TabFocus );
  okButton->setBackgroundMode( QWidget::PaletteBackground );
  okButton->setFontPropagation( QWidget::NoChildren );
  okButton->setPalettePropagation( QWidget::NoChildren );
  okButton->setText( "Ok" );
  okButton->setAutoRepeat( FALSE );
  okButton->setAutoResize( FALSE );
  
  defaultButton = new QPushButton( this, "defaultButton" );
  defaultButton->setGeometry( 370, 390, 100, 30 );
  defaultButton->setMinimumSize( 0, 0 );
  defaultButton->setMaximumSize( 32767, 32767 );
  defaultButton->setFocusPolicy( QWidget::TabFocus );
  defaultButton->setBackgroundMode( QWidget::PaletteBackground );
  defaultButton->setFontPropagation( QWidget::NoChildren );
  defaultButton->setPalettePropagation( QWidget::NoChildren );
  defaultButton->setText( "Defaults" );
  defaultButton->setAutoRepeat( FALSE );
  defaultButton->setAutoResize( FALSE );
  
  cancelButton = new QPushButton( this, "cancelButton" );
  cancelButton->setGeometry( 480, 390, 100, 30 );
  cancelButton->setMinimumSize( 0, 0 );
  cancelButton->setMaximumSize( 32767, 32767 );
  cancelButton->setFocusPolicy( QWidget::TabFocus );
  cancelButton->setBackgroundMode( QWidget::PaletteBackground );
  cancelButton->setFontPropagation( QWidget::NoChildren );
  cancelButton->setPalettePropagation( QWidget::NoChildren );
  cancelButton->setText( "Cancel" );
  cancelButton->setAutoRepeat( FALSE );
  cancelButton->setAutoResize( FALSE );
  
  qtarch_ButtonGroup_177->insert( currentDateButton );
  qtarch_ButtonGroup_177->insert( headertextButton );
  qtarch_ButtonGroup_177->insert( loginButton );
  qtarch_ButtonGroup_177->insert( filenameLine );
  qtarch_ButtonGroup_177->insert( headerButton );
  
  qtarch_ButtonGroup_21->insert( boltFontButton );
  qtarch_ButtonGroup_21->insert( printAsISOLatin );
  qtarch_ButtonGroup_21->insert( interpretButton );
  qtarch_ButtonGroup_21->insert( replaceButton );
  qtarch_ButtonGroup_21->insert( cutLinesButton );
  
  qtarch_ButtonGroup_60->insert( headertextButton );
  qtarch_ButtonGroup_33->insert( numberingLineButton );
  qtarch_ButtonGroup_33->insert( bordersButton );
  
  
  connect (cancelButton,SIGNAL(clicked()),SLOT(slotCancelClicked()));
  connect (previewButton,SIGNAL(clicked()),SLOT(slotPreviewClicked()));
  connect(defaultButton,SIGNAL(clicked()),SLOT(slotDefaultClicked()));
  connect (okButton,SIGNAL(clicked()),SLOT(slotOkClicked()));
  connect(headertextButton,SIGNAL(clicked()),SLOT(slotHeadertextClicked()));
  connect (headerButton,SIGNAL(clicked()),SLOT(slotHeaderClicked()));
  selectedProgram();
}

void CConfigA2psDlg::selectedProgram() {
  interpretButton->setEnabled(true);
  interpretButton->setChecked(false);
  replaceButton->setEnabled(true);
  replaceButton->setChecked(false);
  printAsISOLatin->setEnabled(true);
  printAsISOLatin->setChecked(false);
  boltFontButton->setEnabled(true);
  boltFontButton->setChecked(false);
  a2psFontSize->setEnabled(true);
  qtarch_Label_3->setEnabled(true);
}


void CConfigA2psDlg::slotHeaderClicked() {
  if (headerButton->isChecked()) {
    slotHeadertextClicked();
    slotCurrentDateClicked();
    loginButton->setEnabled(true);
    headertextButton->setEnabled(true);
    filenameLine->setEnabled(true);
    currentDateButton->setEnabled(true);
    qtarch_ButtonGroup_60->setEnabled(true);
  }
  else {
    slotNoHeader();
  }
}

void CConfigA2psDlg::slotNoHeader() {
  headertextLine->setEnabled(false);

  qtarch_Label_20->setEnabled(false);

  loginButton->setEnabled(false);
  headertextButton->setEnabled(false);
  filenameLine->setEnabled(false);
  currentDateButton->setEnabled(false);
  qtarch_ButtonGroup_60->setEnabled(false);
}

void CConfigA2psDlg::slotHeadertextClicked() {
  if (headertextButton->isChecked()) {
    headertextLine->setEnabled(true);
    qtarch_Label_20->setEnabled(true);
  }
  else {
    headertextLine->setEnabled(false);
    qtarch_Label_20->setEnabled(false);
  }
}

void CConfigA2psDlg::slotCurrentDateClicked() {
  if (currentDateButton->isChecked()) {

  }

}


void CConfigA2psDlg::slotDefaultClicked() {
  selectedProgram();
  headerButton->setChecked(true);
  headertextButton->setChecked(false);
  headertextLine->clear();
  loginButton->setChecked(false);
 
  currentDateButton->setChecked(true);
 
  numberingLineButton->setChecked(false);
  bordersButton->setChecked(true);
 
  numberingPagesList->setCurrentItem(0);
 
  setTabSize->setValue(8);
  a2psFontSize->setValue(9);
  linesPerPage->setValue(60);
 
  cutLinesButton->setChecked(false);
  printAsISOLatin->setChecked(false);
  boltFontButton->setChecked(false);

  filenameLine->setChecked(true);

  replaceButton->setChecked(false);
  slotHeaderClicked();
}

void CConfigA2psDlg::slotCreateParameters() {
   QString parameters;
  parameters = "";
  globalpara = "";
  if (headerButton->isChecked()) {
    if (headertextButton->isChecked()) {
      globalpara.append(" -H\"");
      globalpara.append(headertextLine->text());
      globalpara.append("\"");
    }
    if (!loginButton->isChecked()) {
      parameters.append(" -nL");
    }
    if (!filenameLine->isChecked()) {
      parameters.append(" -nu");
    }
    if (currentDateButton->isChecked()) {
      parameters.append(" -d");
    }
    else {
      parameters.append(" -nd");
    }
  }
  else {
    parameters.append(" -nH");
  }
  if (numberingLineButton->isChecked()) {
    parameters.append(" -n");
  }
  else {
    parameters.append(" -nn");
  }
  if (bordersButton->isChecked()) {
    parameters.append(" -s");
  }
  else {
    parameters.append(" -ns");
  }
  if (numberingPagesList->currentItem()==0) {
    globalpara.append(" -r");
  }
  else {
    globalpara.append(" -nr");
  }
  parameters.append(" -l");
  parameters.append(linesPerPage->text());
  if (cutLinesButton->isChecked()) {
    globalpara.append(" -f");
  }
  else {
    globalpara.append(" -nf");
  }
  if (interpretButton->isChecked()) {
    globalpara.append(" -i");
  }
  else {
    globalpara.append(" -ni");
  }
  if (replaceButton->isChecked()) {
    globalpara.append(" -nv");
  }
  else {
    globalpara.append(" -v");
  }
  if (printAsISOLatin->isChecked()) {
    globalpara.append(" -8");
  }
  else {
    globalpara.append(" -n8");
  }
  if (boltFontButton->isChecked()) {
    parameters.append(" -b");
  }
  else {
    parameters.append(" -nb");
  }
  globalpara.append(" -t");
  globalpara.append(setTabSize->text());
  parameters.append(" -F");
  parameters.append(a2psFontSize->text());
  globalpara.append(parameters);
}

void CConfigA2psDlg::slotPreviewClicked() {
  if (!CToolClass::searchProgram("ghostview")) {
    return;
  }
  QString dir,data1,data2,text;
  slotCreateParameters();
  dir =  KApplication::localkdedir() + (QString) "/share/apps/kdevelop/preview.ps";
  data1 = KApplication::kde_datadir() + (QString) "/kdevelop/templates/preview1";
  data2 = KApplication::kde_datadir() + (QString) "/kdevelop/templates/preview2";
  process = new KShellProcess();
  //  cerr << "a2ps -nP" + globalpara + " " + data1 + " " + data2 + " >" + dir;
  *process << "a2ps -nP" + globalpara + " " + data1 + " " + data2 + " >" + dir;
  process->start(KProcess::Block,KProcess::AllOutput);
  process2 = new KShellProcess();
  *process2 << "ghostview";
  *process2 << dir;
  process2->start(KProcess::Block,KProcess::AllOutput);
  process3 = new KShellProcess();
  *process3 << "rm";
  *process3 << dir;
  process3->start(KProcess::Block,KProcess::AllOutput);
}

void CConfigA2psDlg::slotOkClicked() {
  slotCreateParameters();
  reject();
}

void CConfigA2psDlg::slotCancelClicked() {
  reject();
}
