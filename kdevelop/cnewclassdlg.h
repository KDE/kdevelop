/***************************************************************************
                     cnewclassdlg.h -
                             -------------------                                         

    version              :                                   
    begin                : 4 Oct 1998
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


#ifndef __CNEWCLASSDLG_H_
#define __CNEWCLASSDLG_H_


#include <qtabdialog.h>

class QButtonGroup;
class QLineEdit;
class QMultiLineEdit;
class QPushButton;
class QRadioButton;
class QLabel;
class QCheckBox;
class KConfig;
class CProject;


/** generates new class-skeletons (a header and a implementation file)
  *@author Sandy Meier
  */
class CNewClassDlg : public QDialog
{
    Q_OBJECT
public:
    CNewClassDlg( QWidget *parent=0, const char *name=0,CProject* prj=0 );
  QString getHeaderFile();
  QString getImplFile();
private:
    //  KConfig* config;
  QButtonGroup* add_group;
  QButtonGroup* doc_group;
  QButtonGroup* inher_group;
  QButtonGroup* baseclass_group;
  QButtonGroup* files_group;
  QButtonGroup* classname_group;
  QLineEdit* classname_edit;
  QLineEdit* header_edit;
  QMultiLineEdit* doc_edit;
  QLineEdit* baseclass_edit;
  QPushButton* ok_button;
  QPushButton* cancel_button;
  QRadioButton* public_button;
  QRadioButton* protected_button;
  QRadioButton* private_button;
  QLineEdit* impl_edit;
  QLabel* header_label;
  QLabel* impl_label;
  QCheckBox* template_check;
  QCheckBox* qwidget_check;
  bool header_modified;
  bool impl_modified;
  QString m_header_file;
  QString m_impl_file;
  CProject* prj_info;
 private slots:
 void        ok();
  void  slotClassEditChanged(const char*);
  void  slotHeaderEditChanged(const char*);
  void  slotImplEditChanged(const char*);
};

#endif
