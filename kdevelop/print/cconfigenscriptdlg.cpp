/***************************************************************************
                          cconfigenscriptdlg.cpp  -  description                       
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

#include "cconfigenscriptdlg.h"
#include <kapp.h>
#include <iostream.h>
#include <kmessagebox.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <qfile.h>
#include <qlayout.h>
#include <qgrid.h>

CConfigEnscriptDlg::CConfigEnscriptDlg(QWidget* parent,const char* name) : QTabDialog(parent, name, true){
  init();
  slotHeaderClicked();
  slotUnderlayButtonClicked();
  loadSettings();
}

CConfigEnscriptDlg::~CConfigEnscriptDlg(){
    delete (underlayFontButton);
  delete (underlaytextLine);
  delete (qtarch_Label_75);
  delete (underlayPositionDefaultButton);
  delete (underlayStyle);
  delete (underlayAngleDefault);
  delete (underlayGray);
  delete (qtarch_Label_74);
  delete (underlayYPosition);
  delete (underlayXPosition);
  delete (underlayAngle);
  delete (qtarch_Label_73);
  delete (qtarch_Label_72);
  delete (qtarch_Label_71);
  delete (qtarch_Label_70);
  delete (underlayButton);
  delete (qtarch_ButtonGroup_133);
  delete (qtarch_ButtonGroup_132);
  delete (qtarch_ButtonGroup_131);
  delete (qtarch_ButtonGroup_134);
  delete (qtarch_ButtonGroup_135);
  delete (qtarch_ButtonGroup_136);
  delete (qtarch_ButtonGroup_137);
  delete (qtarch_ButtonGroup_138);
  delete (replaceButton);
  delete (cutLinesButton);
  delete (qtarch_ButtonGroup_21);
  delete (alignFileList);
  delete (fontForHeaderButton);
  delete (fontForBodyButton);
  delete (setTabSize);
  delete (qtarch_Label_5);
  delete (qtarch_Label_4);
  delete (qtarch_Label_2);
  delete (highlightBarsButton);
  delete (numberingPagesList);
  delete (qtarch_Label_13);
  delete (cycleOfChange);
  delete (valueForWrappedLine);
  delete (linesPerPage);
  delete (numberingLineButton);
  delete (qtarch_Label_8);
  delete (numberingPagesButton);
  delete (qtarch_label);
  delete (markedWrappedLinesButton);
  delete (tocButton);
  delete (bordersButton);
  delete (qtarch_ButtonGroup_33);
  delete (qtarch_ButtonGroup_53);
  delete (qtarch_Label_14);
  delete (qtarch_ButtonGroup_52);
  delete (qtarch_ButtonGroup_51);
  delete (qtarch_ButtonGroup_20);
  delete (currentDateFormat);
  delete (modificationDatePosition);
  delete (modificationTimeFormat);
  delete (modificationTimePosition);
  delete (currentTimeFormat);
  delete (modificationTimeAmpm);
  delete (currentDatePosition);
  delete (modificationDateFormat);
  delete (currentTimeAmpm);
  delete (currentTimePosition);
  
  delete (modificationTimeButton);
  delete (currentTimeButton);
  delete (modificationDateButton);
  delete (currentDateButton);
  
  delete (hostnameSize);
  delete (hostnamePosition);
  delete (qtarch_Label_22);
  delete (qtarch_Label_23);
  delete (hostnameButton);
  delete (qtarch_ButtonGroup_66);
  delete (filenamePosition);
  delete (filenameSize);
  delete (qtarch_Label_25);
  delete (qtarch_Label_21);
  delete (filenameLine);
  delete (qtarch_ButtonGroup_65);
  delete (fancyHeaderButton);
  delete (loginPosition);
  delete (qtarch_Label_24);
  delete (qtarch_Label_20);
  delete (qtarch_Label_19);
  delete (headertextPosition);
  delete (loginButton);
  delete (headertextButton);
  delete (headertextLine);
  delete (headerButton);
  delete (qtarch_ButtonGroup_60);
  delete (qtarch_ButtonGroup_61);
  delete (qtarch_ButtonGroup_62);
  delete (qtarch_ButtonGroup_59);
  delete (tab4);
  delete (tab3);
  delete (tab2);
  delete (tab1);
}

void CConfigEnscriptDlg::init() {
  setApplyButton(i18n("Preview"));

  connect (this,SIGNAL(applyButtonPressed()),SLOT(slotPreviewClicked()));
  tab1 = new QWidget(this,"header");
  initTab1(tab1);
  addTab (tab1,i18n("Header"));
  tab2 = new QWidget(this,"date & time");
  initTab2(tab2);
  addTab (tab2,i18n("Date & Time"));
  tab3 = new QWidget(this,"layout");
  initTab3(tab3);
  addTab (tab3,i18n("Layout"));
  tab4 = new QWidget(this,"underlay");
  initTab4(tab4);
  addTab (tab4,i18n("Underlay"));
  selectedProgram();
  setCancelButton(i18n("Cancel"));
  setDefaultButton(i18n("Default"));
  connect(this,SIGNAL(defaultButtonPressed()),SLOT(slotDefaultClicked()));
  connect (this,SIGNAL(applyButtonPressed()),SLOT(slotOkClicked()));
}
void CConfigEnscriptDlg::selectedProgram() {
  replaceButton->setEnabled(true);
  replaceButton->setChecked(false);
  qtarch_ButtonGroup_51->setEnabled(true);
  qtarch_ButtonGroup_52->setEnabled(true);
  qtarch_Label_14->setEnabled(true);
  qtarch_ButtonGroup_53->setEnabled(true);
  tocButton->setEnabled(true);
  markedWrappedLinesButton->setEnabled(true);
  valueForWrappedLine->setEnabled(true);
  cycleOfChange->setEnabled(true);
  qtarch_Label_13->setEnabled(true);
  highlightBarsButton->setEnabled(true);
  qtarch_Label_4->setEnabled(true);
  qtarch_Label_5->setEnabled(true);
  fontForBodyButton->setEnabled(true);
  fontForHeaderButton->setEnabled(true);
  qtarch_ButtonGroup_138->setEnabled(true);
  qtarch_Label_75->setEnabled(true);
  qtarch_Label_74->setEnabled(true);
  qtarch_Label_73->setEnabled(true);
  qtarch_Label_72->setEnabled(true);
  qtarch_Label_71->setEnabled(true);
  qtarch_ButtonGroup_133->setEnabled(true);
  qtarch_Label_70->setEnabled(true);
  qtarch_ButtonGroup_132->setEnabled(true);
  qtarch_ButtonGroup_131->setEnabled(true);
  qtarch_ButtonGroup_134->setEnabled(true);
  qtarch_ButtonGroup_135->setEnabled(true);
  qtarch_ButtonGroup_136->setEnabled(true);
  qtarch_ButtonGroup_137->setEnabled(true);
  underlayFontButton->setEnabled(true);
  underlaytextLine->setEnabled(true);
  underlayPositionDefaultButton->setEnabled(true);
  underlayStyle->setEnabled(true);
  underlayAngleDefault->setEnabled(true);
  underlayGray->setEnabled(true);
  underlayYPosition->setEnabled(true);
  underlayXPosition->setEnabled(true);
  underlayAngle->setEnabled(true);
  underlayButton->setEnabled(true);
  qtarch_ButtonGroup_62->setEnabled(true);
  headertextPosition->setEnabled(true);
  qtarch_Label_19->setEnabled(true);
  qtarch_Label_24->setEnabled(true);
  loginPosition->setEnabled(true);
  fancyHeaderButton->setEnabled(true);
  qtarch_Label_21->setEnabled(true);
  qtarch_Label_25->setEnabled(true);
  filenameSize->setEnabled(true);
  filenamePosition->setEnabled(true);
  qtarch_ButtonGroup_66->setEnabled(true);
  hostnameButton->setEnabled(true);
  qtarch_Label_23->setEnabled(true);
  qtarch_Label_22->setEnabled(true);
  hostnamePosition->setEnabled(true);
  hostnameSize->setEnabled(true);
  qtarch_ButtonGroup_77->setEnabled(true);
  qtarch_ButtonGroup_87->setEnabled(true);
  qtarch_ButtonGroup_88->setEnabled(true);
  qtarch_ButtonGroup_89->setEnabled(true);
  qtarch_ButtonGroup_90->setEnabled(true);
  qtarch_ButtonGroup_76->setEnabled(true);
  qtarch_ButtonGroup_86->setEnabled(true);
  qtarch_ButtonGroup_85->setEnabled(true);
  qtarch_ButtonGroup_84->setEnabled(true);
  qtarch_ButtonGroup_75->setEnabled(true);
  qtarch_ButtonGroup_72->setEnabled(true);
  qtarch_ButtonGroup_71->setEnabled(true);
  modificationDateButton->setEnabled(true);
  modificationTimeButton->setEnabled(true);
  currentTimePosition->setEnabled(true);
  currentTimeAmpm->setEnabled(true);
  modificationDateFormat->setEnabled(true);
  currentDatePosition->setEnabled(true);
  modificationTimeAmpm->setEnabled(true);
  currentTimeFormat->setEnabled(true);
  modificationTimePosition->setEnabled(true);
  modificationTimeFormat->setEnabled(true);
  modificationDatePosition->setEnabled(true);
  currentDateFormat->setEnabled(true);
}

void CConfigEnscriptDlg::initTab1(QWidget*parent) {

  QVBoxLayout *box = new QVBoxLayout( parent );
  box->setMargin( 15 );
  box->setSpacing( 10 );

  headerButton = new QCheckBox( parent, "headerButton" );
  headerButton->setFocusPolicy( QWidget::TabFocus );
  headerButton->setBackgroundMode( QWidget::PaletteBackground );
  headerButton->setFontPropagation( QWidget::NoChildren );
  headerButton->setPalettePropagation( QWidget::NoChildren );
  headerButton->setText( i18n("header") );
  headerButton->setAutoRepeat( FALSE );
  headerButton->setAutoResize( FALSE );
  connect(headerButton,SIGNAL(clicked()),SLOT(slotHeaderClicked()));
  box->addWidget(headerButton);

  qtarch_ButtonGroup_59 = new QButtonGroup( parent, "ButtonGroup_59" );
  qtarch_ButtonGroup_59->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_59->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_59->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_59->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_59->setFrameStyle( 49 );
  qtarch_ButtonGroup_59->setTitle(i18n( "Header settings") );
  qtarch_ButtonGroup_59->setAlignment( 1 );
  box->addWidget(qtarch_ButtonGroup_59);


  QGridLayout *grid1 = new QGridLayout( qtarch_ButtonGroup_59,6,2,15,7);
  qtarch_ButtonGroup_62 = new QButtonGroup(qtarch_ButtonGroup_59 , "ButtonGroup_62" );
  qtarch_ButtonGroup_62->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_62->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_62->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_62->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_62->setFrameStyle( 49 );
  qtarch_ButtonGroup_62->setTitle( i18n("Fancy header") );
  qtarch_ButtonGroup_62->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_62,0,0);


  box = new QVBoxLayout( qtarch_ButtonGroup_62 );
  box->setMargin( 15 );
  box->setSpacing( 10 );

  fancyHeaderButton = new QCheckBox( qtarch_ButtonGroup_62, "fancyHeaderButton" );
  fancyHeaderButton->setFocusPolicy( QWidget::TabFocus );
  fancyHeaderButton->setBackgroundMode( QWidget::PaletteBackground );
  fancyHeaderButton->setFontPropagation( QWidget::NoChildren );
  fancyHeaderButton->setPalettePropagation( QWidget::NoChildren );
  fancyHeaderButton->setText( i18n("fancy header") );
  fancyHeaderButton->setAutoRepeat( FALSE );
  fancyHeaderButton->setAutoResize( FALSE );
  box->addWidget(fancyHeaderButton);


  qtarch_ButtonGroup_60 = new QButtonGroup(qtarch_ButtonGroup_59 , "ButtonGroup_60" );
  qtarch_ButtonGroup_60->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_60->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_60->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_60->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_60->setFrameStyle( 49 );
  qtarch_ButtonGroup_60->setTitle( i18n("Headertext") );
  qtarch_ButtonGroup_60->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_60,1,3,0,0);

  QGridLayout *grid2 = new QGridLayout( qtarch_ButtonGroup_60,3,2,15,7);

  headertextButton = new QCheckBox( qtarch_ButtonGroup_60, "headertextButton" );
  headertextButton->setFocusPolicy( QWidget::TabFocus );
  headertextButton->setBackgroundMode( QWidget::PaletteBackground );
  headertextButton->setFontPropagation( QWidget::NoChildren );
  headertextButton->setPalettePropagation( QWidget::NoChildren );
  headertextButton->setText( i18n("headertext") );
  headertextButton->setAutoRepeat( FALSE );
  headertextButton->setAutoResize( FALSE );
  connect(headertextButton,SIGNAL(clicked()),SLOT(slotHeadertextClicked()));
  grid2->addWidget(headertextButton,0,0);

  headertextLine = new QLineEdit(qtarch_ButtonGroup_60 , "headertextLine" );
  headertextLine->setFocusPolicy( QWidget::StrongFocus );
  headertextLine->setBackgroundMode( QWidget::PaletteBase );
  headertextLine->setFontPropagation( QWidget::NoChildren );
  headertextLine->setPalettePropagation( QWidget::NoChildren );
  headertextLine->setText( "" );
  headertextLine->setMaxLength( 32767 );
  headertextLine->setEchoMode( QLineEdit::Normal );
  headertextLine->setFrame( TRUE );
  grid2->addWidget(headertextLine,1,1);


  headertextPosition = new QComboBox( FALSE, qtarch_ButtonGroup_60, "headertextPosition" );
  headertextPosition->setFocusPolicy( QWidget::StrongFocus );
  headertextPosition->setBackgroundMode( QWidget::PaletteBackground );
  headertextPosition->setFontPropagation( QWidget::NoChildren );
  headertextPosition->setPalettePropagation( QWidget::NoChildren );
  headertextPosition->setSizeLimit( 10 );
  headertextPosition->setAutoResize( FALSE );
  headertextPosition->insertItem(i18n("left"));
  headertextPosition->insertItem(i18n("center"));
  headertextPosition->insertItem(i18n("right"));
  grid2->addWidget(headertextPosition,2,1);
  
  qtarch_Label_19 = new QLabel( qtarch_ButtonGroup_60, "Label_19" );
  qtarch_Label_19->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_19->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_19->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_19->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_19->setText(i18n("position") );
  qtarch_Label_19->setAlignment( 289 );
  qtarch_Label_19->setMargin( -1 );
  grid2->addWidget(qtarch_Label_19,2,0);

  qtarch_Label_20 = new QLabel(  qtarch_ButtonGroup_60, "Label_20" );
  qtarch_Label_20->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_20->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_20->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_20->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_20->setText( i18n("text") );
  qtarch_Label_20->setAlignment( 289 );
  qtarch_Label_20->setMargin( -1 );
  grid2->addWidget(qtarch_Label_20,1,0);




  qtarch_ButtonGroup_61 = new QButtonGroup(qtarch_ButtonGroup_59 , "ButtonGroup_61" );
  qtarch_ButtonGroup_61->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_61->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_61->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_61->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_61->setFrameStyle( 49 );
  qtarch_ButtonGroup_61->setTitle( i18n("Login") );
  qtarch_ButtonGroup_61->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_61,4,5,0,0);
 
  grid2 = new QGridLayout( qtarch_ButtonGroup_61,2,2,15,7);

  
  loginButton = new QCheckBox(  qtarch_ButtonGroup_61, "loginButton" );
  loginButton->setFocusPolicy( QWidget::TabFocus );
  loginButton->setBackgroundMode( QWidget::PaletteBackground );
  loginButton->setFontPropagation( QWidget::NoChildren );
  loginButton->setPalettePropagation( QWidget::NoChildren );
  loginButton->setText( i18n("login") );
  loginButton->setAutoRepeat( FALSE );
  loginButton->setAutoResize( FALSE );
  connect(loginButton,SIGNAL(clicked()),SLOT(slotLoginClicked()));
  grid2->addWidget(loginButton,0,0);
  
  qtarch_Label_24 = new QLabel( qtarch_ButtonGroup_61, "Label_24" );
 
  qtarch_Label_24->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_24->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_24->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_24->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_24->setText( i18n("position") );
  qtarch_Label_24->setAlignment( 289 );
  qtarch_Label_24->setMargin( -1 );
  grid2->addWidget(qtarch_Label_24,1,0);
  
  loginPosition = new QComboBox( FALSE,  qtarch_ButtonGroup_61, "loginPosition" );
  loginPosition->setFocusPolicy( QWidget::StrongFocus );
  loginPosition->setBackgroundMode( QWidget::PaletteBackground );
  loginPosition->setFontPropagation( QWidget::NoChildren );
  loginPosition->setPalettePropagation( QWidget::NoChildren );
  loginPosition->setSizeLimit( 10 );
  loginPosition->setAutoResize( FALSE );
  loginPosition->insertItem(i18n("left"));
  loginPosition->insertItem(i18n("center"));
  loginPosition->insertItem(i18n("right"));
  grid2->addWidget(loginPosition,1,1);

  
  qtarch_ButtonGroup_65 = new QButtonGroup( qtarch_ButtonGroup_59, "ButtonGroup_65" );
  qtarch_ButtonGroup_65->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_65->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_65->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_65->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_65->setFrameStyle( 49 );
  qtarch_ButtonGroup_65->setTitle( i18n("Filename") );
  qtarch_ButtonGroup_65->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_65,0,2,1,1);
  
  grid2 = new QGridLayout( qtarch_ButtonGroup_65,3,2,15,7);
  filenameLine = new QCheckBox( qtarch_ButtonGroup_65, "filenameLine" );
  filenameLine->setFocusPolicy( QWidget::TabFocus );
  filenameLine->setBackgroundMode( QWidget::PaletteBackground );
  filenameLine->setFontPropagation( QWidget::NoChildren );
  filenameLine->setPalettePropagation( QWidget::NoChildren );
  filenameLine->setText( i18n("filename") );
  filenameLine->setAutoRepeat( FALSE );
  filenameLine->setAutoResize( FALSE );
  connect(filenameLine,SIGNAL(clicked()),SLOT(slotFilenameClicked()));
  grid2->addWidget(filenameLine,0,0);

  
  qtarch_Label_21 = new QLabel( qtarch_ButtonGroup_65, "Label_21" );
  qtarch_Label_21->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_21->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_21->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_21->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_21->setText( i18n("size of filename") );
  qtarch_Label_21->setAlignment( 289 );
  qtarch_Label_21->setMargin( -1 );
  grid2->addWidget(qtarch_Label_21,1,0);
  
  qtarch_Label_25 = new QLabel( qtarch_ButtonGroup_65, "Label_25" );
  qtarch_Label_25->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_25->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_25->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_25->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_25->setText( i18n("position") );
  qtarch_Label_25->setAlignment( 289 );
  qtarch_Label_25->setMargin( -1 );
  grid2->addWidget( qtarch_Label_25,2,0);
  
  filenameSize = new QComboBox( FALSE,qtarch_ButtonGroup_65 , "filenameSize" );
  filenameSize->setFocusPolicy( QWidget::StrongFocus );
  filenameSize->setBackgroundMode( QWidget::PaletteBackground );
  filenameSize->setFontPropagation( QWidget::NoChildren );
  filenameSize->setPalettePropagation( QWidget::NoChildren );
  filenameSize->setSizeLimit( 10 );
  filenameSize->setAutoResize( FALSE );
  filenameSize->insertItem(i18n("short"));
  filenameSize->insertItem(i18n("full"));
  grid2->addWidget( filenameSize,1,1);


  filenamePosition = new QComboBox( FALSE, qtarch_ButtonGroup_65, "filenamePosition" );
  filenamePosition->setFocusPolicy( QWidget::StrongFocus );
  filenamePosition->setBackgroundMode( QWidget::PaletteBackground );
  filenamePosition->setFontPropagation( QWidget::NoChildren );
  filenamePosition->setPalettePropagation( QWidget::NoChildren );
  filenamePosition->setSizeLimit( 10 );
  filenamePosition->setAutoResize( FALSE );
  filenamePosition->insertItem(i18n("left")); 
  filenamePosition->insertItem(i18n("center"));
  filenamePosition->insertItem(i18n("right"));
  grid2->addWidget(filenamePosition,2,1);


  qtarch_ButtonGroup_66 = new QButtonGroup( qtarch_ButtonGroup_59 , "ButtonGroup_66" );
  qtarch_ButtonGroup_66->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_66->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_66->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_66->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_66->setFrameStyle( 49 );
  qtarch_ButtonGroup_66->setTitle(i18n("Hostname") );
  qtarch_ButtonGroup_66->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_66,3,5,1,1);

  grid2 = new QGridLayout( qtarch_ButtonGroup_66,3,2,15,7);

  hostnameButton = new QCheckBox( qtarch_ButtonGroup_66, "hostnameButton" );
  hostnameButton->setFocusPolicy( QWidget::TabFocus );
  hostnameButton->setBackgroundMode( QWidget::PaletteBackground );
  hostnameButton->setFontPropagation( QWidget::NoChildren );
  hostnameButton->setPalettePropagation( QWidget::NoChildren );
  hostnameButton->setText(i18n("hostname") );
  hostnameButton->setAutoRepeat( FALSE );
  hostnameButton->setAutoResize( FALSE );
  connect(hostnameButton,SIGNAL(clicked()),SLOT(slotHostnameClicked()));
  grid2->addWidget(hostnameButton,0,0);

  
  qtarch_Label_23 = new QLabel( qtarch_ButtonGroup_66, "Label_23" );
  qtarch_Label_23->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_23->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_23->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_23->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_23->setText(i18n("size of hostname") );
  qtarch_Label_23->setAlignment( 289 );
  qtarch_Label_23->setMargin( -1 );
  grid2->addWidget(qtarch_Label_23,1,0);
  
  qtarch_Label_22 = new QLabel( qtarch_ButtonGroup_66, "Label_22" );
  qtarch_Label_22->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_22->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_22->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_22->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_22->setText( i18n("position") );
  qtarch_Label_22->setAlignment( 289 );
  qtarch_Label_22->setMargin( -1 );
  grid2->addWidget( qtarch_Label_22,2,0);
  
  hostnamePosition = new QComboBox( FALSE,  qtarch_ButtonGroup_66, "hostnamePosition" );
  hostnamePosition->setFocusPolicy( QWidget::StrongFocus );
  hostnamePosition->setBackgroundMode( QWidget::PaletteBackground );
  hostnamePosition->setFontPropagation( QWidget::NoChildren );
  hostnamePosition->setPalettePropagation( QWidget::NoChildren );
  hostnamePosition->setSizeLimit( 10 );
  hostnamePosition->setAutoResize( FALSE );
  hostnamePosition->insertItem(i18n("left"));
  hostnamePosition->insertItem(i18n("center"));
  hostnamePosition->insertItem(i18n("right"));
  grid2->addWidget(hostnamePosition,2,1);
  
  hostnameSize = new QComboBox( FALSE,qtarch_ButtonGroup_66 , "hostnameSize" );
  hostnameSize->setFocusPolicy( QWidget::StrongFocus );
  hostnameSize->setBackgroundMode( QWidget::PaletteBackground );
  hostnameSize->setFontPropagation( QWidget::NoChildren );
  hostnameSize->setPalettePropagation( QWidget::NoChildren );
  hostnameSize->setSizeLimit( 10 );
  hostnameSize->setAutoResize( FALSE );
  hostnameSize->insertItem(i18n("short"));
  hostnameSize->insertItem(i18n("full"));
  grid2->addWidget(hostnameSize,1,1);

  QWhatsThis::add(headerButton,
	i18n("If you wish to have a page header, then check it."));

  QWhatsThis::add(headertextLine,
	i18n("Here you can enter the headertext."));

  QWhatsThis::add(headertextButton,
	i18n("If you wish to have a headertext, then check it."));

  QWhatsThis::add(loginButton,
	i18n("If you wish to print a userlogin, then check it."));

  QWhatsThis::add(fancyHeaderButton,
	i18n("If you want a fancy header, then check it."));

  QWhatsThis::add(filenameLine,
	i18n("If you wish to print the filenames, then check it."));

  QString filenameSizeMsg = i18n("Here you can choose, if the only the basename,\n"
                                    "or the fully qualified filename is printed.");
  QWhatsThis::add(qtarch_Label_21, filenameSizeMsg);
  QWhatsThis::add(filenameSize, filenameSizeMsg);

  QString filenamePositionMsg = i18n("Here you can set the position in the header.");
  QWhatsThis::add(qtarch_Label_25, filenamePositionMsg);
  QWhatsThis::add(filenamePosition, filenamePositionMsg);

  QWhatsThis::add(hostnamePosition,
	i18n("Here you can set the position in the header."));

  QString hostnameSizeMsg = i18n("Here you can choose, if the hostname up to the first\n"
                        	     "dot , or the full hostname is printed.");
  QWhatsThis::add(qtarch_Label_23, hostnameSizeMsg);
  QWhatsThis::add(hostnameSize, hostnameSizeMsg);

  QWhatsThis::add(loginPosition,
	i18n("Here you can set the position in the header."));

  QWhatsThis::add(hostnameButton,
	i18n("If you wish to print the hostname, then check it."));

  QWhatsThis::add(headertextPosition,
	i18n("Here you can set the position in the header."));

}

void CConfigEnscriptDlg::initTab2(QWidget*parent)
{
  QVBoxLayout *box = new QVBoxLayout( parent );
  box->setMargin( 15 );
  box->setSpacing( 10 );

  /**current date */
  qtarch_ButtonGroup_74 = new QButtonGroup( parent, "ButtonGroup_74" );
  box->addWidget(qtarch_ButtonGroup_74);
  qtarch_ButtonGroup_74->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_74->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_74->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_74->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_74->setFrameStyle( 49 );
  qtarch_ButtonGroup_74->setTitle( i18n("Current date") );
  qtarch_ButtonGroup_74->setAlignment( 1 );

  QHBoxLayout *box1 = new QHBoxLayout( qtarch_ButtonGroup_74 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );

  currentDateButton = new QCheckBox( qtarch_ButtonGroup_74, "currentDateButton" );
  currentDateButton->setFocusPolicy( QWidget::TabFocus );
  currentDateButton->setBackgroundMode( QWidget::PaletteBackground );
  currentDateButton->setFontPropagation( QWidget::NoChildren );
  currentDateButton->setPalettePropagation( QWidget::NoChildren );
  currentDateButton->setText( i18n("current date") );
  currentDateButton->setAutoRepeat( FALSE );
  currentDateButton->setAutoResize( FALSE );
  box1->addWidget(currentDateButton);
  connect(currentDateButton,SIGNAL(clicked()),SLOT(slotCurrentDateClicked()));

  qtarch_ButtonGroup_88 = new QButtonGroup( qtarch_ButtonGroup_74, "ButtonGroup_88" );
  qtarch_ButtonGroup_88->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_88->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_88->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_88->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_88->setFrameStyle( 49 );
  qtarch_ButtonGroup_88->setTitle(i18n("Position") );
  qtarch_ButtonGroup_88->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_88);

  QHBoxLayout *box2 = new QHBoxLayout( qtarch_ButtonGroup_88 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );

  currentDatePosition = new QComboBox( FALSE,qtarch_ButtonGroup_88 , "currentDatePosition" );
  currentDatePosition->setFocusPolicy( QWidget::StrongFocus );
  currentDatePosition->setBackgroundMode( QWidget::PaletteBackground );
  currentDatePosition->setFontPropagation( QWidget::NoChildren );
  currentDatePosition->setPalettePropagation( QWidget::NoChildren );
  currentDatePosition->setSizeLimit( 10 );
  currentDatePosition->setAutoResize( FALSE );
  currentDatePosition->insertItem(i18n("left"));
  currentDatePosition->insertItem(i18n("center"));
  currentDatePosition->insertItem(i18n("right"));
  box2->addWidget(currentDatePosition);


  qtarch_ButtonGroup_87 = new QButtonGroup( qtarch_ButtonGroup_74, "ButtonGroup_87" );
  qtarch_ButtonGroup_87->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_87->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_87->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_87->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_87->setFrameStyle( 49 );
  qtarch_ButtonGroup_87->setTitle(i18n("Format") );
  qtarch_ButtonGroup_87->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_87);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_87 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );



  currentDateFormat = new QComboBox( FALSE, qtarch_ButtonGroup_87, "currentDateFormat" );
  currentDateFormat->setFocusPolicy( QWidget::StrongFocus );
  currentDateFormat->setBackgroundMode( QWidget::PaletteBackground );
  currentDateFormat->setFontPropagation( QWidget::NoChildren );
  currentDateFormat->setPalettePropagation( QWidget::NoChildren );
  currentDateFormat->setSizeLimit( 10 );
  currentDateFormat->setAutoResize( FALSE );
  currentDateFormat->insertItem( "dd.mm.yyyy" );
  currentDateFormat->insertItem( "yy-mm-dd" );
  currentDateFormat->insertItem( "yy/mm/dd" );
  currentDateFormat->insertItem( "mm/dd/yy" );
  box2->addWidget(currentDateFormat);


  /********************************************************************************/

  /**modification date */
  qtarch_ButtonGroup_76 = new QButtonGroup( parent, "ButtonGroup_76" );
  qtarch_ButtonGroup_76->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_76->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_76->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_76->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_76->setFrameStyle( 49 );
  qtarch_ButtonGroup_76->setTitle( i18n("Modification date") );
  qtarch_ButtonGroup_76->setAlignment( 1 );
  box->addWidget(qtarch_ButtonGroup_76);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_76 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );



  modificationDateButton = new QCheckBox( qtarch_ButtonGroup_76, "modificationDateButton" );
  modificationDateButton->setFocusPolicy( QWidget::TabFocus );
  modificationDateButton->setBackgroundMode( QWidget::PaletteBackground );
  modificationDateButton->setFontPropagation( QWidget::NoChildren );
  modificationDateButton->setPalettePropagation( QWidget::NoChildren );
  modificationDateButton->setText( i18n("modification date") );
  modificationDateButton->setAutoRepeat( FALSE );
  modificationDateButton->setAutoResize( FALSE );
  connect(modificationDateButton,SIGNAL(clicked()),SLOT(slotModiDateClicked()));
  box1->addWidget(modificationDateButton);


  qtarch_ButtonGroup_90 = new QButtonGroup( qtarch_ButtonGroup_76, "ButtonGroup_90" );
  qtarch_ButtonGroup_90->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_90->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_90->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_90->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_90->setFrameStyle( 49 );
  qtarch_ButtonGroup_90->setTitle(i18n("Position") );
  qtarch_ButtonGroup_90->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_90);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_90 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );

  modificationDatePosition = new QComboBox( FALSE,  qtarch_ButtonGroup_90, "modificationDatePosition" );
  modificationDatePosition->setFocusPolicy( QWidget::StrongFocus );
  modificationDatePosition->setBackgroundMode( QWidget::PaletteBackground );
  modificationDatePosition->setFontPropagation( QWidget::NoChildren );
  modificationDatePosition->setPalettePropagation( QWidget::NoChildren );
  modificationDatePosition->setSizeLimit( 10 );
  modificationDatePosition->setAutoResize( FALSE );
  modificationDatePosition->insertItem(i18n("left"));
  modificationDatePosition->insertItem(i18n("center"));
  modificationDatePosition->insertItem(i18n("right"));
  box2->addWidget(modificationDatePosition);


  qtarch_ButtonGroup_84 = new QButtonGroup( qtarch_ButtonGroup_76, "ButtonGroup_84" );
  qtarch_ButtonGroup_84->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_84->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_84->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_84->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_84->setFrameStyle( 49 );
  qtarch_ButtonGroup_84->setTitle(i18n("Format") );
  qtarch_ButtonGroup_84->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_84);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_84 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );

  modificationDateFormat = new QComboBox( FALSE, qtarch_ButtonGroup_84, "modificationDateFormat" );
  modificationDateFormat->setFocusPolicy( QWidget::StrongFocus );
  modificationDateFormat->setBackgroundMode( QWidget::PaletteBackground );
  modificationDateFormat->setFontPropagation( QWidget::NoChildren );
  modificationDateFormat->setPalettePropagation( QWidget::NoChildren );
  modificationDateFormat->setSizeLimit( 10 );
  modificationDateFormat->setAutoResize( FALSE );
  modificationDateFormat->insertItem( "dd.mm.yyyy" );
  modificationDateFormat->insertItem( "yy-mm-dd" );
  modificationDateFormat->insertItem( "yy/mm/dd" );
  modificationDateFormat->insertItem( "mm/dd/yy" );
  box2->addWidget(modificationDateFormat);

  /***********************************************************************************/

  /*current time*/
  qtarch_ButtonGroup_78 = new QButtonGroup( parent, "ButtonGroup_78" );
  qtarch_ButtonGroup_78->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_78->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_78->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_78->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_78->setFrameStyle( 49 );
  qtarch_ButtonGroup_78->setTitle(i18n("Current time") );
  qtarch_ButtonGroup_78->setAlignment( 1 );
  box->addWidget(qtarch_ButtonGroup_78);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_78 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );

  currentTimeButton = new QCheckBox( qtarch_ButtonGroup_78, "currentTimeButton" );
  currentTimeButton->setFocusPolicy( QWidget::TabFocus );
  currentTimeButton->setBackgroundMode( QWidget::PaletteBackground );
  currentTimeButton->setFontPropagation( QWidget::NoChildren );
  currentTimeButton->setPalettePropagation( QWidget::NoChildren );
  currentTimeButton->setText( i18n("current time") );
  currentTimeButton->setAutoRepeat( FALSE );
  currentTimeButton->setAutoResize( FALSE );
  connect(currentTimeButton,SIGNAL(clicked()),SLOT(slotCurrentTimeClicked()));
  box1->addWidget(currentTimeButton);

  qtarch_ButtonGroup_71 = new QButtonGroup( qtarch_ButtonGroup_78, "ButtonGroup_71" );
  qtarch_ButtonGroup_71->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_71->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_71->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_71->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_71->setFrameStyle( 49 );
  qtarch_ButtonGroup_71->setTitle(i18n("AM/PM") );
  qtarch_ButtonGroup_71->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_71);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_71 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );


  currentTimeAmpm = new QComboBox( FALSE,qtarch_ButtonGroup_71 , "currentTimeAmpm" );
  currentTimeAmpm->setFocusPolicy( QWidget::StrongFocus );
  currentTimeAmpm->setBackgroundMode( QWidget::PaletteBackground );
  currentTimeAmpm->setFontPropagation( QWidget::NoChildren );
  currentTimeAmpm->setPalettePropagation( QWidget::NoChildren );
  currentTimeAmpm->setSizeLimit( 10 );
  currentTimeAmpm->setAutoResize( FALSE );
  currentTimeAmpm->insertItem( "am/pm" );
  currentTimeAmpm->insertItem( "24 h" );
  connect(currentTimeAmpm,SIGNAL(activated(int)),SLOT(slotCurrentAmpmClicked(int)));
  box2->addWidget(currentTimeAmpm);


  qtarch_ButtonGroup_85 = new QButtonGroup( qtarch_ButtonGroup_78, "ButtonGroup_85" );
  qtarch_ButtonGroup_85->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_85->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_85->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_85->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_85->setFrameStyle( 49 );
  qtarch_ButtonGroup_85->setTitle( i18n("Position") );
  qtarch_ButtonGroup_85->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_85);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_85 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );

  currentTimePosition = new QComboBox( FALSE, qtarch_ButtonGroup_85, "currentTimePosition" );
  currentTimePosition->setFocusPolicy( QWidget::StrongFocus );
  currentTimePosition->setBackgroundMode( QWidget::PaletteBackground );
  currentTimePosition->setFontPropagation( QWidget::NoChildren );
  currentTimePosition->setPalettePropagation( QWidget::NoChildren );
  currentTimePosition->setSizeLimit( 10 );
  currentTimePosition->setAutoResize( FALSE );
  currentTimePosition->insertItem(i18n("left"));
  currentTimePosition->insertItem(i18n("center"));
  currentTimePosition->insertItem(i18n("right"));
  box2->addWidget(currentTimePosition);


  qtarch_ButtonGroup_89 = new QButtonGroup( qtarch_ButtonGroup_78, "ButtonGroup_89" );
  qtarch_ButtonGroup_89->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_89->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_89->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_89->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_89->setFrameStyle( 49 );
  qtarch_ButtonGroup_89->setTitle(i18n("Format") );
  qtarch_ButtonGroup_89->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_89);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_89 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );


  currentTimeFormat = new QComboBox( FALSE, qtarch_ButtonGroup_89, "currentTimeFormat" );
  currentTimeFormat->setFocusPolicy( QWidget::StrongFocus );
  currentTimeFormat->setBackgroundMode( QWidget::PaletteBackground );
  currentTimeFormat->setFontPropagation( QWidget::NoChildren );
  currentTimeFormat->setPalettePropagation( QWidget::NoChildren );
  currentTimeFormat->setSizeLimit( 10 );
  currentTimeFormat->setAutoResize( FALSE );
  currentTimeFormat->insertItem( "hh:mm:ss" );
  currentTimeFormat->insertItem( "hh:mm" );
  box2->addWidget(currentTimeFormat);

  /***********************************************************************************/

  /*modification time*/
  qtarch_ButtonGroup_77 = new QButtonGroup( parent, "ButtonGroup_77" );
  qtarch_ButtonGroup_77->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_77->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_77->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_77->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_77->setFrameStyle( 49 );
  qtarch_ButtonGroup_77->setTitle(i18n("Modification time") );
  qtarch_ButtonGroup_77->setAlignment( 1 );
  box->addWidget(qtarch_ButtonGroup_77);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_77 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );

  modificationTimeButton = new QCheckBox( qtarch_ButtonGroup_77, "modificationTimeButton" );
  modificationTimeButton->setFocusPolicy( QWidget::TabFocus );
  modificationTimeButton->setBackgroundMode( QWidget::PaletteBackground );
  modificationTimeButton->setFontPropagation( QWidget::NoChildren );
  modificationTimeButton->setPalettePropagation( QWidget::NoChildren );
  modificationTimeButton->setText( i18n("modification time") );
  modificationTimeButton->setAutoRepeat( FALSE );
  modificationTimeButton->setAutoResize( FALSE );
  connect(modificationTimeButton,SIGNAL(clicked()),SLOT(slotModiTimeClicked()));
  box1->addWidget(modificationTimeButton);

  qtarch_ButtonGroup_72 = new QButtonGroup( qtarch_ButtonGroup_77, "ButtonGroup_72" );
  qtarch_ButtonGroup_72->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_72->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_72->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_72->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_72->setFrameStyle( 49 );
  qtarch_ButtonGroup_72->setTitle(i18n("AM/PM") );
  qtarch_ButtonGroup_72->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_72);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_72 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );


  modificationTimeAmpm = new QComboBox( FALSE, qtarch_ButtonGroup_72, "modificationTimeAmpm" );
  modificationTimeAmpm->setFocusPolicy( QWidget::StrongFocus );
  modificationTimeAmpm->setBackgroundMode( QWidget::PaletteBackground );
  modificationTimeAmpm->setFontPropagation( QWidget::NoChildren );
  modificationTimeAmpm->setPalettePropagation( QWidget::NoChildren );
  modificationTimeAmpm->setSizeLimit( 10 );
  modificationTimeAmpm->setAutoResize( FALSE );
  modificationTimeAmpm->insertItem( "am/pm" );
  modificationTimeAmpm->insertItem( "24 h" );
  connect(modificationTimeAmpm,SIGNAL(activated(int)),SLOT(slotModificationAmpmClicked(int)));
  box2->addWidget(modificationTimeAmpm);


  qtarch_ButtonGroup_75 = new QButtonGroup( qtarch_ButtonGroup_77, "ButtonGroup_75" );
  qtarch_ButtonGroup_75->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_75->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_75->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_75->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_75->setFrameStyle( 49 );
  qtarch_ButtonGroup_75->setTitle(i18n("Position") );
  qtarch_ButtonGroup_75->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_75);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_75 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );

  modificationTimePosition = new QComboBox( FALSE,  qtarch_ButtonGroup_75, "modificationTimePosition" );
  modificationTimePosition->setFocusPolicy( QWidget::StrongFocus );
  modificationTimePosition->setBackgroundMode( QWidget::PaletteBackground );
  modificationTimePosition->setFontPropagation( QWidget::NoChildren );
  modificationTimePosition->setPalettePropagation( QWidget::NoChildren );
  modificationTimePosition->setSizeLimit( 10 );
  modificationTimePosition->setAutoResize( FALSE );
  modificationTimePosition->insertItem(i18n("left"));
  modificationTimePosition->insertItem(i18n("center"));
  modificationTimePosition->insertItem(i18n("right"));
  box2->addWidget(modificationTimePosition);

  qtarch_ButtonGroup_86 = new QButtonGroup( qtarch_ButtonGroup_77, "ButtonGroup_86" );
  qtarch_ButtonGroup_86->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_86->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_86->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_86->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_86->setFrameStyle( 49 );
  qtarch_ButtonGroup_86->setTitle( i18n("Format") );
  qtarch_ButtonGroup_86->setAlignment( 1 );
  box1->addWidget(qtarch_ButtonGroup_86);

  box2 = new QHBoxLayout( qtarch_ButtonGroup_86 );
  box2->setMargin( 15 );
  box2->setSpacing( 10 );

  modificationTimeFormat = new QComboBox( FALSE, qtarch_ButtonGroup_86, "modificationTimeFormat" );
  modificationTimeFormat->setFocusPolicy( QWidget::StrongFocus );
  modificationTimeFormat->setBackgroundMode( QWidget::PaletteBackground );
  modificationTimeFormat->setFontPropagation( QWidget::NoChildren );
  modificationTimeFormat->setPalettePropagation( QWidget::NoChildren );
  modificationTimeFormat->setSizeLimit( 10 );
  modificationTimeFormat->setAutoResize( FALSE );
  modificationTimeFormat->insertItem( "hh:mm:ss" );
  modificationTimeFormat->insertItem( "hh:mm" );
  box2->addWidget( modificationTimeFormat);



  /***********************************************************************************/

  QWhatsThis::add(currentTimeButton,
	i18n("If you wish to print the current time, then check it."));

  QWhatsThis::add(modificationTimeButton,
	i18n("If you wish to print the modification time, then check it."));

  QWhatsThis::add(modificationDateButton,
	i18n("If you wish to print the modification date, then check it."));

  QWhatsThis::add( currentDateButton,
	i18n("If you wish to print the current date, then check it."));

  QString timeFormatMsg = i18n("Here you can set the format.");
  QWhatsThis::add(modificationTimeFormat, timeFormatMsg);
  QWhatsThis::add(modificationDateFormat, timeFormatMsg);
  QWhatsThis::add(currentTimeFormat, timeFormatMsg);
  QWhatsThis::add(currentDateFormat, timeFormatMsg);


  QString timePositionMsg =	i18n("Here you can set the position in the header.");
  QWhatsThis::add(currentTimePosition, timePositionMsg);
  QWhatsThis::add(modificationTimePosition, timePositionMsg);
  QWhatsThis::add(currentDatePosition, timePositionMsg);
  QWhatsThis::add(modificationDatePosition, timePositionMsg);

  QString AmpmMsg = i18n("Here you can choose, if your time will be printed in\n"
                	     "am/pm format or in 24h format.");
  QWhatsThis::add(currentTimeAmpm, AmpmMsg);
  QWhatsThis::add(modificationTimeAmpm, AmpmMsg);
}

