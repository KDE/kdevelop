/***************************************************************************
                          ctoolsconfigdlg.h  -  description                              
                             -------------------                                         
    begin                : Thu Apr 15 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
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


#ifndef CTOOLSCONFIGDLG_H
#define CTOOLSCONFIGDLG_H

#include <qdialog.h>
#include <qlabel.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qpushbt.h>
#include <qstrlist.h>

#include <kapp.h>


/**a dialog to configure the tools-menu in the kdevelop menubar.
 *Allows configuration of menuentry, command and arguments
 *@author Ralf Nolden
 */

class CToolsConfigDlg : public QDialog  {
  Q_OBJECT
public: 
  CToolsConfigDlg(QWidget *parent=0, const char *name=0);
  ~CToolsConfigDlg();
  
protected slots:

void slotToolAdd();
  void slotToolDelete();
  void slotToolMoveUp();
  void slotToolMoveDown();
  void slotShowToolProp(int);
  void slotToolsExeSelect();
  void slotOK();
  void slotHelp();
  
protected:
  void readConfig();
  void writeConfig();
  void swap(int item1,int item2);
		
  QListBox* tools_listbox;
  QPushButton* add_button;
  QPushButton* delete_button;
  QPushButton* move_up_button;
  QPushButton* move_down_button;
  QPushButton* ok_button;
  QPushButton* cancel_button;
  QPushButton* help_button;
  QLineEdit* executable_edit;
  QLabel* executable_label;
  QLineEdit* menu_text_edit;
  QLabel* menu_text_label;
  QLineEdit* arguments_edit;
  QLabel* arguments_label;
  QPushButton* executable_button;
  
private:
  KConfig* config;
  QStrList tools_exe;
  QStrList tools_entry;
  QStrList tools_argument;
};

#endif





