/***************************************************************************
                   prjoptionsdlg.cpp - the setup DLG for a project  
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

#include "cprjoptionsdlg.h"

#include "cproject.h"
#include "ctoolclass.h"
#include "debug.h"
#include "vc/versioncontrol.h"

#include <kapp.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <knumvalidator.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <kfiledialog.h>
#include <qfileinfo.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstrlist.h>
#include <qspinbox.h>
//#include <qtabdialog.h>
#include <qwhatsthis.h>

#include <iostream.h>

// OPTIONS DIALOG
CPrjOptionsDlg::CPrjOptionsDlg(CProject* prj, QWidget *parent, const char *name)
 : KDialogBase (  IconList,                 // dialogFace
                  i18n("Project Options" ), // caption
                  Ok|Cancel,                // buttonMask
                  Ok,                       // defaultButton
                  parent,
                  name,
                  true,                     // modal
                  true)                     // separator
{
  prj_info = prj;

  addGeneralPage();
  addCompilerOptionsPage();
  addCompilerWarningsPage();
  addLinkerPage();
  addMakePage();
  addBinPage();

  connect(this, SIGNAL(okClicked()), SLOT(ok()));
}

//
//****************** the General_Widget ********************
//
void CPrjOptionsDlg::addGeneralPage()
{
  QFrame* generalPage = addPage(i18n("General"),i18n("General Project Information"),
		KGlobal::instance()->iconLoader()->loadIcon( "readme", KIcon::NoGroup, KIcon::SizeMedium ));
  QGridLayout *grid = new QGridLayout(generalPage,9,6,15,7,"grid-a");
  QWhatsThis::add(generalPage, i18n("Set the general options of your project here."));

  QLabel* prjname_label;
  prjname_label = new QLabel( generalPage, "prjname_label" );
  grid->addWidget(prjname_label,0,0);
  prjname_label->setText( i18n("Project name:") );

  prjname_edit = new QLineEdit( generalPage, "prjname_edit" );
  grid->addMultiCellWidget(prjname_edit,1,1,0,1);
  prjname_edit->setText( prj_info->getProjectName() );
  QWhatsThis::add(prjname_label, i18n("Set the project name here."));
  QWhatsThis::add(prjname_edit, i18n("Set the project name here."));


  QLabel* version_label;
  version_label = new QLabel( generalPage, "version_label" );
  grid->addWidget(version_label,0,2);
  version_label->setText( i18n("Version:") );

  version_edit = new QLineEdit( generalPage, "version_edit" );
  grid->addWidget(version_edit,1,2);
  version_edit->setText( prj_info->getVersion() );
//  version_edit->setValidator( new KFloatValidator( version_edit ));
  QWhatsThis::add(version_label, i18n("Set your project version number here."));
  QWhatsThis::add(version_edit, i18n("Set your project version number here."));


  QLabel* author_label;
  author_label = new QLabel( generalPage, "author_label" );
  grid->addWidget(author_label,2,0);
  author_label->setText( i18n("Author:") );

  author_edit = new QLineEdit( generalPage, "author_edit" );
  grid->addMultiCellWidget(author_edit,3,3,0,2);
  author_edit->setText( prj_info->getAuthor() );
  QWhatsThis::add(author_label,  i18n("Insert your name or the name of your team here"));
  QWhatsThis::add(author_edit, i18n("Insert your name or the name of your team here"));

  QLabel* email_label;
  email_label = new QLabel( generalPage, "email_label" );
  grid->addWidget(email_label,4,0);
  email_label->setText( i18n("Email:") );

  email_edit = new QLineEdit( generalPage, "email_edit");
  grid->addMultiCellWidget(email_edit,5,5,0,2);
  email_edit->setText( prj_info->getEmail() );
  QWhatsThis::add(email_label, i18n("Insert your email-address here"));
  QWhatsThis::add(email_edit, i18n("Insert your email-address here"));


  modifymakefiles_checkbox = new QCheckBox( generalPage, "" );
  grid->addMultiCellWidget(modifymakefiles_checkbox,6,6,0,1);
  modifymakefiles_checkbox->setText( i18n("Modify Makefiles") );
  modifymakefiles_checkbox->setChecked(prj_info->getModifyMakefiles());

  QLabel *vcsystem_label
    = new QLabel( i18n("Version Control:"), generalPage, "vcsystem_label" );
  grid->addWidget(vcsystem_label,7,0);

  vcsystem_combo = new QComboBox( false, generalPage );
  grid->addWidget(vcsystem_combo,7,2);
  QStrList l;
  VersionControl::getSupportedSystems(&l);
  vcsystem_combo->insertItem(i18n("None"));
  vcsystem_combo->insertStrList(&l);
  QString vcsystem = prj_info->getVCSystem();
  for (int i = 0; i < vcsystem_combo->count(); ++i)
      if (vcsystem_combo->text(i) == vcsystem)
          vcsystem_combo->setCurrentItem(i);

  QLabel* info_label;
  info_label=new QLabel(generalPage,"info_label");
  grid->addWidget(info_label,0,3);
  info_label->setText(i18n("Short Information:"));

  info_edit=new QMultiLineEdit(generalPage,"info_edit");
  grid->addMultiCellWidget(info_edit,1,8,3,5);
  QStrList short_info=prj_info->getShortInfo();
  short_info.first();
  do {
    info_edit->append(short_info.current());
  } while(short_info.next());

  QString infoEditMsg = i18n("Insert some useful information about "
          "your project here. This is only for the "
          "information in the project file-<b>not "
          "for README etc.");

  QWhatsThis::add(info_label, infoEditMsg);
  QWhatsThis::add(info_edit, infoEditMsg);
}

//
// *************** Compiler options *********************
//
void CPrjOptionsDlg::addCompilerOptionsPage()
{
  QString cxxflags=prj_info->getCXXFLAGS();

  need_configure_in_update = false;
  need_makefile_generation = false;

  QFrame* compilerOptions = addPage(i18n("Compiler Options"),i18n("General Compiler Options"),KGlobal::instance()->iconLoader()->loadIcon( "pipe", KIcon::NoGroup, KIcon::SizeMedium ));

  QWhatsThis::add(compilerOptions, i18n("Set your Compiler options here"));
  QGridLayout *grid1 = new QGridLayout(compilerOptions,2,2,15,7,"grid-b");
  QGroupBox* target_group;
  target_group=new QGroupBox(compilerOptions,"target_group");
  grid1->addWidget(target_group,0,0);
  target_group->setTitle(i18n("Target"));
  QWhatsThis::add(target_group, i18n("Set your target options here "
             "by specifying your machine type "
             "and GCC optimization level (0-3)"));

  QGridLayout *grid2 = new QGridLayout( target_group,3,2,15,7,"grid-c");
  QLabel* target_label;
  target_label=new QLabel(target_group,"target_label");
  grid2->addWidget( target_label,0,0);
  target_label->setText(i18n("Target Machine"));

  target=new QComboBox(false,target_group,"target");
  grid2->addWidget( target,0,1);
  target->insertItem(i18n("your machine"),0);
  target->insertItem(i18n("i386v"),1);
  if (cxxflags.contains("-b ")) {
    if (cxxflags.contains("i386v")) {
      target->setCurrentItem(1);
    }
  } else {
    target->setCurrentItem(0);
  }

  QString targetMsg = i18n("Specify the machine type for your program. "
             "Other machine types than your machine is "
             "usually only needed for precompiled "
             "distribution packages. i386v is intended for "
             "cross-compilers to build a binary for an intel "
             "machine running Unix System V.");
  QWhatsThis::add(target_label, targetMsg);
  QWhatsThis::add(target, targetMsg);

  // syntax_check=new QCheckBox(compilerOptions,"syntax_check");
//   syntax_check->setGeometry(20,60,220,20);
//   syntax_check->setText(i18n("only syntax check"));
//   syntax_check->setChecked(cxxflags.contains("-fsyntax-only"));
//   QWhatsThis::add(syntax_check, i18n("This option sets the compiler "
//              "to <i>-fsyntax-only</i> "
//         "which lets you check your code for "
//         "syntax-errors but doesn't do anything "
//         "else beyond that."));

  optimize=new QCheckBox(target_group,"optimize");
  grid2->addWidget( optimize,1,0);
  optimize->setText(i18n("optimize"));
  optimize->setChecked(!cxxflags.contains("-O0"));

  optimize_level=new QSpinBox(target_group,"optimize_level");
  grid2->addWidget( optimize_level,2,1);
  optimize_level->setRange(1,3);
  if (cxxflags.contains("-O1")) optimize_level->setValue(1);
  if (cxxflags.contains("-O2")) optimize_level->setValue(2);
  if (cxxflags.contains("-O3")) optimize_level->setValue(3);
  connect( optimize_level, SIGNAL(valueChanged(int)),this , SLOT(slotOptimize_level_changed(int)) );

  QLabel* optimize_level_label;
  optimize_level_label=new QLabel(target_group,"optimize_level_label");
  grid2->addWidget( optimize_level_label,2,0);
  optimize_level_label->setText(i18n("Optimization-level"));

  QString optimizeMsg = i18n("Set the -O option for the GCC "
                        "here. Turning off optimization "
                            "equals -O0. The higher the level "
                      "the more time you need to compile "
                       "but increases program speed.");

  QWhatsThis::add(optimize, optimizeMsg);
  QWhatsThis::add(optimize_level, optimizeMsg);
  QWhatsThis::add(optimize_level_label, optimizeMsg);

  QGroupBox* debug_group;
  debug_group=new QGroupBox(compilerOptions,"debug_group");
  grid1->addWidget(debug_group,0,1);
  debug_group->setTitle(i18n("Debugging"));
  QWhatsThis::add(debug_group, i18n("Set your debugging options here."));

  grid2 = new QGridLayout( debug_group,4,2,15,7,"grid-d");
  debug=new QCheckBox(debug_group,"debug");
  grid2->addWidget(debug,0,0);
  debug->setText(i18n("generate debugging information"));
  if (cxxflags.contains("-g")) {
    debug->setChecked(true);
  } else {
    debug->setChecked(false);
  }
  QWhatsThis::add(debug, i18n("Checking this turns on the -g flag "
                            "to generate debugging information."));

  debug_level=new QSpinBox(debug_group,"debug_level");
  grid2->addWidget(debug_level,1,1);
  debug_level->setRange(1,3);
  if (cxxflags.contains("-g1")) debug_level->setValue(1);
  if (cxxflags.contains("-g2")) debug_level->setValue(2);
  if (cxxflags.contains("-g3")) debug_level->setValue(3);
  connect( debug_level, SIGNAL(valueChanged(int)),this , SLOT(slotDebug_level_changed(int)) );

  QLabel* debug_level_label;
  debug_level_label=new QLabel(debug_group,"debug_level_label");
  grid2->addWidget(debug_level_label,1,0);
  debug_level_label->setText(i18n("Debug-level"));

  QString debugLevelMsg = i18n("Set the debugging level here. "
          "You can choose from level 1-3 which "
          "sets option -g1 to -g3 debugging "
          "level.");
  QWhatsThis::add(debug_level, debugLevelMsg);
  QWhatsThis::add(debug_level_label, debugLevelMsg);


  gprof_info=new QCheckBox(debug_group,"gprof_info");
  grid2->addWidget(gprof_info,2,0);
  gprof_info->setText(i18n("generate extra information for gprof"));
  if (cxxflags.contains("-pg")) {
    gprof_info->setChecked(true);
  } else {
    gprof_info->setChecked(false);
  }
  QWhatsThis::add(gprof_info, i18n("Generate extra code to write profile "
        "information for the analysis program "
        "<i>gprof</i>."));

  save_temps=new QCheckBox(debug_group,"save_temps");
  grid2->addWidget(save_temps,3,0);
  save_temps->setText(i18n("store temporary intermediate files"));
  if (cxxflags.contains("-save-temps")) {
    save_temps->setChecked(true);
  } else {
    save_temps->setChecked(false);
  }
  QWhatsThis::add(save_temps, i18n("Store the usually temporary intermediate "
        "files in the current directory. This means "
        "compiling a file <i>foo.c</i> will produce "
        "the files <i>foo.o, foo.i</i> and<i>foo.s"));

  QGroupBox* compiler_group;
  compiler_group=new QGroupBox(compilerOptions,"debug_group");
  grid1->addMultiCellWidget(compiler_group,1,1,0,1);

  grid2 = new QGridLayout( compiler_group,4,1,15,7,"grid-e");

  QLabel* addit_gcc_options_label;
  addit_gcc_options_label=new QLabel(compiler_group,"addit_gcc_options_label");
  grid2->addWidget(addit_gcc_options_label,0,0);
  addit_gcc_options_label->setText(i18n("additional options:"));

  addit_gcc_options=new QLineEdit(compiler_group,"addit_gcc_options");
  grid2->addWidget(addit_gcc_options,1,0);
  addit_gcc_options->setText(prj_info->getAdditCXXFLAGS());

  QString gccOptionsMsg = i18n("Insert other GCC-options here "
          "to invoke GCC with by setting the "
          "CXXFLAGS-environment variable.");
  QWhatsThis::add(addit_gcc_options_label, gccOptionsMsg);
  QWhatsThis::add(addit_gcc_options, gccOptionsMsg);

  if(prj_info->isCustomProject())
    compilerOptions->setEnabled(false);
}

//
// *************** Compiler Warnings *********************
//
void CPrjOptionsDlg::addCompilerWarningsPage()
{
  QString cxxflags=prj_info->getCXXFLAGS();
  QString ldflags=prj_info->getLDFLAGS();
  QString ldadd=prj_info->getLDADD();

  old_version = prj_info->getVersion();
  old_ldflags =  ldflags.stripWhiteSpace();
  old_ldadd = ldadd.stripWhiteSpace();
  old_addit_flags = prj_info->getAdditCXXFLAGS().stripWhiteSpace();
  old_cxxflags = cxxflags.stripWhiteSpace();

  QFrame* compilerWarnings = addPage(i18n("Compiler Warnings"),i18n("Compiler Warning Settings"),
		KGlobal::instance()->iconLoader()->loadIcon( "core", KIcon::NoGroup, KIcon::SizeMedium ));
		
  QWhatsThis::add(compilerWarnings, i18n("Set the Compiler warnings here by checking "
      "the -W options you want to use."));

  QGridLayout *grid1 = new QGridLayout(compilerWarnings,13,2,15,7,"grid-f");
  w_all=new QCheckBox(compilerWarnings,"w_all");
  grid1->addWidget(w_all,0,0);
  w_all->setText("-Wall");
  if (cxxflags.contains("-Wall")) {
    w_all->setChecked(true);
  } else {
    w_all->setChecked(false);
  }
  QWhatsThis::add(w_all, i18n("Compile with -Wall. This option "
      "includes several different warning "
      "parameters which are recommended to "
      "turn on."));

  w_=new QCheckBox(compilerWarnings,"w_");
  grid1->addWidget(w_,1,0);
  w_->setText("-W");
  if (cxxflags.contains("-W ")) {
    w_->setChecked(true);
  } else {
    w_->setChecked(false);
  }
  QWhatsThis::add(w_, i18n("Compile with -W. This option "
      "sets options not included in -Wall "
      "which are very specific. Please read "
      "GCC-Info for more information."));

  w_traditional=new QCheckBox(compilerWarnings,"w_traditional");
  grid1->addWidget(w_traditional,2,0);
  w_traditional->setText("-Wtraditional");
  if (cxxflags.contains("-Wtraditional")) {
    w_traditional->setChecked(true);
  } else {
    w_traditional->setChecked(false);
  }
  QWhatsThis::add(w_traditional, i18n("Warn about certain constructs "
        "that behave differently in traditional "
        "and ANSI C."));


  w_undef=new QCheckBox(compilerWarnings,"w_undef");
  grid1->addWidget(w_undef,3,0);
  w_undef->setText("-Wundef");
  if (cxxflags.contains("-Wundef")) {
    w_undef->setChecked(true);
  } else {
    w_undef->setChecked(false);
  }
  QWhatsThis::add(w_undef, i18n("Warn if an undefined identifier is "
        "evaluated in an `#if' directive"));

  w_shadow=new QCheckBox(compilerWarnings,"w_shadow");
  grid1->addWidget(w_shadow,4,0);
  w_shadow->setText("-Wshadow");
  if (cxxflags.contains("-Wshadow")) {
    w_shadow->setChecked(true);
  } else {
    w_shadow->setChecked(false);
  }
  QWhatsThis::add(w_shadow, i18n("Warn whenever a local variable "
        "shadows another local variable."));

  /*  w_id_clash_len=new QCheckBox(compilerWarnings,"w_id_clash_len");
  w_id_clash_len->setGeometry(10,110,230,20);
  w_id_clash_len->setText("-Wid_clash-LEN");
  if (cxxflags.contains("-Wid-clash-LEN")) {
    w_id_clash_len->setChecked(true);
  } else {
    w_id_clash_len->setChecked(false);
  }
  QWhatsThis::add(w_id_clash_len, i18n("Warn whenever two distinct "
        "identifiers match in the first LEN "
        "characters. This may help you prepare "
        "a program that will compile with "
        "certain obsolete, brain-damaged "
        "compilers."));

  w_larger_than_len=new QCheckBox(compilerWarnings,"w_larger_than_len");
  w_larger_than_len->setGeometry(10,130,230,20);
  w_larger_than_len->setText("-Wlarger-than-LEN");
  if (cxxflags.contains("-Wlarger-than-LEN")) {
    w_larger_than_len->setChecked(true);
  } else {
    w_larger_than_len->setChecked(false);
  }
  QWhatsThis::add(w_larger_than_len, i18n("Warn whenever an object "
          "of larger than LEN bytes  "
          "is defined."));
  */

  w_pointer_arith=new QCheckBox(compilerWarnings,"w_pointer_arith");
  grid1->addWidget(w_pointer_arith,5,0);
  w_pointer_arith->setText("-Wpointer-arith");
  if (cxxflags.contains("-Wpointer-arith")) {
    w_pointer_arith->setChecked(true);
  } else {
    w_pointer_arith->setChecked(false);
  }
  QWhatsThis::add(w_pointer_arith, i18n("Warn about anything that "
        "depends on the <i>size of</i> a "
        "function type or of <i>void</i>. "
        "GNU C assigns these types a size of 1, "
        "for convenience in calculations with "
        "<i>void *</i> pointers and pointers "
        "to functions."));


  w_bad_function_cast=new QCheckBox(compilerWarnings,"w_bad_function_cast");
  grid1->addWidget(w_bad_function_cast,6,0);
  w_bad_function_cast->setText("-Wbad-function-cast");
  if (cxxflags.contains("-Wbad-function-cast")) {
    w_bad_function_cast->setChecked(true);
  } else {
    w_bad_function_cast->setChecked(false);
  }
  QWhatsThis::add(w_bad_function_cast, i18n("Warn whenever a function call is "
          "cast to a non-matching type. For "
          "example, warn if <i>int malloc()</i> "
          "is cast to <i>anything *."));


  w_cast_qual=new QCheckBox(compilerWarnings,"w_cast_qual");
  grid1->addWidget(w_cast_qual,7,0);
  w_cast_qual->setText("-Wcast-qual");
  if (cxxflags.contains("-Wcast-qual")) {
    w_cast_qual->setChecked(true);
  } else {
    w_cast_qual->setChecked(false);
  }
  QWhatsThis::add(w_cast_qual, i18n("Warn whenever a pointer is cast "
        "so as to remove a type qualifier "
        "from the target type. For example, "
        "warn if a <i>const char *</i> is "
        "cast to an ordinary <i>char *."));


  w_cast_align=new QCheckBox(compilerWarnings,"w_cast_align");
  grid1->addWidget(w_cast_align,8,0);
  w_cast_align->setText("-Wcast-align");
  if (cxxflags.contains("-Wcast-align")) {
    w_cast_align->setChecked(true);
  } else {
    w_cast_align->setChecked(false);
  }
  QWhatsThis::add(w_cast_align, i18n("Warn whenever a pointer is cast such "
        "that the required alignment of the target "
        "is increased. For example, warn if a "
        "<i>char *</i> is cast to an <i>int *</i> on "
        "machines where integers can only be accessed "
        "at two- or four-byte boundaries."));


  w_write_strings=new QCheckBox(compilerWarnings,"w_write_strings");
  grid1->addWidget(w_write_strings,9,0);
  w_write_strings->setText("-Wwrite-strings");
  if (cxxflags.contains("-Wwrite-strings")) {
    w_write_strings->setChecked(true);
  } else {
    w_write_strings->setChecked(false);
  }
  QWhatsThis::add(w_write_strings,
  i18n("Give string constants the type <i>const char[LENGTH]</i> "
         "so that copying the address of one into a non-<i>const "
         "char *</i> pointer will get a warning. These warnings "
    "will help you find at compile time code that can try to "
    "write into a string constant, but only if you have been "
    "very careful about using <i>const</i> in declarations "
    "and prototypes. Otherwise, it will just be a nuisance; "
    "this is why we did not make <i>-Wall</i> request these "
            "warnings."));


  w_conversion=new QCheckBox(compilerWarnings,"w_conversion");
  grid1->addWidget(w_conversion,10,0);
  w_conversion->setText("-Wconversion");
  if (cxxflags.contains("-Wconversion")) {
    w_conversion->setChecked(true);
  } else {
    w_conversion->setChecked(false);
  }
  QWhatsThis::add(w_conversion,
   i18n("Warn if a prototype causes a type conversion that is different "
    "from what would happen to the same argument in the absence "
    "of a prototype. This includes conversions of fixed point to "
    "floating and vice versa, and conversions changing the width "
    "or signedness of a fixed point argument except when the same "
    "as the default promotion.  Also warn if a negative integer "
    "constant expression is implicitly converted to an unsigned "
    "type."));


  w_sign_compare=new QCheckBox(compilerWarnings,"w_sign_compare");
  grid1->addWidget(w_sign_compare,0,1);
  w_sign_compare->setText("-Wsign-compare");
  if (cxxflags.contains("-Wsign-compare")) {
    w_sign_compare->setChecked(true);
  } else {
    w_sign_compare->setChecked(false);
  }
  QWhatsThis::add(w_sign_compare,
  i18n("Warn when a comparison between signed and unsigned values "
       "could produce an incorrect result when the signed value "
       "is converted to unsigned."));


  w_aggregate_return=new QCheckBox(compilerWarnings,"w_aggregate_return");
  grid1->addWidget(w_aggregate_return,1,1);
  w_aggregate_return->setText("-Waggregate-return");
  if (cxxflags.contains("-Waggregate-return")) {
    w_aggregate_return->setChecked(true);
  } else {
    w_aggregate_return->setChecked(false);
  }
  QWhatsThis::add(w_aggregate_return,
  i18n("Warn if any functions that return structures or unions are "
    "defined or called. (In languages where you can return an "
    "array, this also elicits a warning.)"));


  w_strict_prototypes=new QCheckBox(compilerWarnings,"w_strict_prototypes");
  grid1->addWidget(w_strict_prototypes,2,1);
  w_strict_prototypes->setText("-Wstrict-prototypes");
  if (cxxflags.contains("-Wstrict-prototypes")) {
    w_strict_prototypes->setChecked(true);
  } else {
    w_strict_prototypes->setChecked(false);
  }
  QWhatsThis::add(w_strict_prototypes,
  i18n("Warn if a function is declared or defined without specifying "
    "the argument types. (An old-style function definition is "
    "permitted without a warning if preceded by a declaration "
    "which specifies the argument types.)"));


  w_missing_prototypes=new QCheckBox(compilerWarnings,"w_missing_prototypes");
  grid1->addWidget(w_missing_prototypes,3,1);
  w_missing_prototypes->setText("-Wmissing-prototypes");
  if (cxxflags.contains("-Wmissing-prototypes")) {
    w_missing_prototypes->setChecked(true);
  } else {
    w_missing_prototypes->setChecked(false);
  }
  QWhatsThis::add(w_missing_prototypes,
  i18n("Warn if a global function is defined without a previous "
    "prototype declaration. This warning is issued even if "
    "the definition itself provides a prototype. The aim "
    "is to detect global functions that fail to be declared "
    "in header files."));


  w_missing_declarations=new QCheckBox(compilerWarnings,"w_missing_declarations");
  grid1->addWidget(w_missing_declarations,4,1);
  w_missing_declarations->setText("-Wmissing-declarations");
  if (cxxflags.contains("-Wmissing-declarations")) {
    w_missing_declarations->setChecked(true);
  } else {
    w_missing_declarations->setChecked(false);
  }
  QWhatsThis::add(w_missing_declarations,
  i18n("Warn if a global function is defined without a previous "
    "declaration. Do so even if the definition itself pro- "
    "vides a prototype. Use this option to detect global "
    "functions that are not declared in header files."));


  w_redundant_decls=new QCheckBox(compilerWarnings,"w_redundant_decls");
  grid1->addWidget(w_redundant_decls,5,1);
  w_redundant_decls->setText("-Wredundant-decls");
  if (cxxflags.contains("-Wredundant-decls")) {
    w_redundant_decls->setChecked(true);
  } else {
    w_redundant_decls->setChecked(false);
  }
  QWhatsThis::add(w_redundant_decls,
  i18n("Warn if anything is declared more than once in the same scope "
    "even in cases where multiple declaration is valid and "
    "changes nothing."));


  w_nested_externs=new QCheckBox(compilerWarnings,"w_nested_externs");
  grid1->addWidget(w_nested_externs,6,1);
  w_nested_externs->setText("-Wnested-externs");
  if (cxxflags.contains("-Wnested-externs")) {
    w_nested_externs->setChecked(true);
  } else {
    w_nested_externs->setChecked(false);
  }
  QWhatsThis::add(w_nested_externs,
  i18n("Warn if an <i>extern</i> declaration is "
    "encountered within a function."));


  w_inline=new QCheckBox(compilerWarnings,"w_inline");
  grid1->addWidget(w_inline,7,1);
  w_inline->setText("-Winline");
  if (cxxflags.contains("-Winline")) {
    w_inline->setChecked(true);
  } else {
    w_inline->setChecked(false);
  }
  QWhatsThis::add(w_inline,
  i18n("Warn if a function can not be inlined, and either "
    "it was declared as inline, or else the "
    "<i>-finline-functions</i> option was given."));


  w_old_style_cast=new QCheckBox(compilerWarnings,"w_old_style_cast");
  grid1->addWidget(w_old_style_cast,8,1);
  w_old_style_cast->setText("-Wold-style-cast");
  if (cxxflags.contains("-Wold-style-cast")) {
    w_old_style_cast->setChecked(true);
  } else {
    w_old_style_cast->setChecked(false);
  }
  QWhatsThis::add(w_old_style_cast,
  i18n("Warn if an old-style (C-style) cast is used "
       "within a program"));


  w_overloaded_virtual=new QCheckBox(compilerWarnings,"w_overloaded_virtual");
  grid1->addWidget(w_overloaded_virtual,9,1);
  w_overloaded_virtual->setText("-Woverloaded-virtual");
  if (cxxflags.contains("-Woverloaded-virtual")) {
    w_overloaded_virtual->setChecked(true);
  } else {
    w_overloaded_virtual->setChecked(false);
  }
  QWhatsThis::add(w_overloaded_virtual,
  i18n("Warn when a derived class function declaration may be an "
    "error in defining a virtual function (C++ only). In "
    "a derived class, the definitions of virtual functions "
    "must match the type signature of a virtual function "
    "declared in the base class. With this option, the "
    "compiler warns when you define a function with the same "
    "as a virtual function, but with a type signature that "
    "does not match any declarations from the base class."));


  w_synth=new QCheckBox(compilerWarnings,"w_synth");
  grid1->addWidget(w_synth,10,1);
  w_synth->setText("-Wsynth");
  if (cxxflags.contains("-Wsynth")) {
    w_synth->setChecked(true);
  } else {
    w_synth->setChecked(false);
  }
  QWhatsThis::add(w_synth,
  i18n("Warn when g++'s synthesis behavoir does "
    "not match that of cfront."));


  w_error=new QCheckBox(compilerWarnings,"w_error");
  grid1->addWidget(w_error,12,1);
  w_error->setText(i18n("make all Warnings into errors"));
  if (cxxflags.contains("-Werror")) {
    w_error->setChecked(true);
  } else {
    w_error->setChecked(false);
  }
  QWhatsThis::add(w_error,
  i18n("Make all warnings into errors."));

  if(prj_info->isCustomProject())
    compilerWarnings->setEnabled(false);
}

