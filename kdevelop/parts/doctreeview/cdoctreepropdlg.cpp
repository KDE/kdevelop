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


#include <qwhatsthis.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>
#include "cdoctreepropdlg.h"


CDocTreePropDlg::CDocTreePropDlg(QWidget *parent, const char *name ) : QDialog(parent,name,true) {
  QVBoxLayout *layout = new QVBoxLayout(this, 10);
  QGridLayout *grid = new QGridLayout(2, 3);
  layout->addLayout(grid);
  
  QLabel *name_label = new QLabel( i18n("Name:"), this, "name_label" );
  grid->addWidget(name_label, 0, 0);

  name_edit = new QLineEdit( this, "name_edit" );
  grid->addWidget(name_edit, 0, 1);

  QLabel *file_label = new QLabel( i18n("File:"), this, "file_label" );
  grid->addWidget(file_label, 1, 0);

  file_edit = new QLineEdit( this, "file_edit" );
  grid->addWidget(file_edit, 1, 1);

  QPushButton *file_button = new QPushButton( this, "file_button" );
  file_button->setPixmap(BarIcon("open"));
  file_button->setFixedWidth(file_button->sizeHint().height());
  grid->addWidget(file_button, 1, 2);

  QString text;
  text = i18n("Enter the name of the entry here.");
  QWhatsThis::add(name_label, text);
  QWhatsThis::add(name_edit, text);
  
  text = i18n("Enter the file of the entry here.");
  QWhatsThis::add(file_label, text);
  QWhatsThis::add(file_edit, text);
  
  QWhatsThis::add(file_button, i18n("Here you can browse through the disk"
                                    "to select a file for the entry."));

  KSeparator *sep = new KSeparator(this);
  layout->addWidget(sep);

  QHBoxLayout *box = new QHBoxLayout(6);
  layout->addLayout(box);
  
  QPushButton *ok_button = new QPushButton(i18n("OK"), this);
  ok_button->setDefault(true);
  QPushButton *cancel_button = new QPushButton(i18n("Cancel"), this);
  box->addStretch();
  box->addWidget(ok_button);
  box->addWidget(cancel_button);
  box->addStretch();
  
  connect(file_button,SIGNAL(clicked()),SLOT(slotFileButtonClicked()));
  connect(ok_button,SIGNAL(clicked()),SLOT(accept()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
  resize(sizeHint().width()*2, sizeHint().height());
}


CDocTreePropDlg::~CDocTreePropDlg(){
}

void CDocTreePropDlg::slotFileButtonClicked(){
  KURL str = KFileDialog::getOpenURL(0,"*.html *.htm",this,"test");
  if(!str.isEmpty()){
    file_edit->setText(str.path());
  }
}
#include "cdoctreepropdlg.moc"
