/***************************************************************************
                          cdatepikerdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Mar 4 1999                                           
    copyright            : (C) 1999 by Stefan                         
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "cdatepikerdlg.h"
#include <iostream.h>

CDatepikerDlg::CDatepikerDlg(QWidget* parent,const char* name) : QDialog(parent,name,true){
  init();
}
CDatepikerDlg::~CDatepikerDlg(){
  delete (datepick);
}

void CDatepikerDlg::init() {
  datepick = new KDatePicker (this);
  datepick->resize(300,200);
  connect(datepick,SIGNAL(dateSelected(QDate)),SLOT(slotDateSelected(QDate)));
  datepick->show();
}

void CDatepikerDlg::slotDateSelected(QDate selectedDate) {
  dateString = selectedDate.toString();
  reject();
}

void CDatepikerDlg::getDateString(QString &string) {
  string = dateString;
}