//
// *************** Linker Options *********************
//
void CPrjOptionsDlg::addLinkerPage()
{
  QString ldflags=prj_info->getLDFLAGS();
  QString ldadd=prj_info->getLDADD();

  old_version = prj_info->getVersion();
  old_ldflags =  ldflags.stripWhiteSpace();
  old_ldadd = ldadd.stripWhiteSpace();
  old_addit_flags = prj_info->getAdditCXXFLAGS().stripWhiteSpace();

  int pos;

  QFrame* linkerOptions = addPage(i18n("Linker Options"),i18n("Linker Options"),
		KGlobal::instance()->iconLoader()->loadIcon( "blockdevice", KIcon::NoGroup, KIcon::SizeMedium ));

  QGridLayout *grid2 = new QGridLayout(linkerOptions,2,1,15,7,"grid-g");
  QWhatsThis::add(linkerOptions, i18n("Set the Linker options and choose the "
      "libraries to add to your project."));
  ldflags = " " + ldflags + " ";
  ldadd = " " + ldadd + " ";
//  KDEBUG1(KDEBUG_INFO,DIALOG,"%s",ldflags.data());
//  KDEBUG1(KDEBUG_INFO,DIALOG,"%s",ldadd.data());
  QGroupBox* ldflags_group;
  ldflags_group=new QGroupBox(linkerOptions,"ldflags_group");
  QGridLayout *grid3 = new QGridLayout(ldflags_group,3,2,15,7,"grid-h");

  ldflags_group->setTitle(i18n("library flags"));
  QWhatsThis::add(ldflags_group, i18n("Set your library flags here."));

  l_remove_symbols=new QCheckBox(ldflags_group,"l_remove_symbols");
  grid3->addMultiCellWidget(l_remove_symbols,0,0,0,1);
  l_remove_symbols->setText(i18n("remove all symbol table and relocation information from the executable"));
  if (ldflags.contains(" -s ")) {
    l_remove_symbols->setChecked(true);
    pos=ldflags.find("-s ");
    ldflags.remove(pos,3);
    // cerr << "-s OK" << endl;
  } else {
    l_remove_symbols->setChecked(false);
  }
  QWhatsThis::add(l_remove_symbols, i18n("If you want to use a debugger, you "
      "should keep those informations in the object files. "
      "It's useless to let the compiler generate debug "
      "informations and to remove it with this option."));

  l_static=new QCheckBox(ldflags_group,"l_static");
  grid3->addMultiCellWidget(l_static,1,1,0,1);

  l_static->setText(i18n("prevent using shared libraries"));
  if (ldflags.contains(" -static ")) {
    l_static->setChecked(true);
    pos=ldflags.find("-static ");
    ldflags.remove(pos,8);
    // cerr << "-static OK" << endl;
  } else {
    l_static->setChecked(false);
  }
  QWhatsThis::add(l_static, i18n("On systems that support dynamic linking, "
         "this prevents linking with the shared libraries. "
         "On other systems, this option has no effect."));

  QLabel* addit_ldflags_label;
  addit_ldflags_label=new QLabel(ldflags_group,"addit_ldflags_label");
  grid3->addWidget(addit_ldflags_label,2,0);
  addit_ldflags_label->setText(i18n("additional flags:"));

  addit_ldflags=new QLineEdit(ldflags_group,"addit_ldflags");
  grid3->addWidget(addit_ldflags,2,1);
  ldflags=ldflags.stripWhiteSpace();
  addit_ldflags->setText(ldflags);
  grid2->addWidget(ldflags_group,0,0);

  QString ldflagsMsg = i18n("Insert other linker options here "
              "to invoke the linker with by setting the "
              "LDFLAGS-environment variable.");
  QWhatsThis::add(addit_ldflags_label, ldflagsMsg);
  QWhatsThis::add(addit_ldflags, ldflagsMsg);

  QGroupBox* libs_group;
  libs_group=new QGroupBox(linkerOptions,"libs_group");

  QGridLayout *grid4 = new QGridLayout(libs_group,5,4,15,7,"grid-i");
  libs_group->setTitle(i18n("libraries"));
  QWhatsThis::add(libs_group, i18n("Choose your libraries here."));

  l_X11=new QCheckBox(libs_group,"l_X11");
  grid4->addWidget(l_X11,0,0);
  l_X11->setText("X11");
  if (ldadd.contains(" -lX11 ")) {
    l_X11->setChecked(true);
    pos=ldadd.find("-lX11 ");
    ldadd.remove(pos,6);
    //cerr << "-lX11 OK" << endl;
  } else {
    l_X11->setChecked(false);
  }
  QWhatsThis::add(l_X11, i18n("X11 basics "));

  l_Xext=new QCheckBox(libs_group,"l_Xext");
  grid4->addWidget(l_Xext,1,0);
  l_Xext->setText("Xext");
  if (ldadd.contains(" -lXext ")) {
    l_Xext->setChecked(true);
    pos=ldadd.find("-lXext ");
    ldadd.remove(pos,7);
    //cerr << "-lXext OK" << endl;
  } else {
    l_Xext->setChecked(false);
  }
  QWhatsThis::add(l_Xext, i18n("X11 extensions "));

  l_qt=new QCheckBox(libs_group,"l_qt");
  grid4->addWidget(l_qt,2,0);
  l_qt->setText("qt");
  if(!(prj_info->isKDE2Project() || prj_info->isQt2Project())){
    if (ldadd.contains(" -lqt ")) {
      l_qt->setChecked(true);
      pos=ldadd.find("-lqt ");
      ldadd.remove(pos,5);
      //cerr << "-lqt OK" << endl;
    }
    else {
      l_qt->setChecked(false);
    }
  }
  else{
    if (ldadd.contains(" $(LIB_QT) ")) {
      l_qt->setChecked(true);
      pos=ldadd.find("$(LIB_QT)");
      ldadd.remove(pos,9);
      //cerr << "-lqt OK" << endl;
    }
    else {
      l_qt->setChecked(false);
    }
  }

  QWhatsThis::add(l_qt, i18n("Qt"));

  l_kdecore=new QCheckBox(libs_group,"l_kdecore");
  grid4->addWidget(l_kdecore,3,0);
  l_kdecore->setText("kdecore");
  if (ldadd.contains(" -lkdecore ")) {
    l_kdecore->setChecked(true);
    pos=ldadd.find("-lkdecore ");
    ldadd.remove(pos,10);
    //cerr << "-lkdecore OK" << endl;
  } else {
    l_kdecore->setChecked(false);
  }
  QWhatsThis::add(l_kdecore, i18n("KDE basics"));

  l_kdeui=new QCheckBox(libs_group,"l_kdeui");
  grid4->addWidget(l_kdeui,0,1);
  l_kdeui->setText("kdeui");
  if (ldadd.contains(" -lkdeui ")) {
    l_kdeui->setChecked(true);
    pos=ldadd.find("-lkdeui ");
    ldadd.remove(pos,8);
    //cerr << "-lkdeui OK" << endl;
  } else {
    l_kdeui->setChecked(false);
  }
  QWhatsThis::add(l_kdeui, i18n("KDE user interface"));

  l_khtml=new QCheckBox(libs_group,"l_khtml");
  grid4->addWidget(l_khtml,1,1);
  l_khtml->setText("khtml");
//  if (ldadd.contains(" -lkhtml -lkimgio -ljpeg -ltiff -lpng -lm -ljscript ")) {
//    l_khtml->setChecked(true);
//    pos=ldadd.find("-lkhtml -lkimgio -ljpeg -ltiff -lpng -lm -ljscript ");
//    ldadd.remove(pos,52);
//    //    cerr << "-htmlw OK" << endl;
//  } else {
  if (ldadd.contains(" -lkhtml ")) {
    l_khtml->setChecked(true);
    pos=ldadd.find("-lkhtml ");
    ldadd.remove(pos,8);
    //    cerr << "-lhtml OK" << endl;
  } else {
    l_khtml->setChecked(false);
  }
  QWhatsThis::add(l_khtml, i18n("KDE HTML widget"));
//         "this includes -lkhtml, -lkimgio, -ljpeg, "
//         "-ltiff, -lpng, -lm, -ljscript."));

  l_kfm=new QCheckBox(libs_group,"l_kfm");
  grid4->addWidget(l_kfm,2,1);
  l_kfm->setText("kfm");
  if (ldadd.contains(" -lkfm ")) {
    l_kfm->setChecked(true);
    pos=ldadd.find("-lkfm ");
    ldadd.remove(pos,6);
    //    cerr << "-lkfm OK" << endl;
  } else {
    l_kfm->setChecked(false);
  }
  QWhatsThis::add(l_kfm, i18n("KDE kfm functionality"));

  l_kfile=new QCheckBox(libs_group,"l_kfile");
  grid4->addWidget(l_kfile,3,1);
  l_kfile->setText("kfile");
  if (ldadd.contains(" -lkfile ")) {
    l_kfile->setChecked(true);
    pos=ldadd.find("-lkfile ");
    ldadd.remove(pos,8);
    //    cerr << "-lkfile OK" << endl;
  } else {
    l_kfile->setChecked(false);
  }
  QWhatsThis::add(l_kfile, i18n("KDE file handling"));

  l_kspell=new QCheckBox(libs_group,"l_kspell");
  grid4->addWidget(l_kspell,0,2);
  l_kspell->setText("kspell");
  if (ldadd.contains(" -lkspell ")) {
    l_kspell->setChecked(true);
    pos=ldadd.find("-lkspell ");
    ldadd.remove(pos,9);
    //    cerr << "-lkspell OK" << endl;
  } else {
    l_kspell->setChecked(false);
  }
  QWhatsThis::add(l_kspell, i18n("KDE Spell checking"));

  l_kab=new QCheckBox(libs_group,"l_kab");
  grid4->addWidget(l_kab,1,2);
  l_kab->setText("kab");
  if (ldadd.contains(" -lkab ")) {
    l_kab->setChecked(true);
    pos=ldadd.find("-lkab ");
    ldadd.remove(pos,6);
    //    cerr << "-lkab OK" << endl;
  } else {
    l_kab->setChecked(false);
  }
  QWhatsThis::add(l_kab, i18n("KDE addressbook"));

  l_math=new QCheckBox(libs_group,"l_math");
  grid4->addWidget (l_math,0,3);
  l_math->setText("math");
  if (l_khtml->isChecked() || ldadd.contains(" -lm ")) {
    l_math->setChecked(true);
    pos=ldadd.find("-lm ");
    if (pos>=0)
     ldadd.remove(pos,4);
    //    cerr << "-lm OK" << endl;
  } else {
    l_math->setChecked(false);
  }
  QWhatsThis::add(l_math, i18n("Math library"));

  QLabel* addit_ldadd_label;
  addit_ldadd_label=new QLabel(libs_group,"addit_ldadd_label");
  grid4->addWidget (addit_ldadd_label,4,0);
  addit_ldadd_label->setText(i18n("additional libraries:"));

  addit_ldadd=new QLineEdit(libs_group,"addit_ldadd");
  grid4->addMultiCellWidget(addit_ldadd,4,4,1,3);
  ldadd=ldadd.stripWhiteSpace();

  addit_ldadd->setText(ldadd);
  QWhatsThis::add(addit_ldadd_label, i18n("Add additional libraries here."));
  QWhatsThis::add(addit_ldadd, i18n("Add additional libraries here."));
  grid2->addWidget(libs_group,1,0);
  if(prj_info->isCustomProject())
    linkerOptions->setEnabled(false);
}

