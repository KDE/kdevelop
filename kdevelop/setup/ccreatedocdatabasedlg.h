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

#include <qdialog.h>

class QCheckBox;
class QRadioButton;
class QPushButton;
class QListBox;
class QLineEdit;
class QMultiLineEdit;
class KShellProcess;
class KProcess;
class KConfig;

/**frontend for glimpseindex
  *@author Sandy Meier
  */

class CCreateDocDatabaseDlg : public QWidget  {
   Q_OBJECT
public: 
	CCreateDocDatabaseDlg(QWidget *parent=0, const char *name=0,KShellProcess* proc=0, const QString& kdeDocDir = QString::null, const QString& qtDocDir = QString::null,
	      bool foundGlimpse=false, bool foundHtDig=false, bool bShowIndexingButton = true);
        void setQtDocDir(const QString& qtDocDir);
	~CCreateDocDatabaseDlg();

public slots:
  void slotOkClicked();

signals:
  void indexingStartedNow();
	void indexingFinished(const QString&);
	
protected:
  void createShellProcessOutputWidget();

protected slots:
  void slotAddButtonClicked();
  void slotRemoveButtonClicked();
  void slotDirButtonClicked();
  void slotProcessOK();

  void slotShowToolProcessOutputDlg();
  void slotReceivedStdout(KProcess*,char*,int);
  void slotReceivedStderr(KProcess*,char*,int);
  void slotProcessExited(KProcess*);

protected:
  QCheckBox* kde_checkbox;
  QCheckBox* qt_checkbox;
  QRadioButton* tiny_radio_button;
  QRadioButton* small_radio_button;
  QRadioButton* medium_radio_button;
  QPushButton* start_button;
  QPushButton* cancel_button;
  QPushButton* dir_button;
  QListBox* dir_listbox;
  QPushButton* add_button;
  QPushButton* remove_button;
  QLineEdit* dir_edit;
  QRadioButton *useGlimpse;
  QRadioButton *useHtDig;
	
	QString m_kdeDocDir;
	QString m_qtDocDir;
	
  KShellProcess* m_proc;

  QDialog* m_pShellProcessOutput;
  QMultiLineEdit* m_pShellProcessOutputLines;
  QPushButton* m_pShellProcessOutputOKButton;
  QPushButton* m_pShellProcessOutputCancelButton;
};

#endif