void CConfigEnscriptDlg::initTab3(QWidget*parent) {
  QGridLayout *grid1 = new QGridLayout(parent,4,2,15,7);

  qtarch_ButtonGroup_33 = new QButtonGroup( parent, "ButtonGroup_33" );
  qtarch_ButtonGroup_33->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_33->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_33->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_33->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_33->setFrameStyle( 49 );
  qtarch_ButtonGroup_33->setTitle(i18n("Numbering & border"));
  qtarch_ButtonGroup_33->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_33,0,1,0,0);

  QGridLayout *grid2 = new QGridLayout(qtarch_ButtonGroup_33,4,2,15,7);
  bordersButton = new QCheckBox( qtarch_ButtonGroup_33, "bordersButton" );
  bordersButton->setFocusPolicy( QWidget::TabFocus );
  bordersButton->setBackgroundMode( QWidget::PaletteBackground );
  bordersButton->setFontPropagation( QWidget::NoChildren );
  bordersButton->setPalettePropagation( QWidget::NoChildren );
  bordersButton->setText(i18n("borders"));
  bordersButton->setAutoRepeat( FALSE );
  bordersButton->setAutoResize( FALSE );
  grid2->addWidget(bordersButton,0,1);

  numberingPagesButton = new QCheckBox( qtarch_ButtonGroup_33, "numberingPagesButton" );
  numberingPagesButton->setFocusPolicy( QWidget::TabFocus );
  numberingPagesButton->setBackgroundMode( QWidget::PaletteBackground );
  numberingPagesButton->setFontPropagation( QWidget::NoChildren );
  numberingPagesButton->setPalettePropagation( QWidget::NoChildren );
  numberingPagesButton->setText(i18n("numbering pages"));
  numberingPagesButton->setAutoRepeat( FALSE );
  numberingPagesButton->setAutoResize( FALSE );
  grid2->addWidget(numberingPagesButton,1,0);
  
  qtarch_Label_8 = new QLabel( qtarch_ButtonGroup_33, "Label_8" );
  qtarch_Label_8->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_8->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_8->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_8->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_8->setText(i18n("align files"));
  qtarch_Label_8->setAlignment( 289 );
  qtarch_Label_8->setMargin( -1 );
  grid2->addWidget(qtarch_Label_8,2,0);
  
  numberingLineButton = new QCheckBox(qtarch_ButtonGroup_33 , "numberingLineButton" );
  numberingLineButton->setFocusPolicy( QWidget::TabFocus );
  numberingLineButton->setBackgroundMode( QWidget::PaletteBackground );
  numberingLineButton->setFontPropagation( QWidget::NoChildren );
  numberingLineButton->setPalettePropagation( QWidget::NoChildren );
  numberingLineButton->setText(i18n("numbering lines"));
  numberingLineButton->setAutoRepeat( FALSE );
  numberingLineButton->setAutoResize( FALSE );
  grid2->addWidget(numberingLineButton,0,0);
  
  linesPerPage = new QSpinBox( qtarch_ButtonGroup_33, "linesPerPage" );
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
  grid2->addWidget(linesPerPage,3,1);



  qtarch_label = new QLabel(qtarch_ButtonGroup_33, "label" );
  qtarch_label->setFocusPolicy( QWidget::NoFocus );
  qtarch_label->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_label->setFontPropagation( QWidget::NoChildren );
  qtarch_label->setPalettePropagation( QWidget::NoChildren );
  qtarch_label->setText(i18n("lines per page"));
  qtarch_label->setAlignment( 289 );
  qtarch_label->setMargin( -1 );
  grid2->addWidget(qtarch_label,3,0);


 alignFileList = new QComboBox( FALSE, qtarch_ButtonGroup_33, "alignFileList" );
  alignFileList->setFocusPolicy( QWidget::StrongFocus );
  alignFileList->setBackgroundMode( QWidget::PaletteBackground );
  alignFileList->setFontPropagation( QWidget::AllChildren );
  alignFileList->setPalettePropagation( QWidget::AllChildren );
  alignFileList->setSizeLimit( 10 );
  alignFileList->setAutoResize( FALSE );
  alignFileList->insertItem(i18n("yes"));
  alignFileList->insertItem(i18n("no"));
  grid2->addWidget(alignFileList,2,1);


  numberingPagesList = new QComboBox( FALSE, qtarch_ButtonGroup_33,"numberingPagesList" );
  numberingPagesList->setFocusPolicy( QWidget::StrongFocus );
  numberingPagesList->setBackgroundMode( QWidget::PaletteBackground );
  numberingPagesList->setFontPropagation( QWidget::NoChildren );
  numberingPagesList->setPalettePropagation( QWidget::NoChildren );
  numberingPagesList->setSizeLimit( 10 );
  numberingPagesList->setAutoResize( FALSE );
  numberingPagesList->insertItem(i18n("left"));
  numberingPagesList->insertItem(i18n("center"));
  numberingPagesList->insertItem(i18n("right"));
  grid2->addWidget(numberingPagesList,1,1);




  qtarch_ButtonGroup_20 = new QButtonGroup( parent, "ButtonGroup_20" );
  qtarch_ButtonGroup_20->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_20->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_20->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_20->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_20->setFrameStyle( 49 );
  qtarch_ButtonGroup_20->setTitle(i18n("Font and TAB"));
  qtarch_ButtonGroup_20->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_20,2,3,0,0);


  grid2 = new QGridLayout(qtarch_ButtonGroup_20,3,2,15,7);
  qtarch_Label_2 = new QLabel(qtarch_ButtonGroup_20 , "Label_2" );
  qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_2->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_2->setText(i18n("set TAB size"));
  qtarch_Label_2->setAlignment( 289 );
  qtarch_Label_2->setMargin( -1 );
  grid2->addWidget(qtarch_Label_2,0,0);

  qtarch_Label_4 = new QLabel( qtarch_ButtonGroup_20, "Label_4" );
  qtarch_Label_4->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_4->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_4->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_4->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_4->setText(i18n("font for header"));
  qtarch_Label_4->setAlignment( 289 );
  qtarch_Label_4->setMargin( -1 );
  grid2->addWidget( qtarch_Label_4,1,0);

  setTabSize = new QSpinBox( qtarch_ButtonGroup_20, "setTabSize" );
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
  grid2->addWidget(setTabSize,0,1);
  
  fontForBodyButton = new QPushButton( qtarch_ButtonGroup_20, "fontForBodyButton" );
  fontForBodyButton->setFocusPolicy( QWidget::TabFocus );
  fontForBodyButton->setBackgroundMode( QWidget::PaletteBackground );
  fontForBodyButton->setFontPropagation( QWidget::NoChildren );
  fontForBodyButton->setPalettePropagation( QWidget::NoChildren );
  fontForBodyButton->setText( "" );
  fontForBodyButton->setAutoRepeat( FALSE );
  fontForBodyButton->setAutoResize( FALSE );
  grid2->addWidget(fontForBodyButton,2,1);

  qtarch_Label_5 = new QLabel( qtarch_ButtonGroup_20, "Label_5" );
  qtarch_Label_5->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_5->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_5->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_5->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_5->setText(i18n("font for body"));
  qtarch_Label_5->setAlignment( 289 );
  qtarch_Label_5->setMargin( -1 );
  grid2->addWidget(qtarch_Label_5,2,0);
  

  
  fontForHeaderButton = new QPushButton( qtarch_ButtonGroup_20 , "fontForHeaderButton" );
  fontForHeaderButton->setFocusPolicy( QWidget::TabFocus );
  fontForHeaderButton->setBackgroundMode( QWidget::PaletteBackground );
  fontForHeaderButton->setFontPropagation( QWidget::NoChildren );
  fontForHeaderButton->setPalettePropagation( QWidget::NoChildren );
  fontForHeaderButton->setText( "" );
  fontForHeaderButton->setAutoRepeat( FALSE );
  fontForHeaderButton->setAutoResize( FALSE );
  grid2->addWidget(fontForHeaderButton,1,1);



  qtarch_ButtonGroup_51 = new QButtonGroup( parent, "ButtonGroup_51" );
 
  qtarch_ButtonGroup_51->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_51->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_51->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_51->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_51->setFrameStyle( 49 );
  qtarch_ButtonGroup_51->setTitle(i18n("Other options"));
  qtarch_ButtonGroup_51->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_51,1,3,1,1);

  grid2 = new QGridLayout(qtarch_ButtonGroup_51,3,1,15,7);
  

  tocButton = new QCheckBox( qtarch_ButtonGroup_51, "tocButton" );
  tocButton->setFocusPolicy( QWidget::TabFocus );
  tocButton->setBackgroundMode( QWidget::PaletteBackground );
  tocButton->setFontPropagation( QWidget::NoChildren );
  tocButton->setPalettePropagation( QWidget::NoChildren );
  tocButton->setText(i18n("table of contents"));
  tocButton->setAutoRepeat( FALSE );
  tocButton->setAutoResize( FALSE );
  grid2->addWidget(tocButton,0,0);
  
  qtarch_ButtonGroup_52 = new QButtonGroup( qtarch_ButtonGroup_51, "ButtonGroup_52" );
  qtarch_ButtonGroup_52->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_52->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_52->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_52->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_52->setFrameStyle( 49 );
  qtarch_ButtonGroup_52->setTitle(i18n("Highlight bars"));
  qtarch_ButtonGroup_52->setAlignment( 1 );
  grid2->addWidget(qtarch_ButtonGroup_52,1,0);
  
  QGridLayout *grid3 = new QGridLayout(qtarch_ButtonGroup_52,2,2,15,7);


  qtarch_Label_14 = new QLabel(qtarch_ButtonGroup_52 , "Label_14" );
  qtarch_Label_14->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_14->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_14->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_14->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_14->setText(i18n("cycle of change"));
  qtarch_Label_14->setAlignment( 289 );
  qtarch_Label_14->setMargin( -1 );
  grid3->addWidget(qtarch_Label_14,1,1);


  highlightBarsButton = new QCheckBox(qtarch_ButtonGroup_52 , "highlightBarsButton" );
  highlightBarsButton->setFocusPolicy( QWidget::TabFocus );
  highlightBarsButton->setBackgroundMode( QWidget::PaletteBackground );
  highlightBarsButton->setFontPropagation( QWidget::NoChildren );
  highlightBarsButton->setPalettePropagation( QWidget::NoChildren );
  highlightBarsButton->setText(i18n("highlight bars"));
  highlightBarsButton->setAutoRepeat( FALSE );
  highlightBarsButton->setAutoResize( FALSE );
  grid3->addWidget(highlightBarsButton,0,0);

  cycleOfChange = new QSpinBox( qtarch_ButtonGroup_52, "cycleOfChange" );
  cycleOfChange->setFocusPolicy( QWidget::StrongFocus );
  cycleOfChange->setBackgroundMode( QWidget::PaletteBackground );
  cycleOfChange->setFontPropagation( QWidget::NoChildren );
  cycleOfChange->setPalettePropagation( QWidget::NoChildren );
  cycleOfChange->setFrameStyle( 50 );
  cycleOfChange->setLineWidth( 2 );
  cycleOfChange->setRange( 1, 99 );
  cycleOfChange->setSteps( 1, 0 );
  cycleOfChange->setPrefix( "" );
  cycleOfChange->setSuffix( "" );
  cycleOfChange->setSpecialValueText( "" );
  cycleOfChange->setWrapping( FALSE );
  grid3->addWidget(cycleOfChange,1,1);



  qtarch_ButtonGroup_53 = new QButtonGroup( qtarch_ButtonGroup_51, "ButtonGroup_53" );
  qtarch_ButtonGroup_53->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_53->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_53->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_53->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_53->setFrameStyle( 49 );
  qtarch_ButtonGroup_53->setTitle(i18n("Wrapped line"));
  qtarch_ButtonGroup_53->setAlignment( 1 );
  grid2->addWidget(qtarch_ButtonGroup_53,2,0);
  
 
  grid3 = new QGridLayout(qtarch_ButtonGroup_53,2,2,15,7);
  markedWrappedLinesButton = new QCheckBox( qtarch_ButtonGroup_53, "markedWrappedLinesButton" );
  markedWrappedLinesButton->setFocusPolicy( QWidget::TabFocus );
  markedWrappedLinesButton->setBackgroundMode( QWidget::PaletteBackground );
  markedWrappedLinesButton->setFontPropagation( QWidget::NoChildren );
  markedWrappedLinesButton->setPalettePropagation( QWidget::NoChildren );
  markedWrappedLinesButton->setText(i18n("marked wrapped lines"));
  markedWrappedLinesButton->setAutoRepeat( FALSE );
  markedWrappedLinesButton->setAutoResize( FALSE );
  grid3->addWidget(markedWrappedLinesButton,0,0);
  


  valueForWrappedLine = new QComboBox( FALSE, qtarch_ButtonGroup_53, "valueForWrappedLine" );
  valueForWrappedLine->setFocusPolicy( QWidget::StrongFocus );
  valueForWrappedLine->setBackgroundMode( QWidget::PaletteBackground );
  valueForWrappedLine->setFontPropagation( QWidget::NoChildren );
  valueForWrappedLine->setPalettePropagation( QWidget::NoChildren );
  valueForWrappedLine->setSizeLimit( 10 );
  valueForWrappedLine->setAutoResize( FALSE );
  valueForWrappedLine->insertItem( "black box" );
  valueForWrappedLine->insertItem( "arrow" );
  valueForWrappedLine->insertItem( "plus (+)" );
  valueForWrappedLine->insertItem( "none" );
  grid3->addWidget(valueForWrappedLine,1,1);
  
 
  
  qtarch_Label_13 = new QLabel( qtarch_ButtonGroup_53 , "Label_13" );
  qtarch_Label_13->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_13->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_13->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_13->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_13->setText(i18n("value for wrapped line"));
  qtarch_Label_13->setAlignment( 289 );
  qtarch_Label_13->setMargin( -1 );
  grid3->addWidget( qtarch_Label_13,1,0);
  


  qtarch_ButtonGroup_21 = new QButtonGroup( parent, "ButtonGroup_174" );
  qtarch_ButtonGroup_21->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_21->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_21->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_21->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_21->setFrameStyle( 49 );
  qtarch_ButtonGroup_21->setTitle(i18n("Textprinting"));
  qtarch_ButtonGroup_21->setAlignment( 1 );

  grid1->addWidget(qtarch_ButtonGroup_21,0,1);


  grid2 = new QGridLayout(qtarch_ButtonGroup_21,2,1,15,7);

  cutLinesButton = new QCheckBox( qtarch_ButtonGroup_21, "cutLinesButton" );
  cutLinesButton->setFocusPolicy( QWidget::TabFocus );
  cutLinesButton->setBackgroundMode( QWidget::PaletteBackground );
  cutLinesButton->setFontPropagation( QWidget::NoChildren );
  cutLinesButton->setPalettePropagation( QWidget::NoChildren );
  cutLinesButton->setText(i18n("cut lines"));
  cutLinesButton->setAutoRepeat( FALSE );
  cutLinesButton->setAutoResize( FALSE );
  grid2->addWidget(cutLinesButton,0,0);
  
  replaceButton = new QCheckBox( qtarch_ButtonGroup_21, "replaceButton" );
  replaceButton->setFocusPolicy( QWidget::TabFocus );
  replaceButton->setBackgroundMode( QWidget::PaletteBackground );
  replaceButton->setFontPropagation( QWidget::NoChildren );
  replaceButton->setPalettePropagation( QWidget::NoChildren );
  replaceButton->setText(i18n("replace non-printing character by space"));
  replaceButton->setAutoRepeat( FALSE );
  replaceButton->setAutoResize( FALSE );
  grid2->addWidget(replaceButton,1,0);
  

  QWhatsThis::add(bordersButton,
	i18n("If button is checked, borders around columns are printed."));

  QWhatsThis::add(tocButton,
	i18n("If button is checked, a summary is printed at the end."));

  QWhatsThis::add(markedWrappedLinesButton,
	i18n("If button is checked, wrapped lines are marked in the output."));

  QWhatsThis::add(numberingPagesButton,
	i18n("If button is checked, the sides are numbered."));

  QWhatsThis::add(numberingLineButton,
	i18n("Precede each line with its line number."));

  QString lineperPageMsg = i18n("Print setting number lines per each page.");
  QWhatsThis::add(qtarch_label, lineperPageMsg);
  QWhatsThis::add(linesPerPage, lineperPageMsg);

  QWhatsThis::add(alignFileList,
	i18n("Align separate input files to even align page count.\n"
	     "This is handy for two-side and 2-up printings."));

  QWhatsThis::add(valueForWrappedLine,
	i18n("Set the style of the wrap value."));

  QString cycleOfChangeMsg = i18n("Specify  how high highlight bars are in lines.");
  QWhatsThis::add(qtarch_Label_14, cycleOfChangeMsg);
  QWhatsThis::add(cycleOfChange, cycleOfChangeMsg);

  QWhatsThis::add(numberingPagesList,
	i18n("Set the position in the header."));

  QWhatsThis::add(highlightBarsButton,
	i18n("If button is checked, highlight bars are printed."));

  QWhatsThis::add(qtarch_Label_2, i18n("Set tabulator size to specify number."));
  QWhatsThis::add(setTabSize, i18n("Set tabulator size to specify number."));

  QWhatsThis::add(cutLinesButton,
	i18n("If button is checked, lines that are too long will\n"
	     "be cut. Otherwise enscript wraps long lines  so no\n"
	     "information is lost."));

  QWhatsThis::add(replaceButton,
	i18n("If button is checked, non printing characters are\n"
	     "replace by space."));

  QWhatsThis::add(fontForBodyButton,
	i18n("If you clicked the button, a fontdialog is shown\n"
	     "and you can select a font that is used for body text."));

  QWhatsThis::add(fontForHeaderButton,
	i18n("If you clicked the button, a fontdialog is shown\n"
	     "and you can select a font that is used for header text."));

  connect (markedWrappedLinesButton,SIGNAL(clicked()),SLOT(slotWrappedLinesClicked()));
  connect (fontForHeaderButton,SIGNAL(clicked()),SLOT(slotFontHeaderClicked()));
  connect (fontForBodyButton,SIGNAL(clicked()),SLOT(slotFontBodyClicked()));
  connect (cutLinesButton,SIGNAL(clicked()),SLOT(slotCutLinesClicked()));
  connect (highlightBarsButton,SIGNAL(clicked()),SLOT(slotHighlightBarsClicked()));
  connect (numberingPagesButton,SIGNAL(clicked()),SLOT(slotNumberingPagesClicked()));
  connect (markedWrappedLinesButton,SIGNAL(clicked()),SLOT(slotWrappedLinesClicked()));
 }

