/***************************************************************************
                     ckdevsetupdlg.h - the setup dialog for CKDevelop
                             -------------------                                         

    version              :                                   
    begin                : 17 Aug 1998                                        
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


#ifndef __CKDEVSETUPDLG_H_
#define __CKDEVSETUPDLG_H_

#include <qtabdialog.h>
#include <qdict.h>

#include "./kwrite/kguicommand.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class KConfig;
class KAccel;
class KKeyChooser;
class KKeyEntry;

/** the setup dialog for kdevelop
  *@author Sandy Meier
  */
class CKDevSetupDlg : public QTabDialog
{
    Q_OBJECT
  public:
    CKDevSetupDlg(QWidget *parent, KAccel *accel, KGuiCmdManager &cmdMngr, const char *name=0L);
    bool hasChangedPath() const {return wantsTreeRefresh;};

private:
  QWidget *w;
  QWidget *w1;
  QWidget *w2;
  KKeyChooser* w21;

  QString kde_doc_path, qt_doc_path;
  bool wantsTreeRefresh;

  QDict<KKeyEntry>* dict;
  KConfig* config;
  KAccel* accel;

  QCheckBox* autoSaveCheck;
  QComboBox* autosaveTimeCombo;
  QLineEdit* makeSelectLineEdit;
  QCheckBox* autoSwitchCheck;
  QCheckBox* defaultClassViewCheck;
  QCheckBox* logoCheck;
  QCheckBox* tipDayCheck;
  QCheckBox* lastProjectCheck;

  QLineEdit* kde_edit;
  QLineEdit* qt_edit;


 private slots:
  void slotOkClicked();
  void slotDefault();
  void slotQtClicked();
  void slotKDEClicked();
  void slotKDEUpdateReq();
};

#endif
