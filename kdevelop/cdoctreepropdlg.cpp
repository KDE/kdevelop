/***************************************************************************
                          cdoctreepropdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Feb 3 1999                                           
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


#include "cdoctreepropdlg.h"

#include <kfiledialog.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>

CDocTreePropDlg::CDocTreePropDlg(QWidget *parent, const char *name ) : QDialog(parent,name,true) {
  name_edit = new QLineEdit( this, "name_edit" );
  name_edit->setGeometry( 100, 20, 240, 30 );
  name_edit->setMinimumSize( 0, 0 );
  name_edit->setMaximumSize( 32767, 32767 );
  name_edit->setFocusPolicy( QWidget::StrongFocus );
  name_edit->setBackgroundMode( QWidget::PaletteBase );
  name_edit->setText( "" );
  name_edit->setMaxLength( 32767 );
  name_edit->setEchoMode( QLineEdit::Normal );
  name_edit->setFrame( TRUE );
  
  file_edit = new QLineEdit( this, "file_edit" );
  file_edit->setGeometry( 100, 70, 240, 30 );
  file_edit->setMinimumSize( 0, 0 );
  file_edit->setMaximumSize( 32767, 32767 );
  file_edit->setFocusPolicy( QWidget::StrongFocus );
  file_edit->setBackgroundMode( QWidget::PaletteBase );
  file_edit->setText( "" );
  file_edit->setMaxLength( 32767 );
  file_edit->setEchoMode( QLineEdit::Normal );
  file_edit->setFrame( TRUE );
  
  name_label = new QLabel( this, "name_label" );
  name_label->setGeometry( 10, 20, 90, 30 );
  name_label->setMinimumSize( 0, 0 );
  name_label->setMaximumSize( 32767, 32767 );
  name_label->setFocusPolicy( QWidget::NoFocus );
  name_label->setBackgroundMode( QWidget::PaletteBackground );
  name_label->setText(i18n("Name:") );
  name_label->setAlignment( 289 );
  name_label->setMargin( -1 );
  QWhatsThis::add(name_label, i18n("Enter the name of the entry here."));
  QWhatsThis::add(name_edit,i18n("Enter the name of the entry here."));
  
  file_label = new QLabel( this, "file_label" );
  file_label->setGeometry( 10, 70, 90, 30 );
  file_label->setMinimumSize( 0, 0 );
  file_label->setMaximumSize( 32767, 32767 );
  file_label->setFocusPolicy( QWidget::NoFocus );
  file_label->setBackgroundMode( QWidget::PaletteBackground );
  file_label->setText(i18n("File:") );
  file_label->setAlignment( 289 );
  file_label->setMargin( -1 );
  QWhatsThis::add(file_label, i18n("Enter the file of the entry here."));
  QWhatsThis::add(file_edit,i18n("Enter the file of the entry here."));
  
  ok_button = new QPushButton( this, "ok_button" );
  ok_button->setGeometry( 100, 120, 100, 30 );
  ok_button->setMinimumSize( 0, 0 );
  ok_button->setMaximumSize( 32767, 32767 );
  ok_button->setFocusPolicy( QWidget::TabFocus );
  ok_button->setBackgroundMode( QWidget::PaletteBackground );
  ok_button->setText( i18n("OK") );
  ok_button->setAutoRepeat( FALSE );
  ok_button->setAutoResize( FALSE );
	ok_button->setDefault(true);
  
  cancel_button = new QPushButton( this, "cancel_button" );
  cancel_button->setGeometry( 230, 120, 100, 30 );
  cancel_button->setMinimumSize( 0, 0 );
  cancel_button->setMaximumSize( 32767, 32767 );
  cancel_button->setFocusPolicy( QWidget::TabFocus );
  cancel_button->setBackgroundMode( QWidget::PaletteBackground );
  cancel_button->setText( i18n("Cancel") );
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  
  file_button = new QPushButton( this, "file_button" );
  file_button->setGeometry( 360, 70, 30, 30 );
  file_button->setMinimumSize( 0, 0 );
  file_button->setMaximumSize( 32767, 32767 );
  file_button->setFocusPolicy( QWidget::TabFocus );
  file_button->setBackgroundMode( QWidget::PaletteBackground );
	QPixmap pix = SmallIcon("fileopen");
  file_button->setPixmap(pix);
  file_button->setAutoRepeat( FALSE );
  file_button->setAutoResize( FALSE );
  QWhatsThis::add(file_button,i18n("Here you can browse through the disc to select a file for the entry."));
  
  setMinimumSize( 0, 0 );
  setMaximumSize( 32767, 32767 );
  resize( 410,170 );

  connect(file_button,SIGNAL(clicked()),SLOT(slotFileButtonClicked()));
  connect(ok_button,SIGNAL(clicked()),SLOT(accept()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
}
CDocTreePropDlg::~CDocTreePropDlg(){
}

void CDocTreePropDlg::slotFileButtonClicked(){
  QString str=  KFileDialog::getOpenFileName(QString::null,"*.html *.htm");
  if(!str.isEmpty()){
    file_edit->setText(str);
  }
}





#include "cdoctreepropdlg.moc"
