/***************************************************************************
                          caddexistingfiledlg.h  -  description                              
                             -------------------                                         

    version              :     
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


#ifndef CADDEXISTINGFILEDLG_H
#define CADDEXISTINGFILEDLG_H

#include <qdialog.h>

class CProject;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

//#include "cproject.h"
class CProject;

/** Dlg for adding file(s) to a existing project
  *@author Sandy Meier
  */

class CAddExistingFileDlg : public QDialog  {
   Q_OBJECT
public: 
   CAddExistingFileDlg(QWidget *parent=0, const char *name=0,CProject* p_prj=0);
  ~CAddExistingFileDlg();
  QLineEdit* source_edit;
  QLineEdit* destination_edit;
  bool isTemplateChecked();
protected:
  CProject* prj;
  QLabel* source_label;
  
  QLabel* destination_label;
  QPushButton* source_button;
  QPushButton* destination_button;
  QCheckBox* template_checkbox;
  QPushButton* ok_button;
  QPushButton* cancel_button;
 protected slots:
 void sourceButtonClicked();
  void destinationButtonClicked();
  void OK();
};

#endif