//
// ****************** the Make_Widget ********************
//
void CPrjOptionsDlg::addMakePage()
{
  QFrame* makeOptions = addPage(i18n("Make Options"),i18n("Build Program Settings"),
		KGlobal::instance()->iconLoader()->loadIcon( "make", KIcon::NoGroup, KIcon::SizeMedium ));
  QWhatsThis::add(makeOptions, i18n("This dialog is for setting your make options."));
  QGridLayout *grid1 = new QGridLayout(makeOptions,7,3,15,7, "grid-j");


  m_print_debug_info = new QCheckBox( makeOptions, "m_print_debug_info" );
  grid1->addWidget( m_print_debug_info,0,0);
  m_print_debug_info->setFocusPolicy( QWidget::TabFocus );
  m_print_debug_info->setBackgroundMode( QWidget::PaletteBackground );
  m_print_debug_info->setFontPropagation( QWidget::NoChildren );
  m_print_debug_info->setPalettePropagation( QWidget::NoChildren );
  m_print_debug_info->setText(i18n("Print debug information"));
  m_print_debug_info->setAutoRepeat( FALSE );
//  m_print_debug_info->setAutoResize( FALSE );

  m_cont_after_error = new QCheckBox( makeOptions, "m_cont_after_error" );
  grid1->addWidget(  m_cont_after_error,0,1);
  m_cont_after_error->setFocusPolicy( QWidget::TabFocus );
  m_cont_after_error->setBackgroundMode( QWidget::PaletteBackground );
  m_cont_after_error->setFontPropagation( QWidget::NoChildren );
  m_cont_after_error->setPalettePropagation( QWidget::NoChildren );
  m_cont_after_error->setText(i18n("Continue after errors"));
  m_cont_after_error->setAutoRepeat( FALSE );
//  m_cont_after_error->setAutoResize( FALSE );

  m_print_data_base = new QCheckBox( makeOptions, "m_print_data_base" );
  grid1->addWidget( m_print_data_base,0,2);
  m_print_data_base->setFocusPolicy( QWidget::TabFocus );
  m_print_data_base->setBackgroundMode( QWidget::PaletteBackground );
  m_print_data_base->setFontPropagation( QWidget::NoChildren );
  m_print_data_base->setPalettePropagation( QWidget::NoChildren );
  m_print_data_base->setText(i18n("Print the data base"));
  m_print_data_base->setAutoRepeat( FALSE );
//  m_print_data_base->setAutoResize( FALSE );

  m_env_variables = new QCheckBox( makeOptions, "m_env_variables" );
  grid1->addWidget( m_env_variables,1,0);
  m_env_variables->setFocusPolicy( QWidget::TabFocus );
  m_env_variables->setBackgroundMode( QWidget::PaletteBackground );
  m_env_variables->setFontPropagation( QWidget::NoChildren );
  m_env_variables->setPalettePropagation( QWidget::NoChildren );
  m_env_variables->setText(i18n("Environment variables"));
  m_env_variables->setAutoRepeat( FALSE );
//  m_env_variables->setAutoResize( FALSE );

  m_no_rules = new QCheckBox( makeOptions, "m_no_rules" );
  grid1->addWidget( m_no_rules,1,1);
  m_no_rules->setFocusPolicy( QWidget::TabFocus );
  m_no_rules->setBackgroundMode( QWidget::PaletteBackground );
  m_no_rules->setFontPropagation( QWidget::NoChildren );
  m_no_rules->setPalettePropagation( QWidget::NoChildren );
  m_no_rules->setText(i18n("No built-in rules"));
  m_no_rules->setAutoRepeat( FALSE );
//  m_no_rules->setAutoResize( FALSE );

  m_touch_files = new QCheckBox( makeOptions, "m_touch_files" );
  grid1->addWidget(m_touch_files ,1,2);
  m_touch_files->setFocusPolicy( QWidget::TabFocus );
  m_touch_files->setBackgroundMode( QWidget::PaletteBackground );
  m_touch_files->setFontPropagation( QWidget::NoChildren );
  m_touch_files->setPalettePropagation( QWidget::NoChildren );
  m_touch_files->setText(i18n("Touch files"));
  m_touch_files->setAutoRepeat( FALSE );
//  m_touch_files->setAutoResize( FALSE );

  m_ignor_errors = new QCheckBox( makeOptions, "m_ignor_errors" );
  grid1->addWidget(m_ignor_errors ,2,0);
  m_ignor_errors->setFocusPolicy( QWidget::TabFocus );
  m_ignor_errors->setBackgroundMode( QWidget::PaletteBackground );
  m_ignor_errors->setFontPropagation( QWidget::NoChildren );
  m_ignor_errors->setPalettePropagation( QWidget::NoChildren );
  m_ignor_errors->setText(i18n("Ignore all errors"));
  m_ignor_errors->setAutoRepeat( FALSE );
//  m_ignor_errors->setAutoResize( FALSE );

  m_silent_operation = new QCheckBox( makeOptions, "m_silent_operation" );
  grid1->addWidget(m_silent_operation ,2,1);
  m_silent_operation->setFocusPolicy( QWidget::TabFocus );
  m_silent_operation->setBackgroundMode( QWidget::PaletteBackground );
  m_silent_operation->setFontPropagation( QWidget::NoChildren );
  m_silent_operation->setPalettePropagation( QWidget::NoChildren );
  m_silent_operation->setText(i18n("Silent operation"));
  m_silent_operation->setAutoRepeat( FALSE );
//  m_silent_operation->setAutoResize( FALSE );

  m_print_work_dir = new QCheckBox( makeOptions, "m_print_work_dir" );
  grid1->addWidget( m_print_work_dir ,2,2);
  m_print_work_dir->setFocusPolicy( QWidget::TabFocus );
  m_print_work_dir->setBackgroundMode( QWidget::PaletteBackground );
  m_print_work_dir->setFontPropagation( QWidget::NoChildren );
  m_print_work_dir->setPalettePropagation( QWidget::NoChildren );
  m_print_work_dir->setText(i18n("Print working directory"));
  m_print_work_dir->setAutoRepeat( FALSE );
//  m_print_work_dir->setAutoResize( FALSE );


  QGridLayout *grid2 = new QGridLayout(makeOptions,1,4,15,7, "grid-k");
  grid1->addMultiCellLayout(grid2,3,3,0,2);

  m_job_number_label = new QLabel( makeOptions, "m_job_number_label" );
  grid2->addWidget(m_job_number_label,0,0);
  m_job_number_label->setFocusPolicy( QWidget::NoFocus );
  m_job_number_label->setBackgroundMode( QWidget::PaletteBackground );
  m_job_number_label->setFontPropagation( QWidget::NoChildren );
  m_job_number_label->setPalettePropagation( QWidget::NoChildren );
  m_job_number_label->setText(i18n("job number"));
  m_job_number_label->setAlignment( 289 );
  m_job_number_label->setMargin( -1 );

  m_job_number = new QSpinBox( makeOptions, "m_job_number" );
  grid2->addWidget(m_job_number,0,1);
  m_job_number->setFocusPolicy( QWidget::StrongFocus );
  m_job_number->setBackgroundMode( QWidget::PaletteBackground );
  m_job_number->setFontPropagation( QWidget::NoChildren );
  m_job_number->setPalettePropagation( QWidget::NoChildren );
  m_job_number->setFrameStyle( 50 );
  m_job_number->setLineWidth( 2 );
  m_job_number->setRange( 1, 99 );
  m_job_number->setSteps( 1, 0 );
  m_job_number->setPrefix( "" );
  m_job_number->setSuffix( "" );
  m_job_number->setSpecialValueText( "" );
  m_job_number->setWrapping( FALSE );

  m_rebuild_label = new QLabel( makeOptions, "m_rebuild_label" );
  grid2->addWidget(m_rebuild_label,0,2);
  m_rebuild_label->setFocusPolicy( QWidget::NoFocus );
  m_rebuild_label->setBackgroundMode( QWidget::PaletteBackground );
  m_rebuild_label->setFontPropagation( QWidget::NoChildren );
  m_rebuild_label->setPalettePropagation( QWidget::NoChildren );
  m_rebuild_label->setText(i18n("Rebuild behaviour on run/debug:"));
  m_rebuild_label->setAlignment( 289 );
  m_rebuild_label->setMargin( -1 );

  m_rebuild_combo = new QComboBox( makeOptions, "m_rebuild_label" );
  grid2->addWidget(m_rebuild_combo,0,3);
  m_rebuild_combo->setFocusPolicy( QWidget::NoFocus );
  m_rebuild_combo->setBackgroundMode( QWidget::PaletteBase );
  m_rebuild_combo->setFontPropagation( QWidget::NoChildren );
  m_rebuild_combo->setPalettePropagation( QWidget::NoChildren );
  m_rebuild_combo->insertItem(i18n("warning on modification"));
  m_rebuild_combo->insertItem(i18n("only on modification"));
  m_rebuild_combo->insertItem(i18n("always rebuild"));


  grid2 = new QGridLayout(makeOptions,3,3,15,7,"grid-l");
  grid1->addMultiCellLayout(grid2,4,6,0,2);

  m_set_modify_label = new QLabel( makeOptions, "m_set_modify_label" );
  grid2->addWidget(m_set_modify_label,0,0);
  m_set_modify_label->setFocusPolicy( QWidget::NoFocus );
  m_set_modify_label->setBackgroundMode( QWidget::PaletteBackground );
  m_set_modify_label->setFontPropagation( QWidget::NoChildren );
  m_set_modify_label->setPalettePropagation( QWidget::NoChildren );
  m_set_modify_label->setText(i18n("set modified"));
  m_set_modify_label->setAlignment( 289 );
  m_set_modify_label->setMargin( -1 );

  m_set_modify_line = new QLineEdit( makeOptions, "m_set_modify_line" );
  grid2->addWidget(m_set_modify_line,0,1);
  m_set_modify_line->setFocusPolicy( QWidget::StrongFocus );
  m_set_modify_line->setBackgroundMode( QWidget::PaletteBase );
  m_set_modify_line->setFontPropagation( QWidget::NoChildren );
  m_set_modify_line->setPalettePropagation( QWidget::NoChildren );
  m_set_modify_line->setText("");
  m_set_modify_line->setMaxLength( 32767 );
  m_set_modify_line->setEchoMode( QLineEdit::Normal );
  m_set_modify_line->setFrame( TRUE );

  m_set_modify_dir = new QPushButton( makeOptions, "m_set_modify_dir" );
  grid2->addWidget(m_set_modify_dir,0,2);
  m_set_modify_dir->setFocusPolicy( QWidget::TabFocus );
  m_set_modify_dir->setBackgroundMode( QWidget::PaletteBackground );
  m_set_modify_dir->setFontPropagation( QWidget::NoChildren );
  m_set_modify_dir->setPalettePropagation( QWidget::NoChildren );
  m_set_modify_dir->setPixmap(SmallIcon("fileopen"));
  m_set_modify_dir->setAutoRepeat( FALSE );
//  m_set_modify_dir->setAutoResize( FALSE );

  m_optional_label = new QLabel( makeOptions, "m_optional_label" );
  m_optional_label->setFocusPolicy( QWidget::NoFocus );
  m_optional_label->setBackgroundMode( QWidget::PaletteBackground );
  m_optional_label->setFontPropagation( QWidget::NoChildren );
  m_optional_label->setPalettePropagation( QWidget::NoChildren );
  m_optional_label->setText(i18n("additional options"));
  m_optional_label->setAlignment( 289 );
  m_optional_label->setMargin( -1 );
  grid2->addWidget(m_optional_label,1,0);

  m_optional_line = new QLineEdit( makeOptions, "m_optional_line" );
  grid2->addMultiCellWidget( m_optional_line,1,1,1,2);
  m_optional_line->setFocusPolicy( QWidget::StrongFocus );
  m_optional_line->setBackgroundMode( QWidget::PaletteBase );
  m_optional_line->setFontPropagation( QWidget::NoChildren );
  m_optional_line->setPalettePropagation( QWidget::NoChildren );
  m_optional_line->setText("");
  m_optional_line->setMaxLength( 32767 );
  m_optional_line->setEchoMode( QLineEdit::Normal );
  m_optional_line->setFrame( TRUE );

  m_makestartpoint_label = new QLabel( makeOptions, "m_makestartpoint_label" );
  grid2->addWidget(m_makestartpoint_label,2,0);
  m_makestartpoint_label->setFocusPolicy( QWidget::NoFocus );
  m_makestartpoint_label->setBackgroundMode( QWidget::PaletteBackground );
  m_makestartpoint_label->setFontPropagation( QWidget::NoChildren );
  m_makestartpoint_label->setPalettePropagation( QWidget::NoChildren );
  m_makestartpoint_label->setText(i18n("Run make in"));
  m_makestartpoint_label->setAlignment( 289 );
  m_makestartpoint_label->setMargin( -1 );

  m_makestartpoint_line = new QLineEdit( makeOptions, "m_makestartpoint_line" );
  grid2->addWidget(m_makestartpoint_line,2,1);
  m_makestartpoint_line->setFocusPolicy( QWidget::StrongFocus );
  m_makestartpoint_line->setBackgroundMode( QWidget::PaletteBase );
  m_makestartpoint_line->setFontPropagation( QWidget::NoChildren );
  m_makestartpoint_line->setPalettePropagation( QWidget::NoChildren );
  m_makestartpoint_line->setText("");
  m_makestartpoint_line->setMaxLength( 32767 );
  m_makestartpoint_line->setEchoMode( QLineEdit::Normal );
  m_makestartpoint_line->setFrame( TRUE );

  m_makestartpoint_dir = new QPushButton( makeOptions, "m_makestartpoint_dir" );
  grid2->addWidget(m_makestartpoint_dir,2,2);
  m_makestartpoint_dir->setFocusPolicy( QWidget::TabFocus );
  m_makestartpoint_dir->setBackgroundMode( QWidget::PaletteBackground );
  m_makestartpoint_dir->setFontPropagation( QWidget::NoChildren );
  m_makestartpoint_dir->setPalettePropagation( QWidget::NoChildren );
  m_makestartpoint_dir->setPixmap(SmallIcon("fileopen"));
  m_makestartpoint_dir->setAutoRepeat( FALSE );
//  m_makestartpoint_dir->setAutoResize( FALSE );


  QWhatsThis::add(m_set_modify_dir, i18n("Pressing the folder button lets you choose "
             "a file which will be set modified. This will "
             "recompile the file on the next build run."));

  QWhatsThis::add(m_print_debug_info, i18n("Print  debugging  information  in  addition to normal "
             "processing. The debugging information tells you which "
             "files  are being considered for remaking, which file- "
             "times are being compared and with what results, which "
             "files  actually  need  to  be  remade, which implicit "
             "rules are considered and which  are  applied---everything "
             "interesting about how make decides what to do."));

  QWhatsThis::add(m_optional_label, i18n("Set any other additional options for your make-program here."));
  QWhatsThis::add(m_optional_line, i18n("Set any other additional options for your make-program here."));

  QString m_rebuildMsg = i18n("Set the rebuild type on pressing run/debug here. This allows you to select if you either "
            "want to rebuild always on run/debug or rebuild only on modifications or "
          "give you first a warning on modification of your sources.");
  QWhatsThis::add(m_rebuild_label, m_rebuildMsg);
  QWhatsThis::add(m_rebuild_combo, m_rebuildMsg);

  QWhatsThis::add(m_print_data_base, i18n("Print  the data base (rules and variable values) that "
            "results from reading the makefiles; then  execute  as "
            "usual  or  as  otherwise specified.  This also prints "
            "the version information."));

  QWhatsThis::add(m_no_rules, i18n("Eliminate  use  of the built-in implicit rules.  Also "
           "clear out the default list  of  suffixes  for  suffix "
           "rules."));

  QWhatsThis::add(m_env_variables, i18n("Give variables taken from the environment  precedence "
          "over variables from makefiles."));

  QWhatsThis::add(m_cont_after_error, i18n("Continue  as  much as possible after an error. While "
             "the target that failed, and those that depend on it, "
             "cannot  be  remade, the  other dependencies of these "
             "targets can be processed all the same."));

  QWhatsThis::add(m_touch_files, i18n("Touch  files  (mark  them  up  to date without really "
              "changing them) instead  of  running  their  commands. "
              "This  is used to pretend that the commands were done, "
              "in order to fool future invocations of make."));

  QWhatsThis::add(m_print_work_dir, i18n("Print  a  message  containing  the  working directory "
           "before and after other processing.  This may be  useful "
           "for  tracking down errors from complicated nests "
           "of recursive make commands."));

  QWhatsThis::add(m_silent_operation, i18n("Silent operation; do not print the commands as they "
             "are executed."));

  QWhatsThis::add(m_ignor_errors, i18n("Ignore all errors in commands executed to "
               "remake files."));

  QString m_job_numberMsg = i18n("Specifies the number of jobs (commands) to run "
             "simultaneously. If you have a single CPU-System "
             "it is not recommended to choose more then 2.");
  QWhatsThis::add(m_job_number_label, m_job_numberMsg);
  QWhatsThis::add(m_job_number, m_job_numberMsg);

  QWhatsThis::add(m_set_modify_label, i18n("Pretend that the target file has just been  modified. "
             "It  is  almost the same as running a touch command on "
             "the given file before running make, except  that  the "
             "modification  time is changed only in the imagination "
             "of make."));

  QString m_makestartpointMsg = i18n("Often if you click on 'Build' or 'Make Clean', "
                  "you just want to run a Makefile from a certain subdirectory. "
                  "Specify the location of that Makefile here. "
                  "It will be your new root-Makefile then.");
  QWhatsThis::add(m_makestartpoint_label, m_makestartpointMsg);
  QWhatsThis::add(m_makestartpoint_line, m_makestartpointMsg);

  connect(m_set_modify_dir,SIGNAL(clicked()),SLOT(slotFileDialogClicked()));
  connect(m_makestartpoint_dir,SIGNAL(clicked()),SLOT(slotFileDialogMakeStartPointClicked()));

  settings = KGlobal::config();
  settings->setGroup("MakeOptionsSettings");

  if (settings->readBoolEntry("PrintDebugInfo")) {
    m_print_debug_info->setChecked(TRUE);
  }
   else m_print_debug_info->setChecked(FALSE);

   if (settings->readBoolEntry("PrintDataBase")) {
     m_print_data_base->setChecked(TRUE);
  }
  else m_print_data_base->setChecked(FALSE);

  if (settings->readBoolEntry("NoRules")) {
    m_no_rules->setChecked(TRUE);
  }
  else m_no_rules->setChecked(FALSE);

  if (settings->readBoolEntry("EnvVariables")) {
    m_env_variables->setChecked(TRUE);
  }
  else m_env_variables->setChecked(FALSE);

  if (settings->readBoolEntry("ContAfterError")) {
    m_cont_after_error->setChecked(TRUE);
  }
  else m_cont_after_error->setChecked(FALSE);

  if (settings->readBoolEntry("TouchFiles")) {
    m_touch_files->setChecked(TRUE);
  }
  else m_touch_files->setChecked(FALSE);

  if (settings->readBoolEntry("PrintWorkDir")){
    m_print_work_dir->setChecked(TRUE);
  }
  else m_print_work_dir->setChecked(FALSE);

  if (settings->readBoolEntry("SilentOperation")) {
    m_silent_operation->setChecked(TRUE);
  }
  else m_silent_operation->setChecked(FALSE);

  if (settings->readBoolEntry("IgnorErrors")) {
    m_ignor_errors->setChecked(TRUE);
  }
  else m_ignor_errors->setChecked(FALSE);

  m_set_modify_line->setText(settings->readEntry("SetModifyLine"));
  m_optional_line->setText(settings->readEntry("OptionalLine"));
  m_job_number->setValue(settings->readNumEntry ("JobNumber"));

  m_rebuild_combo->setCurrentItem(settings->readNumEntry("RebuildType", 2));
  
  m_makestartpoint_line->setText(
        prj_info->getDirWhereMakeWillBeCalled(prj_info->getProjectDir()));

  if (!prj_info->isCustomProject())
  {
    m_makestartpoint_line->setEnabled(false);
    m_makestartpoint_dir->setEnabled(false);
  }
}