void CConfigEnscriptDlg::initTab4(QWidget*parent) {

  QGridLayout *grid1 = new QGridLayout(parent,5,2,15,7);
  qtarch_ButtonGroup_138 = new QButtonGroup( parent, "ButtonGroup_138" );
  qtarch_ButtonGroup_138->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_138->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_138->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_138->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_138->setFrameStyle( 49 );
  qtarch_ButtonGroup_138->setTitle(i18n("Underlaytext") );
  qtarch_ButtonGroup_138->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_138,1,1,0,1);

  QGridLayout *grid2 = new QGridLayout(qtarch_ButtonGroup_138,1,2,15,7);

  qtarch_Label_75 = new QLabel( qtarch_ButtonGroup_138, "Label_75" );
  qtarch_Label_75->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_75->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_75->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_75->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_75->setText(i18n("underlaytext") );
  qtarch_Label_75->setAlignment( 289 );
  qtarch_Label_75->setMargin( -1 );
  grid2->addWidget( qtarch_Label_75,0,0);
  
  underlaytextLine = new QLineEdit(  qtarch_ButtonGroup_138, "underlaytextLine" );
  underlaytextLine->setFocusPolicy( QWidget::StrongFocus );
  underlaytextLine->setBackgroundMode( QWidget::PaletteBase );
  underlaytextLine->setFontPropagation( QWidget::NoChildren );
  underlaytextLine->setPalettePropagation( QWidget::NoChildren );
  underlaytextLine->setText( "" );
  underlaytextLine->setMaxLength( 32767 );
  underlaytextLine->setEchoMode( QLineEdit::Normal );
  underlaytextLine->setFrame( TRUE );
  grid2->addWidget(underlaytextLine ,0,1);


  
  qtarch_ButtonGroup_137 = new QButtonGroup( parent, "ButtonGroup_137" );
  qtarch_ButtonGroup_137->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_137->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_137->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_137->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_137->setFrameStyle( 49 );
  qtarch_ButtonGroup_137->setTitle(i18n("Underlaystyle") );
  qtarch_ButtonGroup_137->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_137,4,1);


  grid2 = new QGridLayout(qtarch_ButtonGroup_137,1,2,15,7);
  qtarch_Label_70 = new QLabel(qtarch_ButtonGroup_137 , "Label_70" );
  qtarch_Label_70->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_70->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_70->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_70->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_70->setText( i18n("underlaystyle") );
  qtarch_Label_70->setAlignment( 289 );
  qtarch_Label_70->setMargin( -1 );
  grid2->addWidget(qtarch_Label_70,0,0);
  
  underlayStyle = new QComboBox( FALSE,qtarch_ButtonGroup_137 , "underlayStyle" );
  underlayStyle->setFocusPolicy( QWidget::StrongFocus );
  underlayStyle->setBackgroundMode( QWidget::PaletteBackground );
  underlayStyle->setFontPropagation( QWidget::AllChildren );
  underlayStyle->setPalettePropagation( QWidget::AllChildren );
  underlayStyle->setSizeLimit( 10 );
  underlayStyle->setAutoResize( FALSE );
  underlayStyle->insertItem(i18n("outline"));
  underlayStyle->insertItem(i18n("filled"));
  grid2->addWidget(underlayStyle,0,1);


  
  qtarch_ButtonGroup_136 = new QButtonGroup( parent, "ButtonGroup_136" );
  qtarch_ButtonGroup_136->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_136->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_136->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_136->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_136->setFrameStyle( 49 );
  qtarch_ButtonGroup_136->setTitle(i18n("Underlayangle") );
  qtarch_ButtonGroup_136->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_136,3,1);

  grid2 = new QGridLayout(qtarch_ButtonGroup_136,2,2,15,7);
  qtarch_Label_71 = new QLabel(qtarch_ButtonGroup_136 , "Label_71" );
  qtarch_Label_71->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_71->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_71->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_71->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_71->setText( i18n("angle") );
  qtarch_Label_71->setAlignment( 289 );
  qtarch_Label_71->setMargin( -1 );
  grid2->addWidget(qtarch_Label_71,1,0);



  underlayAngleDefault = new QCheckBox(qtarch_ButtonGroup_136 , "underlayAngleDefault" );
  underlayAngleDefault->setFocusPolicy( QWidget::TabFocus );
  underlayAngleDefault->setBackgroundMode( QWidget::PaletteBackground );
  underlayAngleDefault->setFontPropagation( QWidget::NoChildren );
  underlayAngleDefault->setPalettePropagation( QWidget::NoChildren );
  underlayAngleDefault->setText( i18n("default") );
  underlayAngleDefault->setAutoRepeat( FALSE );
  underlayAngleDefault->setAutoResize( FALSE );
  connect (underlayAngleDefault,SIGNAL(clicked()),SLOT(slotUnderlayAngleDefaultClicked()));
  grid2->addWidget(underlayAngleDefault,0,0);

  underlayAngle = new QSpinBox( qtarch_ButtonGroup_136, "underlayAngle" );
  underlayAngle->setFocusPolicy( QWidget::StrongFocus );
  underlayAngle->setBackgroundMode( QWidget::PaletteBackground );
  underlayAngle->setFontPropagation( QWidget::NoChildren );
  underlayAngle->setPalettePropagation( QWidget::NoChildren );
  underlayAngle->setFrameStyle( 50 );
  underlayAngle->setLineWidth( 2 );
  underlayAngle->setRange( 0, 360 );
  underlayAngle->setSteps( 1, 0 );
  underlayAngle->setPrefix( "" );
  underlayAngle->setSuffix( "" );
  underlayAngle->setSpecialValueText( "" );
  underlayAngle->setWrapping( FALSE );
  grid2->addWidget(underlayAngle,1,1);

  
  qtarch_ButtonGroup_135 = new QButtonGroup( parent, "ButtonGroup_135" );
  qtarch_ButtonGroup_135->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_135->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_135->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_135->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_135->setFrameStyle( 49 );
  qtarch_ButtonGroup_135->setTitle( i18n("Underlaygray") );
  qtarch_ButtonGroup_135->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_135,4,0);
  grid2  = new QGridLayout(qtarch_ButtonGroup_135,1,2,15,7);


  qtarch_Label_74 = new QLabel(qtarch_ButtonGroup_135 , "Label_74" );
  qtarch_Label_74->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_74->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_74->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_74->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_74->setText(i18n("underlaygray (x(1/10))"));
  qtarch_Label_74->setAlignment( 289 );
  qtarch_Label_74->setMargin( -1 );
  grid2->addWidget( qtarch_Label_74,0,0);


  underlayGray = new QSpinBox( qtarch_ButtonGroup_135, "underlayGray" );
  underlayGray->setFocusPolicy( QWidget::StrongFocus );
  underlayGray->setBackgroundMode( QWidget::PaletteBackground );
  underlayGray->setFontPropagation( QWidget::NoChildren );
  underlayGray->setPalettePropagation( QWidget::NoChildren );
  underlayGray->setFrameStyle( 50 );
  underlayGray->setLineWidth( 2 );
  underlayGray->setRange( 0, 10 );
  underlayGray->setSteps( 1, 0 );
  underlayGray->setPrefix( "" );
  underlayGray->setSuffix( "" );
  underlayGray->setSpecialValueText( "" );
  underlayGray->setWrapping( FALSE );
  grid2->addWidget(underlayGray ,0,1);

  
  qtarch_ButtonGroup_134 = new QButtonGroup( parent, "ButtonGroup_134" );
  qtarch_ButtonGroup_134->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_134->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_134->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_134->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_134->setFrameStyle( 49 );
  qtarch_ButtonGroup_134->setTitle( i18n("Underlayfont") );
  qtarch_ButtonGroup_134->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_134,3,0);
  grid2  = new QGridLayout(qtarch_ButtonGroup_134,1,2,15,7);

  underlayFontButton = new QPushButton( qtarch_ButtonGroup_134, "underlayFontButton" );
  underlayFontButton->setFocusPolicy( QWidget::TabFocus );
  underlayFontButton->setBackgroundMode( QWidget::PaletteBackground );
  underlayFontButton->setFontPropagation( QWidget::NoChildren );
  underlayFontButton->setPalettePropagation( QWidget::NoChildren );
  underlayFontButton->setText( "" );
  underlayFontButton->setAutoRepeat( FALSE );
  underlayFontButton->setAutoResize( FALSE );
  connect (underlayFontButton, SIGNAL(clicked()),SLOT(slotFontUnderlayClicked()));
  grid2->addWidget(underlayFontButton,0,1);


  qtarch_Label_73 = new QLabel( qtarch_ButtonGroup_134, "Label_73" );
  qtarch_Label_73->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_73->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_73->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_73->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_73->setText(i18n("font") );
  qtarch_Label_73->setAlignment( 289 );
  qtarch_Label_73->setMargin( -1 );
  grid2->addWidget(qtarch_Label_73,0,0);
  


  
  qtarch_ButtonGroup_131 = new QButtonGroup( parent, "ButtonGroup_131" );
 
  qtarch_ButtonGroup_131->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_131->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_131->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_131->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_131->setFrameStyle( 49 );
  qtarch_ButtonGroup_131->setTitle( i18n("Underlayposition") );
  qtarch_ButtonGroup_131->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_131,2,2,0,1);

  grid2  = new QGridLayout(qtarch_ButtonGroup_131,1,4,15,7);

  underlayPositionDefaultButton = new QCheckBox( qtarch_ButtonGroup_131, "underlayPositionDefaultButton" );
  
  underlayPositionDefaultButton->setFocusPolicy( QWidget::TabFocus );
  underlayPositionDefaultButton->setBackgroundMode( QWidget::PaletteBackground );
  underlayPositionDefaultButton->setFontPropagation( QWidget::NoChildren );
  underlayPositionDefaultButton->setPalettePropagation( QWidget::NoChildren );
  underlayPositionDefaultButton->setText(i18n("default") );
  underlayPositionDefaultButton->setAutoRepeat( FALSE );
  underlayPositionDefaultButton->setAutoResize( FALSE );
  connect (underlayPositionDefaultButton,SIGNAL(clicked()),SLOT(slotUnderlayPositionDefaultClicked()));
  grid2->addWidget(underlayPositionDefaultButton,0,0);


  qtarch_Label_72 = new QLabel( qtarch_ButtonGroup_131, "Label_72" );
  qtarch_Label_72->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_72->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_72->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_72->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_72->setText( i18n("position") );
  qtarch_Label_72->setAlignment( 289 );
  qtarch_Label_72->setMargin( -1 );
  grid2->addWidget(qtarch_Label_72,0,1);
  

  qtarch_ButtonGroup_133 = new QButtonGroup( qtarch_ButtonGroup_131, "ButtonGroup_133" );
  qtarch_ButtonGroup_133->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_133->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_133->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_133->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_133->setFrameStyle( 49 );
  qtarch_ButtonGroup_133->setTitle( i18n("X-Position") );
  qtarch_ButtonGroup_133->setAlignment( 1 );
  grid2->addWidget(qtarch_ButtonGroup_133,0,2);

  QGridLayout *grid3 = new QGridLayout(qtarch_ButtonGroup_133,1,1,15,7);
  
  underlayXPosition = new QSpinBox( qtarch_ButtonGroup_133, "underlayXPosition" );
  underlayXPosition->setFocusPolicy( QWidget::StrongFocus );
  underlayXPosition->setBackgroundMode( QWidget::PaletteBackground );
  underlayXPosition->setFontPropagation( QWidget::NoChildren );
  underlayXPosition->setPalettePropagation( QWidget::NoChildren );
  underlayXPosition->setFrameStyle( 50 );
  underlayXPosition->setLineWidth( 2 );
  underlayXPosition->setRange( 0, 10000 );
  underlayXPosition->setSteps( 1, 0 );
  underlayXPosition->setPrefix( "" );
  underlayXPosition->setSuffix( "" );
  underlayXPosition->setSpecialValueText( "" );
  underlayXPosition->setWrapping( FALSE );
  grid3->addWidget(underlayXPosition,0,0);


  qtarch_ButtonGroup_132 = new QButtonGroup( qtarch_ButtonGroup_131, "ButtonGroup_132" );
  qtarch_ButtonGroup_132->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_132->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_132->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_132->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_132->setFrameStyle( 49 );
  qtarch_ButtonGroup_132->setTitle( i18n("Y-Position") );
  qtarch_ButtonGroup_132->setAlignment( 1 );
  grid2->addWidget(qtarch_ButtonGroup_132,0,3);

  grid3 = new QGridLayout(qtarch_ButtonGroup_132,1,1,15,7);
  underlayYPosition = new QSpinBox( qtarch_ButtonGroup_132, "underlayYPosition" );
  underlayYPosition->setFocusPolicy( QWidget::StrongFocus );
  underlayYPosition->setBackgroundMode( QWidget::PaletteBackground );
  underlayYPosition->setFontPropagation( QWidget::NoChildren );
  underlayYPosition->setPalettePropagation( QWidget::NoChildren );
  underlayYPosition->setFrameStyle( 50 );
  underlayYPosition->setLineWidth( 2 );
  underlayYPosition->setRange( 0, 10000 );
  underlayYPosition->setSteps( 1, 0 );
  underlayYPosition->setPrefix( "" );
  underlayYPosition->setSuffix( "" );
  underlayYPosition->setSpecialValueText( "" );
  underlayYPosition->setWrapping( FALSE );

  grid3->addWidget(underlayYPosition,0,0);

 
  
  underlayButton = new QCheckBox(parent , "underlayButton" );
  underlayButton->setFocusPolicy( QWidget::TabFocus );
  underlayButton->setBackgroundMode( QWidget::PaletteBackground );
  underlayButton->setFontPropagation( QWidget::NoChildren );
  underlayButton->setPalettePropagation( QWidget::NoChildren );
  underlayButton->setText( i18n("underlay") );
  underlayButton->setAutoRepeat( FALSE );
  underlayButton->setAutoResize( FALSE );
  grid1->addWidget(underlayButton,0,0);
  connect (underlayButton,SIGNAL(clicked()),SLOT(slotUnderlayButtonClicked()));
  

  QWhatsThis::add(underlayButton,
	i18n("If you wish to print underlayed text, then check it."));

  QWhatsThis::add(qtarch_Label_71,
	i18n("Here you can set the angle of the underlayed text."));
  QWhatsThis::add(underlayAngle,
	i18n("Here you can set the angle of the underlayed text."));

  QWhatsThis::add(qtarch_ButtonGroup_133,
	i18n("Here you can set the x-position of the underlayed text."));
  QWhatsThis::add(underlayXPosition,
	i18n("Here you can set the x-position of the underlayed text."));

  QWhatsThis::add(qtarch_ButtonGroup_132,
	i18n("Here you can set the y-position of the underlayed text."));
  QWhatsThis::add(underlayYPosition,
	i18n("Here you can set the y-position of the underlayed text."));

  QWhatsThis::add(qtarch_Label_74,
	i18n("Here you can set the underlayed text gray.\nThe lower the darker is the text."));
  QWhatsThis::add(underlayGray,
	i18n("Here you can set the underlayed text gray.\nThe lower the darker is the text."));

  QWhatsThis::add(underlayAngleDefault,
	i18n("If you wish to print the underlayed text with the\n"
	     "default angle, then check it."));

  QWhatsThis::add(underlayStyle,
	i18n("Here you can set the style of the underlayed text."));

  QWhatsThis::add(underlayPositionDefaultButton,
	i18n("If you wish to print the underlayed text with the\n"
	     "default position, then check it."));

  QWhatsThis::add(underlaytextLine,
	i18n("Here you can enter the text."));

  QWhatsThis::add(underlayFontButton,
	i18n("Here you can set another font by clicking on it."));
}

