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

#include <qtabdialog.h>

class CProject;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QLabel;
class QMultiLineEdit;
class FlagBox;


/** the setup-dialog for a project 
  *@author Sandy Meier, Stefan Bartel
  */
class CPrjOptionsDlg : public QTabDialog
{
    Q_OBJECT
public:
    /**constructor*/
    CPrjOptionsDlg( QWidget *parent, const char *name,CProject* prj );
  bool needConfigureInUpdate();
  bool needMakefileUpdate(){return need_makefile_generation;}
protected:
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
  // TAB GCC-options
  QComboBox* target;
  FlagBox* syntax_check;
  QCheckBox* optimize;
  QSpinBox*  optimize_level;
  FlagBox* debug;
  QSpinBox*  debug_level;
  FlagBox* gprof_info;
  FlagBox* save_temps;
  QLineEdit* addit_gcc_options;
  // TAB GCC-Warnings
  FlagBox* w_all;
  FlagBox* w_;
  FlagBox* w_traditional;
  FlagBox* w_undef;
  FlagBox* w_shadow;
  //  QSpinBox*  w_id_clash_len;
  //  QSpinBox*  w_larger_than_len;
  FlagBox* w_pointer_arith;
  FlagBox* w_bad_function_cast;
  FlagBox* w_cast_qual;
  FlagBox* w_cast_align;
  FlagBox* w_write_strings;
  FlagBox* w_conversion;
  FlagBox* w_sign_compare;
  FlagBox* w_aggregate_return;
  FlagBox* w_strict_prototypes;
  FlagBox* w_missing_prototypes;
  FlagBox* w_missing_declarations;
  FlagBox* w_redundant_decls;
  FlagBox* w_nested_externs;
  FlagBox* w_inline;
  FlagBox* w_old_style_cast;
  FlagBox* w_overloaded_virtual;
  FlagBox* w_synth;
  FlagBox* w_error;
  // TAB Linker options
  QCheckBox* l_remove_symbols;
  QCheckBox* l_static;
  QLineEdit* addit_ldflags;
  QCheckBox* l_X11;
  QCheckBox* l_Xext;
  QCheckBox* l_qt;
  QCheckBox* l_kdecore;
  QCheckBox* l_kdeui;
  QCheckBox* l_khtmlw;
  QCheckBox* l_kfm;
  QCheckBox* l_kfile;
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
  QLabel* m_job_number_label;
  QLabel* m_optional_label;
  QLabel* m_set_modify_label;

private:
  void setupTab1();
  void setupTab2();
  void setupTab3();
  void setupTab4();
  void setupTab5();
  void ok1();
  void ok2_3();
  void ok4();
  void ok5();
    
  QString old_version;
  QString old_ldflags;
  QString old_ldadd;
  QString old_addit_flags;
  QString old_cxxflags;
  bool need_configure_in_update;
  bool need_makefile_generation;
	
protected slots:
   /** is called, if the ok-button were clicked*/
  void  ok();
  void slotFileDialogClicked();
    //  void slotOptimize_level_changed(int v);
    //  void slotDebug_level_changed(int v);
};

#endif