//
//************************** binary selection *************************
//
void CPrjOptionsDlg::addBinPage()
{
  QFrame* binaryOptions = addPage(i18n("Binary"),  i18n("Binary Program to start"),
                                  KGlobal::instance()->iconLoader()->loadIcon( "exec",
                                  KIcon::NoGroup, KIcon::SizeMedium ));

  QGridLayout *grid = new QGridLayout( binaryOptions ,3,2,15,7,"grid-m");

  QGroupBox* binary_box= new QGroupBox(binaryOptions,"binary_box");
  grid->addWidget(binary_box,0,0);
  binary_box->setTitle(i18n("Binary details"));

  QGridLayout *grid1 = new QGridLayout( binary_box ,2,2,15,7,"grid-n");
  QLabel* binary = new QLabel(binary_box,"binary_label");
  binary->setText(i18n("Path and Filename of binary:"));
  grid1->addWidget(binary,0,0);
  binary_edit= new QLineEdit(binary_box,"binary_edit");
  grid1->addWidget(binary_edit,1,0);

//  QString underDir=prj_info->pathToBinPROGRAM();
//  if (underDir.isEmpty())
//  {
//    underDir = prj_info->getProjectDir() + prj_info->getSubDir();
//    if (underDir[0] == '/')
//      underDir = CToolClass::getRelativePath(prj_info->getProjectDir(), underDir);
//
//  }
//  if (underDir.right(1)!="/")
//    underDir+="/";

  QString underDir=prj_info->getExecutableDir();
  binary_edit->setText(underDir+prj_info->getBinPROGRAM());

  QPushButton* binary_button= new QPushButton(binary_box,"binary_button");
  binary_button->setPixmap(SmallIcon("fileopen"));
  grid1->addWidget(binary_button,1,1);

  QString binaryMsg = i18n("Set the path and filename of the binary that will be started on Run or Debug. "
            "Hint: Use a relative path starting from your project base directory to be location independent.");
  QWhatsThis::add(binary_edit, binaryMsg);
  QWhatsThis::add(binary_button, binaryMsg);
  QWhatsThis::add(binary, binaryMsg);

  connect(binary_button,SIGNAL(clicked()),SLOT(slotBinaryClicked()));

// **************************************************************************

  QGroupBox* libtool_box= new QGroupBox(binaryOptions,"libtool_box");
  grid->addWidget(libtool_box,2,0);
  libtool_box->setTitle(i18n("Libtool details"));

  QGridLayout *grid2 = new QGridLayout( libtool_box ,2,2,15,7,"grid-o");
  QLabel* libtool = new QLabel(libtool_box,"libtool_label");
  libtool->setText(i18n("Path of libtool:"));
  grid2->addWidget(libtool,0,0);
  libtool_edit= new QLineEdit(libtool_box,"libtool_edit");
  grid2->addWidget(libtool_edit,1,0);

  QString libtoolDir = prj_info->getLibtoolDir();
//  if (libtoolDir.isEmpty())
//  {
//    libtoolDir = prj_info->getProjectDir() + prj_info->getSubDir();
//    if (libtoolDir[0] == '/')
//      libtoolDir = CToolClass::getRelativePath(prj_info->getProjectDir(), libtoolDir);
//  }
//
//  if (libtoolDir.right(1)!="/")
//    libtoolDir+="/";

  libtool_edit->setText(libtoolDir);

  QPushButton* libtool_button= new QPushButton(libtool_box,"libtool_button");
  libtool_button->setPixmap(SmallIcon("fileopen"));
  grid2->addWidget(libtool_button,1,1);

  QString libtoolMsg = i18n("Set the path where the libtool is that wraps the executable\n"
                            "when started on Run or Debug. Hint: Use a relative path starting\n"
                            "from your project base directory to be location independent.");
  QWhatsThis::add(libtool_edit, libtoolMsg);
  QWhatsThis::add(libtool_button, libtoolMsg);
  QWhatsThis::add(libtool, libtoolMsg);

  connect(libtool_button,SIGNAL(clicked()),SLOT(slotLibtoolClicked()));

// **************************************************************************

  if (!prj_info->isCustomProject())
  {
    binaryOptions->setEnabled(false);
  }
}

