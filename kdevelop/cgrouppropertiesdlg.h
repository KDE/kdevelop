/***************************************************************************
                          cgrouppropertiesdlg.h  -  description                              
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


#ifndef CGROUPPROPERTIESDLG_H
#define CGROUPPROPERTIESDLG_H

#include <qdialog.h>

class QLineEdit;
class QLabel;
class QPushButton;

/** Dlg for change the group-properties in the LFV (logical file view)
  *@author Sandy Meier
  */

class CGroupPropertiesDlg : public QDialog
{
   Q_OBJECT

public: 
  CGroupPropertiesDlg(QWidget *parent=0, const char *name=0);
  ~CGroupPropertiesDlg();
  QLineEdit* name_edit;
  QLineEdit* filters_edit;
  QLabel* name_label;
  QLabel* filter_label;
  QPushButton* cancel_button;
  QPushButton* ok_button;
protected slots:
  void slotOK();
};

#endif
