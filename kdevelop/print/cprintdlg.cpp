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

#include "../ckdevelop.h"
#include "cprintdlg.h"

#include "cfileprintdlg.h"

#include "../resource.h"
#include "../cproject.h"

//#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qgrid.h>

class CKDevelop;
//#include <string.h>

// Remove all null parameters (ie of the form "xyz= ")
QString removeNullParams(const QString& params)
{
  QStringList paramsList = QStringList::split(" ", params, false);
  QStringList::Iterator paramsListIt = paramsList.begin();
  QString returnParams = *paramsListIt;   // WARNING: There's must be a nice first param

  while ( ++paramsListIt != paramsList.end())
  {
    if ((*paramsListIt).right(1) != "=")
      returnParams += " " + *paramsListIt;
  }

  kdDebug() << "params <" << params << ">" << endl;
  kdDebug() << "return <" << returnParams << ">" << endl;

  return returnParams;
}

CPrintDlg::CPrintDlg(QWidget* parent,const char* edittab,const char* name, bool html) :
  QDialog(parent, name, true)
{
  init();
  loadSettings();
  string = "";
  globalpara = "";
	oldfiles = (QString) edittab;
  files = createFileString();
  doctab = html;
  // the processes are maintained by CKDevelop
  print_process = (dynamic_cast<CKDevelop*>(parent))->print_process();
  preview_process = (dynamic_cast<CKDevelop*>(parent))->preview_process();
}

CPrintDlg::~CPrintDlg(){
  delete (cancelButton);
  delete (previewButton);
  delete (okButton);
  delete (printToFileButton);
  delete (mediaCombBox);
  delete (prettyCombBox);
  delete (filesConfButton);
  delete (pagePerSide);
  delete (pageSide);
  delete (prettyColorCheckBox);
  delete (prettyPrintCheckBox);
  delete (copySpinBox);
  delete (defaultCombBox);
  delete (formatCombBox);
  delete (printToFileDlg);
  delete (printerLine);
  delete (qtarch_Label_1);
  delete (programCombBox);
  delete (paperCombBox);
  delete (printToFileLine);
  delete (printingConfButton);
  delete (paperFormatGroup);
  delete (qtarch_ButtonGroup_2);
  delete (qtarch_ButtonGroup_3);
  delete (qtarch_ButtonGroup_4);
  delete (qtarch_ButtonGroup_6);
  delete (qtarch_ButtonGroup_5);
  delete (qtarch_ButtonGroup_7);
  delete (qtarch_ButtonGroup_10);
  delete (qtarch_ButtonGroup_11);
  delete (qtarch_ButtonGroup_34);
  delete (qtarch_ButtonGroup_9);
  delete (qtarch_ButtonGroup_8);
  delete (qtarch_ButtonGroup_35);

}


