/***************************************************************************
                          cupdatekdedocdlg.h  -  description                              
                             -------------------                                         

    version              :     
    begin                : Mon Nov 9 1998                                           
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


#ifndef CUPDATEKDEDOCDLG_H
#define CUPDATEKDEDOCDLG_H

class KShellProcess;

class QButtonGroup;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

#include <qwidget.h>

/** Generates the API-Documention from all KDE-Libs with kdoc
  *@author Sandy Meier
  */

class CUpdateKDEDocDlg : public QWidget {
   Q_OBJECT
public: 
   CUpdateKDEDocDlg(KShellProcess* proc=0, const QString& kdeDocDir = QString::null, const QString& qtDocDir = QString::null, QWidget *parent=0, bool bShowCancelButton = false, const char *name=0);
  ~CUpdateKDEDocDlg();
  bool isUpdated() const {return bUpdated;};
  QString getDocPath() const {return kde_doc_path;};

  QButtonGroup* install_box;
  QLineEdit* source_edit;
  QLabel* source_label;
  QLineEdit* doc_edit;
  QLabel* doc_label;
  QPushButton* source_button;
  QPushButton* doc_button;
  QPushButton* ok_button;
  QPushButton* cancel_button;
  QRadioButton* del_recent_radio_button;
  QRadioButton* del_new_radio_button;
  QRadioButton* leave_new_radio_button;

signals:
	void newDocPathIsSetNow(const QString&);
public slots:
 void OK();
  void slotLeaveNewRadioButtonClicked();
  void slotDelNewRadioButtonClicked();
  void slotDelRecentRadioButtonClicked();
  void slotDocButtonClicked();
  void slotSourceButtonClicked();
protected:
  KShellProcess* m_proc;
  QString kde_doc_path;
  QString qt_doc_path;
  bool bUpdated;
};

#endif
