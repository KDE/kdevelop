/***************************************************************************
                cnewfiledlg.h - the new file dialog in kdevelop 
                             -------------------                                         

    version              :                                   
    begin                : 20 Aug 1998                                        
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

#ifndef CNEWFILEDLG_H
#define CNEWFILEDLG_H

#include <kdialogbase.h>

class CProject;
class QLineEdit;
class QLabel;
class QPushButton;
class QCheckBox;
class QListBox;
class QButtonGroup;
class KTabCtl;


/** the new file dialog
  *@author Sandy Meier
  */
class CNewFileDlg : public KDialogBase {
  Q_OBJECT
public:
  /**constructor*/
  CNewFileDlg(QWidget* parent =0,const char* name = 0,bool modal = false,WFlags f =0,CProject* prj=0); 
  /** return the filename*/
  QString fileName();
  /** return the filetype*/
  QString fileType();
  /** set the checkbox*/
  void setUseTemplate();
  /** set the checkbox*/
  void setAddToProject();
  bool useTemplate();
  bool addToProject();
  QString location();
 protected slots:
  void slotTabSelected(int item);
  void slotOKClicked();
  void slotLocButtonClicked();
  void slotAddToProject();
  void slotEditTextChanged(const char* text);
  void slotListHighlighted(int);
  
protected:
  /** if true, it helps while typeing */
  bool  autocompletion;
  CProject* prj;
  QLineEdit* prj_loc_edit;
  QPushButton* loc_button;
  QLabel* label_filename;
  QLabel* label_filetyp;
  QCheckBox* check_use_template;
  QCheckBox* check_add_project;
  QLabel* location_label;
  KTabCtl* tab;
  QListBox* list_cpp;
 
  QListBox* list_linux;
  QListBox* list_manuals;
  QLineEdit* edit;
  QPushButton* ok;
  QPushButton* cancel;
  QButtonGroup* button_group;
  int current;
};

#endif