void CPrintDlg::init(){
  QGridLayout *grid1 = new QGridLayout(this,6,4,15,7);

  ////////// Program select ///////////
  qtarch_ButtonGroup_2 = new QButtonGroup( this, "ButtonGroup_2" );
  qtarch_ButtonGroup_2->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_2->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_2->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_2->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_2->setFrameStyle( 49 );
  qtarch_ButtonGroup_2->setTitle(i18n( "Program" ));
  qtarch_ButtonGroup_2->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_2,0,0);

  QHBoxLayout *box1 = new QHBoxLayout( qtarch_ButtonGroup_2 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );
  programCombBox = new QComboBox( FALSE, qtarch_ButtonGroup_2, "programCombBox" );
  programCombBox->setFocusPolicy( QWidget::StrongFocus );
  programCombBox->setBackgroundMode( QWidget::PaletteBackground );
  programCombBox->setFontPropagation( QWidget::NoChildren );
  programCombBox->setPalettePropagation( QWidget::NoChildren );
  programCombBox->setSizeLimit( 10 );
  programCombBox->setAutoResize( FALSE );
  programCombBox->insertItem( "a2ps" );
  programCombBox->insertItem( "enscript" );
  programCombBox->insertItem( "lpr" );
  connect(programCombBox,SIGNAL(activated (int)),SLOT(slotProgramActivated(int)));
  box1->addWidget(programCombBox);
  ////////// Printer select /////////////
  qtarch_ButtonGroup_5 = new QButtonGroup( this, "ButtonGroup_5" );
  qtarch_ButtonGroup_5->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_5->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_5->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_5->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_5->setFrameStyle( 49 );
  qtarch_ButtonGroup_5->setTitle(i18n( "Printer" ));
  qtarch_ButtonGroup_5->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_5,0,1);

  QVBoxLayout *box2 = new QVBoxLayout( qtarch_ButtonGroup_5 );
  box2->setMargin( 15 );
  box2->setSpacing( 15 );
  qtarch_Label_1 = new QLabel(  qtarch_ButtonGroup_5, "Label_1" );
  qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
  qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
  qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
  qtarch_Label_1->setText(i18n( "printername" ));
  qtarch_Label_1->setAlignment( 289 );
  qtarch_Label_1->setMargin( -1 );
  box2->addWidget(qtarch_Label_1);

  printerLine = new QLineEdit( qtarch_ButtonGroup_5, "printerLine" );
  printerLine->setFocusPolicy( QWidget::StrongFocus );
  printerLine->setBackgroundMode( QWidget::PaletteBase );
  printerLine->setFontPropagation( QWidget::NoChildren );
  printerLine->setPalettePropagation( QWidget::NoChildren );
  printerLine->setText( "default" );
  printerLine->setMaxLength( 32767 );
  printerLine->setEchoMode( QLineEdit::Normal );
  printerLine->setFrame( TRUE );
  box2->addWidget(printerLine);

	//////////// Outputlocation /////////////
  qtarch_ButtonGroup_6 = new QButtonGroup( this, "ButtonGroup_6" );
  qtarch_ButtonGroup_6->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_6->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_6->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_6->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_6->setFrameStyle( 49 );
  qtarch_ButtonGroup_6->setTitle(i18n( "Outputlocation" ));
  qtarch_ButtonGroup_6->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_6,1,1,0,1);

  QGridLayout *grid2 = new QGridLayout(qtarch_ButtonGroup_6,2,2,15,7);

  printToFileButton = new QCheckBox(qtarch_ButtonGroup_6 , "printToFileButton" );
  printToFileButton->setFocusPolicy( QWidget::TabFocus );
  printToFileButton->setBackgroundMode( QWidget::PaletteBackground );
  printToFileButton->setFontPropagation( QWidget::NoChildren );
  printToFileButton->setPalettePropagation( QWidget::NoChildren );
  printToFileButton->setText(i18n( "print to file" ));
  printToFileButton->setAutoRepeat( FALSE );
  printToFileButton->setAutoResize( FALSE );
  connect (printToFileButton,SIGNAL(toggled(bool)),SLOT(slotPrintToFileClicked(bool)));
  grid2->addWidget(printToFileButton,0,0);

  printToFileLine = new QLineEdit( qtarch_ButtonGroup_6, "printToFileLine" );
  printToFileLine->setFocusPolicy( QWidget::StrongFocus );
  printToFileLine->setBackgroundMode( QWidget::PaletteBase );
  printToFileLine->setFontPropagation( QWidget::NoChildren );
  printToFileLine->setPalettePropagation( QWidget::NoChildren );
  printToFileLine->setText( "" );
  printToFileLine->setMaxLength( 32767 );
  printToFileLine->setEchoMode( QLineEdit::Normal );
  printToFileLine->setFrame( TRUE );
  grid2->addWidget(printToFileLine,1,0);

  printToFileDlg = new QPushButton( qtarch_ButtonGroup_6, "PrintToFileDlg" );
  printToFileDlg->setFocusPolicy( QWidget::TabFocus );
  printToFileDlg->setBackgroundMode( QWidget::PaletteBackground );
  printToFileDlg->setFontPropagation( QWidget::NoChildren );
  printToFileDlg->setPalettePropagation( QWidget::NoChildren );
  QPixmap pix(SmallIcon("fileopen"));
  printToFileDlg->setPixmap(pix);
  printToFileDlg->setAutoRepeat( FALSE );
  printToFileDlg->setAutoResize( FALSE );
  connect (printToFileDlg,SIGNAL(clicked()),SLOT(slotPrintToFileDlgClicked()));
  grid2->addWidget(printToFileDlg,1,1);

	/////////// Paper format ///////////
  paperFormatGroup = new QButtonGroup( this, "PaperFormatGroup" );
  paperFormatGroup->setFocusPolicy( QWidget::NoFocus );
  paperFormatGroup->setBackgroundMode( QWidget::PaletteBackground );
  paperFormatGroup->setFontPropagation( QWidget::NoChildren );
  paperFormatGroup->setPalettePropagation( QWidget::NoChildren );
  paperFormatGroup->setFrameStyle( 49 );
  paperFormatGroup->setTitle(i18n( "Paper format" ));
  paperFormatGroup->setAlignment( 1 );
  grid1->addWidget(paperFormatGroup,2,0);



  box1 = new QHBoxLayout( paperFormatGroup );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );
  paperCombBox = new QComboBox( FALSE, paperFormatGroup, "paperCombBox" );
  paperCombBox->setFocusPolicy( QWidget::StrongFocus );
  paperCombBox->setBackgroundMode( QWidget::PaletteBackground );
  paperCombBox->setFontPropagation( QWidget::NoChildren );
  paperCombBox->setPalettePropagation( QWidget::NoChildren );
  paperCombBox->setSizeLimit( 10 );
  paperCombBox->setAutoResize( FALSE );
  paperCombBox->insertItem(i18n("portrait"));
  paperCombBox->insertItem(i18n("landscape"));
  box1->addWidget(paperCombBox);

	////////// Copy //////////////
  qtarch_ButtonGroup_7 = new QButtonGroup( this, "ButtonGroup_7" );
  qtarch_ButtonGroup_7->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_7->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_7->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_7->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_7->setFrameStyle( 49 );
  qtarch_ButtonGroup_7->setTitle(i18n( "Copy" ));
  qtarch_ButtonGroup_7->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_7,2,1);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_7 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );

  copySpinBox = new QSpinBox( qtarch_ButtonGroup_7, "copySpinBox" );
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
  box1->addWidget(copySpinBox);

	////////////// Outputmedia //////////////
  qtarch_ButtonGroup_35 = new QButtonGroup( this, "ButtonGroup_35" );
  qtarch_ButtonGroup_35->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_35->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_35->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_35->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_35->setFrameStyle( 49 );
  qtarch_ButtonGroup_35->setTitle(i18n( "Outputmedia" ));
  qtarch_ButtonGroup_35->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_35,3,0);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_35 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );

  mediaCombBox = new QComboBox( FALSE, qtarch_ButtonGroup_35, "mediaCombBox" );
  mediaCombBox->setGeometry( 30, 300, 120, 30 );
  mediaCombBox->setMinimumSize( 0, 0 );
  mediaCombBox->setMaximumSize( 32767, 32767 );
  mediaCombBox->setFocusPolicy( QWidget::StrongFocus );
  mediaCombBox->setBackgroundMode( QWidget::PaletteBackground );
  mediaCombBox->setFontPropagation( QWidget::NoChildren );
  mediaCombBox->setPalettePropagation( QWidget::NoChildren );
  mediaCombBox->setSizeLimit( 10 );
  mediaCombBox->setAutoResize( FALSE );
  box1->addWidget(mediaCombBox);

  ///////// Outputformat ////////
  qtarch_ButtonGroup_3 = new QButtonGroup( this, "ButtonGroup_3" );
  qtarch_ButtonGroup_3->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_3->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_3->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_3->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_3->setFrameStyle( 49 );
  qtarch_ButtonGroup_3->setTitle(i18n( "Outputformat" ));
  qtarch_ButtonGroup_3->setAlignment( 1 );
  grid1->addWidget(qtarch_ButtonGroup_3,3,1);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_3 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );
  formatCombBox = new QComboBox( FALSE, qtarch_ButtonGroup_3, "formatCombBox" );
  formatCombBox->setFocusPolicy( QWidget::StrongFocus );
  formatCombBox->setBackgroundMode( QWidget::PaletteBackground );
  formatCombBox->setFontPropagation( QWidget::NoChildren );
  formatCombBox->setPalettePropagation( QWidget::NoChildren );
  formatCombBox->setSizeLimit( 10 );
  formatCombBox->setAutoResize( FALSE );
  formatCombBox->insertItem( "postscript" );
  formatCombBox->insertItem( "html" );
  box1->addWidget(formatCombBox);

	////////////// Default Printsettings /////////
  qtarch_ButtonGroup_4 = new QButtonGroup( this, "ButtonGroup_4" );
  qtarch_ButtonGroup_4->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_4->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_4->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_4->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_4->setFrameStyle( 49 );
  qtarch_ButtonGroup_4->setTitle(i18n( "Default printsettings" ));
  qtarch_ButtonGroup_4->setAlignment( 1 );
  grid1->addMultiCellWidget( qtarch_ButtonGroup_4,4,4,0,1);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_4 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );

  defaultCombBox = new QComboBox( FALSE, qtarch_ButtonGroup_4, "defaultCombBox" );
  defaultCombBox->setFocusPolicy( QWidget::StrongFocus );
  defaultCombBox->setBackgroundMode( QWidget::PaletteBackground );
  defaultCombBox->setFontPropagation( QWidget::NoChildren );
  defaultCombBox->setPalettePropagation( QWidget::NoChildren );
  defaultCombBox->setSizeLimit( 10 );
  defaultCombBox->setAutoResize( FALSE );
  defaultCombBox->insertItem(i18n("settings"));
  box1->addWidget(defaultCombBox);

	///////// Top buttons //////
  filesConfButton = new QPushButton( this, "filesConfButton" );
  filesConfButton->setFocusPolicy( QWidget::TabFocus );
  filesConfButton->setBackgroundMode( QWidget::PaletteBackground );
  filesConfButton->setFontPropagation( QWidget::NoChildren );
  filesConfButton->setPalettePropagation( QWidget::NoChildren );
  filesConfButton->setText(i18n( "Files..." ));
  filesConfButton->setAutoRepeat( FALSE );
  filesConfButton->setAutoResize( FALSE );
  connect (filesConfButton, SIGNAL(clicked()),SLOT(slotFilesConfClicked()));
  grid1->addWidget(filesConfButton,0,2);

  printingConfButton = new QPushButton( this, "printingConfButton" );
  printingConfButton->setFocusPolicy( QWidget::TabFocus );
  printingConfButton->setBackgroundMode( QWidget::PaletteBackground );
  printingConfButton->setFontPropagation( QWidget::NoChildren );
  printingConfButton->setPalettePropagation( QWidget::NoChildren );
  printingConfButton->setText(i18n( "Options..." ));
  printingConfButton->setAutoRepeat( FALSE );
  printingConfButton->setAutoResize( FALSE );
  connect (printingConfButton,SIGNAL(clicked()),SLOT(slotPrintingConfClicked()));
  grid1->addWidget(printingConfButton,0,3);

  ///////// Outprinting ////////
  qtarch_ButtonGroup_8 = new QButtonGroup( this, "ButtonGroup_8" );
  qtarch_ButtonGroup_8->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_8->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_8->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_8->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_8->setFrameStyle( 49 );
  qtarch_ButtonGroup_8->setTitle(i18n( "Outprinting" ));
  qtarch_ButtonGroup_8->setAlignment( 1 );
  grid1->addMultiCellWidget( qtarch_ButtonGroup_8,1,4,2,3);


  grid2 = new QGridLayout(qtarch_ButtonGroup_8,2,2,15,7);

  ///// Page printing ////////
  qtarch_ButtonGroup_10 = new QButtonGroup( qtarch_ButtonGroup_8, "ButtonGroup_10" );
  qtarch_ButtonGroup_10->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_10->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_10->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_10->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_10->setFrameStyle( 49 );
  qtarch_ButtonGroup_10->setTitle(i18n( "Page printing" ));
  qtarch_ButtonGroup_10->setAlignment( 1 );
  grid2->addWidget(qtarch_ButtonGroup_10,0,0);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_10 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );


  pagePerSide = new QComboBox( FALSE,qtarch_ButtonGroup_10 , "pagePerSide" );
  pagePerSide->setFocusPolicy( QWidget::StrongFocus );
  pagePerSide->setBackgroundMode( QWidget::PaletteBackground );
  pagePerSide->setFontPropagation( QWidget::NoChildren );
  pagePerSide->setPalettePropagation( QWidget::NoChildren );
  pagePerSide->setSizeLimit( 10 );
  pagePerSide->setAutoResize( FALSE );
  pagePerSide->insertItem(i18n( "one page per side" ));
  pagePerSide->insertItem(i18n( "two pages per side" ));
  box1->addWidget(pagePerSide);

  ///////// Pages  /////////
  qtarch_ButtonGroup_11 = new QButtonGroup( qtarch_ButtonGroup_8, "ButtonGroup_11" );
  qtarch_ButtonGroup_11->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_11->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_11->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_11->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_11->setFrameStyle( 49 );
  qtarch_ButtonGroup_11->setTitle(i18n( "Pages" ));
  qtarch_ButtonGroup_11->setAlignment( 1 );
  grid2->addWidget(qtarch_ButtonGroup_11,1,0);

  box1 = new QHBoxLayout( qtarch_ButtonGroup_11 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );

  pageSide = new QComboBox( TRUE, qtarch_ButtonGroup_11, "pageSide" );
  pageSide->setFocusPolicy( QWidget::StrongFocus );
  pageSide->setBackgroundMode( QWidget::PaletteBackground );
  pageSide->setFontPropagation( QWidget::NoChildren );
  pageSide->setPalettePropagation( QWidget::NoChildren );
  pageSide->setInsertionPolicy( QComboBox::NoInsertion );
  pageSide->setSizeLimit( 10 );
  pageSide->setAutoResize( FALSE );

 
  pageSide->insertItem(i18n ("all"));
  pageSide->insertItem(i18n ("odd"));
  pageSide->insertItem(i18n ("even"));
  box1->addWidget(pageSide);

  ////// Pretty-print //////////
  qtarch_ButtonGroup_9 = new QButtonGroup( qtarch_ButtonGroup_8, "ButtonGroup_9" );
  qtarch_ButtonGroup_9->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_9->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_9->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_9->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_9->setFrameStyle( 49 );
  qtarch_ButtonGroup_9->setTitle(i18n( "Pretty-print" ));
  qtarch_ButtonGroup_9->setAlignment( 1 );
  grid2->addMultiCellWidget(qtarch_ButtonGroup_9,2,3,0,0);


  QGridLayout *grid3 = new QGridLayout(qtarch_ButtonGroup_9,2,2,15,7);
  prettyPrintCheckBox = new QCheckBox(qtarch_ButtonGroup_9 , "prettyPrintCheckBox" );
  prettyPrintCheckBox->setFocusPolicy( QWidget::TabFocus );
  prettyPrintCheckBox->setBackgroundMode( QWidget::PaletteBackground );
  prettyPrintCheckBox->setFontPropagation( QWidget::NoChildren );
  prettyPrintCheckBox->setPalettePropagation( QWidget::NoChildren );
  prettyPrintCheckBox->setText(i18n( "pretty-print" ));
  prettyPrintCheckBox->setAutoRepeat( FALSE );
  prettyPrintCheckBox->setAutoResize( FALSE );
  connect (prettyPrintCheckBox,SIGNAL(toggled(bool)),SLOT(slotPrettyPrintClicked(bool)));
  grid3->addWidget(prettyPrintCheckBox,0,0);
  
  prettyColorCheckBox = new QCheckBox(qtarch_ButtonGroup_9 , "prettyColorCheckBox" );

  prettyColorCheckBox->setFocusPolicy( QWidget::TabFocus );
  prettyColorCheckBox->setBackgroundMode( QWidget::PaletteBackground );
  prettyColorCheckBox->setFontPropagation( QWidget::NoChildren );
  prettyColorCheckBox->setPalettePropagation( QWidget::NoChildren );
  prettyColorCheckBox->setText(i18n( "color" ));
  prettyColorCheckBox->setAutoRepeat( FALSE );
  prettyColorCheckBox->setAutoResize( FALSE );
  grid3->addWidget(prettyColorCheckBox,0,1);

  qtarch_ButtonGroup_34 = new QButtonGroup(qtarch_ButtonGroup_9 , "ButtonGroup_34" );
  qtarch_ButtonGroup_34->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_34->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_34->setFontPropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_34->setPalettePropagation( QWidget::NoChildren );
  qtarch_ButtonGroup_34->setFrameStyle( 49 );
  qtarch_ButtonGroup_34->setTitle(i18n( "Pretty-print mode" ));
  qtarch_ButtonGroup_34->setAlignment( 1 );
  grid3->addMultiCellWidget(qtarch_ButtonGroup_34,1,1,0,1);


  box1 = new QHBoxLayout( qtarch_ButtonGroup_34 );
  box1->setMargin( 15 );
  box1->setSpacing( 15 );
  

  prettyCombBox = new QComboBox( FALSE,qtarch_ButtonGroup_34, "prettyCombBox" );
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
  prettyCombBox->setCurrentItem(2);
  box1->addWidget(prettyCombBox);
  

  previewButton = new QPushButton( this, "previewButton" );
  previewButton->setText(i18n("Preview"));
  connect(previewButton,SIGNAL(clicked()),SLOT(slotPreviewClicked()));
  grid1->addWidget(previewButton,5,0);

  okButton = new QPushButton( this, "okButton" );
  okButton->setText(i18n("OK"));
  okButton->setDefault(true);
  connect(okButton,SIGNAL(clicked()),SLOT(slotOkClicked()));
  grid1->addWidget(okButton,5,2);

  cancelButton = new QPushButton( this, "cancelButton" );
  cancelButton->setText(i18n("Cancel"));
  connect(cancelButton,SIGNAL(clicked()),SLOT(slotCancelClicked()));
  grid1->addWidget(cancelButton,5,3);

  QWhatsThis::add(printingConfButton,
	i18n("If you click this button a configdialog will be shown.\n"
	     "Which configdialog will be shown is depend on the\n"
	     "choosen program."));

  QWhatsThis::add(printToFileDlg,
	i18n("If you click this button a filedialog will be shown."));

  QWhatsThis::add(filesConfButton,
	i18n("If you click this button the fileprintdialog will be shown."));

  QWhatsThis::add(printToFileLine,
	i18n("In this file the printing is gone."));

  QString printerLineMsg = i18n("In this line you can specify the printer, with which\n"
	                            "you will print. If default is written in the line, the\n"
	                            "default printer ($PRINTER) is choosen.");
  QWhatsThis::add(qtarch_Label_1, printerLineMsg);
  QWhatsThis::add(printerLine, printerLineMsg);

  QWhatsThis::add(paperCombBox,
	i18n("Here you can change the paper direction."));

  QWhatsThis::add(programCombBox,
	i18n("Here you can set the program, you whish to use."));

  QWhatsThis::add(formatCombBox,
	i18n("Here you can choose, if you print out in postscript or html."));

  QWhatsThis::add(defaultCombBox,
	i18n("Here you can choose, if you print with your personal\n"
	     "settings. Or with some other settings."));

  QString pageSideMsg =	i18n("Here you can set the sides, which you will print.\n"
                    	     "You can also specify an interval like:\n"
                    	     "5-    :print from side 5 to end\n"
                    	     "5-10 :print from side 5 to side 10\n"
                    	     "-10   :print from side 1 to side 10");
  QWhatsThis::add(qtarch_ButtonGroup_11, pageSideMsg);
  QWhatsThis::add(pageSide, pageSideMsg);

  QWhatsThis::add(pagePerSide,
	i18n("Here you can choose, if you wish to print 1 page per\n"
	     "physical side or 2 page."));

  QWhatsThis::add(prettyCombBox,
	i18n("Here you can set the pretty-print-mode."));

  QWhatsThis::add(mediaCombBox,
	i18n("Here you can change the paperformat."));

  QString copySpinBoxMsg = i18n("Here you can set, how many times you will print the files.");
  QWhatsThis::add(qtarch_ButtonGroup_7, copySpinBoxMsg);
  QWhatsThis::add(copySpinBox, copySpinBoxMsg);

  QWhatsThis::add(prettyPrintCheckBox,
	i18n("If you wish to print in pretty-print-mode then check this button."));

  QWhatsThis::add(prettyColorCheckBox,
	i18n("If you wish to print in pretty-print-color-mode,\n"
	     "then check this button."));

  QWhatsThis::add(printToFileButton,
	i18n("If you wish to print to file, then check this button."));

}