void CPrjOptionsDlg::slotOptimize_level_changed(int v) {

  if (v>3) {
    optimize_level->setValue(3);
  }
  if (v<1) {
    optimize_level->setValue(1);
  }
}

void CPrjOptionsDlg::slotDebug_level_changed(int v) {

    if (v>3) {
  debug_level->setValue(3);
    }
    if (v<1) {
  debug_level->setValue(1);
    }

}

void CPrjOptionsDlg::ok(){

  QString text,text2;
  QStrList short_info;
  int i,n;


  if (optimize_level->value()>3) {
      optimize_level->setValue(3);
  }
  if (optimize_level->value()<1) {
      optimize_level->setValue(1);
  }
  if (debug_level->value()>3) {
      debug_level->setValue(3);
  }
  if (debug_level->value()<1) {
      debug_level->setValue(1);
  }


  //*********general******************
  text = prjname_edit->text();
  prj_info->setProjectName(text);
  text = author_edit->text();
  prj_info->setAuthor(text);
  text = email_edit->text();
  prj_info->setEmail(text);
  text = version_edit->text();
  prj_info->setVersion(text);
  n=info_edit->numLines();
  for (i=0;(i!=n);i++) {
    text=info_edit->textLine(i);
    short_info.append(text);
  }
  prj_info->setShortInfo(short_info);
  text="";

  QString vcsystem = vcsystem_combo->currentText();
  prj_info->setVCSystem(vcsystem == i18n("None") ? QString("None") : vcsystem);

  prj_info->setModifyMakefiles(modifymakefiles_checkbox->isChecked());

  //********gcc-options***************
  if (target->currentItem()) {
    text=" -b "+QString(target->currentText());
  }
  // if (syntax_check->isChecked()) {
//     text+=" -fsyntax-only";
//   }
  if (optimize->isChecked()) {
    text2.setNum(optimize_level->value());
    text+=" -O"+text2;
  } else {
    text+=" -O0";
  }
  if (debug->isChecked()) {
    text2.setNum(debug_level->value());
    text+=" -g"+text2;
  }
  if (gprof_info->isChecked()) {
    text+=" -pg";
  }
  if (save_temps->isChecked()) {
    text+=" -save-temps";
  }
  prj_info->setAdditCXXFLAGS(addit_gcc_options->text());
  if(old_addit_flags != prj_info->getAdditCXXFLAGS().stripWhiteSpace()){
    need_makefile_generation = true;
  }
  //***********gcc-warnings***********
  if (w_all->isChecked()) {
    text+=" -Wall";
  }
  if (w_->isChecked()) {
    text+=" -W ";
  }
  if (w_traditional->isChecked()) {
    text+=" -Wtraditional";
  }
  if (w_undef->isChecked()) {
    text+=" -Wundef";
  }
  if (w_shadow->isChecked()) {
    text+=" -Wshadow";
  }
  //  if (w_id_clash_len->isChecked()) {
  //    text+=" -Wid-clash-LEN";
  //  }
  //  if (w_larger_than_len->isChecked()) {
  //    text+=" -Wlarger-than-LEN";
  //  }
  if (w_pointer_arith->isChecked()) {
    text+=" -Wpointer-arith";
  }
  if (w_bad_function_cast->isChecked()) {
    text+=" -Wbad-function-cast";
  }
  if (w_cast_qual->isChecked()) {
    text+=" -Wcast-qual";
  }
  if (w_cast_align->isChecked()) {
    text+=" -Wcast-align";
  }
  if (w_write_strings->isChecked()) {
    text+=" -Wwrite-strings";
  }
  if (w_conversion->isChecked()) {
    text+=" -Wconversion";
  }
  if (w_sign_compare->isChecked()) {
    text+=" -Wsign-compare";
  }
  if (w_aggregate_return->isChecked()) {
    text+=" -Waggregate-return";
  }
  if (w_strict_prototypes->isChecked()) {
    text+=" -Wstrict-prototypes";
  }
  if (w_missing_prototypes->isChecked()) {
    text+=" -Wmissing-prototypes";
  }
  if (w_missing_declarations->isChecked()) {
    text+=" -Wmissing-declarations";
  }
  if (w_redundant_decls->isChecked()) {
    text+=" -Wredundant-decls";
  }
  if (w_nested_externs->isChecked()) {
    text+=" -Wnested-externs";
  }
  if (w_inline->isChecked()) {
    text+=" -Winline";
  }
  if (w_old_style_cast->isChecked()) {
    text+=" -Wold-style-cast";
  }
  if (w_overloaded_virtual->isChecked()) {
    text+=" -Woverloaded-virtual";
  }
  if (w_synth->isChecked()) {
    text+=" -Wsynth";
  }
  if (w_error->isChecked()) {
    text+=" -Werror";
  }
  prj_info->setCXXFLAGS(text);
  if(old_cxxflags !=  prj_info->getCXXFLAGS().stripWhiteSpace()){
    need_makefile_generation = true;
  }
  //**********linker options*************
  text=addit_ldflags->text();

  if (l_remove_symbols->isChecked()) {
    text+=" -s ";
  }
  if (l_static->isChecked()) {
    text+=" -static";
  }
  prj_info->setLDFLAGS(text);
  if(old_ldflags != prj_info->getLDFLAGS().stripWhiteSpace()){
    need_makefile_generation = true;
  }

  text= addit_ldadd->text();

  if (l_math->isChecked() && !l_khtml->isChecked()) {
    text+=" -lm";
  }
  if (l_kab->isChecked()) {
    text+=" -lkab";
  }
  if (l_kspell->isChecked()) {
    text+=" -lkspell";
  }
  if (l_kfile->isChecked()) {
    text+=" -lkfile";
  }
  if (l_kfm->isChecked()) {
    text+=" -lkfm";
  }
  if (l_khtml->isChecked()) {
//    text+=" -lkhtml -lkimgio -ljpeg -ltiff -lpng -lm -ljscript";
    text+=" -lkhtml";
  }
  if (l_kdeui->isChecked()) {
    text+=" -lkdeui";
  }
  if (l_kdecore->isChecked()) {
    text+=" -lkdecore";
  }
  if (l_qt->isChecked()) {
   if(!(prj_info->isKDE2Project() || prj_info->isQt2Project()))
    text+=" -lqt";
   else
    text+=" $(LIB_QT)";
  }
  if (l_Xext->isChecked()) {
    text+=" -lXext";
  }
  if (l_X11->isChecked()) {
    text+=" -lX11";
  }
  prj_info->setLDADD(text);
  if(old_ldadd != prj_info->getLDADD().stripWhiteSpace()){
    need_makefile_generation = true;
  }

  //**********make options*************
  settings = KGlobal::config();
  settings->setGroup("MakeOptionsSettings");

  if (m_print_debug_info->isChecked()) {
    settings->writeEntry("PrintDebugInfo",TRUE);
  }
  else settings->writeEntry("PrintDebugInfo",FALSE);

  if (m_print_data_base->isChecked()) {
    settings->writeEntry("PrintDataBase",TRUE);
  }
  else settings->writeEntry("PrintDataBase",FALSE);

  if (m_no_rules->isChecked()) {
    settings->writeEntry("NoRules",TRUE);
  }
  else settings->writeEntry("NoRules",FALSE);

  if (m_env_variables->isChecked()) {
    settings->writeEntry("EnvVariables",TRUE);
  }
  else settings->writeEntry("EnvVariables",FALSE);

  if (m_cont_after_error->isChecked()) {
    settings->writeEntry("ContAfterError",TRUE);
  }
  else settings->writeEntry("ContAfterError",FALSE);

  if (m_touch_files->isChecked()) {
    settings->writeEntry("TouchFiles", TRUE);
  }
  else settings->writeEntry("TouchFiles", FALSE);

  if (m_print_work_dir->isChecked()){
    settings->writeEntry("PrintWorkDir", TRUE);
  }
  else settings->writeEntry("PrintWorkDir", FALSE);

  if (m_silent_operation->isChecked()) {
    settings->writeEntry("SilentOperation", TRUE);
  }
  else settings->writeEntry("SilentOperation", FALSE);

  if (m_ignor_errors->isChecked()) {
    settings->writeEntry("IgnorErrors", TRUE);
  }
  else settings->writeEntry("IgnorErrors", FALSE);

  settings->writeEntry("RebuildType", m_rebuild_combo->currentItem());
  settings->writeEntry("SetModifyLine", m_set_modify_line->text());
  settings->writeEntry("OptionalLine", m_optional_line->text());
  settings->writeEntry("JobNumber", m_job_number->text());  
  settings->sync();
  // reject();  

  text = "";

  if (m_print_debug_info->isChecked()) {
    text+=" -d";
  }
  if (m_print_data_base->isChecked()) {
    text+=" -p";
  }

  if (m_no_rules->isChecked()) {
    text+=" -r";
  }
  if (m_env_variables->isChecked()) {
    text+=" -e";
  }
  if (m_cont_after_error->isChecked()) {
    text+=" -k";
  }
  if (m_touch_files->isChecked()) {
    text+=" -t";
  }
  if (m_print_work_dir->isChecked()) {
    text+=" -w";
  }
  if (m_silent_operation->isChecked()) {
    text+=" -s";
  }
  if (m_ignor_errors->isChecked()) {
    text+=" -i";
  }

  text+=" -j";
  text+= m_job_number->text();

  QString *str1 = new QString(m_set_modify_line->text());
  if (!str1->isEmpty()) {
    text+=" -W";
    text+= m_set_modify_line->text();
  }
  delete (str1);

  QString *str2 = new QString(m_optional_line->text());
  if (!str2->isEmpty()) {
    text+=" ";
    text+= m_optional_line->text();
  }
  delete (str2);

  prj_info->setMakeOptions (text);

  /******** sort out the make directory *********/

  if (prj_info->isCustomProject())
  {
    QString makeDir = m_makestartpoint_line->text();
    if (!makeDir.isEmpty())
    {
      if (makeDir == prj_info->getProjectDir())
        makeDir = "./";
      else
      {
        // Make sure that the user is aware of the consequences of setting an
        // absolute directory path here.
        if (makeDir[0] == '/')
        {
          if (KMessageBox::warningYesNo(this,
                      i18n("The path\n\n") + makeDir +
                        i18n("\n\nwhich you set as directory where make should\n"
                              "run is not a relative path.\nThis can cause problems,\n"
                              "if you move the project. What path do you want to save\n"
                              "to your project file?"),
                      i18n("Path decision"),
                      i18n("&Relative path"),
                      i18n("&Absolute path")) == KMessageBox::Yes)
          {
            makeDir = CToolClass::getRelativePath(prj_info->getProjectDir(), makeDir);
          }
        }
      }
    }

    prj_info->setDirWhereMakeWillBeCalled(makeDir);
  }

  /******** binary options *********/
  if (prj_info->isCustomProject())
  {
    QString binaryPath = binary_edit->text();
    if (!binaryPath.isEmpty())
    {
      if (binaryPath[0] == '/')
      {
        if(KMessageBox::warningYesNo(this,
                  i18n("The path\n\n") + binaryPath +
                  i18n("\n\nto your binary which should be run on 'Execute'\n"
                        "is not a relative path. This can cause problems,\n"
                        "if you move the project. What path do you want to\n"
                        "save to your project file?"),
                  i18n("Path decision"),
                  i18n("&Relative path"),
                  i18n("&Absolute path")) == KMessageBox::Yes)
        {
          binaryPath = CToolClass::getRelativePath(prj_info->getProjectDir(), binaryPath);
        }
      }
    }

    // split the path into dir and program
    QFileInfo fileInfo( binaryPath );

    prj_info->setBinPROGRAM( fileInfo.fileName() );
    prj_info->setPathToBinPROGRAM( fileInfo.dirPath() );
  }

  if (prj_info->isCustomProject())
  {
    QString libtoolDir = libtool_edit->text();
    prj_info->setLibtoolDir(libtoolDir);
  }

  /***************************************/

  // write it to the disk
  prj_info->writeProject();
  if (version_edit->text() != old_version){
    need_configure_in_update = true;
  }
}

