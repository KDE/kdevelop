/***************************************************************************
                          kdlgnewdialogdlg.h  -  description                              
                             -------------------                                         
    begin                : Mon Apr 12 1999                                           
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


#ifndef KDLGNEWDIALOGDLG_H
#define KDLGNEWDIALOGDLG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
class CProject;

/**
  *@author Sandy Meier
  */

class KDlgNewDialogDlg : public QDialog  {
   Q_OBJECT
public: 
   KDlgNewDialogDlg(QWidget *parent=0, const char *name=0,CProject* prj=0);
  ~KDlgNewDialogDlg();

	QString getClassname(){return classname_edit->text();}
  QString getHeaderName(){return header_edit->text();}
  QString getSourceName(){return cpp_edit->text();}
  QString getDataName(){return data_edit->text();}
  QString getLocation(){return loc_edit->text();}
  /** QDialog,QTabDialog,QFrame,QWidget*/
  QString getBaseClass();
  QString getBaseClassHeader();
  
  
protected:
  void setCustomPropsEnabled(bool show);

  QButtonGroup* custom_prob_gbox;
  QRadioButton* qdialog_radio_button;
  QRadioButton* qwidget_radio_button;
  QRadioButton* qtabdialog_radio_button;
  QRadioButton* qframe_radio_button;
  QPushButton* ok_button;
  QPushButton* cancel_button;
  QLineEdit* data_edit;
  QLineEdit* cpp_edit;
  QLineEdit* classname_edit;
  QPushButton* loc_button;
  QLineEdit* header_edit;
  QLabel* custom_header_label;
  QRadioButton* custom_radio_button;
  QLabel* custom_class_label;
  QLineEdit* custom_class_edit;
  QLineEdit* custom_header_edit;
  QLineEdit* loc_edit;
  
  bool  header_modified;
  bool source_modified;
  bool data_modified;

  CProject* project;

protected slots:

void slotOKClicked();
  void slotLocButtonClicked();
  void slotRadioButtonClicked(); 
  void  slotClassEditChanged(const QString &text);
  void  slotHeaderEditChanged(const QString &text);
  void  slotSourceEditChanged(const QString &text);
  void  slotDataEditChanged(const QString &text);
  
};

#endif
