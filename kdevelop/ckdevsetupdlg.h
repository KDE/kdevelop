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

class QCheckBox;
class QComboBox;
class KAccel;
class KConfig;
class KKeyChooser;
class QComboBox;
class QLineEdit;
class QButtonGroup;
class QLabel;

#include <qtabdialog.h>
#include <kaccel.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



/** the setup dialog for kdevelop
  *@author Sandy Meier
  */
class CKDevSetupDlg : public QTabDialog
{
    Q_OBJECT
public:
    CKDevSetupDlg( KAccel* accel, QWidget *parent=0, const char *name=0);
    bool hasChangedPath() const {return wantsTreeRefresh;};

private:
  QWidget *w;
  QWidget *w1;
  QWidget *w2;
  QWidget *w3;
  QWidget *w4;
  KKeyChooser* w21;

  QString kde_doc_path, qt_doc_path;
  bool wantsTreeRefresh;

  KKeyEntryMap keyMap;
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
  QCheckBox *kdocCheck;

  QLineEdit* kde_edit;
  QLineEdit* qt_edit;
  QLineEdit* qt2_edit;
  QLineEdit* kde2_edit;
// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
	QLineEdit* ppath_edit;
// ---

  // Dbg external options
  QButtonGroup* dbgExternalGroup;
  QCheckBox* dbgExternalCheck;
  QLabel* dbgSelectCmdLabel;
  QLineEdit* dbgExternalSelectLineEdit;

  // Dbg internal options
  QButtonGroup* dbgInternalGroup;
  QCheckBox* dbgMembersCheck;
  QCheckBox* dbgAsmCheck;
  QCheckBox* dbgLibCheck;
  QCheckBox* dbgFloatCheck;
  QCheckBox* dbgTerminalCheck;

 private slots:
  void slotOkClicked();
  void slotDefault();
  void slotQtClicked();
  void slotKDEClicked();
  void slotKDEUpdateReq();
  void slotSetDebug();
  void slotQt2Clicked();
  void slotKDE2Clicked();
// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
	void slotPPathClicked();
// ---

};

#endif


