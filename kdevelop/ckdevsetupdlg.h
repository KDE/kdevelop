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
class QFrame;
class QButtonGroup;
class CCompletionOpts;

#include <kdialogbase.h>
#include <kaccel.h>
#include <ccompconf.h>

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

/** the setup dialog for kdevelop
  *@author Sandy Meier
  */
class CKDevSetupDlg : public KDialogBase
{
    Q_OBJECT
public:
  CKDevSetupDlg( KAccel* accel, QWidget *parent=0, const char *name=0, int curMdiMode = 0L);
  bool hasChangedPath() const { return wantsTreeRefresh; };
	int mdiMode() { return m_mdiMode; };

private:
  void addGeneralTab();
  void addKeysTab();
  void addDocTab();
  void addCodeCompletionTab();
  /** adds the compiler page for setting up the compile environment */
  void addCompilerTab();
  void addDebuggerTab();
  void addQT2Tab();
  void addUserInterfaceTab();

 private slots:
  void slotOkClicked();
  void slotDefault();
  void slotQtClicked();
  void slotKDEClicked();
  void slotKDEUpdateReq();
  void slotSetDebug();
  void slotQt2Clicked();
  void slotKDE2Clicked();
  void slotSetSearchDatabase(const QString&);
  /** shows the create search database dialog called by the setup button */
  void slotCreateSearchDatabase();
// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
	void slotPPathClicked();
// ---

private:
  bool wantsTreeRefresh;
  KConfig* config;
  KAccel* m_accel;

  QFrame* generalPage;
  QFrame* keysPage;
  QFrame* debuggerPage;
  QFrame* compilerPage;
  CCompConf* compdlg;

  QString kde_doc_path;
  QString qt_doc_path;


  KKeyChooser* keyChooser;
#if QT_VERSION < 300
  KKeyEntryMap keyMap;
#endif
  QCheckBox* autoSaveCheck;
  QComboBox* autosaveTimeCombo;
  QLineEdit* makeSelectLineEdit;
  QCheckBox* autoSwitchCheck;
  QCheckBox* startupEditingCheck;
  QCheckBox* defaultClassViewCheck;
  QCheckBox* logoCheck;
  QCheckBox* tipDayCheck;
  QCheckBox* lastProjectCheck;
  QCheckBox* kdocCheck;
  QCheckBox* useCTags;

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

  // user interface mode
  QButtonGroup* bg;
  int m_mdiMode;

  // code completion page
  CCompletionOpts* completionOptsDlg;
};

#endif