void CConfigEnscriptDlg::slotHeaderClicked() {
  if (headerButton->isChecked()) {
    loginButton->setEnabled(true);
    headertextButton->setEnabled(true);
    filenameLine->setEnabled(true);
    currentDateButton->setEnabled(true);
    modificationDateButton->setEnabled(true);
    currentTimeButton->setEnabled(true);
    modificationTimeButton->setEnabled(true);
    fancyHeaderButton->setEnabled(true);
    hostnameButton->setEnabled(true);
    fontForHeaderButton->setEnabled(true);
    slotHeadertextClicked();
    slotLoginClicked();
    slotFilenameClicked();
    slotHostnameClicked();
    slotCurrentDateClicked();
    slotCurrentTimeClicked();
    slotModiDateClicked();
    slotModiTimeClicked();
    qtarch_ButtonGroup_59->setEnabled(true);
    qtarch_ButtonGroup_61->setEnabled(true);
    qtarch_ButtonGroup_62->setEnabled(true);
    qtarch_ButtonGroup_60->setEnabled(true);
    qtarch_ButtonGroup_65->setEnabled(true);
    qtarch_ButtonGroup_66->setEnabled(true);
    qtarch_ButtonGroup_78->setEnabled(true);
    qtarch_ButtonGroup_77->setEnabled(true);
    qtarch_ButtonGroup_76->setEnabled(true);
    qtarch_ButtonGroup_74->setEnabled(true);
    qtarch_Label_4->setEnabled(true);
  }
  else {
    slotNoHeader();
    
  }
}

