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

#include <klocale.h>

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgrid.h>
#include <kbuttonbox.h>


CGroupPropertiesDlg::CGroupPropertiesDlg(QWidget *parent, const char *name ) : QDialog(parent,name,true) 
{
  QGridLayout *grid1 = new QGridLayout(this,3,2,15,7);  
  name_edit = new QLineEdit( this, "name_edit" );
  name_edit->setFocusPolicy( QWidget::StrongFocus );
  name_edit->setBackgroundMode( QWidget::PaletteBase );
  name_edit->setText( "" );
  name_edit->setMaxLength( 32767 );
  name_edit->setEchoMode( QLineEdit::Normal );
  name_edit->setFrame( TRUE );
  grid1->addWidget(name_edit,0,1);
  
  
  filters_edit = new QLineEdit( this, "filters_edit" );
  filters_edit->setFocusPolicy( QWidget::StrongFocus );
  filters_edit->setBackgroundMode( QWidget::PaletteBase );
  filters_edit->setText( "" );
  filters_edit->setMaxLength( 32767 );
  filters_edit->setEchoMode( QLineEdit::Normal );
  filters_edit->setFrame( TRUE );
  grid1->addWidget(filters_edit,1,1);
  
  
  name_label = new QLabel( this, "name_label" );
  name_label->setFocusPolicy( QWidget::NoFocus );
  name_label->setBackgroundMode( QWidget::PaletteBackground );
  name_label->setText(i18n("Groupname:") );
  name_label->setAlignment( 289 );
  name_label->setMargin( -1 );
  grid1->addWidget(name_label,0,0);

  
  filter_label = new QLabel( this, "filer_label" );
  filter_label->setFocusPolicy( QWidget::NoFocus );
  filter_label->setBackgroundMode( QWidget::PaletteBackground );
  filter_label->setText(i18n("Filters:") );
  filter_label->setAlignment( 289 );
  filter_label->setMargin( -1 );
  grid1->addWidget(filter_label,1,0);
  

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  ok_button  = bb->addButton( i18n("OK") );
  ok_button->setFocusPolicy( QWidget::TabFocus );
  ok_button->setBackgroundMode( QWidget::PaletteBackground );
  ok_button->setAutoRepeat( FALSE );
  ok_button->setAutoResize( FALSE );
  ok_button->setDefault( TRUE );
  cancel_button = bb->addButton( i18n( "Close" ) );
  cancel_button->setFocusPolicy( QWidget::TabFocus );
  cancel_button->setBackgroundMode( QWidget::PaletteBackground );
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  bb->layout();
  grid1->addWidget(bb,2,1);
 
  connect(ok_button,SIGNAL(clicked()),SLOT(slotOK()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
}
CGroupPropertiesDlg::~CGroupPropertiesDlg(){
}
void CGroupPropertiesDlg::slotOK(){
  accept();
}



#include "cgrouppropertiesdlg.moc"
