/***************************************************************************
                          cfilepropdlg.h  -  description
                             -------------------

    version              :     
    begin                : Sat Oct 17 1998                                           
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


#ifndef CFILEPROPDLG_H
#define CFILEPROPDLG_H

#include <qwidget.h>
#include <qdialog.h>
#include "cproject.h"
#include <qbuttongroup.h>
#include "clogfileview.h"
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <kquickhelp.h>

/** 
 * Dlg for changing the properties of a file in an existing project
 *@author Sandy Meier
 */

class CFilePropDlg : public QDialog  {
   Q_OBJECT
public: 
	CFilePropDlg(QWidget *parent=0, const char *name=0,CProject* prj=0);
	~CFilePropDlg();
 public slots:
void slotSingleSelected(int index);
  void  slotInstallCheckToogled(bool on);
  void slotOk();

protected:
  QList<TFileInfo>* file_list;
  CProject* prj;
  TFileInfo* saved_info;
  QButtonGroup* distribution_group;
  QButtonGroup* type_group;
  QButtonGroup* installion_group;
  QButtonGroup* file_group;
  CLogFileView* log_tree;
  QComboBox* type_combo;
  QLineEdit* install_loc_edit;
  QLabel* install_loc_label;
  QLabel* size_label;
  QLabel* name_label;
  
  QCheckBox* incdist_check;
  QCheckBox* install_check;
  QPushButton* ok_button;
  QPushButton* cancel_button;
  QLabel* name_e_label;
  QLabel* size_e_label;
};

#endif