// connection to set_modify_dir
void CPrjOptionsDlg::slotFileDialogClicked() {
  QString file,dir;
  dir = prj_info->getProjectDir();
  file = KFileDialog::getOpenFileName(dir,"*",0,"File");
  m_set_modify_line->setText(file);
}


bool CPrjOptionsDlg::needConfigureInUpdate(){
  return  need_configure_in_update;
}


void CPrjOptionsDlg::slotBinaryClicked(){
  QString dir;
  dir = KFileDialog::getOpenFileName(prj_info->getBinPROGRAM());
  if (!dir.isEmpty()){
    bool isRelativePath = false;
    if (dir.length() > 0)
      if ('/' != dir[0])
        isRelativePath = true;
    if (!isRelativePath) {
      if(KMessageBox::warningYesNo(this,
                              i18n("The path\n\n") + dir +
                                i18n("\n\nto your binary which should be run on\n"
                                "'Execute' is not a relative path. This can cause\n"
                                "problems, if you move the project. What path do\n"
                                "you want to save to your project file?"),
                              i18n("Path decision"),
                              i18n("&Relative path"),
                              i18n("&Absolute path")) == KMessageBox::Yes)
        dir = CToolClass::getRelativePath(prj_info->getProjectDir(), dir);
    }
    binary_edit->setText(dir);
  }
}