void CPrintDlg::slotProgramActivated(int i) {
  if (i==1) 
    {
      prettyPrintCheckBox->setEnabled(true);
      //      slotPrettyPrintClicked(false);
      formatCombBox->setEnabled(true);
      pageSide->setEnabled(true);
      qtarch_ButtonGroup_9->setEnabled(true);
      qtarch_ButtonGroup_11->setEnabled(true);
      qtarch_ButtonGroup_3->setEnabled(true);
      defaultCombBox->setEnabled(true);
      mediaCombBox->setEnabled(true);
      paperFormatGroup->setEnabled(true);
      pagePerSide->setEnabled(true);
      paperCombBox->setEnabled(true);
      qtarch_ButtonGroup_4->setEnabled(true);
      qtarch_ButtonGroup_10->setEnabled(true);
      qtarch_ButtonGroup_8->setEnabled(true);
      qtarch_ButtonGroup_35->setEnabled(true);
      printingConfButton->setEnabled(true);
      previewButton->setEnabled(true);
      printToFileDlg->setEnabled(printToFileButton->isChecked());
      printToFileButton->setEnabled(true);
      qtarch_ButtonGroup_6->setEnabled(true); 

      int j =defaultCombBox->count();
      int state=0;
      for (int a=0;a<j;a++) {
	if (!(strcmp(defaultCombBox->text(a),i18n("a2ps default")))) {
	  defaultCombBox->removeItem(a);
	  a--;
	  j--;
	}
	if (!(strcmp(defaultCombBox->text(a),i18n("lineprinter"))) ||
	    !(strcmp(defaultCombBox->text(a),i18n("enscript default")))) {
	  state++;
	}
      }
      if (state == 0) {
      defaultCombBox->insertItem(i18n("enscript default"));
      defaultCombBox->insertItem(i18n("lineprinter"));
      }
      mediaCombBox->clear();
      mediaCombBox->insertItem( "a4" );
      mediaCombBox->insertItem( "a3" );
      mediaCombBox->insertItem( "a5" );
      mediaCombBox->insertItem( "legal" );
      mediaCombBox->insertItem( "letter" );
      mediaCombBox->insertItem( "a4dj" );
      mediaCombBox->insertItem( "letterdj" );
    }
  else if (i==0) {
    
    prettyPrintCheckBox->setEnabled(false);
    formatCombBox->setEnabled(false);
    pageSide->setEnabled(false);
    qtarch_ButtonGroup_9->setEnabled(false);
    qtarch_ButtonGroup_11->setEnabled(false);
    qtarch_ButtonGroup_3->setEnabled(false);
    defaultCombBox->setEnabled(true);
    mediaCombBox->setEnabled(true);
    paperFormatGroup->setEnabled(true);
    pagePerSide->setEnabled(true);
    paperCombBox->setEnabled(true);
    qtarch_ButtonGroup_4->setEnabled(true);
    qtarch_ButtonGroup_10->setEnabled(true);
    qtarch_ButtonGroup_8->setEnabled(true);
    qtarch_ButtonGroup_35->setEnabled(true);
    printingConfButton->setEnabled(false);
    previewButton->setEnabled(true);
    printToFileDlg->setEnabled(printToFileButton->isChecked());
    printToFileButton->setEnabled(true);
    qtarch_ButtonGroup_6->setEnabled(true);
    
    prettyPrintCheckBox->setChecked(false);
    prettyCombBox->setEnabled(false);
    prettyColorCheckBox->setEnabled(false);
    prettyPrintCheckBox->setChecked(false);
    qtarch_ButtonGroup_34->setEnabled(false);
    //slotPrettyPrintClicked(false);
    int j =defaultCombBox->count();
    int state=0;
    for (int a=0;a<j;a++) {
      if (!(strcmp(defaultCombBox->text(a),i18n("lineprinter"))) ||
	  !(strcmp(defaultCombBox->text(a),i18n("enscript default")))) {
	defaultCombBox->removeItem(a);
	a--;
	j--;
      }
      if (!(strcmp(defaultCombBox->text(a),i18n("a2ps default"))) ||
	  !(strcmp(defaultCombBox->text(a),i18n("a2ps default")))) {
	state++;
      }
    }
    if (state == 0) {
      defaultCombBox->insertItem(i18n("a2ps default"));
    }
    mediaCombBox->clear();
    mediaCombBox->insertItem( "a4" );
    mediaCombBox->insertItem( "a3" );
    mediaCombBox->insertItem( "a5" );
    mediaCombBox->insertItem( "legal" );
    mediaCombBox->insertItem( "letter" );
  }
  else {
    
    prettyPrintCheckBox->setEnabled(false);
    formatCombBox->setEnabled(false);
    pageSide->setEnabled(false);
    qtarch_ButtonGroup_9->setEnabled(false);
    qtarch_ButtonGroup_11->setEnabled(false);
    qtarch_ButtonGroup_3->setEnabled(false);
    defaultCombBox->setEnabled(false);
    mediaCombBox->setEnabled(false);
    paperFormatGroup->setEnabled(false);
    pagePerSide->setEnabled(false);
    paperCombBox->setEnabled(false);
    qtarch_ButtonGroup_4->setEnabled(false);
    qtarch_ButtonGroup_10->setEnabled(false);
    qtarch_ButtonGroup_8->setEnabled(false);
    qtarch_ButtonGroup_35->setEnabled(false);
    printingConfButton->setEnabled(false);
    previewButton->setEnabled(false);
    printToFileDlg->setEnabled(printToFileButton->isChecked());
    printToFileButton->setEnabled(true);
    qtarch_ButtonGroup_6->setEnabled(true);
    
    prettyPrintCheckBox->setChecked(false);
    prettyCombBox->setEnabled(false);
    prettyColorCheckBox->setEnabled(false);
    prettyPrintCheckBox->setChecked(false);
    qtarch_ButtonGroup_34->setEnabled(false);
    //slotPrettyPrintClicked(false);
  }
}

