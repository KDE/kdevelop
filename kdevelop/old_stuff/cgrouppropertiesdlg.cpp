/***************************************************************************
                          cgrouppropertiesdlg.cpp  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sun Nov 1 1998                                           
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


#include "cgrouppropertiesdlg.h"
#include <kapp.h>
#include <klocale.h>

CGroupPropertiesDlg::CGroupPropertiesDlg(QWidget *parent, const char *name ) : QDialog(parent,name,true) {
  
  name_edit = new QLineEdit( this, "name_edit" );
  name_edit->setGeometry( 110, 20, 210, 30 );
  name_edit->setMinimumSize( 0, 0 );
  name_edit->setMaximumSize( 32767, 32767 );
  name_edit->setFocusPolicy( QWidget::StrongFocus );
  name_edit->setBackgroundMode( QWidget::PaletteBase );
  name_edit->setFontPropagation( QWidget::NoChildren );
  name_edit->setPalettePropagation( QWidget::NoChildren );
  name_edit->setText( "" );
  name_edit->setMaxLength( 32767 );
  name_edit->setEchoMode( QLineEdit::Normal );
  name_edit->setFrame( TRUE );
  
  
  filters_edit = new QLineEdit( this, "filters_edit" );
  filters_edit->setGeometry( 110, 70, 210, 30 );
  filters_edit->setMinimumSize( 0, 0 );
  filters_edit->setMaximumSize( 32767, 32767 );
  filters_edit->setFocusPolicy( QWidget::StrongFocus );
  filters_edit->setBackgroundMode( QWidget::PaletteBase );
  filters_edit->setFontPropagation( QWidget::NoChildren );
  filters_edit->setPalettePropagation( QWidget::NoChildren );
  filters_edit->setText( "" );
  filters_edit->setMaxLength( 32767 );
  filters_edit->setEchoMode( QLineEdit::Normal );
  filters_edit->setFrame( TRUE );
  
  
  name_label = new QLabel( this, "name_label" );
  name_label->setGeometry( 10, 20, 100, 30 );
  name_label->setMinimumSize( 0, 0 );
  name_label->setMaximumSize( 32767, 32767 );
  name_label->setFocusPolicy( QWidget::NoFocus );
  name_label->setBackgroundMode( QWidget::PaletteBackground );
  name_label->setFontPropagation( QWidget::NoChildren );
  name_label->setPalettePropagation( QWidget::NoChildren );
  name_label->setText(i18n("Groupname:") );
  name_label->setAlignment( 289 );
  name_label->setMargin( -1 );

  
  filter_label = new QLabel( this, "filer_label" );
  filter_label->setGeometry( 10, 70, 100, 30 );
  filter_label->setMinimumSize( 0, 0 );
  filter_label->setMaximumSize( 32767, 32767 );
  filter_label->setFocusPolicy( QWidget::NoFocus );
  filter_label->setBackgroundMode( QWidget::PaletteBackground );
  filter_label->setFontPropagation( QWidget::NoChildren );
  filter_label->setPalettePropagation( QWidget::NoChildren );
  filter_label->setText(i18n("Filters:") );
  filter_label->setAlignment( 289 );
  filter_label->setMargin( -1 );
  
 
  ok_button = new QPushButton( this, "ok_button" );
  ok_button->setGeometry( 60, 120, 100, 30 );
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
  
  cancel_button = new QPushButton( this, "chancel_button" );
  cancel_button->setGeometry( 190, 120, 100, 30 );
  cancel_button->setMinimumSize( 0, 0 );
  cancel_button->setMaximumSize( 32767, 32767 );
  cancel_button->setFocusPolicy( QWidget::TabFocus );
  cancel_button->setBackgroundMode( QWidget::PaletteBackground );
  cancel_button->setFontPropagation( QWidget::NoChildren );
  cancel_button->setPalettePropagation( QWidget::NoChildren );
  cancel_button->setText(i18n("Cancel") );
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  
  resize( 340,170 );
  setMinimumSize( 0, 0 );
  setMaximumSize( 32767, 32767 );
  connect(ok_button,SIGNAL(clicked()),SLOT(slotOK()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
}
CGroupPropertiesDlg::~CGroupPropertiesDlg(){
}
void CGroupPropertiesDlg::slotOK(){
  accept();
}



#include "cgrouppropertiesdlg.moc"
