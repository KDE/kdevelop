/***************************************************************************
                          cdoctreepropdlg.h  -  description                              
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


#ifndef CDOCTREEPROPDLG_H
#define CDOCTREEPROPDLG_H

#include <qdialog.h>

class QLineEdit;
class QLabel;
class QPushButton;

/**
  *@author Sandy Meier
  */

class CDocTreePropDlg : public QDialog  {
   Q_OBJECT
public: 
   CDocTreePropDlg(QWidget *parent=0, const char *name=0);
  ~CDocTreePropDlg();
 protected slots:
 void  slotFileButtonClicked();
public:
  QLineEdit* name_edit;
  QLineEdit* file_edit;
protected:
  
  QLabel* name_label;
  QLabel* file_label;
  QPushButton* ok_button;
  QPushButton* cancel_button;
  QPushButton* file_button;
};

#endif