void CConfigEnscriptDlg::slotNoHeader() {
  headertextLine->setEnabled(false);
  headertextPosition->setEnabled(false);
  qtarch_Label_19->setEnabled(false);
  qtarch_Label_20->setEnabled(false);
  loginPosition->setEnabled(false);
  qtarch_Label_24->setEnabled(false);
  qtarch_Label_21->setEnabled(false);
  qtarch_Label_25->setEnabled(false);
  filenameSize->setEnabled(false);
  filenamePosition->setEnabled(false);
  qtarch_Label_23->setEnabled(false);
  qtarch_Label_22->setEnabled(false);
  hostnamePosition->setEnabled(false);
  hostnameSize->setEnabled(false);
  qtarch_ButtonGroup_75->setEnabled(false);
  qtarch_ButtonGroup_84->setEnabled(false);
  currentDatePosition->setEnabled(false);
  currentDateFormat->setEnabled(false);
  qtarch_ButtonGroup_90->setEnabled(false);
  qtarch_ButtonGroup_71->setEnabled(false);
  qtarch_ButtonGroup_89->setEnabled(false);
  currentTimeAmpm->setEnabled(false);
  currentTimePosition->setEnabled(false);
  currentTimeFormat->setEnabled(false);
  qtarch_ButtonGroup_86->setEnabled(false);
  qtarch_ButtonGroup_85->setEnabled(false);
  modificationDatePosition->setEnabled(false);
  modificationDateFormat->setEnabled(false);
  qtarch_ButtonGroup_87->setEnabled(false);
  qtarch_ButtonGroup_72->setEnabled(false);
  qtarch_ButtonGroup_88->setEnabled(false);
  modificationTimeAmpm->setEnabled(false);
  modificationTimePosition->setEnabled(false);
  modificationTimeFormat->setEnabled(false);
  loginButton->setEnabled(false);
  headertextButton->setEnabled(false);
  filenameLine->setEnabled(false);
  currentDateButton->setEnabled(false);
  modificationDateButton->setEnabled(false);
  currentTimeButton->setEnabled(false);
  modificationTimeButton->setEnabled(false);
  fancyHeaderButton->setEnabled(false);
  hostnameButton->setEnabled(false);
  fontForHeaderButton->setEnabled(false);
  qtarch_ButtonGroup_59->setEnabled(false);
  qtarch_ButtonGroup_61->setEnabled(false);
  qtarch_ButtonGroup_62->setEnabled(false);
  qtarch_ButtonGroup_60->setEnabled(false);
  qtarch_ButtonGroup_65->setEnabled(false);
  qtarch_ButtonGroup_66->setEnabled(false);
  qtarch_ButtonGroup_78->setEnabled(false);
  qtarch_ButtonGroup_77->setEnabled(false);
  qtarch_ButtonGroup_76->setEnabled(false);
  qtarch_ButtonGroup_74->setEnabled(false);
  qtarch_Label_4->setEnabled(false);
}

