/***************************************************************************
                          cconfigprintdlg.h  -  description                       
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

#include "cconfigprintdlg.h"
#include <qlabel.h>
#include <qbttngrp.h>

CConfigPrintDlg::CConfigPrintDlg(QWidget* parent,const char* name,int prog) : QTabDialog(parent, name, true){
  init(prog);
  if (prog==1) {
      slotHeader1Clicked();
  }
  else {
      slotHeader2Clicked();
      slotUnderlayButtonClicked();
  }
}

CConfigPrintDlg::~CConfigPrintDlg(){
}

void CConfigPrintDlg::init(int prog) {
  QWidget *tab1 = new QWidget(this,"header");
  initTab1(tab1,prog);
  addTab (tab1,"Header");
  QWidget *tab2 = new QWidget(this,"date & time");
  initTab2(tab2,prog);
  addTab (tab2,"Date & Time");
  QWidget *tab3 = new QWidget(this,"layout");
  initTab3(tab3,prog);
  addTab (tab3,"Layout");
  QWidget *tab4 = new QWidget(this,"underlay");
  initTab4(tab4,prog);
  addTab (tab4,"Underlay");
  if (prog==1) {
    interpretButton->setEnabled(true);
    replaceButton->setEnabled(true);
    printAsISOLatin->setEnabled(true);
    boltFontButton->setEnabled(true);
    a2psFontButton->setEnabled(true);
    qtarch_Label_3->setEnabled(true);
    qtarch_ButtonGroup_51->setEnabled(false);
    qtarch_ButtonGroup_52->setEnabled(false);
    qtarch_Label_14->setEnabled(false);
    qtarch_ButtonGroup_53->setEnabled(false);
    tocButton->setEnabled(false);
    markedWrappedLinesButton->setEnabled(false);
    valueForWrappedLine->setEnabled(false);
    cycleOfChange->setEnabled(false);
    qtarch_Label_13->setEnabled(false);
    highlightBarsButton->setEnabled(false);
    qtarch_Label_4->setEnabled(false);
    qtarch_Label_5->setEnabled(false);
    fontForBodyButton->setEnabled(false);
    fontForHeaderButton->setEnabled(false);
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
    underlayButton->setEnabled(false);
    qtarch_ButtonGroup_62->setEnabled(false);
    headertextPosition->setEnabled(false);
    qtarch_Label_19->setEnabled(false);
    qtarch_Label_24->setEnabled(false);
    loginPosition->setEnabled(false);
    fancyHeaderButton->setEnabled(false);
    qtarch_Label_21->setEnabled(false);
    qtarch_Label_25->setEnabled(false);
    filenameSize->setEnabled(false);
    filenamePosition->setEnabled(false);
    qtarch_ButtonGroup_66->setEnabled(false);
    hostnameButton->setEnabled(false);
    qtarch_Label_23->setEnabled(false);
    qtarch_Label_22->setEnabled(false);
    hostnamePosition->setEnabled(false);
    hostnameSize->setEnabled(false);
    qtarch_ButtonGroup_77->setEnabled(false);
    qtarch_ButtonGroup_87->setEnabled(false);
    qtarch_ButtonGroup_88->setEnabled(false);
    qtarch_ButtonGroup_89->setEnabled(false);
    qtarch_ButtonGroup_90->setEnabled(false);
    qtarch_ButtonGroup_76->setEnabled(false);
    qtarch_ButtonGroup_86->setEnabled(false);
    qtarch_ButtonGroup_85->setEnabled(false);
    qtarch_ButtonGroup_84->setEnabled(false);
    qtarch_ButtonGroup_75->setEnabled(false);
    qtarch_ButtonGroup_72->setEnabled(false);
    qtarch_ButtonGroup_71->setEnabled(false);
    modificationDateButton->setEnabled(false);
    modificationTimeButton->setEnabled(false);
    currentTimePosition->setEnabled(false);
    currentTimeAmpm->setEnabled(false);
    modificationDateFormat->setEnabled(false);
    currentDatePosition->setEnabled(false);
    modificationTimeAmpm->setEnabled(false);
    currentTimeFormat->setEnabled(false);
    modificationTimePosition->setEnabled(false);
    modificationTimeFormat->setEnabled(false);
    modificationDatePosition->setEnabled(false);
    currentDateFormat->setEnabled(false);
  }
  else {
    interpretButton->setEnabled(false);
    replaceButton->setEnabled(false);
    printAsISOLatin->setEnabled(false);
    boltFontButton->setEnabled(false);
    a2psFontButton->setEnabled(false);
    qtarch_Label_3->setEnabled(false);
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
}

void CConfigPrintDlg::initTab1(QWidget*parent, int prog) {
    qtarch_ButtonGroup_59 = new QButtonGroup( parent, "ButtonGroup_59" );
    qtarch_ButtonGroup_59->setGeometry( 20, 40, 560, 380 );
    qtarch_ButtonGroup_59->setMinimumSize( 0, 0 );
    qtarch_ButtonGroup_59->setMaximumSize( 32767, 32767 );
    qtarch_ButtonGroup_59->setFocusPolicy( QWidget::NoFocus );
    qtarch_ButtonGroup_59->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_ButtonGroup_59->setFontPropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_59->setPalettePropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_59->setFrameStyle( 49 );
    qtarch_ButtonGroup_59->setTitle( "Header settings" );
    qtarch_ButtonGroup_59->setAlignment( 1 );
    
    qtarch_ButtonGroup_62 = new QButtonGroup( parent, "ButtonGroup_62" );
    qtarch_ButtonGroup_62->setGeometry( 30, 60, 270, 60 );
    qtarch_ButtonGroup_62->setMinimumSize( 0, 0 );
    qtarch_ButtonGroup_62->setMaximumSize( 32767, 32767 );
    qtarch_ButtonGroup_62->setFocusPolicy( QWidget::NoFocus );
    qtarch_ButtonGroup_62->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_ButtonGroup_62->setFontPropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_62->setPalettePropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_62->setFrameStyle( 49 );
    qtarch_ButtonGroup_62->setTitle( "Fancy header" );
    qtarch_ButtonGroup_62->setAlignment( 1 );
    
    qtarch_ButtonGroup_61 = new QButtonGroup( parent, "ButtonGroup_61" );
    qtarch_ButtonGroup_61->setGeometry( 30, 300, 270, 110 );
    qtarch_ButtonGroup_61->setMinimumSize( 0, 0 );
    qtarch_ButtonGroup_61->setMaximumSize( 32767, 32767 );
    qtarch_ButtonGroup_61->setFocusPolicy( QWidget::NoFocus );
    qtarch_ButtonGroup_61->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_ButtonGroup_61->setFontPropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_61->setPalettePropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_61->setFrameStyle( 49 );
    qtarch_ButtonGroup_61->setTitle( "Login" );
    qtarch_ButtonGroup_61->setAlignment( 1 );
    
    qtarch_ButtonGroup_60 = new QButtonGroup( parent, "ButtonGroup_60" );
    qtarch_ButtonGroup_60->setGeometry( 30, 130, 270, 160 );
    qtarch_ButtonGroup_60->setMinimumSize( 0, 0 );
    qtarch_ButtonGroup_60->setMaximumSize( 32767, 32767 );
    qtarch_ButtonGroup_60->setFocusPolicy( QWidget::NoFocus );
    qtarch_ButtonGroup_60->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_ButtonGroup_60->setFontPropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_60->setPalettePropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_60->setFrameStyle( 49 );
    qtarch_ButtonGroup_60->setTitle( "Headertext" );
    qtarch_ButtonGroup_60->setAlignment( 1 );
    
    headerButton = new QCheckBox( parent, "headerButton" );
    headerButton->setGeometry( 20, 10, 100, 30 );
    headerButton->setMinimumSize( 0, 0 );
    headerButton->setMaximumSize( 32767, 32767 );
    headerButton->setFocusPolicy( QWidget::TabFocus );
    headerButton->setBackgroundMode( QWidget::PaletteBackground );
    headerButton->setFontPropagation( QWidget::NoChildren );
    headerButton->setPalettePropagation( QWidget::NoChildren );
    headerButton->setText( "header" );
    headerButton->setAutoRepeat( FALSE );
    headerButton->setAutoResize( FALSE );
    if (prog==1) {
	connect(headerButton,SIGNAL(clicked()),SLOT(slotHeader1Clicked()));
    }
    else {
	connect(headerButton,SIGNAL(clicked()),SLOT(slotHeader2Clicked()));
    }

    headertextLine = new QLineEdit( parent, "headertextLine" );
    headertextLine->setGeometry( 90, 190, 200, 30 );
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
    
    headertextButton = new QCheckBox( parent, "headertextButton" );
    headertextButton->setGeometry( 40, 150, 250, 30 );
    headertextButton->setMinimumSize( 0, 0 );
    headertextButton->setMaximumSize( 32767, 32767 );
    headertextButton->setFocusPolicy( QWidget::TabFocus );
    headertextButton->setBackgroundMode( QWidget::PaletteBackground );
    headertextButton->setFontPropagation( QWidget::NoChildren );
    headertextButton->setPalettePropagation( QWidget::NoChildren );
    headertextButton->setText( "headertext" );
    headertextButton->setAutoRepeat( FALSE );
    headertextButton->setAutoResize( FALSE );
    if (prog==1) {
	connect(headertextButton,SIGNAL(clicked()),SLOT(slotHeadertext1Clicked()));
    }
    else {
	connect(headertextButton,SIGNAL(clicked()),SLOT(slotHeadertext2Clicked()));
    }
    loginButton = new QCheckBox( parent, "loginButton" );
    loginButton->setGeometry( 40, 320, 250, 30 );
    loginButton->setMinimumSize( 0, 0 );
    loginButton->setMaximumSize( 32767, 32767 );
    loginButton->setFocusPolicy( QWidget::TabFocus );
    loginButton->setBackgroundMode( QWidget::PaletteBackground );
    loginButton->setFontPropagation( QWidget::NoChildren );
    loginButton->setPalettePropagation( QWidget::NoChildren );
    loginButton->setText( "login" );
    loginButton->setAutoRepeat( FALSE );
    loginButton->setAutoResize( FALSE );
    if (prog!=1) {
	connect(loginButton,SIGNAL(clicked()),SLOT(slotLoginClicked()));
    }
    
    headertextPosition = new QComboBox( FALSE, parent, "headertextPosition" );
    headertextPosition->setGeometry( 90, 240, 200, 30 );
    headertextPosition->setMinimumSize( 0, 0 );
    headertextPosition->setMaximumSize( 32767, 32767 );
    headertextPosition->setFocusPolicy( QWidget::StrongFocus );
    headertextPosition->setBackgroundMode( QWidget::PaletteBackground );
    headertextPosition->setFontPropagation( QWidget::NoChildren );
    headertextPosition->setPalettePropagation( QWidget::NoChildren );
    headertextPosition->setSizeLimit( 10 );
    headertextPosition->setAutoResize( FALSE );
    headertextPosition->insertItem( "left" );
    headertextPosition->insertItem( "center" );
    headertextPosition->insertItem( "right" );
    
    qtarch_Label_19 = new QLabel( parent, "Label_19" );
    qtarch_Label_19->setGeometry( 40, 240, 50, 30 );
    qtarch_Label_19->setMinimumSize( 0, 0 );
    qtarch_Label_19->setMaximumSize( 32767, 32767 );
    qtarch_Label_19->setFocusPolicy( QWidget::NoFocus );
    qtarch_Label_19->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_Label_19->setFontPropagation( QWidget::NoChildren );
    qtarch_Label_19->setPalettePropagation( QWidget::NoChildren );
    qtarch_Label_19->setText( "position" );
    qtarch_Label_19->setAlignment( 289 );
    qtarch_Label_19->setMargin( -1 );
    
    qtarch_Label_20 = new QLabel( parent, "Label_20" );
    qtarch_Label_20->setGeometry( 40, 190, 50, 30 );
    qtarch_Label_20->setMinimumSize( 0, 0 );
    qtarch_Label_20->setMaximumSize( 32767, 32767 );
    qtarch_Label_20->setFocusPolicy( QWidget::NoFocus );
    qtarch_Label_20->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_Label_20->setFontPropagation( QWidget::NoChildren );
    qtarch_Label_20->setPalettePropagation( QWidget::NoChildren );
    qtarch_Label_20->setText( "text" );
    qtarch_Label_20->setAlignment( 289 );
    qtarch_Label_20->setMargin( -1 );
    
    qtarch_Label_24 = new QLabel( parent, "Label_24" );
    qtarch_Label_24->setGeometry( 40, 360, 50, 30 );
    qtarch_Label_24->setMinimumSize( 0, 0 );
    qtarch_Label_24->setMaximumSize( 32767, 32767 );
    qtarch_Label_24->setFocusPolicy( QWidget::NoFocus );
    qtarch_Label_24->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_Label_24->setFontPropagation( QWidget::NoChildren );
    qtarch_Label_24->setPalettePropagation( QWidget::NoChildren );
    qtarch_Label_24->setText( "position" );
    qtarch_Label_24->setAlignment( 289 );
    qtarch_Label_24->setMargin( -1 );
    
    loginPosition = new QComboBox( FALSE, parent, "loginPosition" );
    loginPosition->setGeometry( 90, 360, 200, 30 );
    loginPosition->setMinimumSize( 0, 0 );
    loginPosition->setMaximumSize( 32767, 32767 );
    loginPosition->setFocusPolicy( QWidget::StrongFocus );
    loginPosition->setBackgroundMode( QWidget::PaletteBackground );
    loginPosition->setFontPropagation( QWidget::NoChildren );
    loginPosition->setPalettePropagation( QWidget::NoChildren );
    loginPosition->setSizeLimit( 10 );
    loginPosition->setAutoResize( FALSE );
    loginPosition->insertItem( "left" );
    loginPosition->insertItem( "center" );
    loginPosition->insertItem( "right" );
    
    fancyHeaderButton = new QCheckBox( parent, "fancyHeaderButton" );
    fancyHeaderButton->setGeometry( 40, 80, 250, 30 );
    fancyHeaderButton->setMinimumSize( 0, 0 );
    fancyHeaderButton->setMaximumSize( 32767, 32767 );
    fancyHeaderButton->setFocusPolicy( QWidget::TabFocus );
    fancyHeaderButton->setBackgroundMode( QWidget::PaletteBackground );
    fancyHeaderButton->setFontPropagation( QWidget::NoChildren );
    fancyHeaderButton->setPalettePropagation( QWidget::NoChildren );
    fancyHeaderButton->setText( "fancy header" );
    fancyHeaderButton->setAutoRepeat( FALSE );
    fancyHeaderButton->setAutoResize( FALSE );
    
    qtarch_ButtonGroup_65 = new QButtonGroup( parent, "ButtonGroup_65" );
    qtarch_ButtonGroup_65->setGeometry( 310, 60, 260, 170 );
    qtarch_ButtonGroup_65->setMinimumSize( 0, 0 );
    qtarch_ButtonGroup_65->setMaximumSize( 32767, 32767 );
    qtarch_ButtonGroup_65->setFocusPolicy( QWidget::NoFocus );
    qtarch_ButtonGroup_65->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_ButtonGroup_65->setFontPropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_65->setPalettePropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_65->setFrameStyle( 49 );
    qtarch_ButtonGroup_65->setTitle( "Filename" );
    qtarch_ButtonGroup_65->setAlignment( 1 );
    
    filenameLine = new QCheckBox( parent, "filenameLine" );
    filenameLine->setGeometry( 320, 80, 240, 30 );
    filenameLine->setMinimumSize( 0, 0 );
    filenameLine->setMaximumSize( 32767, 32767 );
    filenameLine->setFocusPolicy( QWidget::TabFocus );
    filenameLine->setBackgroundMode( QWidget::PaletteBackground );
    filenameLine->setFontPropagation( QWidget::NoChildren );
    filenameLine->setPalettePropagation( QWidget::NoChildren );
    filenameLine->setText( "filename" );
    filenameLine->setAutoRepeat( FALSE );
    filenameLine->setAutoResize( FALSE );
    if (prog!=1) {
	connect(filenameLine,SIGNAL(clicked()),SLOT(slotFilenameClicked()));
    }
    
    qtarch_Label_21 = new QLabel( parent, "Label_21" );
    qtarch_Label_21->setGeometry( 320, 130, 100, 30 );
    qtarch_Label_21->setMinimumSize( 0, 0 );
    qtarch_Label_21->setMaximumSize( 32767, 32767 );
    qtarch_Label_21->setFocusPolicy( QWidget::NoFocus );
    qtarch_Label_21->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_Label_21->setFontPropagation( QWidget::NoChildren );
    qtarch_Label_21->setPalettePropagation( QWidget::NoChildren );
    qtarch_Label_21->setText( "size of filename" );
    qtarch_Label_21->setAlignment( 289 );
    qtarch_Label_21->setMargin( -1 );
    
    qtarch_Label_25 = new QLabel( parent, "Label_25" );
    qtarch_Label_25->setGeometry( 320, 180, 100, 30 );
    qtarch_Label_25->setMinimumSize( 0, 0 );
    qtarch_Label_25->setMaximumSize( 32767, 32767 );
    qtarch_Label_25->setFocusPolicy( QWidget::NoFocus );
    qtarch_Label_25->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_Label_25->setFontPropagation( QWidget::NoChildren );
    qtarch_Label_25->setPalettePropagation( QWidget::NoChildren );
    qtarch_Label_25->setText( "position" );
    qtarch_Label_25->setAlignment( 289 );
    qtarch_Label_25->setMargin( -1 );
    
    filenameSize = new QComboBox( FALSE, parent, "filenameSize" );
    filenameSize->setGeometry( 430, 130, 130, 30 );
    filenameSize->setMinimumSize( 0, 0 );
    filenameSize->setMaximumSize( 32767, 32767 );
    filenameSize->setFocusPolicy( QWidget::StrongFocus );
    filenameSize->setBackgroundMode( QWidget::PaletteBackground );
    filenameSize->setFontPropagation( QWidget::NoChildren );
    filenameSize->setPalettePropagation( QWidget::NoChildren );
    filenameSize->setSizeLimit( 10 );
    filenameSize->setAutoResize( FALSE );
    filenameSize->insertItem( "short" );
    filenameSize->insertItem( "full" );

    filenamePosition = new QComboBox( FALSE, parent, "filenamePosition" );
    filenamePosition->setGeometry( 430, 180, 130, 30 );
    filenamePosition->setMinimumSize( 0, 0 );
    filenamePosition->setMaximumSize( 32767, 32767 );
    filenamePosition->setFocusPolicy( QWidget::StrongFocus );
    filenamePosition->setBackgroundMode( QWidget::PaletteBackground );
    filenamePosition->setFontPropagation( QWidget::NoChildren );
    filenamePosition->setPalettePropagation( QWidget::NoChildren );
    filenamePosition->setSizeLimit( 10 );
    filenamePosition->setAutoResize( FALSE );
    filenamePosition->insertItem( "left" );
    filenamePosition->insertItem( "center" );
    filenamePosition->insertItem( "right" );
    
    qtarch_ButtonGroup_66 = new QButtonGroup( parent, "ButtonGroup_66" );
    qtarch_ButtonGroup_66->setGeometry( 310, 240, 260, 170 );
    qtarch_ButtonGroup_66->setMinimumSize( 0, 0 );
    qtarch_ButtonGroup_66->setMaximumSize( 32767, 32767 );
    qtarch_ButtonGroup_66->setFocusPolicy( QWidget::NoFocus );
    qtarch_ButtonGroup_66->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_ButtonGroup_66->setFontPropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_66->setPalettePropagation( QWidget::NoChildren );
    qtarch_ButtonGroup_66->setFrameStyle( 49 );
    qtarch_ButtonGroup_66->setTitle( "Hostname" );
    qtarch_ButtonGroup_66->setAlignment( 1 );
    
    hostnameButton = new QCheckBox( parent, "hostnameButton" );
    hostnameButton->setGeometry( 320, 260, 240, 30 );
    hostnameButton->setMinimumSize( 0, 0 );
    hostnameButton->setMaximumSize( 32767, 32767 );
    hostnameButton->setFocusPolicy( QWidget::TabFocus );
    hostnameButton->setBackgroundMode( QWidget::PaletteBackground );
    hostnameButton->setFontPropagation( QWidget::NoChildren );
    hostnameButton->setPalettePropagation( QWidget::NoChildren );
    hostnameButton->setText( "hostname" );
    hostnameButton->setAutoRepeat( FALSE );
    hostnameButton->setAutoResize( FALSE );
    if (prog!=1) {
	connect(hostnameButton,SIGNAL(clicked()),SLOT(slotHostnameClicked()));
    }
    
    qtarch_Label_23 = new QLabel( parent, "Label_23" );
    qtarch_Label_23->setGeometry( 320, 310, 100, 30 );
    qtarch_Label_23->setMinimumSize( 0, 0 );
    qtarch_Label_23->setMaximumSize( 32767, 32767 );
    qtarch_Label_23->setFocusPolicy( QWidget::NoFocus );
    qtarch_Label_23->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_Label_23->setFontPropagation( QWidget::NoChildren );
    qtarch_Label_23->setPalettePropagation( QWidget::NoChildren );
    qtarch_Label_23->setText( "size of hostname " );
    qtarch_Label_23->setAlignment( 289 );
    qtarch_Label_23->setMargin( -1 );
    
    qtarch_Label_22 = new QLabel( parent, "Label_22" );
    qtarch_Label_22->setGeometry( 320, 360, 100, 30 );
    qtarch_Label_22->setMinimumSize( 0, 0 );
    qtarch_Label_22->setMaximumSize( 32767, 32767 );
    qtarch_Label_22->setFocusPolicy( QWidget::NoFocus );
    qtarch_Label_22->setBackgroundMode( QWidget::PaletteBackground );
    qtarch_Label_22->setFontPropagation( QWidget::NoChildren );
    qtarch_Label_22->setPalettePropagation( QWidget::NoChildren );
    qtarch_Label_22->setText( "position" );
    qtarch_Label_22->setAlignment( 289 );
    qtarch_Label_22->setMargin( -1 );
    
    hostnamePosition = new QComboBox( FALSE, parent, "hostnamePosition" );
    hostnamePosition->setGeometry( 430, 360, 130, 30 );
    hostnamePosition->setMinimumSize( 0, 0 );
    hostnamePosition->setMaximumSize( 32767, 32767 );
    hostnamePosition->setFocusPolicy( QWidget::StrongFocus );
    hostnamePosition->setBackgroundMode( QWidget::PaletteBackground );
    hostnamePosition->setFontPropagation( QWidget::NoChildren );
    hostnamePosition->setPalettePropagation( QWidget::NoChildren );
    hostnamePosition->setSizeLimit( 10 );
    hostnamePosition->setAutoResize( FALSE );
    hostnamePosition->insertItem( "left" );
    hostnamePosition->insertItem( "center" );
    hostnamePosition->insertItem( "right" );
    
    hostnameSize = new QComboBox( FALSE, parent, "hostnameSize" );
    hostnameSize->setGeometry( 430, 310, 130, 30 );
    hostnameSize->setMinimumSize( 0, 0 );
    hostnameSize->setMaximumSize( 32767, 32767 );
    hostnameSize->setFocusPolicy( QWidget::StrongFocus );
    hostnameSize->setBackgroundMode( QWidget::PaletteBackground );
    hostnameSize->setFontPropagation( QWidget::NoChildren );
    hostnameSize->setPalettePropagation( QWidget::NoChildren );
    hostnameSize->setSizeLimit( 10 );
    hostnameSize->setAutoResize( FALSE );
    hostnameSize->insertItem( "short" );
    hostnameSize->insertItem( "full" );
    
    qtarch_ButtonGroup_59->insert( headertextButton );
    qtarch_ButtonGroup_59->insert( loginButton );
    qtarch_ButtonGroup_59->insert( fancyHeaderButton );
    qtarch_ButtonGroup_59->insert( filenameLine );
    qtarch_ButtonGroup_59->insert( hostnameButton );
    qtarch_ButtonGroup_62->insert( fancyHeaderButton );
    qtarch_ButtonGroup_61->insert( loginButton );
    qtarch_ButtonGroup_60->insert( headertextButton );
    qtarch_ButtonGroup_65->insert( filenameLine );
    qtarch_ButtonGroup_66->insert( hostnameButton );
}

void CConfigPrintDlg::initTab2(QWidget*parent, int prog) {
	qtarch_ButtonGroup_78 = new QButtonGroup( parent, "ButtonGroup_78" );
	qtarch_ButtonGroup_78->setGeometry( 20, 217, 560, 100 );
	qtarch_ButtonGroup_78->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_78->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_78->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_78->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_78->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_78->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_78->setFrameStyle( 49 );
	qtarch_ButtonGroup_78->setTitle( "Current time" );
	qtarch_ButtonGroup_78->setAlignment( 1 );

	qtarch_ButtonGroup_77 = new QButtonGroup( parent, "ButtonGroup_77" );
	qtarch_ButtonGroup_77->setGeometry( 20, 320, 560, 100 );
	qtarch_ButtonGroup_77->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_77->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_77->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_77->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_77->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_77->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_77->setFrameStyle( 49 );
	qtarch_ButtonGroup_77->setTitle( "Modification time" );
	qtarch_ButtonGroup_77->setAlignment( 1 );

	qtarch_ButtonGroup_87 = new QButtonGroup( parent, "ButtonGroup_87" );
	qtarch_ButtonGroup_87->setGeometry( 420, 350, 150, 60 );
	qtarch_ButtonGroup_87->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_87->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_87->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_87->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_87->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_87->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_87->setFrameStyle( 49 );
	qtarch_ButtonGroup_87->setTitle( "Format" );
	qtarch_ButtonGroup_87->setAlignment( 1 );

	qtarch_ButtonGroup_88 = new QButtonGroup( parent, "ButtonGroup_88" );
	qtarch_ButtonGroup_88->setGeometry( 260, 350, 150, 60 );
	qtarch_ButtonGroup_88->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_88->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_88->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_88->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_88->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_88->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_88->setFrameStyle( 49 );
	qtarch_ButtonGroup_88->setTitle( "Position" );
	qtarch_ButtonGroup_88->setAlignment( 1 );

	qtarch_ButtonGroup_89 = new QButtonGroup( parent, "ButtonGroup_89" );
	qtarch_ButtonGroup_89->setGeometry( 420, 250, 150, 60 );
	qtarch_ButtonGroup_89->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_89->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_89->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_89->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_89->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_89->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_89->setFrameStyle( 49 );
	qtarch_ButtonGroup_89->setTitle( "Format" );
	qtarch_ButtonGroup_89->setAlignment( 1 );

	qtarch_ButtonGroup_90 = new QButtonGroup( parent, "ButtonGroup_90" );
	qtarch_ButtonGroup_90->setGeometry( 260, 250, 150, 60 );
	qtarch_ButtonGroup_90->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_90->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_90->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_90->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_90->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_90->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_90->setFrameStyle( 49 );
	qtarch_ButtonGroup_90->setTitle( "Position" );
	qtarch_ButtonGroup_90->setAlignment( 1 );

	qtarch_ButtonGroup_76 = new QButtonGroup( parent, "ButtonGroup_76" );
	qtarch_ButtonGroup_76->setGeometry( 21, 114, 560, 100 );
	qtarch_ButtonGroup_76->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_76->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_76->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_76->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_76->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_76->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_76->setFrameStyle( 49 );
	qtarch_ButtonGroup_76->setTitle( "Modification date" );
	qtarch_ButtonGroup_76->setAlignment( 1 );

	qtarch_ButtonGroup_86 = new QButtonGroup( parent, "ButtonGroup_86" );
	qtarch_ButtonGroup_86->setGeometry( 420, 140, 150, 60 );
	qtarch_ButtonGroup_86->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_86->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_86->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_86->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_86->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_86->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_86->setFrameStyle( 49 );
	qtarch_ButtonGroup_86->setTitle( "Format" );
	qtarch_ButtonGroup_86->setAlignment( 1 );

	qtarch_ButtonGroup_85 = new QButtonGroup( parent, "ButtonGroup_85" );
	qtarch_ButtonGroup_85->setGeometry( 260, 140, 150, 60 );
	qtarch_ButtonGroup_85->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_85->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_85->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_85->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_85->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_85->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_85->setFrameStyle( 49 );
	qtarch_ButtonGroup_85->setTitle( "Position" );
	qtarch_ButtonGroup_85->setAlignment( 1 );

	qtarch_ButtonGroup_74 = new QButtonGroup( parent, "ButtonGroup_74" );
	qtarch_ButtonGroup_74->setGeometry( 20, 10, 560, 100 );
	qtarch_ButtonGroup_74->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_74->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_74->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_74->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_74->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_74->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_74->setFrameStyle( 49 );
	qtarch_ButtonGroup_74->setTitle( "Current date" );
	qtarch_ButtonGroup_74->setAlignment( 1 );

	qtarch_ButtonGroup_84 = new QButtonGroup( parent, "ButtonGroup_84" );
	qtarch_ButtonGroup_84->setGeometry( 420, 40, 150, 60 );
	qtarch_ButtonGroup_84->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_84->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_84->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_84->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_84->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_84->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_84->setFrameStyle( 49 );
	qtarch_ButtonGroup_84->setTitle( "Format" );
	qtarch_ButtonGroup_84->setAlignment( 1 );

	qtarch_ButtonGroup_75 = new QButtonGroup( parent, "ButtonGroup_75" );
	qtarch_ButtonGroup_75->setGeometry( 260, 40, 150, 60 );
	qtarch_ButtonGroup_75->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_75->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_75->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_75->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_75->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_75->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_75->setFrameStyle( 49 );
	qtarch_ButtonGroup_75->setTitle( "Position" );
	qtarch_ButtonGroup_75->setAlignment( 1 );

	qtarch_ButtonGroup_72 = new QButtonGroup( parent, "ButtonGroup_72" );
	qtarch_ButtonGroup_72->setGeometry( 160, 350, 90, 60 );
	qtarch_ButtonGroup_72->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_72->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_72->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_72->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_72->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_72->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_72->setFrameStyle( 49 );
	qtarch_ButtonGroup_72->setTitle( "AM/PM" );
	qtarch_ButtonGroup_72->setAlignment( 1 );

	qtarch_ButtonGroup_71 = new QButtonGroup( parent, "ButtonGroup_71" );
	qtarch_ButtonGroup_71->setGeometry( 160, 250, 90, 60 );
	qtarch_ButtonGroup_71->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_71->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_71->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_71->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_71->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_71->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_71->setFrameStyle( 49 );
	qtarch_ButtonGroup_71->setTitle( "AM/PM" );
	qtarch_ButtonGroup_71->setAlignment( 1 );

	currentDateButton = new QCheckBox( parent, "currentDateButton" );
	currentDateButton->setGeometry( 30, 30, 200, 30 );
	currentDateButton->setMinimumSize( 0, 0 );
	currentDateButton->setMaximumSize( 32767, 32767 );
	currentDateButton->setFocusPolicy( QWidget::TabFocus );
	currentDateButton->setBackgroundMode( QWidget::PaletteBackground );
	currentDateButton->setFontPropagation( QWidget::NoChildren );
	currentDateButton->setPalettePropagation( QWidget::NoChildren );
	currentDateButton->setText( "current date" );
	currentDateButton->setAutoRepeat( FALSE );
	currentDateButton->setAutoResize( FALSE );
	if (prog!=1) {
	    connect(currentDateButton,SIGNAL(clicked()),SLOT(slotCurrentDate2Clicked()));
	}
	else {
	    connect(currentDateButton,SIGNAL(clicked()),SLOT(slotCurrentDate1Clicked()));
	}
	
	modificationDateButton = new QCheckBox( parent, "modificationDateButton" );
	modificationDateButton->setGeometry( 31, 134, 200, 30 );
	modificationDateButton->setMinimumSize( 0, 0 );
	modificationDateButton->setMaximumSize( 32767, 32767 );
	modificationDateButton->setFocusPolicy( QWidget::TabFocus );
	modificationDateButton->setBackgroundMode( QWidget::PaletteBackground );
	modificationDateButton->setFontPropagation( QWidget::NoChildren );
	modificationDateButton->setPalettePropagation( QWidget::NoChildren );
	modificationDateButton->setText( "modification date" );
	modificationDateButton->setAutoRepeat( FALSE );
	modificationDateButton->setAutoResize( FALSE );
	if (prog!=1) {
	    connect(modificationDateButton,SIGNAL(clicked()),SLOT(slotModiDateClicked()));
	}

	currentTimeButton = new QCheckBox( parent, "currentTimeButton" );
	currentTimeButton->setGeometry( 30, 237, 110, 30 );
	currentTimeButton->setMinimumSize( 0, 0 );
	currentTimeButton->setMaximumSize( 32767, 32767 );
	currentTimeButton->setFocusPolicy( QWidget::TabFocus );
	currentTimeButton->setBackgroundMode( QWidget::PaletteBackground );
	currentTimeButton->setFontPropagation( QWidget::NoChildren );
	currentTimeButton->setPalettePropagation( QWidget::NoChildren );
	currentTimeButton->setText( "current time" );
	currentTimeButton->setAutoRepeat( FALSE );
	currentTimeButton->setAutoResize( FALSE );
	if (prog!=1) {
	    connect(currentTimeButton,SIGNAL(clicked()),SLOT(slotCurrentTime2Clicked()));
	}
	else {
	    connect(currentTimeButton,SIGNAL(clicked()),SLOT(slotCurrentTime1Clicked()));
	}

	modificationTimeButton = new QCheckBox( parent, "modificationTimeButton" );
	modificationTimeButton->setGeometry( 30, 340, 110, 30 );
	modificationTimeButton->setMinimumSize( 0, 0 );
	modificationTimeButton->setMaximumSize( 32767, 32767 );
	modificationTimeButton->setFocusPolicy( QWidget::TabFocus );
	modificationTimeButton->setBackgroundMode( QWidget::PaletteBackground );
	modificationTimeButton->setFontPropagation( QWidget::NoChildren );
	modificationTimeButton->setPalettePropagation( QWidget::NoChildren );
	modificationTimeButton->setText( "modification time" );
	modificationTimeButton->setAutoRepeat( FALSE );
	modificationTimeButton->setAutoResize( FALSE );
	if (prog!=1) {
	    connect(modificationTimeButton,SIGNAL(clicked()),SLOT(slotModiTimeClicked()));
	}

	currentTimePosition = new QComboBox( FALSE, parent, "currentTimePosition" );
	currentTimePosition->setGeometry( 270, 270, 130, 30 );
	currentTimePosition->setMinimumSize( 0, 0 );
	currentTimePosition->setMaximumSize( 32767, 32767 );
	currentTimePosition->setFocusPolicy( QWidget::StrongFocus );
	currentTimePosition->setBackgroundMode( QWidget::PaletteBackground );
	currentTimePosition->setFontPropagation( QWidget::NoChildren );
	currentTimePosition->setPalettePropagation( QWidget::NoChildren );
	currentTimePosition->setSizeLimit( 10 );
	currentTimePosition->setAutoResize( FALSE );
	currentTimePosition->insertItem( "left" );
	currentTimePosition->insertItem( "center" );
	currentTimePosition->insertItem( "right" );

	currentTimeAmpm = new QComboBox( FALSE, parent, "currentTimeAmpm" );
	currentTimeAmpm->setGeometry( 170, 270, 70, 30 );
	currentTimeAmpm->setMinimumSize( 0, 0 );
	currentTimeAmpm->setMaximumSize( 32767, 32767 );
	currentTimeAmpm->setFocusPolicy( QWidget::StrongFocus );
	currentTimeAmpm->setBackgroundMode( QWidget::PaletteBackground );
	currentTimeAmpm->setFontPropagation( QWidget::NoChildren );
	currentTimeAmpm->setPalettePropagation( QWidget::NoChildren );
	currentTimeAmpm->setSizeLimit( 10 );
	currentTimeAmpm->setAutoResize( FALSE );
	currentTimeAmpm->insertItem( "am/pm" );
	currentTimeAmpm->insertItem( "24 h" );

	modificationDateFormat = new QComboBox( FALSE, parent, "modificationDateFormat" );
	modificationDateFormat->setGeometry( 430, 160, 130, 30 );
	modificationDateFormat->setMinimumSize( 0, 0 );
	modificationDateFormat->setMaximumSize( 32767, 32767 );
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

	currentDatePosition = new QComboBox( FALSE, parent, "currentDatePosition" );
	currentDatePosition->setGeometry( 270, 60, 130, 30 );
	currentDatePosition->setMinimumSize( 0, 0 );
	currentDatePosition->setMaximumSize( 32767, 32767 );
	currentDatePosition->setFocusPolicy( QWidget::StrongFocus );
	currentDatePosition->setBackgroundMode( QWidget::PaletteBackground );
	currentDatePosition->setFontPropagation( QWidget::NoChildren );
	currentDatePosition->setPalettePropagation( QWidget::NoChildren );
	currentDatePosition->setSizeLimit( 10 );
	currentDatePosition->setAutoResize( FALSE );
	currentDatePosition->insertItem( "left" );
	currentDatePosition->insertItem( "center" );
	currentDatePosition->insertItem( "right" );

	modificationTimeAmpm = new QComboBox( FALSE, parent, "modificationTimeAmpm" );
	modificationTimeAmpm->setGeometry( 170, 370, 70, 30 );
	modificationTimeAmpm->setMinimumSize( 0, 0 );
	modificationTimeAmpm->setMaximumSize( 32767, 32767 );
	modificationTimeAmpm->setFocusPolicy( QWidget::StrongFocus );
	modificationTimeAmpm->setBackgroundMode( QWidget::PaletteBackground );
	modificationTimeAmpm->setFontPropagation( QWidget::NoChildren );
	modificationTimeAmpm->setPalettePropagation( QWidget::NoChildren );
	modificationTimeAmpm->setSizeLimit( 10 );
	modificationTimeAmpm->setAutoResize( FALSE );
	modificationTimeAmpm->insertItem( "am/pm" );
	modificationTimeAmpm->insertItem( "24 h" );

	currentTimeFormat = new QComboBox( FALSE, parent, "currentTimeFormat" );
	currentTimeFormat->setGeometry( 430, 270, 130, 30 );
	currentTimeFormat->setMinimumSize( 0, 0 );
	currentTimeFormat->setMaximumSize( 32767, 32767 );
	currentTimeFormat->setFocusPolicy( QWidget::StrongFocus );
	currentTimeFormat->setBackgroundMode( QWidget::PaletteBackground );
	currentTimeFormat->setFontPropagation( QWidget::NoChildren );
	currentTimeFormat->setPalettePropagation( QWidget::NoChildren );
	currentTimeFormat->setSizeLimit( 10 );
	currentTimeFormat->setAutoResize( FALSE );
	currentTimeFormat->insertItem( "hh:mm:ss" );
	currentTimeFormat->insertItem( "hh:mm" );

	modificationTimePosition = new QComboBox( FALSE, parent, "modificationTimePosition" );
	modificationTimePosition->setGeometry( 270, 370, 130, 30 );
	modificationTimePosition->setMinimumSize( 0, 0 );
	modificationTimePosition->setMaximumSize( 32767, 32767 );
	modificationTimePosition->setFocusPolicy( QWidget::StrongFocus );
	modificationTimePosition->setBackgroundMode( QWidget::PaletteBackground );
	modificationTimePosition->setFontPropagation( QWidget::NoChildren );
	modificationTimePosition->setPalettePropagation( QWidget::NoChildren );
	modificationTimePosition->setSizeLimit( 10 );
	modificationTimePosition->setAutoResize( FALSE );
	modificationTimePosition->insertItem( "left" );
	modificationTimePosition->insertItem( "center" );
	modificationTimePosition->insertItem( "right" );

	modificationTimeFormat = new QComboBox( FALSE, parent, "modificationTimeFormat" );
	modificationTimeFormat->setGeometry( 430, 370, 130, 30 );
	modificationTimeFormat->setMinimumSize( 0, 0 );
	modificationTimeFormat->setMaximumSize( 32767, 32767 );
	modificationTimeFormat->setFocusPolicy( QWidget::StrongFocus );
	modificationTimeFormat->setBackgroundMode( QWidget::PaletteBackground );
	modificationTimeFormat->setFontPropagation( QWidget::NoChildren );
	modificationTimeFormat->setPalettePropagation( QWidget::NoChildren );
	modificationTimeFormat->setSizeLimit( 10 );
	modificationTimeFormat->setAutoResize( FALSE );
	modificationTimeFormat->insertItem( "hh:mm:ss" );
	modificationTimeFormat->insertItem( "hh:mm" );

	modificationDatePosition = new QComboBox( FALSE, parent, "modificationDatePosition" );
	modificationDatePosition->setGeometry( 270, 160, 130, 30 );
	modificationDatePosition->setMinimumSize( 0, 0 );
	modificationDatePosition->setMaximumSize( 32767, 32767 );
	modificationDatePosition->setFocusPolicy( QWidget::StrongFocus );
	modificationDatePosition->setBackgroundMode( QWidget::PaletteBackground );
	modificationDatePosition->setFontPropagation( QWidget::NoChildren );
	modificationDatePosition->setPalettePropagation( QWidget::NoChildren );
	modificationDatePosition->setSizeLimit( 10 );
	modificationDatePosition->setAutoResize( FALSE );
	modificationDatePosition->insertItem( "left" );
	modificationDatePosition->insertItem( "center" );
	modificationDatePosition->insertItem( "right" );

	currentDateFormat = new QComboBox( FALSE, parent, "currentDateFormat" );
	currentDateFormat->setGeometry( 430, 60, 130, 30 );
	currentDateFormat->setMinimumSize( 0, 0 );
	currentDateFormat->setMaximumSize( 32767, 32767 );
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

	qtarch_ButtonGroup_78->insert( currentTimeButton );
	qtarch_ButtonGroup_77->insert( modificationTimeButton );
	qtarch_ButtonGroup_76->insert( modificationDateButton );
	qtarch_ButtonGroup_74->insert( currentDateButton );

}

void CConfigPrintDlg::initTab3(QWidget*parent, int prog) {
	QButtonGroup* qtarch_ButtonGroup_20;
	qtarch_ButtonGroup_20 = new QButtonGroup( parent, "ButtonGroup_20" );
	qtarch_ButtonGroup_20->setGeometry( 300, 230, 280, 190 );
	qtarch_ButtonGroup_20->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_20->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_20->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_20->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_20->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_20->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_20->setFrameStyle( 49 );
	qtarch_ButtonGroup_20->setTitle( "Font and TAB" );
	qtarch_ButtonGroup_20->setAlignment( 1 );

	QButtonGroup* qtarch_ButtonGroup_21;
	qtarch_ButtonGroup_21 = new QButtonGroup( parent, "ButtonGroup_21" );
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

	qtarch_ButtonGroup_51 = new QButtonGroup( parent, "ButtonGroup_51" );
	qtarch_ButtonGroup_51->setGeometry( 20, 180, 270, 240 );
	qtarch_ButtonGroup_51->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_51->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_51->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_51->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_51->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_51->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_51->setFrameStyle( 49 );
	qtarch_ButtonGroup_51->setTitle( "Other options" );
	qtarch_ButtonGroup_51->setAlignment( 1 );

	qtarch_ButtonGroup_52 = new QButtonGroup( parent, "ButtonGroup_52" );
	qtarch_ButtonGroup_52->setGeometry( 30, 235, 250, 85 );
	qtarch_ButtonGroup_52->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_52->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_52->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_52->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_52->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_52->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_52->setFrameStyle( 49 );
	qtarch_ButtonGroup_52->setTitle( "Highlight bars" );
	qtarch_ButtonGroup_52->setAlignment( 1 );

	qtarch_Label_14 = new QLabel( parent, "Label_14" );
	qtarch_Label_14->setGeometry( 40, 280, 100, 30 );
	qtarch_Label_14->setMinimumSize( 0, 0 );
	qtarch_Label_14->setMaximumSize( 32767, 32767 );
	qtarch_Label_14->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_14->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_14->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_14->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_14->setText( "cycle of change" );
	qtarch_Label_14->setAlignment( 289 );
	qtarch_Label_14->setMargin( -1 );

	qtarch_ButtonGroup_53 = new QButtonGroup( parent, "ButtonGroup_53" );
	qtarch_ButtonGroup_53->setGeometry( 30, 325, 250, 85 );
	qtarch_ButtonGroup_53->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_53->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_53->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_53->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_53->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_53->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_53->setFrameStyle( 49 );
	qtarch_ButtonGroup_53->setTitle( "Wrapped line" );
	qtarch_ButtonGroup_53->setAlignment( 1 );

	QButtonGroup* qtarch_ButtonGroup_33;
	qtarch_ButtonGroup_33 = new QButtonGroup( parent, "ButtonGroup_33" );
	qtarch_ButtonGroup_33->setGeometry( 20, 10, 270, 170 );
	qtarch_ButtonGroup_33->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_33->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_33->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_33->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_33->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_33->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_33->setFrameStyle( 49 );
	qtarch_ButtonGroup_33->setTitle( "Numbering & border" );
	qtarch_ButtonGroup_33->setAlignment( 1 );

	bordersButton = new QCheckBox( parent, "bordersButton" );
	bordersButton->setGeometry( 160, 30, 100, 30 );
	bordersButton->setMinimumSize( 0, 0 );
	bordersButton->setMaximumSize( 32767, 32767 );
	bordersButton->setFocusPolicy( QWidget::TabFocus );
	bordersButton->setBackgroundMode( QWidget::PaletteBackground );
	bordersButton->setFontPropagation( QWidget::NoChildren );
	bordersButton->setPalettePropagation( QWidget::NoChildren );
	bordersButton->setText( "borders" );
	bordersButton->setAutoRepeat( FALSE );
	bordersButton->setAutoResize( FALSE );

	tocButton = new QCheckBox( parent, "tocButton" );
	tocButton->setGeometry( 40, 200, 120, 30 );
	tocButton->setMinimumSize( 0, 0 );
	tocButton->setMaximumSize( 32767, 32767 );
	tocButton->setFocusPolicy( QWidget::TabFocus );
	tocButton->setBackgroundMode( QWidget::PaletteBackground );
	tocButton->setFontPropagation( QWidget::NoChildren );
	tocButton->setPalettePropagation( QWidget::NoChildren );
	tocButton->setText( "table of contents" );
	tocButton->setAutoRepeat( FALSE );
	tocButton->setAutoResize( FALSE );

	markedWrappedLinesButton = new QCheckBox( parent, "markedWrappedLinesButton" );
	markedWrappedLinesButton->setGeometry( 40, 340, 150, 30 );
	markedWrappedLinesButton->setMinimumSize( 0, 0 );
	markedWrappedLinesButton->setMaximumSize( 32767, 32767 );
	markedWrappedLinesButton->setFocusPolicy( QWidget::TabFocus );
	markedWrappedLinesButton->setBackgroundMode( QWidget::PaletteBackground );
	markedWrappedLinesButton->setFontPropagation( QWidget::NoChildren );
	markedWrappedLinesButton->setPalettePropagation( QWidget::NoChildren );
	markedWrappedLinesButton->setText( "marked wrapped lines" );
	markedWrappedLinesButton->setAutoRepeat( FALSE );
	markedWrappedLinesButton->setAutoResize( FALSE );
	connect (markedWrappedLinesButton,SIGNAL(clicked()),SLOT(slotWrappedLinesClicked()));
 
	QLabel* qtarch_label;
	qtarch_label = new QLabel( parent, "label" );
	qtarch_label->setGeometry( 30, 140, 90, 30 );
	qtarch_label->setMinimumSize( 0, 0 );
	qtarch_label->setMaximumSize( 32767, 32767 );
	qtarch_label->setFocusPolicy( QWidget::NoFocus );
	qtarch_label->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_label->setFontPropagation( QWidget::NoChildren );
	qtarch_label->setPalettePropagation( QWidget::NoChildren );
	qtarch_label->setText( "lines per page" );
	qtarch_label->setAlignment( 289 );
	qtarch_label->setMargin( -1 );

	numberingPagesButton = new QCheckBox( parent, "numberingPagesButton" );
	numberingPagesButton->setGeometry( 30, 65, 115, 25 );
	numberingPagesButton->setMinimumSize( 0, 0 );
	numberingPagesButton->setMaximumSize( 32767, 32767 );
	numberingPagesButton->setFocusPolicy( QWidget::TabFocus );
	numberingPagesButton->setBackgroundMode( QWidget::PaletteBackground );
	numberingPagesButton->setFontPropagation( QWidget::NoChildren );
	numberingPagesButton->setPalettePropagation( QWidget::NoChildren );
	numberingPagesButton->setText( "numbering pages" );
	numberingPagesButton->setAutoRepeat( FALSE );
	numberingPagesButton->setAutoResize( FALSE );

	QLabel* qtarch_Label_8;
	qtarch_Label_8 = new QLabel( parent, "Label_8" );
	qtarch_Label_8->setGeometry( 30, 100, 110, 30 );
	qtarch_Label_8->setMinimumSize( 0, 0 );
	qtarch_Label_8->setMaximumSize( 32767, 32767 );
	qtarch_Label_8->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_8->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_8->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_8->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_8->setText( "characters per line" );
	qtarch_Label_8->setAlignment( 289 );
	qtarch_Label_8->setMargin( -1 );

	numberingLineButton = new QCheckBox( parent, "numberingLineButton" );
	numberingLineButton->setGeometry( 30, 30, 100, 30 );
	numberingLineButton->setMinimumSize( 0, 0 );
	numberingLineButton->setMaximumSize( 32767, 32767 );
	numberingLineButton->setFocusPolicy( QWidget::TabFocus );
	numberingLineButton->setBackgroundMode( QWidget::PaletteBackground );
	numberingLineButton->setFontPropagation( QWidget::NoChildren );
	numberingLineButton->setPalettePropagation( QWidget::NoChildren );
	numberingLineButton->setText( "numbering lines" );
	numberingLineButton->setAutoRepeat( FALSE );
	numberingLineButton->setAutoResize( FALSE );

	linesPerPage = new QSpinBox( parent, "linesPerPage" );
	linesPerPage->setGeometry( 160, 140, 120, 30 );
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

	characterPerLine = new QSpinBox( parent, "characterPerLine" );
	characterPerLine->setGeometry( 160, 100, 120, 30 );
	characterPerLine->setMinimumSize( 0, 0 );
	characterPerLine->setMaximumSize( 32767, 32767 );
	characterPerLine->setFocusPolicy( QWidget::StrongFocus );
	characterPerLine->setBackgroundMode( QWidget::PaletteBackground );
	characterPerLine->setFontPropagation( QWidget::NoChildren );
	characterPerLine->setPalettePropagation( QWidget::NoChildren );
	characterPerLine->setFrameStyle( 50 );
	characterPerLine->setLineWidth( 2 );
	characterPerLine->setRange( 0, 99 );
	characterPerLine->setSteps( 1, 0 );
	characterPerLine->setPrefix( "" );
	characterPerLine->setSuffix( "" );
	characterPerLine->setSpecialValueText( "" );
	characterPerLine->setWrapping( FALSE );

	valueForWrappedLine = new QComboBox( FALSE, parent, "valueForWrappedLine" );
	valueForWrappedLine->setGeometry( 180, 370, 90, 30 );
	valueForWrappedLine->setMinimumSize( 0, 0 );
	valueForWrappedLine->setMaximumSize( 32767, 32767 );
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

	cycleOfChange = new QSpinBox( parent, "cycleOfChange" );
	cycleOfChange->setGeometry( 160, 280, 110, 30 );
	cycleOfChange->setMinimumSize( 0, 0 );
	cycleOfChange->setMaximumSize( 32767, 32767 );
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

	qtarch_Label_13 = new QLabel( parent, "Label_13" );
	qtarch_Label_13->setGeometry( 40, 370, 130, 30 );
	qtarch_Label_13->setMinimumSize( 0, 0 );
	qtarch_Label_13->setMaximumSize( 32767, 32767 );
	qtarch_Label_13->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_13->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_13->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_13->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_13->setText( "value for wrapped line" );
	qtarch_Label_13->setAlignment( 289 );
	qtarch_Label_13->setMargin( -1 );

	numberingPagesList = new QComboBox( FALSE, parent, "numberingPagesList" );
	numberingPagesList->setGeometry( 160, 60, 120, 30 );
	numberingPagesList->setMinimumSize( 0, 0 );
	numberingPagesList->setMaximumSize( 32767, 32767 );
	numberingPagesList->setFocusPolicy( QWidget::StrongFocus );
	numberingPagesList->setBackgroundMode( QWidget::PaletteBackground );
	numberingPagesList->setFontPropagation( QWidget::NoChildren );
	numberingPagesList->setPalettePropagation( QWidget::NoChildren );
	numberingPagesList->setSizeLimit( 10 );
	numberingPagesList->setAutoResize( FALSE );
	numberingPagesList->insertItem( "files single" );
	numberingPagesList->insertItem( "files together" );

	highlightBarsButton = new QCheckBox( parent, "highlightBarsButton" );
	highlightBarsButton->setGeometry( 40, 250, 100, 30 );
	highlightBarsButton->setMinimumSize( 0, 0 );
	highlightBarsButton->setMaximumSize( 32767, 32767 );
	highlightBarsButton->setFocusPolicy( QWidget::TabFocus );
	highlightBarsButton->setBackgroundMode( QWidget::PaletteBackground );
	highlightBarsButton->setFontPropagation( QWidget::NoChildren );
	highlightBarsButton->setPalettePropagation( QWidget::NoChildren );
	highlightBarsButton->setText( "highlight bars" );
	highlightBarsButton->setAutoRepeat( FALSE );
	highlightBarsButton->setAutoResize( FALSE );
	connect (highlightBarsButton,SIGNAL(clicked()),SLOT(slotHighlightBarsClicked()));

	interpretButton = new QCheckBox( parent, "interpretButton" );
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

	cutLinesButton = new QCheckBox( parent, "cutLinesButton" );
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
	if (prog!=1) {
	    connect (cutLinesButton,SIGNAL(clicked()),SLOT(slotCutLinesClicked()));
	}

	replaceButton = new QCheckBox( parent, "replaceButton" );
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

	printAsISOLatin = new QCheckBox( parent, "printAsISOLatin" );
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

	boltFontButton = new QCheckBox( parent, "boltFontButton" );
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

	QLabel* qtarch_Label_2;
	qtarch_Label_2 = new QLabel( parent, "Label_2" );
	qtarch_Label_2->setGeometry( 310, 260, 100, 30 );
	qtarch_Label_2->setMinimumSize( 0, 0 );
	qtarch_Label_2->setMaximumSize( 32767, 32767 );
	qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_2->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_2->setText( "set TAB size" );
	qtarch_Label_2->setAlignment( 289 );
	qtarch_Label_2->setMargin( -1 );

	qtarch_Label_3 = new QLabel( parent, "Label_3" );
	qtarch_Label_3->setGeometry( 310, 300, 100, 30 );
	qtarch_Label_3->setMinimumSize( 0, 0 );
	qtarch_Label_3->setMaximumSize( 32767, 32767 );
	qtarch_Label_3->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_3->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_3->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_3->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_3->setText( "font for a2ps" );
	qtarch_Label_3->setAlignment( 289 );
	qtarch_Label_3->setMargin( -1 );

	qtarch_Label_4 = new QLabel( parent, "Label_4" );
	qtarch_Label_4->setGeometry( 310, 340, 100, 30 );
	qtarch_Label_4->setMinimumSize( 0, 0 );
	qtarch_Label_4->setMaximumSize( 32767, 32767 );
	qtarch_Label_4->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_4->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_4->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_4->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_4->setText( "font for header" );
	qtarch_Label_4->setAlignment( 289 );
	qtarch_Label_4->setMargin( -1 );

	qtarch_Label_5 = new QLabel( parent, "Label_5" );
	qtarch_Label_5->setGeometry( 310, 380, 100, 30 );
	qtarch_Label_5->setMinimumSize( 0, 0 );
	qtarch_Label_5->setMaximumSize( 32767, 32767 );
	qtarch_Label_5->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_5->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_5->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_5->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_5->setText( "font for body" );
	qtarch_Label_5->setAlignment( 289 );
	qtarch_Label_5->setMargin( -1 );

	setTabSize = new QSpinBox( parent, "setTabSize" );
	setTabSize->setGeometry( 430, 260, 140, 30 );
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

	a2psFontButton = new QPushButton( parent, "a2psFontButton" );
	a2psFontButton->setGeometry( 430, 300, 140, 30 );
	a2psFontButton->setMinimumSize( 0, 0 );
	a2psFontButton->setMaximumSize( 32767, 32767 );
	a2psFontButton->setFocusPolicy( QWidget::TabFocus );
	a2psFontButton->setBackgroundMode( QWidget::PaletteBackground );
	a2psFontButton->setFontPropagation( QWidget::NoChildren );
	a2psFontButton->setPalettePropagation( QWidget::NoChildren );
	a2psFontButton->setText( "" );
	a2psFontButton->setAutoRepeat( FALSE );
	a2psFontButton->setAutoResize( FALSE );
	connect (a2psFontButton,SIGNAL(clicked()),SLOT(slotFontA2psClicked()));

	fontForBodyButton = new QPushButton( parent, "fontForBodyButton" );
	fontForBodyButton->setGeometry( 430, 380, 140, 30 );
	fontForBodyButton->setMinimumSize( 0, 0 );
	fontForBodyButton->setMaximumSize( 32767, 32767 );
	fontForBodyButton->setFocusPolicy( QWidget::TabFocus );
	fontForBodyButton->setBackgroundMode( QWidget::PaletteBackground );
	fontForBodyButton->setFontPropagation( QWidget::NoChildren );
	fontForBodyButton->setPalettePropagation( QWidget::NoChildren );
	fontForBodyButton->setText( "" );
	fontForBodyButton->setAutoRepeat( FALSE );
	fontForBodyButton->setAutoResize( FALSE );
	connect (fontForBodyButton,SIGNAL(clicked()),SLOT(slotFontBodyClicked()));

	fontForHeaderButton = new QPushButton( parent, "fontForHeaderButton" );
	fontForHeaderButton->setGeometry( 430, 340, 140, 30 );
	fontForHeaderButton->setMinimumSize( 0, 0 );
	fontForHeaderButton->setMaximumSize( 32767, 32767 );
	fontForHeaderButton->setFocusPolicy( QWidget::TabFocus );
	fontForHeaderButton->setBackgroundMode( QWidget::PaletteBackground );
	fontForHeaderButton->setFontPropagation( QWidget::NoChildren );
	fontForHeaderButton->setPalettePropagation( QWidget::NoChildren );
	fontForHeaderButton->setText( "" );
	fontForHeaderButton->setAutoRepeat( FALSE );
	fontForHeaderButton->setAutoResize( FALSE );
	connect (fontForHeaderButton,SIGNAL(clicked()),SLOT(slotFontHeaderClicked()));

	qtarch_ButtonGroup_20->insert( a2psFontButton );
	qtarch_ButtonGroup_20->insert( fontForBodyButton );
	qtarch_ButtonGroup_20->insert( fontForHeaderButton );
	qtarch_ButtonGroup_21->insert( interpretButton );
	qtarch_ButtonGroup_21->insert( cutLinesButton );
	qtarch_ButtonGroup_21->insert( replaceButton );
	qtarch_ButtonGroup_21->insert( printAsISOLatin );
	qtarch_ButtonGroup_21->insert( boltFontButton );
	qtarch_ButtonGroup_51->insert( tocButton );
	qtarch_ButtonGroup_51->insert( markedWrappedLinesButton );
	qtarch_ButtonGroup_51->insert( highlightBarsButton );
	qtarch_ButtonGroup_52->insert( highlightBarsButton );
	qtarch_ButtonGroup_53->insert( markedWrappedLinesButton );
	qtarch_ButtonGroup_33->insert( bordersButton );
	qtarch_ButtonGroup_33->insert( numberingPagesButton );
	qtarch_ButtonGroup_33->insert( numberingLineButton );
}

void CConfigPrintDlg::initTab4(QWidget*parent, int prog) {
	qtarch_ButtonGroup_138 = new QButtonGroup( parent, "ButtonGroup_138" );
	qtarch_ButtonGroup_138->setGeometry( 20, 50, 560, 60 );
	qtarch_ButtonGroup_138->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_138->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_138->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_138->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_138->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_138->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_138->setFrameStyle( 49 );
	qtarch_ButtonGroup_138->setTitle( "Underlaytext" );
	qtarch_ButtonGroup_138->setAlignment( 1 );

	qtarch_ButtonGroup_137 = new QButtonGroup( parent, "ButtonGroup_137" );
	qtarch_ButtonGroup_137->setGeometry( 280, 360, 300, 60 );
	qtarch_ButtonGroup_137->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_137->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_137->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_137->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_137->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_137->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_137->setFrameStyle( 49 );
	qtarch_ButtonGroup_137->setTitle( "Underlaystyle" );
	qtarch_ButtonGroup_137->setAlignment( 1 );

	qtarch_ButtonGroup_136 = new QButtonGroup( parent, "ButtonGroup_136" );
	qtarch_ButtonGroup_136->setGeometry( 380, 240, 190, 100 );
	qtarch_ButtonGroup_136->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_136->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_136->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_136->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_136->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_136->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_136->setFrameStyle( 49 );
	qtarch_ButtonGroup_136->setTitle( "Underlayangle" );
	qtarch_ButtonGroup_136->setAlignment( 1 );

	qtarch_ButtonGroup_135 = new QButtonGroup( parent, "ButtonGroup_135" );
	qtarch_ButtonGroup_135->setGeometry( 20, 360, 250, 60 );
	qtarch_ButtonGroup_135->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_135->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_135->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_135->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_135->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_135->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_135->setFrameStyle( 49 );
	qtarch_ButtonGroup_135->setTitle( "Underlaygray" );
	qtarch_ButtonGroup_135->setAlignment( 1 );

	qtarch_ButtonGroup_134 = new QButtonGroup( parent, "ButtonGroup_134" );
	qtarch_ButtonGroup_134->setGeometry( 20, 240, 350, 100 );
	qtarch_ButtonGroup_134->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_134->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_134->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_134->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_134->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_134->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_134->setFrameStyle( 49 );
	qtarch_ButtonGroup_134->setTitle( "Underlayfont" );
	qtarch_ButtonGroup_134->setAlignment( 1 );

	qtarch_ButtonGroup_131 = new QButtonGroup( parent, "ButtonGroup_131" );
	qtarch_ButtonGroup_131->setGeometry( 20, 130, 560, 90 );
	qtarch_ButtonGroup_131->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_131->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_131->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_131->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_131->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_131->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_131->setFrameStyle( 49 );
	qtarch_ButtonGroup_131->setTitle( "Underlayposition" );
	qtarch_ButtonGroup_131->setAlignment( 1 );

	qtarch_ButtonGroup_132 = new QButtonGroup( parent, "ButtonGroup_132" );
	qtarch_ButtonGroup_132->setGeometry( 450, 150, 120, 60 );
	qtarch_ButtonGroup_132->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_132->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_132->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_132->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_132->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_132->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_132->setFrameStyle( 49 );
	qtarch_ButtonGroup_132->setTitle( "Y-Position" );
	qtarch_ButtonGroup_132->setAlignment( 1 );

	qtarch_ButtonGroup_133 = new QButtonGroup( parent, "ButtonGroup_133" );
	qtarch_ButtonGroup_133->setGeometry( 320, 150, 120, 60 );
	qtarch_ButtonGroup_133->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_133->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_133->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_133->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_133->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_133->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_133->setFrameStyle( 49 );
	qtarch_ButtonGroup_133->setTitle( "X-Position" );
	qtarch_ButtonGroup_133->setAlignment( 1 );

	underlayButton = new QCheckBox( parent, "underlayButton" );
	underlayButton->setGeometry( 20, 10, 100, 30 );
	underlayButton->setMinimumSize( 0, 0 );
	underlayButton->setMaximumSize( 32767, 32767 );
	underlayButton->setFocusPolicy( QWidget::TabFocus );
	underlayButton->setBackgroundMode( QWidget::PaletteBackground );
	underlayButton->setFontPropagation( QWidget::NoChildren );
	underlayButton->setPalettePropagation( QWidget::NoChildren );
	underlayButton->setText( "underlay" );
	underlayButton->setAutoRepeat( FALSE );
	underlayButton->setAutoResize( FALSE );
	connect (underlayButton,SIGNAL(clicked()),SLOT(slotUnderlayButtonClicked()));

	qtarch_Label_70 = new QLabel( parent, "Label_70" );
	qtarch_Label_70->setGeometry( 290, 380, 100, 30 );
	qtarch_Label_70->setMinimumSize( 0, 0 );
	qtarch_Label_70->setMaximumSize( 32767, 32767 );
	qtarch_Label_70->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_70->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_70->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_70->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_70->setText( "underlaystyle" );
	qtarch_Label_70->setAlignment( 289 );
	qtarch_Label_70->setMargin( -1 );

	qtarch_Label_71 = new QLabel( parent, "Label_71" );
	qtarch_Label_71->setGeometry( 390, 300, 60, 30 );
	qtarch_Label_71->setMinimumSize( 0, 0 );
	qtarch_Label_71->setMaximumSize( 32767, 32767 );
	qtarch_Label_71->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_71->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_71->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_71->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_71->setText( "angle" );
	qtarch_Label_71->setAlignment( 289 );
	qtarch_Label_71->setMargin( -1 );

	qtarch_Label_72 = new QLabel( parent, "Label_72" );
	qtarch_Label_72->setGeometry( 220, 170, 100, 30 );
	qtarch_Label_72->setMinimumSize( 0, 0 );
	qtarch_Label_72->setMaximumSize( 32767, 32767 );
	qtarch_Label_72->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_72->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_72->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_72->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_72->setText( "position" );
	qtarch_Label_72->setAlignment( 289 );
	qtarch_Label_72->setMargin( -1 );

	qtarch_Label_73 = new QLabel( parent, "Label_73" );
	qtarch_Label_73->setGeometry( 30, 280, 100, 30 );
	qtarch_Label_73->setMinimumSize( 0, 0 );
	qtarch_Label_73->setMaximumSize( 32767, 32767 );
	qtarch_Label_73->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_73->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_73->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_73->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_73->setText( "font" );
	qtarch_Label_73->setAlignment( 289 );
	qtarch_Label_73->setMargin( -1 );

	underlayAngle = new QSpinBox( parent, "underlayAngle" );
	underlayAngle->setGeometry( 460, 300, 100, 30 );
	underlayAngle->setMinimumSize( 0, 0 );
	underlayAngle->setMaximumSize( 32767, 32767 );
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

	underlayXPosition = new QSpinBox( parent, "underlayXPosition" );
	underlayXPosition->setGeometry( 330, 170, 100, 30 );
	underlayXPosition->setMinimumSize( 0, 0 );
	underlayXPosition->setMaximumSize( 32767, 32767 );
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

	underlayYPosition = new QSpinBox( parent, "underlayYPosition" );
	underlayYPosition->setGeometry( 460, 170, 100, 30 );
	underlayYPosition->setMinimumSize( 0, 0 );
	underlayYPosition->setMaximumSize( 32767, 32767 );
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

	qtarch_Label_74 = new QLabel( parent, "Label_74" );
	qtarch_Label_74->setGeometry( 30, 380, 120, 30 );
	qtarch_Label_74->setMinimumSize( 0, 0 );
	qtarch_Label_74->setMaximumSize( 32767, 32767 );
	qtarch_Label_74->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_74->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_74->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_74->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_74->setText( "underlaygray (x(1/10))" );
	qtarch_Label_74->setAlignment( 289 );
	qtarch_Label_74->setMargin( -1 );

	underlayGray = new QSpinBox( parent, "underlayGray" );
	underlayGray->setGeometry( 180, 380, 80, 30 );
	underlayGray->setMinimumSize( 0, 0 );
	underlayGray->setMaximumSize( 32767, 32767 );
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

	underlayAngleDefault = new QCheckBox( parent, "underlayAngleDefault" );
	underlayAngleDefault->setGeometry( 390, 260, 100, 30 );
	underlayAngleDefault->setMinimumSize( 0, 0 );
	underlayAngleDefault->setMaximumSize( 32767, 32767 );
	underlayAngleDefault->setFocusPolicy( QWidget::TabFocus );
	underlayAngleDefault->setBackgroundMode( QWidget::PaletteBackground );
	underlayAngleDefault->setFontPropagation( QWidget::NoChildren );
	underlayAngleDefault->setPalettePropagation( QWidget::NoChildren );
	underlayAngleDefault->setText( "default" );
	underlayAngleDefault->setAutoRepeat( FALSE );
	underlayAngleDefault->setAutoResize( FALSE );
	connect (underlayAngleDefault,SIGNAL(clicked()),SLOT(slotUnderlayAngleDefaultClicked()));

	underlayStyle = new QComboBox( FALSE, parent, "underlayStyle" );
	underlayStyle->setGeometry( 400, 380, 170, 30 );
	underlayStyle->setMinimumSize( 0, 0 );
	underlayStyle->setMaximumSize( 32767, 32767 );
	underlayStyle->setFocusPolicy( QWidget::StrongFocus );
	underlayStyle->setBackgroundMode( QWidget::PaletteBackground );
	underlayStyle->setFontPropagation( QWidget::AllChildren );
	underlayStyle->setPalettePropagation( QWidget::AllChildren );
	underlayStyle->setSizeLimit( 10 );
	underlayStyle->setAutoResize( FALSE );
	underlayStyle->insertItem( "outline" );
	underlayStyle->insertItem( "filled" );

	underlayPositionDefaultButton = new QCheckBox( parent, "underlayPositionDefaultButton" );
	underlayPositionDefaultButton->setGeometry( 30, 170, 100, 30 );
	underlayPositionDefaultButton->setMinimumSize( 0, 0 );
	underlayPositionDefaultButton->setMaximumSize( 32767, 32767 );
	underlayPositionDefaultButton->setFocusPolicy( QWidget::TabFocus );
	underlayPositionDefaultButton->setBackgroundMode( QWidget::PaletteBackground );
	underlayPositionDefaultButton->setFontPropagation( QWidget::NoChildren );
	underlayPositionDefaultButton->setPalettePropagation( QWidget::NoChildren );
	underlayPositionDefaultButton->setText( "default" );
	underlayPositionDefaultButton->setAutoRepeat( FALSE );
	underlayPositionDefaultButton->setAutoResize( FALSE );
	connect (underlayPositionDefaultButton,SIGNAL(clicked()),SLOT(slotUnderlayPositionDefaultClicked()));

	qtarch_Label_75 = new QLabel( parent, "Label_75" );
	qtarch_Label_75->setGeometry( 30, 70, 120, 30 );
	qtarch_Label_75->setMinimumSize( 0, 0 );
	qtarch_Label_75->setMaximumSize( 32767, 32767 );
	qtarch_Label_75->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_75->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_75->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_75->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_75->setText( "underlaytext" );
	qtarch_Label_75->setAlignment( 289 );
	qtarch_Label_75->setMargin( -1 );

	underlaytextLine = new QLineEdit( parent, "underlaytextLine" );
	underlaytextLine->setGeometry( 160, 70, 410, 30 );
	underlaytextLine->setMinimumSize( 0, 0 );
	underlaytextLine->setMaximumSize( 32767, 32767 );
	underlaytextLine->setFocusPolicy( QWidget::StrongFocus );
	underlaytextLine->setBackgroundMode( QWidget::PaletteBase );
	underlaytextLine->setFontPropagation( QWidget::NoChildren );
	underlaytextLine->setPalettePropagation( QWidget::NoChildren );
	underlaytextLine->setText( "" );
	underlaytextLine->setMaxLength( 32767 );
	underlaytextLine->setEchoMode( QLineEdit::Normal );
	underlaytextLine->setFrame( TRUE );

	underlayFontButton = new QPushButton( parent, "underlayFontButton" );
	underlayFontButton->setGeometry( 140, 280, 220, 30 );
	underlayFontButton->setMinimumSize( 0, 0 );
	underlayFontButton->setMaximumSize( 32767, 32767 );
	underlayFontButton->setFocusPolicy( QWidget::TabFocus );
	underlayFontButton->setBackgroundMode( QWidget::PaletteBackground );
	underlayFontButton->setFontPropagation( QWidget::NoChildren );
	underlayFontButton->setPalettePropagation( QWidget::NoChildren );
	underlayFontButton->setText( "" );
	underlayFontButton->setAutoRepeat( FALSE );
	underlayFontButton->setAutoResize( FALSE );
	connect (underlayFontButton, SIGNAL(clicked()),SLOT(slotFontUnderlayClicked()));

	qtarch_ButtonGroup_136->insert( underlayAngleDefault );
	qtarch_ButtonGroup_134->insert( underlayFontButton );
	qtarch_ButtonGroup_131->insert( underlayPositionDefaultButton );

}

void CConfigPrintDlg::slotHeader1Clicked() {
    if (headerButton->isChecked()) {
	slotHeadertext1Clicked();
	slotCurrentDate1Clicked();
	slotCurrentTime1Clicked();
	loginButton->setEnabled(true);
	headertextButton->setEnabled(true);
	filenameLine->setEnabled(true);
	currentDateButton->setEnabled(true);
	currentTimeButton->setEnabled(true);
	qtarch_ButtonGroup_59->setEnabled(true);
	qtarch_ButtonGroup_61->setEnabled(true);
	qtarch_ButtonGroup_60->setEnabled(true);
	qtarch_ButtonGroup_65->setEnabled(true);
	qtarch_ButtonGroup_78->setEnabled(true);
	qtarch_ButtonGroup_74->setEnabled(true);
    }
    else {
	slotNoHeader();
    }
}

void CConfigPrintDlg::slotHeader2Clicked() {
    if (headerButton->isChecked()) {
	slotHeadertext2Clicked();
	slotLoginClicked();
	slotFilenameClicked();
	slotHostnameClicked();
	slotCurrentDate2Clicked();
	slotCurrentTime2Clicked();
	slotModiDateClicked();
	slotModiTimeClicked();
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

void CConfigPrintDlg::slotNoHeader() {
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

void CConfigPrintDlg::slotHeadertext2Clicked() {
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

void CConfigPrintDlg::slotHeadertext1Clicked() {
    if (headertextButton->isChecked()) {
	headertextLine->setEnabled(true);
	qtarch_Label_20->setEnabled(true);
    }
    else {
	headertextLine->setEnabled(false);
	qtarch_Label_20->setEnabled(false);
    }
}

void CConfigPrintDlg::slotLoginClicked() {
    if (loginButton->isChecked()) {
	loginPosition->setEnabled(true);
	qtarch_Label_24->setEnabled(true);
    }
    else {
	loginPosition->setEnabled(false);
	qtarch_Label_24->setEnabled(false);
    }
}

void CConfigPrintDlg::slotFilenameClicked() {
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

void CConfigPrintDlg::slotHostnameClicked() {
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

void CConfigPrintDlg::slotCurrentDate2Clicked() {
    if (currentDateButton->isChecked()) {
	qtarch_ButtonGroup_75->setEnabled(true);
	qtarch_ButtonGroup_84->setEnabled(true);
	currentDatePosition->setEnabled(true);
	currentDateFormat->setEnabled(true);
    }
    else {
	qtarch_ButtonGroup_75->setEnabled(false);
	qtarch_ButtonGroup_84->setEnabled(false);
	currentDatePosition->setEnabled(false);
	currentDateFormat->setEnabled(false);
    }
}
void CConfigPrintDlg::slotCurrentDate1Clicked() {
    if (currentDateButton->isChecked()) {
	currentTimeButton->setChecked(true);
    }
    else currentTimeButton->setChecked(false);
}

void CConfigPrintDlg::slotCurrentTime2Clicked() {
    if (currentTimeButton->isChecked()) {
	qtarch_ButtonGroup_90->setEnabled(true);
	qtarch_ButtonGroup_71->setEnabled(true);
	qtarch_ButtonGroup_89->setEnabled(true);
	currentTimeAmpm->setEnabled(true);
	currentTimePosition->setEnabled(true);
	currentTimeFormat->setEnabled(true);
    }
    else {
	qtarch_ButtonGroup_90->setEnabled(false);
	qtarch_ButtonGroup_71->setEnabled(false);
	qtarch_ButtonGroup_89->setEnabled(false);
	currentTimeAmpm->setEnabled(false);
	currentTimePosition->setEnabled(false);
	currentTimeFormat->setEnabled(false);
    }
}

void CConfigPrintDlg::slotCurrentTime1Clicked() {
    if (currentTimeButton->isChecked()) {
	currentDateButton->setChecked(true);
    }
    else currentDateButton->setChecked(false);
}

void CConfigPrintDlg::slotModiDateClicked() {
    if (modificationDateButton->isChecked()) {
	qtarch_ButtonGroup_86->setEnabled(true);
	qtarch_ButtonGroup_85->setEnabled(true);
	modificationDatePosition->setEnabled(true);
	modificationDateFormat->setEnabled(true);
    }
    else {
	qtarch_ButtonGroup_86->setEnabled(false);
	qtarch_ButtonGroup_85->setEnabled(false);
	modificationDatePosition->setEnabled(false);
	modificationDateFormat->setEnabled(false);
    }
}

void CConfigPrintDlg::slotModiTimeClicked() {
    if (modificationTimeButton->isChecked()) {
	qtarch_ButtonGroup_88->setEnabled(true);
	qtarch_ButtonGroup_72->setEnabled(true);
	qtarch_ButtonGroup_87->setEnabled(true);
	modificationTimeAmpm->setEnabled(true);
	modificationTimePosition->setEnabled(true);
	modificationTimeFormat->setEnabled(true);
    }
    else {
	qtarch_ButtonGroup_87->setEnabled(false);
	qtarch_ButtonGroup_72->setEnabled(false);
	qtarch_ButtonGroup_88->setEnabled(false);
	modificationTimeAmpm->setEnabled(false);
	modificationTimePosition->setEnabled(false);
	modificationTimeFormat->setEnabled(false);
    }
}

void CConfigPrintDlg::slotCutLinesClicked() {
    if (cutLinesButton->isChecked()) {
	qtarch_ButtonGroup_53->setEnabled(false);
	markedWrappedLinesButton->setEnabled(false);
	slotWrappedLinesClicked();
    }
    else {
	qtarch_ButtonGroup_53->setEnabled(true);
	markedWrappedLinesButton->setEnabled(true);
	slotWrappedLinesClicked();
    }
}

void CConfigPrintDlg::slotWrappedLinesClicked() {
    if (markedWrappedLinesButton->isChecked()) {
	qtarch_Label_13->setEnabled(true);
	valueForWrappedLine->setEnabled(true);
    }
    else {
	qtarch_Label_13->setEnabled(false);
	valueForWrappedLine->setEnabled(false);
    }
}

void CConfigPrintDlg::slotHighlightBarsClicked() {
    if (highlightBarsButton->isChecked()) {
	qtarch_Label_14->setEnabled(true);
	cycleOfChange->setEnabled(true);
    }
    else {
	qtarch_Label_14->setEnabled(false);
	cycleOfChange->setEnabled(false);
    }
}

void CConfigPrintDlg::slotUnderlayButtonClicked() {
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

void CConfigPrintDlg::slotUnderlayPositionDefaultClicked() {
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

void CConfigPrintDlg::slotUnderlayAngleDefaultClicked() {
  if (underlayAngleDefault->isChecked()) {
    underlayAngle->setEnabled(false);
    qtarch_Label_71->setEnabled(false);
  }
  else {
    underlayAngle->setEnabled(true);
    qtarch_Label_71->setEnabled(true);
  }
}

void CConfigPrintDlg::slotFontUnderlayClicked() {
  QFont font;
  QString string1,string2;
  int size;
  fontDialog = new KFontDialog(this,"Fontdialog",TRUE);
  fontDialog->exec();
  font=fontDialog->font();
  string1=font.family();
  size=font.pointSize();
  underlayFontButton->setText(string1+string2.setNum(size));
}

void CConfigPrintDlg::slotFontBodyClicked() {
  QFont font;
  QString string1,string2;
  int size;
  fontDialog = new KFontDialog(this,"Fontdialog",TRUE);
  fontDialog->exec();
  font=fontDialog->font();
  string1=font.family();
  size=font.pointSize();
  fontForBodyButton->setText(string1+string2.setNum(size));
}

void CConfigPrintDlg::slotFontHeaderClicked() {
  QFont font;
  QString string1,string2;
  int size;
  fontDialog = new KFontDialog(this,"Fontdialog",TRUE);
  fontDialog->exec();
  font=fontDialog->font();
  string1=font.family();
  size=font.pointSize();
  fontForHeaderButton->setText(string1+string2.setNum(size));
}

void CConfigPrintDlg::slotFontA2psClicked() {
  QFont font;
  QString string1,string2;
  int size;
  fontDialog = new KFontDialog(this,"Fontdialog",TRUE);
  fontDialog->exec();
  font=fontDialog->font();
  string1=font.family();
  size=font.pointSize();
  a2psFontButton->setText(string1+string2.setNum(size));
}