void CPrintDlg::slotPrettyPrintClicked(bool status) {
  if (prettyPrintCheckBox->isEnabled()) {
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
  else {
    prettyPrintCheckBox->setChecked(false);
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

void CPrintDlg::slotCreateParameters() {
   QString parameters;
  parameters = "";
  string = "";
  if (programCombBox->currentItem()==0) {
    if (defaultCombBox->currentItem()==0) {
      if (strcmp (printerLine->text(),"default")) {
				parameters = (QString) "-P" + printerLine->text() + (QString) " ";
      }
      //parameters = parameters + globalpara;
      parameters = parameters + (QString) " -M" + mediaCombBox->text(mediaCombBox->currentItem());
      if (pagePerSide->currentItem()==0) {
	parameters.append(" -1");
      }
      else {
	parameters.append(" -2");
      }
      if (paperCombBox->currentItem()==0) {
	parameters.append(" -R");
      }
      else {
	parameters.append(" -r");
      }
    }
    else 
      if (defaultCombBox->currentItem()==1) {
	parameters = "";
      }
    string = parameters;
  }
  else if (programCombBox->currentItem()==1) {
    if (defaultCombBox->currentItem()==0) {
      if (strcmp (printerLine->text(),"default")) {
	parameters = (QString) "--printer=" + printerLine->text();
      }
      parameters.append ((QString)" " + globalpara);
      if (!strcmp(mediaCombBox->text(mediaCombBox->currentItem()),"a4")) {
	parameters.append (" --media=A4");
      }
      else if (!strcmp(mediaCombBox->text(mediaCombBox->currentItem()),"a3")) {
	parameters.append (" --media=A3");
      }
      else if (!strcmp(mediaCombBox->text(mediaCombBox->currentItem()),"a5")) {
	parameters.append (" --media=A5");
      }
      else if (!strcmp(mediaCombBox->text(mediaCombBox->currentItem()),"legal")) {
	parameters.append (" --media=Legal");
      }
      else if (!strcmp(mediaCombBox->text(mediaCombBox->currentItem()),"letter")) {
	parameters.append (" --media=Letter");
      }
      else if (!strcmp(mediaCombBox->text(mediaCombBox->currentItem()),"a4dj")) {
	parameters.append (" --media=A4dj");
      }
      else {
	parameters.append (" --media=Letterdj");
      }

      if (!strcmp(pageSide->currentText(),i18n ("all"))) {
      }
      else if (!strcmp(pageSide->currentText(),i18n ( "odd" ))) {
	parameters.append ((QString)" --pages=odd");
      }
      else if (!strcmp(pageSide->currentText(),i18n( "even" ))) {
	parameters.append ((QString)" --pages=even");
      }
      else {
	parameters.append ((QString)" --pages=" + pageSide->currentText());
      }
      if (prettyColorCheckBox->isChecked()) {
	parameters.append(" --color");
      }
      if (prettyPrintCheckBox->isChecked()) {
	if (prettyCombBox->currentItem()==0) {
	  parameters.append (" --pretty-print=ada");
	}
	else if (prettyCombBox->currentItem()==1) {
	  parameters.append (" --pretty-print=awk");
	}
	else if (prettyCombBox->currentItem()==2) {
	  parameters.append (" --pretty-print=c");
	}
	else if (prettyCombBox->currentItem()==3) {
	  parameters.append (" --pretty-print=changelog");
	}
	else if (prettyCombBox->currentItem()==4) {
	  parameters.append (" --pretty-print=cpp");
	}
	else if (prettyCombBox->currentItem()==5) {
	  parameters.append (" --pretty-print=diff");
	}
	else if (prettyCombBox->currentItem()==6) {
	  parameters.append (" --pretty-print=diffu");
	}
	else if (prettyCombBox->currentItem()==7) {
	  parameters.append (" --pretty-print=delphi");
	}
	else if (prettyCombBox->currentItem()==8) {
	  parameters.append (" --pretty-print=elisp");
	}
	else if (prettyCombBox->currentItem()==9) {
	  parameters.append (" --pretty-print=fortran");
	}
	else if (prettyCombBox->currentItem()==10) {
	  parameters.append (" --pretty-print=heskell");
	}
	else if (prettyCombBox->currentItem()==11) {
	  parameters.append (" --pretty-print=html");
	}
	else if (prettyCombBox->currentItem()==12) {
	  parameters.append (" --pretty-print=java");
	}
	else if (prettyCombBox->currentItem()==13) {
	  parameters.append (" --pretty-print=javascript");
	}
	else if (prettyCombBox->currentItem()==14) {
	  parameters.append (" --pretty-print=mail");
	}
	else if (prettyCombBox->currentItem()==15) {
	  parameters.append (" --pretty-print=makefile");
	}
	else if (prettyCombBox->currentItem()==16) {
	  parameters.append (" --pretty-print=nroff");
	}
	else if (prettyCombBox->currentItem()==17) {
	  parameters.append (" --pretty-print=objc");
	}
	else if (prettyCombBox->currentItem()==18) {
	  parameters.append (" --pretty-print=pascal");
	}
	else if (prettyCombBox->currentItem()==19) {
	  parameters.append (" --pretty-print=perl");
	}
	else if (prettyCombBox->currentItem()==20) {
	  parameters.append (" --pretty-print=postscript");
	}
	else if (prettyCombBox->currentItem()==21) {
	  parameters.append (" --pretty-print=scheme");
	}
	else if (prettyCombBox->currentItem()==22) {
	  parameters.append (" --pretty-print=sh");
	}
	else if (prettyCombBox->currentItem()==23) {
	  parameters.append (" --pretty-print=sql");
	}
	else if (prettyCombBox->currentItem()==24) {
	  parameters.append (" --pretty-print=states");
	}
	else if (prettyCombBox->currentItem()==25) {
	  parameters.append (" --pretty-print=synopsys");
	}
	else if (prettyCombBox->currentItem()==26) {
	  parameters.append (" --pretty-print=tcl");
	}
	else if (prettyCombBox->currentItem()==27) {
	  parameters.append (" --pretty-print=verilog");
	}
	else {
	  parameters.append (" --pretty-print=vhdl");
	}
      }
      if (pagePerSide->currentItem()==1 && paperCombBox->currentItem()==0) {
	parameters.append(" -r -U2");
      }
      else if (pagePerSide->currentItem()==1 && paperCombBox->currentItem()==1) {
	parameters.append(" -r -2");
      }
      else {
	parameters.append(" -1");
	if (paperCombBox->currentItem()==0) {
	  parameters.append(" -R");
	}
	else {
	  parameters.append(" -r");
	}
      }
      if (formatCombBox->currentItem()==0) {
	parameters.append(" --language=PostScript");
      }
      else {
	parameters.append(" --language=html");
      }
    }
    else 
      if (defaultCombBox->currentItem()==1) {
	parameters = "";
      }
      else {
	parameters = "-l";
      }
    string = parameters;
  }
  else {
    if (strcmp (printerLine->text(),"default")) {
      parameters = (QString) "-P" + printerLine->text() + (QString) " ";
    }
    string = parameters;
  }
}

void CPrintDlg::slotPreviewClicked() {
  if (!(lookProgram("gv") || lookProgram("ghostview") || lookProgram("kghostview"))) {
    KMessageBox::information(0,i18n("KDevelop needs \"gv\" or \"ghostview\" or \"kghostview\" to work properly.\n\t\t    Please install one!"),i18n("Program not found!"));
    return;
  }
  files = createFileString();
  if (!strcmp(files,"")) {
    KMessageBox::error(0,i18n("No file is printing!"),i18n("No File"));
    return;
  }
  else {
    QString dir,data1,data2,text;
    if ((programCombBox->currentItem()==1) && (formatCombBox->currentItem()==1)) {
      dir = locateLocal("appdata", "preview.html");
    }
    else {
      dir =  locateLocal("appdata", "preview.ps");
    }
    data1 = locate("appdata", "templates/preview1");
    data2 = locate("appdata", "templates/preview2");

    //process = new KShellProcess();
    if (programCombBox->currentItem()==1) {
      text = (QString) " --output="+ dir;
      settings = KGlobal::config();
      settings->setGroup("LastSettings");
      globalpara = settings->readEntry("EnscriptSettings");
      slotCreateParameters();
      print_process->clearArguments();
      *print_process << removeNullParams("enscript " + string + " " + text + " " + files);
    }
    else {
      //settings = KGlobal::config();
      //settings->setGroup("LastSettings");
      //  globalpara = settings->readEntry("A2psSettings");
      slotCreateParameters();
      print_process->clearArguments();
      *print_process << removeNullParams("a2ps " + string + " -o " + dir + " " + files);
    }
    print_process->start(KProcess::Block,KProcess::AllOutput);
    //delete (process);
    //process2 = new KShellProcess();
    preview_process->clearArguments();
    if ((programCombBox->currentItem()==1) && (formatCombBox->currentItem()==1)) {
      *preview_process << "kdehelp";
      *preview_process << dir;
      preview_process->start(KProcess::NotifyOnExit,KProcess::AllOutput);
      return;
    }
    else {
      /* NOTE: gv and ghostview will get removed soon, they are
       * obsolete and can be replace by an embedded kghostview part
       * (rokrau)
       */
      if (lookProgram("gv")) {
	*preview_process << "gv";
	*preview_process << dir;
	preview_process->start(KProcess::NotifyOnExit,KProcess::AllOutput);
	return;
      }
      else if (lookProgram("ghostview")) {
	*preview_process << "ghostview";
	*preview_process << dir;
	preview_process->start(KProcess::NotifyOnExit,KProcess::AllOutput);
	return;
      }
      else if (lookProgram("kghostview")) {
	*preview_process << "kghostview";
	*preview_process << dir;
	preview_process->start(KProcess::NotifyOnExit,KProcess::AllOutput);
	return;
      }
    }
  }
}

void CPrintDlg::slotFilesConfClicked() {
  CFilePrintDlg *fileconf = new CFilePrintDlg(this, "filedialog");
  fileconf->setCaption("FilePrintDialog");
  fileconf->exec(); 
  delete (fileconf);
}

void CPrintDlg::slotPrintingConfClicked() {
  int prog=programCombBox->currentItem();
  if (prog==0) {
    if (!CToolClass::searchProgram("a2ps")) {
      return;
    }
    a2psconf = new CConfigA2psDlg(this, "confdialog");
    a2psconf->resize(600,430);
    a2psconf->setCaption("A2ps Configdialog");
    a2psconf->exec();
    settings = KGlobal::config();
  settings->setGroup("LastSettings");
  globalpara = settings->readEntry("A2psSettings");
  delete (a2psconf);
  }
  else
    if (prog==1) {
      if (!CToolClass::searchProgram("enscript")) {
        return;
      }
      enscriptconf = new CConfigEnscriptDlg(this, "confdialog");
      enscriptconf->resize(610,510);
      enscriptconf->setCaption("Enscript Configdialog");
      enscriptconf->exec();
      settings = KGlobal::config();
      settings->setGroup("LastSettings");
      globalpara = settings->readEntry("EnscriptSettings");
      delete (enscriptconf);
    }
}

void CPrintDlg::slotPrintToFileDlgClicked() {
  printToFileLine->setText(KFileDialog::getOpenFileName());
}

//bool CPrintDlg::lookProgram(QString name) {
//
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

void CPrintDlg::slotOkClicked() {

    debug ("in CPrintDlg::slotOkClicked");

  settings = KGlobal::config();
  settings->setGroup("PrintDialog");
  settings->writeEntry("Program",programCombBox->currentItem());
  settings->writeEntry("Printer",printerLine->text());
  settings->writeEntry("PagePrinting",pagePerSide->currentItem());
  settings->writeEntry("Pages",pageSide->currentItem());
  settings->writeEntry("PrettyMode",prettyCombBox->currentItem());
  settings->writeEntry("PaperFormat",formatCombBox->currentItem());
  settings->writeEntry("Copy",copySpinBox->value());
  settings->writeEntry("OutputMedia",mediaCombBox->currentItem());
  settings->writeEntry("OutputFormat",formatCombBox->currentItem());
  settings->writeEntry("Default",defaultCombBox->currentItem());
  settings->writeEntry("PrettyPrintButton",prettyPrintCheckBox->isChecked());
  settings->writeEntry("PrettyColor",prettyColorCheckBox->isChecked());
  settings->writeEntry("PrintToFileButton",printToFileButton->isChecked());
  settings->writeEntry("PrintToFileLine",printToFileLine->text());
  settings->sync();

  if (!(lookProgram("gv") || lookProgram("ghostview") || lookProgram("kghostview"))) {
    KMessageBox::error(0,i18n("KDevelop needs \"gv\" or \"ghostview\" or \"kghostview\" to work properly.\n\t\t    Please install one!"),
                        i18n("Program not found!"));
    return;
  }
  if (printToFileButton->isChecked()) {
    QString filetext = printToFileLine->text();
    if (!strcmp(filetext.right(1),"/") || !strcmp(printToFileLine->text(),"")) {
      KMessageBox::error(0,i18n("You need a filename.\nPlease enter one!"),
                            i18n("No Filename"));
      return;
    }
  }
  files = createFileString();
  if (!strcmp(files,"")) {
    KMessageBox::error(0,i18n("No file is printing!"),i18n("No File"));
    return;
  }
  else {
    QString dir="";
    QString data1,data2;
    QString text="";
    //process = new KShellProcess();
    if (programCombBox->currentItem()==1) {
      if (printToFileButton->isChecked()) {
	dir =  printToFileLine->text();
	text = (QString) " --output="+ dir;
	settings = KGlobal::config();
	settings->setGroup("LastSettings");
	globalpara = settings->readEntry("EnscriptSettings");
	slotCreateParameters();
	print_process->clearArguments();
//	*print_process << removeNullParams("enscript " + string + text + " " + files);
	*print_process << "enscript" ;
	*print_process << string ;
	*print_process << text ;
	*print_process << files ;

        QString dbg_string1 = "print command: enscript " + string + text + files ;
        debug(dbg_string1);
	bool success = print_process->start(KProcess::Block,KProcess::AllOutput);
        QString dbg_string2 = "printing ";
        if (success) dbg_string2 += "was succesful";
        else dbg_string2 += "has failed";
        debug(dbg_string2);
      }
      else {
	settings = KGlobal::config();
	settings->setGroup("LastSettings");
	globalpara = settings->readEntry("EnscriptSettings");
	slotCreateParameters();
	for (int i=0;i<((QString) copySpinBox->text()).toInt();i++) {
	  print_process->clearArguments();
	  *print_process << removeNullParams("enscript " + string + " " + files);
	  print_process->start(KProcess::Block,KProcess::AllOutput);
	}
      }
    }
    else if (programCombBox->currentItem()==0) {
      //settings = KGlobal::config();
      //settings->setGroup("LastSettings");
      //      globalpara = settings->readEntry("A2psSettings");
//      kdDebug() << string << endl;
      string = "";
      slotCreateParameters();
      if (printToFileButton->isChecked()) {
	dir =  printToFileLine->text();
	print_process->clearArguments();
//	cerr << "a2ps " + string + " -o " + dir + " " + files << endl;
	*print_process << removeNullParams("a2ps " + string + " -o " + dir + " " + files);
	print_process->start(KProcess::Block,KProcess::AllOutput);
      }
      else {
	for (int i=0;i<((QString) copySpinBox->text()).toInt();i++) {
	  print_process->clearArguments();
	  *print_process << removeNullParams("a2ps " + string + " " + files);
	  print_process->start(KProcess::Block,KProcess::AllOutput);
	}
      }
    }
    else {
      if (printToFileButton->isChecked()) {
	dir =  printToFileLine->text();
	print_process->clearArguments();
	*print_process << removeNullParams("lpr " + string + " " + files + " > " + dir);
//	cerr << "lpr " + string + " " + files + " > " + dir << endl;
	print_process->start(KProcess::Block,KProcess::AllOutput);
      }
      else {
      	for (int i=0;i<((QString) copySpinBox->text()).toInt();i++) {
	  print_process->clearArguments();
	  *print_process << removeNullParams("lpr " + string + " " + files);
//	  cerr << "lpr " + string + " " + files << endl;
	  print_process->start(KProcess::Block,KProcess::AllOutput);
	}
      }	
    }
    //delete (process);
    reject();
  }
}

QString CPrintDlg::createFileString() {
  globalpara = "";
  QString str = "";
  QString sources = "";
  QString dir = "";
  QString underdir = "";
  QString prj_str2 = "";
  QString test="";
  QString test2 = "";
  QStrList filelist;
  settings = KGlobal::config();
  settings->setGroup("LastSettings");
  globalpara = settings->readEntry("FileSettings");
  settings->setGroup("Files");
  prj_str2 = settings->readEntry("project_file");
  CProject *project = new CProject(prj_str2);
  project->readProject();
  prj_str2.truncate(prj_str2.findRev("/"));
  test = prj_str2;
  test2 = prj_str2;
  test.truncate(test.findRev("/"));
  underdir = test2.remove(0,test.length());
  if (!strcmp(globalpara,"current")) {
    delete (project);
    return oldfiles;
  }
  else if (!strcmp(globalpara,"cppFiles")) {
    filelist = project->getSources();
    for(str= filelist.first();str !=0;str = filelist.next()){
      sources =  str + " " + sources ;
    }
    delete (project);
    return sources;
  }
  else if (!strcmp(globalpara,"allFiles")) {
    project->getAllFiles(filelist);
    for(str= filelist.first();str !=0;str = filelist.next()){
      sources =  prj_str2 + "/" + str + " " + sources ;
    }
    delete (project);
    return sources;
  }
  else if (!strcmp(globalpara,"headerFiles")) {
    filelist = project->getHeaders();
    for(str= filelist.first();str !=0;str = filelist.next()){
      sources =  str + " " + sources ;
    }
    delete (project);
    return sources;
  } 
  else {
    delete (project);
    return globalpara;
  } 
}

void CPrintDlg::loadSettings() {
  settings = KGlobal::config();
  settings->setGroup("PrintDialog");
  programCombBox->setCurrentItem(settings->readNumEntry("Program"));
  if (programCombBox->currentItem()==0) {
    slotProgramActivated(0);
  }
  else if (programCombBox->currentItem()==1) {
    slotProgramActivated(1);
  }
  else {
    slotProgramActivated(2);
  }
  printerLine->setText(settings->readEntry("Printer"));
  printToFileButton->setChecked(settings->readBoolEntry("PrintToFileButton"));
  if (printToFileButton->isChecked()) {
    slotPrintToFileClicked(true);
  }
  else {
    slotPrintToFileClicked(false);
  }
  printToFileLine->setText(settings->readEntry("PrintToFileLine"));
  pagePerSide->setCurrentItem(settings->readNumEntry("PagePrinting"));
  pageSide->setCurrentItem(settings->readNumEntry("Pages"));
  prettyColorCheckBox->setChecked(settings->readBoolEntry("PrettyColor"));
  prettyPrintCheckBox->setChecked(settings->readBoolEntry("PrettyPrintButton"));
  if (prettyPrintCheckBox->isChecked()) {
    slotPrettyPrintClicked(true);
  }
  else {
    slotPrettyPrintClicked(false);
  }
  prettyCombBox->setCurrentItem(settings->readNumEntry("PrettyMode"));
  formatCombBox->setCurrentItem(settings->readNumEntry("PaperFormat"));
  copySpinBox->setValue(settings->readNumEntry("Copy"));
  mediaCombBox->setCurrentItem(settings->readNumEntry("OutputMedia"));
  formatCombBox->setCurrentItem(settings->readNumEntry("OutputFormat"));
  defaultCombBox->setCurrentItem(settings->readNumEntry("Default"));
}

#include "cprintdlg.moc"
