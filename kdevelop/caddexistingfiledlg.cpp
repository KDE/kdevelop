/***************************************************************************
                          caddexistingfiledlg.cpp  -                              
                             -------------------                                         

    begin                : Tue Oct 20 1998                                           
    copyright            : (C) 1998 by Sandy Meier                         
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

#include <qdir.h>
#include "caddexistingfiledlg.h"
#include <kfiledialog.h>
#include <kmsgbox.h>
#include <kapp.h>
#include <qfileinfo.h>
#include <iostream.h>
#include <qfiledialog.h>

CAddExistingFileDlg::CAddExistingFileDlg(QWidget *parent, const char *name,CProject* p_prj ) : QDialog(parent,name,true) {

  prj = p_prj;
  setCaption(i18n("Add existing files to project"));

  source_label = new QLabel( this, "source_label" );
  source_label->setGeometry( 20, 30, 90, 30 );
  source_label->setMinimumSize( 0, 0 );
  source_label->setMaximumSize( 32767, 32767 );
  source_label->setFocusPolicy( QWidget::NoFocus );
  source_label->setBackgroundMode( QWidget::PaletteBackground );
  source_label->setFontPropagation( QWidget::NoChildren );
  source_label->setPalettePropagation( QWidget::NoChildren );
  source_label->setText(i18n("Sourcefile(s):") );
  source_label->setAlignment( 289 );
  source_label->setMargin( -1 );

  source_edit = new QLineEdit( this, "source_edit" );
  source_edit->setGeometry( 120, 30, 230, 30 );
  source_edit->setMinimumSize( 0, 0 );
  source_edit->setMaximumSize( 32767, 32767 );
  source_edit->setFocusPolicy( QWidget::StrongFocus );
  source_edit->setBackgroundMode( QWidget::PaletteBase );
  source_edit->setFontPropagation( QWidget::NoChildren );
  source_edit->setPalettePropagation( QWidget::NoChildren );
  source_edit->setText( "" );
  source_edit->setMaxLength( 32767 );
  source_edit->setEchoMode( QLineEdit::Normal );
  source_edit->setFrame( TRUE );

  source_button = new QPushButton( this, "source_button" );
  source_button->setGeometry( 360, 30, 30, 30 );
  source_button->setMinimumSize( 0, 0 );
  source_button->setMaximumSize( 32767, 32767 );
  source_button->setFocusPolicy( QWidget::TabFocus );
  source_button->setBackgroundMode( QWidget::PaletteBackground );
  source_button->setFontPropagation( QWidget::NoChildren );
  source_button->setPalettePropagation( QWidget::NoChildren );
	QPixmap pix;
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  source_button->setPixmap(pix);
  source_button->setAutoRepeat( FALSE );
  source_button->setAutoResize( FALSE );

  destination_label = new QLabel( this, "destination_label" );
  destination_label->setGeometry( 20, 80, 90, 30 );
  destination_label->setMinimumSize( 0, 0 );
  destination_label->setMaximumSize( 32767, 32767 );
  destination_label->setFocusPolicy( QWidget::NoFocus );
  destination_label->setBackgroundMode( QWidget::PaletteBackground );
  destination_label->setFontPropagation( QWidget::NoChildren );
  destination_label->setPalettePropagation( QWidget::NoChildren );
  destination_label->setText(i18n("Destinationdir:") );
  destination_label->setAlignment( 289 );
  destination_label->setMargin( -1 );

  destination_edit = new QLineEdit( this, "destination_edit" );
  destination_edit->setGeometry( 120, 80, 230, 30 );
  destination_edit->setMinimumSize( 0, 0 );
  destination_edit->setMaximumSize( 32767, 32767 );
  destination_edit->setFocusPolicy( QWidget::StrongFocus );
  destination_edit->setBackgroundMode( QWidget::PaletteBase );
  destination_edit->setFontPropagation( QWidget::NoChildren );
  destination_edit->setPalettePropagation( QWidget::NoChildren );
  destination_edit->setText( "" );
  destination_edit->setMaxLength( 32767 );
  destination_edit->setEchoMode( QLineEdit::Normal );
  destination_edit->setFrame( TRUE );
  	
  destination_button = new QPushButton( this, "destination_button" );
  destination_button->setGeometry( 360, 80, 30, 30 );
  destination_button->setMinimumSize( 0, 0 );
  destination_button->setMaximumSize( 32767, 32767 );
  destination_button->setFocusPolicy( QWidget::TabFocus );
  destination_button->setBackgroundMode( QWidget::PaletteBackground );
  destination_button->setFontPropagation( QWidget::NoChildren );
  destination_button->setPalettePropagation( QWidget::NoChildren );
  destination_button->setPixmap(pix);
  destination_button->setAutoRepeat( FALSE );
  destination_button->setAutoResize( FALSE );
  

  KQuickHelp::add(source_edit,
  KQuickHelp::add(source_label,
  KQuickHelp::add(source_button, i18n("Select the source files to be added\n"
				"to the project here."))));

  KQuickHelp::add(destination_edit,
  KQuickHelp::add(destination_label,
  KQuickHelp::add(destination_button, i18n("select the directory where the new\n"
					"source files will be copied to here."))));

  ok_button = new QPushButton( this, "ok_button" );
  ok_button->setGeometry( 90, 130, 100, 25 );
  ok_button->setMinimumSize( 0, 0 );
  ok_button->setMaximumSize( 32767, 32767 );
  ok_button->setFocusPolicy( QWidget::TabFocus );
  ok_button->setBackgroundMode( QWidget::PaletteBackground );
  ok_button->setFontPropagation( QWidget::NoChildren );
  ok_button->setPalettePropagation( QWidget::NoChildren );
  ok_button->setText( i18n("OK") );
  ok_button->setAutoRepeat( FALSE );
  ok_button->setAutoResize( FALSE );
	ok_button->setDefault( TRUE );
	
  cancel_button = new QPushButton( this, "cancel_button" );
  cancel_button->setGeometry( 220, 130, 100, 25 );
  cancel_button->setMinimumSize( 0, 0 );
  cancel_button->setMaximumSize( 32767, 32767 );
  cancel_button->setFocusPolicy( QWidget::TabFocus );
  cancel_button->setBackgroundMode( QWidget::PaletteBackground );
  cancel_button->setFontPropagation( QWidget::NoChildren );
  cancel_button->setPalettePropagation( QWidget::NoChildren );
  cancel_button->setText(i18n("Cancel") );
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  
  connect(source_button,SIGNAL(clicked()),SLOT(sourceButtonClicked()));
  connect(destination_button,SIGNAL(clicked()),SLOT(destinationButtonClicked()));
  connect(ok_button,SIGNAL(clicked()),SLOT(OK()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
  resize( 410,175 );
  setMinimumSize( 0, 0 );
  setMaximumSize( 32767, 32767 );
  
}
CAddExistingFileDlg::~CAddExistingFileDlg(){
}
void CAddExistingFileDlg::sourceButtonClicked(){
 
  QStrList files( QFileDialog::getOpenFileNames(0,QDir::homeDirPath(),this,i18n("Source File(s)...")) );
  files.setAutoDelete(true);

  QString comp_str;
  if(!files.isEmpty()){
    for(QString str=files.first();str != 0;str = files.next()){
      comp_str = comp_str + str + ",";
    }
  }
  source_edit->setText(comp_str);
  files.clear();

}
void CAddExistingFileDlg::destinationButtonClicked(){
 QString name=KDirDialog::getDirectory(destination_edit->text(),this,i18n("Destination Directory"));
 if(!name.isEmpty()){
    destination_edit->setText(name);
  }
}
void CAddExistingFileDlg::OK(){
  QFileInfo file_info(source_edit->text());
  QDir dir(destination_edit->text());
  //   QString source_name = file_info.fileName();
  QString dest_name = destination_edit->text();// + source_name

  // if (!file_info.exists()){
//     KMsgBox::message(this,i18n("Error..."),i18n("You must choose an existing sourcefile!")
// 		     ,KMsgBox::EXCLAMATION);
//     return;
//   }
  if(dest_name.contains(prj->getProjectDir()) == 0 ){
    KMsgBox::message(this,i18n("Error..."),
		     i18n("You must choose a destination,that is in your project-dir!")
		     ,KMsgBox::EXCLAMATION);
    return;
  }
  if(!dir.exists()){
    KMsgBox::message(this,i18n("Error..."),
		     i18n("You must choose a valid dir as a destination!")
		     ,KMsgBox::EXCLAMATION);
    return;
  }
  
  accept();
}








