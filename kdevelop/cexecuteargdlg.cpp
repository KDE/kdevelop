/***************************************************************************
                          cexecuteargdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Tue Mar 30 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kapp.h>

#include "cexecuteargdlg.h"

CExecuteArgDlg::CExecuteArgDlg(QWidget *parent, const char *name, CProject* p_prj ) : QDialog(parent,name,true) {
  prj = p_prj;
  resize(0,0);
  setCaption(i18n("Execute with Arguments"));
  QVBoxLayout *vl = new QVBoxLayout(this,10);
  
  QHBoxLayout *hl = new QHBoxLayout( 15 );
  vl->addLayout( hl );
  
  QLabel *label = new QLabel(this,"label" );
  label->setText(i18n("Arguments: " ));
  label->setFixedSize( label->sizeHint() );
  
  edit = new QLineEdit( this );
  edit->setFixedSize(200,edit->sizeHint().height() );
  edit->setFocus();
  text=prj->getExecuteArgs();
  edit->setText(text);
  connect( edit, SIGNAL( textChanged( const char * ) ),
	   SLOT( slotTextChanged( const char * ) ) );
  
  hl->addWidget( label );
  hl->addWidget( edit );
  
  hl = new QHBoxLayout;
  vl->addLayout( hl );
  
  hl->addStretch();
  
  hl = new QHBoxLayout( 15 );
  vl->addLayout( hl );
  
  QPushButton *btn = new QPushButton(this,"Clear Arguments");
  btn->setText(i18n("Clear Arguments"));
  btn->setFixedSize( btn->sizeHint() );
  connect( btn, SIGNAL( clicked() ), this, SLOT(slotClose() ) );
  hl->addWidget( btn );
  
  btn = new QPushButton(this,"OK");
  btn->setText(i18n("OK"));
  btn->setFixedSize( btn->sizeHint() );
  btn->setDefault( true );
  connect( btn, SIGNAL( clicked() ), this, SLOT( slotSetArg() ) );
  hl->addWidget( btn );
  
  btn = new QPushButton(this,"Cancel");
  btn->setText(i18n("Cancel"));
  btn->setFixedSize( btn->sizeHint() );
  connect( btn, SIGNAL( clicked() ), this, SLOT(reject() ) );
  hl->addWidget( btn );
  
  hl->addStretch();
  
  vl->activate();
}
CExecuteArgDlg::~CExecuteArgDlg(){
}

void CExecuteArgDlg::slotSetArg(){
  prj->setExecuteArgs(text);		
  prj->writeProject();
  accept();
}

void CExecuteArgDlg::slotTextChanged( const char *t )
{
  text = t;
}

void CExecuteArgDlg::slotClose(){
  edit->clear();
}