void CPrjOptionsDlg::slotLibtoolClicked()
{
  QString dir;
  dir = KFileDialog::getExistingDirectory(prj_info->getLibtoolDir());
  if (!dir.isEmpty())
  {
    bool isRelativePath = false;
    if (dir.length() > 0)
      if ('/' != dir[0])
        isRelativePath = true;

    if (!isRelativePath)
    {
      if(KMessageBox::warningYesNo(this,
                              i18n("The path\n\n") + dir +
                                i18n("\n\nto the libtool script which should be run on\n"
                                "'Execute' is not a relative path. Often this is a good\n"
                                "idea, but it may cause problems if you move the project.\n"
                                "What path do you want to save to your project file?"),
                              i18n("Path decision"),
                              i18n("&Relative path"),
                              i18n("&Absolute path")) == KMessageBox::Yes)
      {
        dir = CToolClass::getRelativePath(prj_info->getProjectDir(), dir);
      }
    }
    libtool_edit->setText(dir);
  }
}

void CPrjOptionsDlg::slotFileDialogMakeStartPointClicked() {
  QString file,dir;
  dir = prj_info->getProjectDir();
  file = KFileDialog::getExistingDirectory(dir,0,"Dir");
  if (!file.isEmpty()){
    bool isRelativePath = false;
    if (file.length() > 0)
      if ('/' != dir[0])
        isRelativePath = true;
    if (!isRelativePath) {
      if(KMessageBox::warningYesNo(this,
                              i18n("The path\n\n") + file +
                              i18n("\n\nwhich you set as directory where make should\n"
                                "run is not a relative path. This can cause problems,\n"
                                "if you move the project. What path do you want\n"
                                "to save to your project file?"),
                              i18n("Path decision"),
                              i18n("&Relative path"),
                              i18n("&Absolute path")) == KMessageBox::Yes)
        file = CToolClass::getRelativePath(prj_info->getProjectDir(), file);
    }
    m_makestartpoint_line->setText(file);
  }
}
#include "cprjoptionsdlg.moc"
