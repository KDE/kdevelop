/***************************************************************************
                    cprjoptionsdlg.h - the setup DLG for a project
                             -------------------

    begin                : 10 Aug 1998
    copyright            : (C) 1998 by Sandy Meier,Stefan Bartel
    email                : smeier@rz.uni-potsdam.de,bartel@rz.uni-potsdam.de

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __CPRJOPTIONSDLG_H_
#define __CPRJOPTIONSDLG_H_

class CProject;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class KConfig;
class QMultiLineEdit;
class CPrjCompOpts;
class CPrjAddOpts;
class KDevSession;

#include <kdialogbase.h>
#include "cprjconfchange.h"

//#include <qtabdialog.h>

/** the setup-dialog for a project
  *@author Sandy Meier, Stefan Bartel
  */
class CPrjOptionsDlg : public KDialogBase // QTabDialog
{
    Q_OBJECT
public:
  /**constructor*/
  CPrjOptionsDlg(CProject* prj, KDevSession* session, const QString& curr, QWidget *parent, const char *name );
  bool needConfigureInUpdate();
  bool needMakefileUpdate(){return need_makefile_generation;}

private:
  void addGeneralPage();
  void addAdditionalOptionsPage();
  void addCompilerOptionsPage();
  void addCompilerWarningsPage();
  void addLinkerPage();
  void addMakePage();
  void addBinPage();

protected:
	CPrjCompOpts* compdlg;
	CPrjAddOpts *addOptsDlg;
  /** local projectinfo object*/
  CProject* prj_info;
  // TAB General
  QLineEdit* email_edit;
  QLineEdit* author_edit;
  QLineEdit* version_edit;
  QLineEdit* prjname_edit;
  QCheckBox* modifymakefiles_checkbox;
  QComboBox* vcsystem_combo;
  QMultiLineEdit* info_edit;
  // TAB Linker options
  QCheckBox* l_remove_symbols;
  QCheckBox* l_static;
  QLineEdit* addit_ldflags;
  QCheckBox* l_X11;
  QCheckBox* l_Xext;
  QCheckBox* l_qt;
  QCheckBox* l_kdecore;
  QCheckBox* l_kdeui;
  QCheckBox* l_khtml;
  QCheckBox* l_kfm;
  QCheckBox* l_kfile;
  QCheckBox* l_kparts;
  QCheckBox* l_kspell;
  QCheckBox* l_kab;
  QCheckBox* l_math;
  QLineEdit* addit_ldadd;
  // TAB Make options
  QLineEdit* m_set_modify_line;
  QSpinBox* m_job_number;
  QPushButton* m_set_modify_dir;
  QCheckBox* m_print_debug_info;
  QLineEdit* m_optional_line;
  QCheckBox* m_print_data_base;
  QCheckBox* m_no_rules;
  QCheckBox* m_env_variables;
  QCheckBox* m_cont_after_error;
  QCheckBox* m_touch_files;
  QCheckBox* m_print_work_dir;
  QCheckBox* m_silent_operation;
  QCheckBox* m_ignor_errors;
  QComboBox* m_rebuild_combo;
  QLabel* m_job_number_label;
  QLabel* m_optional_label;
  QLabel* m_rebuild_label;
  QLabel* m_set_modify_label;
  QLabel* m_makestartpoint_label;
  QLineEdit* m_makestartpoint_line;
  QPushButton* m_makestartpoint_dir;
private:
	KDevSession* sess;
	QString currentcfg;
  QString old_version;
  QString old_name;
  QString old_ldflags;
  QString old_ldadd;
  QString old_addit_flags;
  QString old_cxxflags;
  bool need_configure_in_update;
  bool need_makefile_generation;
  KConfig *settings;
  // Binary path
  QLineEdit* binary_edit;
  QLineEdit* libtool_edit;

  CPrjConfChange configureIn;

protected slots:
   /** is called, if the ok-button were clicked*/
  void ok();
  void slotFileDialogClicked();
  void slotBinaryClicked();
  void slotLibtoolClicked();
  void slotFileDialogMakeStartPointClicked();
};

#endif


