/***************************************************************************
                          cmakemanualdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Jul 14 1999                                           
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


#include <qmessagebox.h>
#include <qfile.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include "cmakemanualdlg.h"
#include "ctoolclass.h"


CMakeManualDlg::CMakeManualDlg(QWidget *parent, const char *name,QString  manual_file) : QDialog(parent,name,true){
	initDialog();
	program_group = new QButtonGroup(this,"NoName");
	program_group->setGeometry(10,10,330,70);
	program_group->setMinimumSize(0,0);
	program_group->setTitle("Program");
	program_group->insert(sgml2html_radiobutton);
	program_group->insert(ksgml2html_radiobutton);
	program_group->lower();

	file_button->setPixmap(BarIcon("open"));
	file_edit->setText(manual_file);

	ok_button->setDefault(true);
	
	connect(ok_button,SIGNAL(clicked()),this,SLOT(slotOkClicked()));
	connect(cancel_button,SIGNAL(clicked()),this,SLOT(reject()));		
	connect(file_button,SIGNAL(clicked()),this,SLOT(slotFileButtonClicked()));			

}

CMakeManualDlg::~CMakeManualDlg(){
}


void CMakeManualDlg::slotOkClicked(){
    if( !CToolClass::searchProgram("sgml2html")){
	return;
    }
    if(ksgml2html_radiobutton->isChecked()){
			if(!CToolClass::searchProgram("ksgml2html")){
	    return;
	}
	    }

    if (ksgml2html_radiobutton->isChecked()){
		program = "ksgml2html";
    }
    else{
		program = "sgml2html";
    }
    file = file_edit->text();
    if(!QFile::exists(file)){
      QMessageBox::warning(this,i18n("Error..."),
		       i18n("You must choose an existing file!"));
      return;
    }
    accept();
}
void CMakeManualDlg::slotFileButtonClicked(){
    QString str =  KFileDialog::getOpenFileName(0,"*.sgml",this,"test");
    if(!str.isEmpty()){
	file_edit->setText(str);
    }    
}




