void CConfigEnscriptDlg::slotHeadertextClicked() {
  if (headertextButton->isChecked()) {
    headertextLine->setEnabled(true);
    headertextPosition->setEnabled(true);
    qtarch_Label_19->setEnabled(true);
    qtarch_Label_20->setEnabled(true);
  }
  else {
    headertextLine->setEnabled(false);
    headertextPosition->setEnabled(false);
    qtarch_Label_19->setEnabled(false);
    qtarch_Label_20->setEnabled(false);
  }
}


void CConfigEnscriptDlg::slotLoginClicked() {
  if (loginButton->isChecked()) {
    loginPosition->setEnabled(true);
    qtarch_Label_24->setEnabled(true);
  }
  else {
    loginPosition->setEnabled(false);
    qtarch_Label_24->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotFilenameClicked() {
  if (filenameLine->isChecked()) {
    qtarch_Label_21->setEnabled(true);
    qtarch_Label_25->setEnabled(true);
    filenameSize->setEnabled(true);
    filenamePosition->setEnabled(true);
  }
  else {
    qtarch_Label_21->setEnabled(false);
    qtarch_Label_25->setEnabled(false);
    filenameSize->setEnabled(false);
    filenamePosition->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotHostnameClicked() {
  if (hostnameButton->isChecked()) {
    qtarch_Label_23->setEnabled(true);
    qtarch_Label_22->setEnabled(true);
    hostnamePosition->setEnabled(true);
    hostnameSize->setEnabled(true);
  }
  else {
    qtarch_Label_23->setEnabled(false);
    qtarch_Label_22->setEnabled(false);
    hostnamePosition->setEnabled(false);
    hostnameSize->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotCurrentDateClicked() {
  if (currentDateButton->isChecked()) {
    qtarch_ButtonGroup_88->setEnabled(true);
    qtarch_ButtonGroup_87->setEnabled(true);
    currentDatePosition->setEnabled(true);
    currentDateFormat->setEnabled(true);
  }
  else {
    qtarch_ButtonGroup_88->setEnabled(false);
    qtarch_ButtonGroup_87->setEnabled(false);
    currentDatePosition->setEnabled(false);
    currentDateFormat->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotCurrentTimeClicked() {
  if (currentTimeButton->isChecked()) {
    qtarch_ButtonGroup_71->setEnabled(true);
    qtarch_ButtonGroup_85->setEnabled(true);
    qtarch_ButtonGroup_89->setEnabled(true);
    currentTimeAmpm->setEnabled(true);
    currentTimePosition->setEnabled(true);
    if (currentTimeAmpm->currentItem()==0) {
      currentTimeFormat->setEnabled(false);
    }
    else {
      currentTimeFormat->setEnabled(true);
    }
  }
  else {
    qtarch_ButtonGroup_71->setEnabled(false);
    qtarch_ButtonGroup_85->setEnabled(false);
    qtarch_ButtonGroup_89->setEnabled(false);
    currentTimeAmpm->setEnabled(false);
    currentTimePosition->setEnabled(false);
    currentTimeFormat->setEnabled(false);
  }
}


void CConfigEnscriptDlg::slotModiDateClicked() {
  if (modificationDateButton->isChecked()) {
    qtarch_ButtonGroup_90->setEnabled(true);
    qtarch_ButtonGroup_84->setEnabled(true);
    modificationDatePosition->setEnabled(true);
    modificationDateFormat->setEnabled(true);
  }
  else {
    qtarch_ButtonGroup_90->setEnabled(false);
    qtarch_ButtonGroup_84->setEnabled(false);
    modificationDatePosition->setEnabled(false);
    modificationDateFormat->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotModiTimeClicked() {
  if (modificationTimeButton->isChecked()) {
    qtarch_ButtonGroup_75->setEnabled(true);
    qtarch_ButtonGroup_72->setEnabled(true);
    qtarch_ButtonGroup_86->setEnabled(true);
    modificationTimeAmpm->setEnabled(true);
    modificationTimePosition->setEnabled(true);
    if (modificationTimeAmpm->currentItem()==0) {
      modificationTimeFormat->setEnabled(false);
    }
    else {
      modificationTimeFormat->setEnabled(true);
    }
  }
  else {
    qtarch_ButtonGroup_75->setEnabled(false);
    qtarch_ButtonGroup_72->setEnabled(false);
    qtarch_ButtonGroup_86->setEnabled(false);
    modificationTimeAmpm->setEnabled(false);
    modificationTimePosition->setEnabled(false);
    modificationTimeFormat->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotCutLinesClicked() {
  if (cutLinesButton->isChecked()) {
    qtarch_ButtonGroup_53->setEnabled(false);
    markedWrappedLinesButton->setEnabled(false);
    valueForWrappedLine->setEnabled(false);       
    qtarch_Label_13->setEnabled(false);
  }
  else {
    qtarch_ButtonGroup_53->setEnabled(true);
    markedWrappedLinesButton->setEnabled(true);
    slotWrappedLinesClicked();
  }
}

void CConfigEnscriptDlg::slotWrappedLinesClicked() {
  if (markedWrappedLinesButton->isChecked()) {
    qtarch_Label_13->setEnabled(true);
    valueForWrappedLine->setEnabled(true);
  }
  else {
    qtarch_Label_13->setEnabled(false);
    valueForWrappedLine->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotHighlightBarsClicked() {
  if (highlightBarsButton->isChecked()) {
    qtarch_Label_14->setEnabled(true);
    cycleOfChange->setEnabled(true);
  }
  else {
    qtarch_Label_14->setEnabled(false);
    cycleOfChange->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotUnderlayButtonClicked() {
  if (underlayButton->isChecked()) {
    qtarch_ButtonGroup_138->setEnabled(true);
    qtarch_Label_75->setEnabled(true);
    qtarch_Label_74->setEnabled(true);
    qtarch_Label_73->setEnabled(true);
    qtarch_Label_70->setEnabled(true);
    qtarch_ButtonGroup_131->setEnabled(true);
    qtarch_ButtonGroup_134->setEnabled(true);
    qtarch_ButtonGroup_135->setEnabled(true);
    qtarch_ButtonGroup_136->setEnabled(true);
    qtarch_ButtonGroup_137->setEnabled(true);
    underlayFontButton->setEnabled(true);
    underlaytextLine->setEnabled(true);
    underlayPositionDefaultButton->setEnabled(true);
    underlayStyle->setEnabled(true);
    underlayAngleDefault->setEnabled(true);
    underlayGray->setEnabled(true);
    slotUnderlayPositionDefaultClicked();
    slotUnderlayAngleDefaultClicked();
  }
  else {
    qtarch_ButtonGroup_138->setEnabled(false);
    qtarch_Label_75->setEnabled(false);
    qtarch_Label_74->setEnabled(false);
    qtarch_Label_73->setEnabled(false);
    qtarch_Label_72->setEnabled(false);
    qtarch_Label_71->setEnabled(false);
    qtarch_ButtonGroup_133->setEnabled(false);
    qtarch_Label_70->setEnabled(false);
    qtarch_ButtonGroup_132->setEnabled(false);
    qtarch_ButtonGroup_131->setEnabled(false);
    qtarch_ButtonGroup_134->setEnabled(false);
    qtarch_ButtonGroup_135->setEnabled(false);
    qtarch_ButtonGroup_136->setEnabled(false);
    qtarch_ButtonGroup_137->setEnabled(false);
    underlayFontButton->setEnabled(false);
    underlaytextLine->setEnabled(false);
    underlayPositionDefaultButton->setEnabled(false);
    underlayStyle->setEnabled(false);
    underlayAngleDefault->setEnabled(false);
    underlayGray->setEnabled(false);
    underlayYPosition->setEnabled(false);
    underlayXPosition->setEnabled(false);
    underlayAngle->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotUnderlayPositionDefaultClicked() {
  if (underlayPositionDefaultButton->isChecked()) {
    underlayYPosition->setEnabled(false);
    underlayXPosition->setEnabled(false);
    qtarch_ButtonGroup_133->setEnabled(false);
    qtarch_ButtonGroup_132->setEnabled(false);
    qtarch_Label_72->setEnabled(false);
  }
  else {
    underlayYPosition->setEnabled(true);
    underlayXPosition->setEnabled(true);
    qtarch_ButtonGroup_133->setEnabled(true);
    qtarch_ButtonGroup_132->setEnabled(true);
    qtarch_Label_72->setEnabled(true);
  }
}

void CConfigEnscriptDlg::slotUnderlayAngleDefaultClicked() {
  if (underlayAngleDefault->isChecked()) {
    underlayAngle->setEnabled(false);
    qtarch_Label_71->setEnabled(false);
  }
  else {
    underlayAngle->setEnabled(true);
    qtarch_Label_71->setEnabled(true);
  }
}

void CConfigEnscriptDlg::slotFontUnderlayClicked() {
  QFont font;
  QString string1,string2,string3,string;
  int size;
  //fontDialog->setCaption("UnderlayFontDialog");
  if (KFontDialog::getFont( font,true,this ) )
        {
        string1=font.family();
        string=string1.left(1).upper();
        string1=string1.replace(0,1,string);
        if (font.bold())
                string1 = string1 + "-Bold";
        if (font.italic())
                string3="Italic";
        else
                string3="Roman";
        size=font.pointSize();
        underlayFontButton->setText(string1+ "-" + string3 + string2.setNum(size));
        }
}

void CConfigEnscriptDlg::slotFontBodyClicked() {
  QFont font;
  QString string1,string2,string3,string;
  int size;
//  fontDialog->setCaption("BodyFontDialog");
    if (KFontDialog::getFont( font,true,this ) )
        {
        string1=font.family();
        string=string1.left(1).upper();
        string1=string1.replace(0,1,string);
        if (font.bold())
                string1 = string1 + "-Bold";
        if (font.italic())
                string3="Italic";
        else
                string3="Roman";
        size=font.pointSize();
        fontForBodyButton->setText(string1+ "-" + string3 + string2.setNum(size));
        }
}

void CConfigEnscriptDlg::slotFontHeaderClicked() {
  QFont font;
  QString string1,string2,string3,string;
  int size;
  if (KFontDialog::getFont( font,true,this ) )
        {
        string1=font.family();
        string=string1.left(1).upper();
        string1=string1.replace(0,1,string);
        if (font.bold())
                string1 = string1 + "-Bold";
        if (font.italic())
                string3="Italic";
        else
                string3="Roman";
    size=font.pointSize();
    fontForHeaderButton->setText(string1+ "-" + string3 + string2.setNum(size));
    }
}

void CConfigEnscriptDlg::slotDefaultClicked() {
  headerButton->setChecked(true);
  headertextButton->setChecked(false);
  fancyHeaderButton->setChecked(false);
  headertextLine->clear();
  headertextPosition->setCurrentItem(0);
  loginPosition->setCurrentItem(0);
  filenameSize->setCurrentItem(0);
  filenamePosition->setCurrentItem(0);
  hostnameButton->setChecked(false),
  hostnamePosition->setCurrentItem(0);
  hostnameSize->setCurrentItem(0);
  loginButton->setChecked(false);
  currentTimeButton->setChecked(true);
  currentDateButton->setChecked(true);
  currentTimePosition->setCurrentItem(2);
  currentDatePosition->setCurrentItem(2);
  currentTimeAmpm->setCurrentItem(1);
  currentTimeFormat->setCurrentItem(0);
  currentDateFormat->setCurrentItem(0);
  modificationTimeButton->setChecked(false);
  modificationDateButton->setChecked(false);
  modificationTimePosition->setCurrentItem(0);
  modificationDatePosition->setCurrentItem(0);
  modificationTimeAmpm->setCurrentItem(0);
  modificationTimeFormat->setCurrentItem(0);
  modificationDateFormat->setCurrentItem(0);
  numberingLineButton->setChecked(false);
  bordersButton->setChecked(true);
  numberingPagesButton->setChecked(true);
  numberingPagesList->setCurrentItem(0);
  tocButton->setChecked(false);
  highlightBarsButton->setChecked(false);
  markedWrappedLinesButton->setChecked(true);
  valueForWrappedLine->setCurrentItem(1);
  setTabSize->setValue(8);
  linesPerPage->setValue(60);
  cycleOfChange->setValue(1);
  cutLinesButton->setChecked(false);
  fontForBodyButton->setText("Times-Roman10");
  fontForHeaderButton->setText("Times-Bold-Roman10");
  underlayFontButton->setText("Times-Roman150");
  underlayPositionDefaultButton->setChecked(true);
  underlayAngleDefault->setChecked(true);
  filenameLine->setChecked(true);
  underlayGray->setValue(8);
  underlayButton->setChecked(false);
  underlayXPosition->setValue(0);
  underlayYPosition->setValue(0);
  underlaytextLine->clear();
  underlayAngle->setValue(0);
  underlayStyle->setCurrentItem(0);
  replaceButton->setChecked(false);
  slotLoginClicked();
  alignFileList->setCurrentItem(0);
  slotUnderlayButtonClicked();
  slotHeaderClicked();
  slotCutLinesClicked();
  slotWrappedLinesClicked();
  slotHighlightBarsClicked();
  slotNumberingPagesClicked();
}

QString CConfigEnscriptDlg::createParameters(int i) {
  parameters = "";
  middlestr = "";
  leftstr = "";
  rightstr = "";
  headerstr = "";
  globalpara = "";
  if (headerButton->isChecked()) {
    
    if (headertextButton->isChecked()) {
      if (headertextPosition->currentItem()==0) {
	leftstr.append("\"");
	leftstr.append(headertextLine->text());
	leftstr.append("\"");
      }
      else if (headertextPosition->currentItem()==1) {
	middlestr.append("\"");
	middlestr.append(headertextLine->text());
	middlestr.append("\"");
      }
      else {
	rightstr.append("\"");
	rightstr.append(headertextLine->text());
	rightstr.append("\"");
      }
    }
    if (loginButton->isChecked()) {
      if (loginPosition->currentItem()==0) {
	leftstr.append(" %n");
      }
      else if (loginPosition->currentItem()==1) {
	middlestr.append(" %n");
      }
      else {
	rightstr.append(" %n");
      }
    }
    if (filenameLine->isChecked()) {
      if (filenamePosition->currentItem()==0) {
	if (filenameSize->currentItem()==0) {
	  if (i==0) leftstr.append(" $n");
	  else leftstr.append(" $$n");
	}
	else {
	  if (i==0) leftstr.append(" $N");
	  else leftstr.append(" $$N");
	}
      }
      else if (filenamePosition->currentItem()==1) {
	if (filenameSize->currentItem()==0) {
	  if (i==0) middlestr.append(" $n");
	  else middlestr.append(" $$n");
	}
	else {
	  if (i==0) middlestr.append(" $N");
	  else middlestr.append(" $$N");
	}
      }
      else {
	if (filenameSize->currentItem()==0) {
	  if (i==0) rightstr.append(" $n");
	  else rightstr.append(" $$n");
	}
	else {
	  if (i==0) rightstr.append(" $N");
	  else rightstr.append(" $$N");
	}
      }
    }
    if (hostnameButton->isChecked()) {
      if (hostnamePosition->currentItem()==0) {
	if (hostnameSize->currentItem()==0) {
	  leftstr.append(" %m");
	}
	else {
	  leftstr.append(" %M");
	}
      }
      else if (hostnamePosition->currentItem()==1) {
	if (hostnameSize->currentItem()==0) {
	  middlestr.append(" %m");
	}
	else {
	  middlestr.append(" %M");
	}
      }
      else {
	if (hostnameSize->currentItem()==0) {
	  rightstr.append(" %m");
	}
	else {
	  rightstr.append(" %M");
	}
      }
    }
    
    if (currentTimeButton->isChecked()) {
      if (currentTimeAmpm->currentItem()==1) {
	if (currentTimePosition->currentItem()==0) {
	  if (currentTimeFormat->currentItem()==0) {
	    leftstr.append(" %*");
	  }
	  else {
	    leftstr.append(" %T");
	  }
	}
	else if (currentTimePosition->currentItem()==1) {
	  if (currentTimeFormat->currentItem()==0) {
	    middlestr.append(" %*");
	  }
	  else {
	    middlestr.append(" %T");
	  }
	}
	else {
	  if (currentTimeFormat->currentItem()==0) {
	    rightstr.append(" %*");
	  }
	  else {
	    rightstr.append(" %T");
	  }
	}
      }
      else {
	if (currentTimePosition->currentItem()==0) {
	  leftstr.append(" %t");
	}
	else if (currentTimePosition->currentItem()==1) {
	  middlestr.append(" %t");
	}
	else {
	  if (currentTimeFormat->currentItem()==0) {
	    rightstr.append(" %t");
	  }
	}
      }
    }
    
    if (currentDateButton->isChecked()) {
      if (currentDatePosition->currentItem()==0) {
	if (currentDateFormat->currentItem()==0) {
	  leftstr.append(" %F");
	}
	else if (currentDateFormat->currentItem()==1) {
	  leftstr.append(" %D");
	}
	else if (currentDateFormat->currentItem()==2) {
	  leftstr.append(" %E");
	}
	else {
	  leftstr.append(" %W");
	}
      }
      else if (currentDatePosition->currentItem()==1) {
	if (currentDateFormat->currentItem()==0) {
	  middlestr.append(" %F");
	}
	else if (currentDateFormat->currentItem()==1) {
	  middlestr.append(" %D");
	}
	else if (currentDateFormat->currentItem()==2) {
	  middlestr.append(" %E");
	}
	else {
	  middlestr.append(" %W");
	}
      }
      else if (currentDateFormat->currentItem()==0) {
	rightstr.append(" %F");
      }
      else if (currentDateFormat->currentItem()==1) {
	rightstr.append(" %D");
      }
      else if (currentDateFormat->currentItem()==2) {
	rightstr.append(" %E");
      }
      else {
	rightstr.append(" %W");
      }
    }
    
    if (modificationDateButton->isChecked()) {
      if (modificationDatePosition->currentItem()==0) {
	if (modificationDateFormat->currentItem()==0) {
	  if (i==0) leftstr.append(" $F");
	  else leftstr.append(" $$F");
	}
	else if (modificationDateFormat->currentItem()==1) {
	  if (i==0) leftstr.append(" $D");
	  else leftstr.append(" $$D");
	}
	else if (modificationDateFormat->currentItem()==2) {
	  if (i==0) leftstr.append(" $E");
	  else leftstr.append(" $$E");
	}
	else {
	  if (i==0) leftstr.append(" $W");
	  else leftstr.append(" $$W");
	}
      }
      else if (modificationDatePosition->currentItem()==1) {
	if (modificationDateFormat->currentItem()==0) {
	  if (i==0) middlestr.append(" $F");
	  else middlestr.append(" $$F");
	}
	else if (modificationDateFormat->currentItem()==1) {
	  if (i==0) middlestr.append(" $D");
	  else middlestr.append(" $$D");
	}
	else if (modificationDateFormat->currentItem()==2) {
	  if (i==0) middlestr.append(" $E");
	  else middlestr.append(" $$E");
	}
	else {
	  if (i==0) middlestr.append(" $W");
	  else middlestr.append(" $$W");
	}
      }
      else if (modificationDateFormat->currentItem()==0) {
	if (i==0) rightstr.append(" $F");
	else rightstr.append(" $$F");
      }
      else if (modificationDateFormat->currentItem()==1) {
	if (i==0) rightstr.append(" $D");
	else rightstr.append(" $$D");
      }
      else if (modificationDateFormat->currentItem()==2) {
	if (i==0) rightstr.append(" $E");
	else rightstr.append(" $$E");
      }
      else {
	if (i==0) rightstr.append(" $W");
	else rightstr.append(" $$W");
      }
    }
    
    if (modificationTimeButton->isChecked()) {
      if (modificationTimeAmpm->currentItem()==1) {
	if (modificationTimePosition->currentItem()==0) {
	  if (modificationTimeFormat->currentItem()==0) {
	    if (i==0) leftstr.append(" $*");
	    else leftstr.append(" $$*");
	  }
	  else {
	    if (i==0) leftstr.append(" $T");
	    else leftstr.append(" $$T");
	  }
	}
	else if (modificationTimePosition->currentItem()==1) {
	  if (modificationTimeFormat->currentItem()==0) {
	    if (i==0) middlestr.append(" $*");
	    else middlestr.append(" $$*");
	  }
	  else {
	    if (i==0) middlestr.append(" $T");
	    else middlestr.append(" $$T");
	  }
	}
	else {
	  if (modificationTimeFormat->currentItem()==0) {
	    if (i==0) rightstr.append(" $*");
	    else rightstr.append(" $$*");
	  }
	  else {
	    if (i==0) rightstr.append(" $T");
	    else rightstr.append(" $$T");
	  }
	}
      }
      else {
	if (modificationTimePosition->currentItem()==0) {
	  if (i==0) leftstr.append(" $t");
	  else leftstr.append(" $$t");
	}
	else if (modificationTimePosition->currentItem()==1) {
	  if (i==0) middlestr.append(" $t");
	  else middlestr.append(" $$t");
	  }
	else {
	  if (modificationTimeFormat->currentItem()==0) {
	    if (i==0) rightstr.append(" $t");
	    else rightstr.append(" $$t");
	  }
	}
      }
    }
    if (fancyHeaderButton->isChecked()) {
      globalpara.append(" -G");
    }
    
    if (numberingPagesButton->isChecked()) {
      if (numberingPagesList->currentItem()==0) {
	if (i==0) leftstr.append(" Page $% of $=");
	else leftstr.append(" Page $$% of $$=");
      }
      else if (numberingPagesList->currentItem()==1) {
	if (i==0) middlestr.append(" Page $% of $=");
	else middlestr.append(" Page $$% of $$=");
      }
      else  {
	if (i==0) rightstr.append(" Page $% of $=");
	else rightstr.append(" Page $$% of $$=");
      }
    }
    globalpara.append(" --header='");
    globalpara.append(leftstr);
    globalpara.append("|");
    globalpara.append(middlestr);
    globalpara.append("|");
    globalpara.append(rightstr);
    globalpara.append("'");
  }
  else {
    globalpara.append(" -B");
  }
  
  if (numberingLineButton->isChecked()) {
    globalpara.append(" -C");
  }
  if (bordersButton->isChecked()) {
    globalpara.append(" -j");
  }
  if (alignFileList->currentItem()==0) {
    globalpara.append(" --file-align=1");
  }
  else {
    globalpara.append(" --file-align=2");
  }
  globalpara.append(" -L");
  globalpara.append(linesPerPage->text());
  if (tocButton->isChecked()) {
    globalpara.append(" --toc");
  }
  if (highlightBarsButton->isChecked()) {
    globalpara.append(" -H");
    globalpara.append(cycleOfChange->text());
  }
  if (cutLinesButton->isChecked()) {
    globalpara.append(" -c");
  }
  else {
    if (markedWrappedLinesButton->isChecked()) {
      globalpara.append(" --mark-wrapped-lines=");
      globalpara.append(valueForWrappedLine->text(valueForWrappedLine->currentItem()));
    }
  }
  if (replaceButton->isChecked()) {
    globalpara.append(" --non-printable-format=");
    globalpara.append("space");
  }
  globalpara.append(" -T");
  globalpara.append(setTabSize->text());
  globalpara.append(" --header-font=");
  globalpara.append(fontForHeaderButton->text());
  globalpara.append(" --font=");
  globalpara.append(fontForBodyButton->text());
  if (underlayButton->isChecked()) {
    globalpara.append(" --underlay=\"");
    globalpara.append(underlaytextLine->text());
    globalpara.append("\"");
    globalpara.append(" --ul-font=");
    globalpara.append(underlayFontButton->text());
    globalpara.append(" --ul-gray=");
    float zahl = 0.8;
    zahl = ((QString) underlayGray->text()).toFloat() / 10;
    globalpara.append(parameters.setNum(zahl));
    globalpara.append(" --ul-style=");
    if (!strcmp (i18n (underlayStyle->text(underlayStyle->currentItem())), i18n ( "outline" ))) {
      globalpara.append("outline");
      }
    else {
      globalpara.append("filled");
    }
    if (!underlayPositionDefaultButton->isChecked()) { 
      globalpara.append(" --ul-position=+");
      globalpara.append(underlayXPosition->text());
      globalpara.append("+");
      globalpara.append(underlayYPosition->text());
    }
    if (!underlayAngleDefault->isChecked()) {
      globalpara.append(" --ul-angle=");
      globalpara.append(underlayAngle->text());
    }
  }
  return globalpara;
}

void CConfigEnscriptDlg::slotNumberingPagesClicked() {
  if (numberingPagesButton->isChecked()) {
    numberingPagesList->setEnabled(true);
  }
  else {
    numberingPagesList->setEnabled(false);
  }
}

void CConfigEnscriptDlg::slotCurrentAmpmClicked(int prog) {
  if (prog==0) {
    currentTimeFormat->setEnabled(false);
  }
  else {
    currentTimeFormat->setEnabled(true);
  }
}

void CConfigEnscriptDlg::slotModificationAmpmClicked(int prog) {
  if (prog==0) {
    modificationTimeFormat->setEnabled(false);
  }
  else {
    modificationTimeFormat->setEnabled(true);
  }
}

void CConfigEnscriptDlg::slotPreviewClicked() {
  if (!(lookProgram("gv") || lookProgram("ghostview") || lookProgram("kghostview"))) {
    KMessageBox::error(0,i18n("KDevelop needs \"gv\" or \"ghostview\" or \"kghostview\" to work properly.\n\t\t    Please install one!"),
                        i18n("Program not found!"));
   return;
  }
  QString dir,data1,data2,param,text;
  createParameters(0);

  dir =  locate("appdata", "preview.ps");
  data1 = locate("appdata", "templates/preview1");
  data2 = locate("appdata", "templates/preview2");
  param = (QString) " --output="+ dir;
  process = new KShellProcess();
  *process << "enscript " + globalpara + param + " " + data1 + " " + data2;
  process->start(KProcess::Block,KProcess::AllOutput);
  delete (process);
  process2 = new KShellProcess();
  if (lookProgram("gv")) {
    *process2 << "gv";
    *process2 << dir;
    process2->start(KProcess::NotifyOnExit,KProcess::AllOutput);
    return;
  }
  else if (lookProgram("ghostview")) {
    *process2 << "ghostview";
    *process2 << dir;
    process2->start(KProcess::NotifyOnExit,KProcess::AllOutput);
    return;
  }
  else if (lookProgram("kghostview")) {
    *process2 << "kghostview";
    *process2 << dir;
    process2->start(KProcess::NotifyOnExit,KProcess::AllOutput);
    return;
  }
}

void CConfigEnscriptDlg::slotOkClicked() {
  QString lastSettings;
  lastSettings = createParameters(1);
  settings = KGlobal::config();
  settings->setGroup("Enscript");
  settings->writeEntry("Header",headerButton->isChecked());
  settings->writeEntry("FancyHeader",fancyHeaderButton->isChecked());
  settings->writeEntry("Headertext",headertextButton->isChecked());
  settings->writeEntry("HeadertextLine",headertextLine->text());
  settings->writeEntry("HeadertextPosition",headertextPosition->currentItem());
  settings->writeEntry("Login",loginButton->isChecked());
  settings->writeEntry("LoginPosition",loginPosition->currentItem());
  settings->writeEntry("Filename",filenameLine->isChecked());
  settings->writeEntry("FilenameSize",filenameSize->currentItem());
  settings->writeEntry("FilenamePosition",filenamePosition->currentItem());
  settings->writeEntry("Hostname",hostnameButton->isChecked());
  settings->writeEntry("HostnameSize",hostnameSize->currentItem());
  settings->writeEntry("HostnamePosition",hostnamePosition->currentItem());
  settings->writeEntry("CurrentDate",currentDateButton->isChecked());
  settings->writeEntry("CurrentDateFormat",currentDateFormat->currentItem());
  settings->writeEntry("CurrentDatePosition",currentDatePosition->currentItem());
  settings->writeEntry("CurrentTime",currentTimeButton->isChecked());
  settings->writeEntry("CurrentTimeAmpm",currentTimeAmpm->currentItem());
  settings->writeEntry("CurrentTimeFormat",currentTimeFormat->currentItem());
  settings->writeEntry("CurrentTimePosition",currentTimePosition->currentItem());
  settings->writeEntry("ModificationDate",modificationDateButton->isChecked());
  settings->writeEntry("ModificationDateFormat",modificationDateFormat->currentItem());
  settings->writeEntry("ModificationDatePosition",modificationDatePosition->currentItem());
  settings->writeEntry("ModificationTime",modificationTimeButton->isChecked());
  settings->writeEntry("ModificationTimeAmpm",modificationTimeAmpm->currentItem());
  settings->writeEntry("ModificationTimeFormat",modificationTimeFormat->currentItem());
  settings->writeEntry("ModificationTimePosition",modificationTimePosition->currentItem());
  settings->writeEntry("NumberingLines",numberingLineButton->isChecked());
  settings->writeEntry("Borders",bordersButton->isChecked());
  settings->writeEntry("NumberingPages",numberingPagesButton->isChecked());
  settings->writeEntry("NumberingPagesPosition",numberingPagesList->currentItem());
  settings->writeEntry("AlignFiles",alignFileList->currentItem());
  settings->writeEntry("LinesPerPageLine",linesPerPage->text());
  settings->writeEntry("TabSize",setTabSize->text());
  settings->writeEntry("FontForHeader",fontForHeaderButton->text());
  settings->writeEntry("FontForBody",fontForBodyButton->text());
  settings->writeEntry("CutLines",cutLinesButton->isChecked());
  settings->writeEntry("Replace",replaceButton->isChecked());
  settings->writeEntry("TOC",tocButton->isChecked());
  settings->writeEntry("HighlightBars",highlightBarsButton->isChecked());
  settings->writeEntry("CycleOfChange",cycleOfChange->text());
  settings->writeEntry("WrappedLines",markedWrappedLinesButton->isChecked());
  settings->writeEntry("WrappedLInesValue",valueForWrappedLine->currentItem());
  settings->writeEntry("Underlay",underlayButton->isChecked());
  settings->writeEntry("Underlaytext",underlaytextLine->text());
  settings->writeEntry("UnderlayPositionDefault",underlayPositionDefaultButton->isChecked());
  settings->writeEntry("UnderlayXPosition",underlayXPosition->text());
  settings->writeEntry("UnderlayYPosition",underlayYPosition->text());
  settings->writeEntry("Underlayfont",underlayFontButton->text());
  settings->writeEntry("UnderlayAngleDefault",underlayAngleDefault->isChecked());
  settings->writeEntry("UnderlayAngle",underlayAngle->text());
  settings->writeEntry("UnderlayGray",underlayGray->text());
  settings->writeEntry("UnderlayStyle",underlayStyle->currentItem());
  settings->setGroup("LastSettings");
  settings->writeEntry("EnscriptSettings",lastSettings);
  settings->sync();
  reject();
}

//bool CConfigEnscriptDlg::lookProgram(QString name) {
//  QStringList paths;
//  bool found=false;
//  QString complete_path = getenv("PATH");
//  paths = QStringList::split ( ":", complete_path, FALSE );
//
//  for ( QStringList::Iterator it = paths.begin(); it != paths.end(); ++it )
//  {
//    if (QFile::exists((*it) + "/" + name))
//    {
//      found = true;
//      break;
//    }
//  }
//  return found;
//}

void CConfigEnscriptDlg::loadSettings() {
  settings = KGlobal::config();
  settings->setGroup("Enscript");
  if (!strcmp(settings->readEntry("Header"),"true")) {
    headerButton->setChecked(true);
  }
  else {
   headerButton->setChecked(false);
  }
  if (!strcmp(settings->readEntry("FancyHeader"),"true")) {
 fancyHeaderButton->setChecked(true);
  }
  else {
 fancyHeaderButton->setChecked(false);
  }
  if (!strcmp(settings->readEntry("Headertext"),"true")) {
  headertextButton->setChecked(true);
  }
  else {
  headertextButton->setChecked(false);
  }
  headertextLine->setText(settings->readEntry("HeadertextLine"));
    headertextPosition->setCurrentItem((settings->readEntry("HeadertextPosition")).toInt());
  if (!strcmp(settings->readEntry("Login"),"true")) {
  loginButton->setChecked(true);
  }
  else {
  loginButton->setChecked(false);
  }
  loginPosition->setCurrentItem((settings->readEntry("LoginPosition")).toInt());
  if (!strcmp(settings->readEntry("Filename"),"true")) {
  filenameLine->setChecked(true);
  }
  else {
  filenameLine->setChecked(false);
  }
    filenameSize->setCurrentItem((settings->readEntry("FilenameSize")).toInt());
  filenamePosition->setCurrentItem((settings->readEntry("FilenamePosition")).toInt());
  if (!strcmp(settings->readEntry("Hostname"),"true")) {
  hostnameButton->setChecked(true);
  }
  else {
    hostnameButton->setChecked(false);
  }
  hostnameSize->setCurrentItem((settings->readEntry("HostnameSize")).toInt());
  hostnamePosition->setCurrentItem((settings->readEntry("HostnamePosition")).toInt());
  if (!strcmp(settings->readEntry("CurrentDate"),"true")) {
  currentDateButton->setChecked(true);
  }
  else {
  currentDateButton->setChecked(false);
  }
  currentDateFormat->setCurrentItem((settings->readEntry("CurrentDateFormat")).toInt());
  currentDatePosition->setCurrentItem((settings->readEntry("CurrentDatePosition")).toInt());
  if (!strcmp(settings->readEntry("CurrentTime"),"true")) {
  currentTimeButton->setChecked(true);
  }
  else {
  currentTimeButton->setChecked(false);
  }
  currentTimeAmpm->setCurrentItem((settings->readEntry("CurrentTimeAmpm")).toInt());
  currentTimeFormat->setCurrentItem((settings->readEntry("CurrentTimeFormat")).toInt());
  currentTimePosition->setCurrentItem((settings->readEntry("CurrentTimePosition")).toInt());
  if (!strcmp(settings->readEntry("ModificationDate"),"true")) {
  modificationDateButton->setChecked(true);
  }
  else {
  modificationDateButton->setChecked(false);
  }
  modificationDateFormat->setCurrentItem((settings->readEntry("ModificationDateFormat")).toInt());
  modificationDatePosition->setCurrentItem((settings->readEntry("ModificationDatePosition")).toInt());
  if (!strcmp(settings->readEntry("ModificationTime"),"true")) {
  modificationTimeButton->setChecked(true);
  }
  else {
  modificationTimeButton->setChecked(false);
  }
  modificationTimeAmpm->setCurrentItem((settings->readEntry("ModificationTimeAmpm")).toInt());
  modificationTimeFormat->setCurrentItem((settings->readEntry("ModificationTimeFormat")).toInt());
  modificationTimePosition->setCurrentItem((settings->readEntry("ModificationTimePosition")).toInt());
  if (!strcmp(settings->readEntry("NumberingLines"),"true")) {
  numberingLineButton->setChecked(true);
  }
  else {
  numberingLineButton->setChecked(false);
  }
  if (!strcmp(settings->readEntry("Borders"),"true")) {
  bordersButton->setChecked(true);
  }
  else {
  bordersButton->setChecked(false);
  }
  if (!strcmp(settings->readEntry("NumberingPages"),"true")) {
  numberingPagesButton->setChecked(true);
  }
  else {
  numberingPagesButton->setChecked(false);
  }
  numberingPagesList->setCurrentItem((settings->readEntry("NumberingPagesPosition")).toInt());
  alignFileList->setCurrentItem((settings->readEntry("AlignFiles")).toInt());
  linesPerPage->setValue((settings->readEntry("LinesPerPageLine")).toInt());
  setTabSize->setValue((settings->readEntry("TabSize")).toInt());
  fontForHeaderButton->setText(settings->readEntry("FontForHeader"));
  fontForBodyButton->setText(settings->readEntry("FontForBody"));
  if (!strcmp(settings->readEntry("CutLines"),"true")) {
  cutLinesButton->setChecked(true);
  }
  else {
  cutLinesButton->setChecked(false);
  }
  if (!strcmp(settings->readEntry("Replace"),"true")) {
  replaceButton->setChecked(true);
  }
  else {
  replaceButton->setChecked(false);
  }
  if (!strcmp(settings->readEntry("TOC"),"true")) {
  tocButton->setChecked(true);
  }
  else {
  tocButton->setChecked(false);
  }
  if (!strcmp(settings->readEntry("HighlightBars"),"true")) {
  highlightBarsButton->setChecked(true);
  }
  else {
  highlightBarsButton->setChecked(false);
  }
  cycleOfChange->setValue((settings->readEntry("CycleOfChange")).toInt());
  if (!strcmp(settings->readEntry("WrappedLines"),"true")) {
  markedWrappedLinesButton->setChecked(true);
  }
  else {
  markedWrappedLinesButton->setChecked(false);
  }
  valueForWrappedLine->setCurrentItem((settings->readEntry("WrappedLInesValue")).toInt());
  if (!strcmp(settings->readEntry("Underlay"),"true")) {
  underlayButton->setChecked(true);
  }
  else {
  underlayButton->setChecked(false);
  }
  underlaytextLine->setText(settings->readEntry("Underlaytext"));
  if (!strcmp(settings->readEntry("UnderlayPositionDefault"),"true")) {
  underlayPositionDefaultButton->setChecked(true);
  }
  else {
  underlayPositionDefaultButton->setChecked(false);
  }
  underlayXPosition->setValue((settings->readEntry("UnderlayXPosition")).toInt());
  underlayYPosition->setValue((settings->readEntry("UnderlayYPosition")).toInt());
  underlayFontButton->setText(settings->readEntry("Underlayfont"));
  if (!strcmp(settings->readEntry("UnderlayAngleDefault"),"true")) {
  underlayAngleDefault->setChecked(true);
  }
  else {
  underlayAngleDefault->setChecked(false);
  }
  underlayAngle->setValue((settings->readEntry("UnderlayAngle")).toInt());
  underlayGray->setValue((settings->readEntry("UnderlayGray")).toFloat());
  underlayStyle->setCurrentItem((settings->readEntry("UnderlayStyle")).toInt());
  slotLoginClicked();
  slotUnderlayButtonClicked();
  slotHeaderClicked();
  slotCutLinesClicked();
  slotWrappedLinesClicked();
  slotHighlightBarsClicked();
  slotNumberingPagesClicked();
}


