/***************************************************************************
                          cdatepikerdlg.h  -  description                              
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


#ifndef CDATEPIKERDLG_H
#define CDATEPIKERDLG_H

#include <qdialog.h>
#include <kdatepik.h>

/**
  *@author Stefan
  */

class CDatepikerDlg : public QDialog {
  Q_OBJECT
public: 
  CDatepikerDlg(QWidget* parent=0,const char* name=0);
  ~CDatepikerDlg();
  QString dateString;
  QDate date;
  void init();
  void getDateString(QString&);
  void getDate(QDate&);

public slots:
    void slotDateSelected(QDate date);    

protected:
  KDatePicker *datepick;
};

#endif //CDATEPIKERDLG_H
