/***************************************************************************
                    cprjoptionsdlg.h - the setup DLG for a project  
                             -------------------                                         

    version              :                                   
    begin                : 10 Aug 1998                                        
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
#ifndef __CPRJOPTIONSDLG_H_
#define __CPRJOPTIONSDLG_H_

#include "cproject.h"
#include <kfiledialog.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qlistbox.h>
#include <qlined.h>
#include <qmultilinedit.h>
#include <qtabdialog.h>
#include <kapp.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <kmsgbox.h>
#include <klined.h>
#include <kspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <kquickhelp.h>

/** the setup-dialog for a project 
  *@author Sandy Meier, Stefan Bartel
  */
class CPrjOptionsDlg : public QTabDialog
{
    Q_OBJECT
public:
    /**constructor*/
    CPrjOptionsDlg( QWidget *parent, const char *name,CProject* prj );
protected:
  /** local projectinfo object*/
  CProject* prj_info; 
  // TAB General
  QLineEdit* email_edit;
  QLineEdit* author_edit;
  QLineEdit* version_edit;
  QLineEdit* prjname_edit;
  QLineEdit* handbook_edit;
  QMultiLineEdit* info_edit;
  // TAB GCC-options
  QComboBox* target;
  QCheckBox* syntax_check;
  QCheckBox* optimize;
  KNumericSpinBox* optimize_level;
  QCheckBox* debug;
  KNumericSpinBox* debug_level;
  QCheckBox* gprof_info;
  QCheckBox* save_temps;
  QLineEdit* addit_gcc_options;
  // TAB GCC-Warnings
  QCheckBox* w_all;
  QCheckBox* w_;
  QCheckBox* w_traditional;
  QCheckBox* w_undef;
  QCheckBox* w_shadow;
  QCheckBox* w_id_clash_len;
  QCheckBox* w_larger_than_len;
  QCheckBox* w_pointer_arith;
  QCheckBox* w_bad_function_cast;
  QCheckBox* w_cast_qual;
  QCheckBox* w_cast_align;
  QCheckBox* w_write_strings;
  QCheckBox* w_conversion;
  QCheckBox* w_sign_compare;
  QCheckBox* w_aggregate_return;
  QCheckBox* w_strict_prototypes;
  QCheckBox* w_missing_prototypes;
  QCheckBox* w_missing_declarations;
  QCheckBox* w_redundant_decls;
  QCheckBox* w_nested_externs;
  QCheckBox* w_inline;
  QCheckBox* w_old_style_cast;
  QCheckBox* w_overloaded_virtual;
  QCheckBox* w_synth;
  QCheckBox* w_error;
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
  QLineEdit* addit_ldadd;

protected slots:
   /** is called, if the ok-button were clicked*/
  void  ok();

};

#endif
