/***************************************************************************
                          ccreatedocdatabasedlg.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Jan 9 1999                                           
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


#ifndef CCREATEDOCDATABASEDLG_H
#define CCREATEDOCDATABASEDLG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <kprocess.h>
#include <kconfig.h>

class ProcessView;


/**frontend for glimpseindex
  *@author Sandy Meier
  */

class CCreateDocDatabaseDlg : public QDialog  {
   Q_OBJECT
public: 
	CCreateDocDatabaseDlg(ProcessView *proc, KConfig *config,
                              QWidget *parent=0, const char *name=0);
	~CCreateDocDatabaseDlg();
 protected slots:

 void slotOkClicked();
  void slotAddButtonClicked();
  void slotRemoveButtonClicked();
  void slotDirButtonClicked();
  
protected:
    QCheckBox* kde_checkbox;
    QCheckBox* qt_checkbox;
    QRadioButton* tiny_radio_button;
    QRadioButton* small_radio_button;
    QRadioButton* medium_radio_button;
    QPushButton* ok_button;
    QPushButton* cancel_button;
    QPushButton* dir_button;
    QListBox* dir_listbox;
    QPushButton* add_button;
    QPushButton* remove_button;
    QLineEdit* dir_edit;
  
  ProcessView* proc;
  KConfig* conf;
};

#endif

