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
#include "cprjcompopts.h"
#include "cprjaddopts.h"

#include "cproject.h"
#include "ctoolclass.h"
#include "debug.h"
#include "vc/versioncontrol.h"
#include "kdevsession.h"

#include <kapp.h>
#include <kiconloader.h>
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
#include <qregexp.h>
#include <qstrlist.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <iostream>
using namespace std;

// OPTIONS DIALOG
CPrjOptionsDlg::CPrjOptionsDlg(CProject* prj,KDevSession* session,const QString& curr, QWidget *parent, const char *name)
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
	sess=session;
	currentcfg=curr;

  configureIn.setConfDir(prj->getProjectDir());
  need_configure_in_update = false;
  need_makefile_generation = false;


  addGeneralPage();
  addAdditionalOptionsPage();
  addCompilerOptionsPage();
//  addCompilerWarningsPage();
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
  old_name=prj_info->getProjectName();
  prjname_edit->setText( old_name );
  QWhatsThis::add(prjname_label, i18n("Set the project name here."));
  QWhatsThis::add(prjname_edit, i18n("Set the project name here."));


  QLabel* version_label;
  version_label = new QLabel( generalPage, "version_label" );
  grid->addWidget(version_label,0,2);
  version_label->setText( i18n("Version:") );

  version_edit = new QLineEdit( generalPage, "version_edit" );
  grid->addWidget(version_edit,1,2);

  old_version = prj_info->getVersion();
  version_edit->setText( old_version );
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
//****************** the additional options page ********************
//
void CPrjOptionsDlg::addAdditionalOptionsPage()
{
  QFrame* additionalPage = addPage(i18n("Configure-Settings"),i18n("\"configure\" Settings"),
  KGlobal::instance()->iconLoader()->loadIcon( "configure", KIcon::NoGroup, KIcon::SizeMedium ));
  QGridLayout *grid = new QGridLayout(additionalPage);
  QWhatsThis::add(additionalPage, i18n("Set some configure options of your project here."));
  addOptsDlg = new CPrjAddOpts(&configureIn, additionalPage);
  grid->addWidget(addOptsDlg,0,0);
}
//
// *************** Compiler options *********************
//
void CPrjOptionsDlg::addCompilerOptionsPage()
{

  QFrame* compilerOptions = addPage(i18n("Compiler Options"),i18n("Compiler Options and"
	" Compiler Environment Configuration"),KGlobal::instance()->iconLoader()->loadIcon( "pipe", KIcon::NoGroup, KIcon::SizeMedium ));

  QWhatsThis::add(compilerOptions, i18n("Set your Compiler options here"));
  compdlg = new CPrjCompOpts(prj_info, sess,currentcfg, compilerOptions);
  QGridLayout *grid = new QGridLayout(compilerOptions);
  grid->addWidget(compdlg,0,0);

  if(prj_info->isCustomProject())
    compilerOptions->setEnabled(false);
}

//
// *************** Compiler Warnings *********************
//
void CPrjOptionsDlg::addCompilerWarningsPage()
{
//  if(prj_info->isCustomProject())
//    compilerWarnings->setEnabled(false);
}

//
// *************** Linker Options *********************
//
void CPrjOptionsDlg::addLinkerPage()
{
//  QString ldflags=prj_info->getLDFLAGS();
  QString ldadd=prj_info->getLDADD();

//  old_ldflags =  ldflags.stripWhiteSpace();
  old_ldadd = ldadd.stripWhiteSpace();
//  old_addit_flags = prj_info->getAdditCXXFLAGS().stripWhiteSpace();

  QFrame* linkerOptions = addPage(i18n("Linker Options"),i18n("Linker Options"),
  KGlobal::instance()->iconLoader()->loadIcon( "blockdevice", KIcon::NoGroup, KIcon::SizeMedium ));

  QGridLayout *grid2 = new QGridLayout(linkerOptions,2,1,15,7,"grid-g");
  QWhatsThis::add(linkerOptions, i18n("Set the Linker options and choose the "
      "libraries to add to your project."));
//  ldflags = " " + ldflags + " ";
  ldadd = " " + ldadd + " ";
//  QGroupBox* ldflags_group;
//  ldflags_group=new QGroupBox(linkerOptions,"ldflags_group");
//  QGridLayout *grid3 = new QGridLayout(ldflags_group,3,2,15,7,"grid-h");
//
//  ldflags_group->setTitle(i18n("library flags"));
//  QWhatsThis::add(ldflags_group, i18n("Set your library flags here."));
//
//  l_remove_symbols=new QCheckBox(ldflags_group,"l_remove_symbols");
//  grid3->addMultiCellWidget(l_remove_symbols,0,0,0,1);
//  l_remove_symbols->setText(i18n("remove all symbol table and relocation information from the executable"));
//  l_remove_symbols->setChecked(ldflags.find(" -s ") != -1);
//  if (l_remove_symbols->isChecked())
//    ldadd = ldadd.replace( QRegExp("-s "), "" );
//  QWhatsThis::add(l_remove_symbols, i18n("If you want to use a debugger, you "
//      "should keep those informations in the object files. "
//      "It's useless to let the compiler generate debug "
//      "informations and to remove it with this option."));
//
//  l_static=new QCheckBox(ldflags_group,"l_static");
//  grid3->addMultiCellWidget(l_static,1,1,0,1);
//
//  l_static->setText(i18n("prevent using shared libraries"));
//  l_static->setChecked(ldflags.find(" -static ") != -1);
//  if (l_static->isChecked())
//    ldadd = ldadd.replace( QRegExp(" -static"), "" );
//  QWhatsThis::add(l_static, i18n("On systems that support dynamic linking, "
//         "this prevents linking with the shared libraries. "
//         "On other systems, this option has no effect."));
//
//  QLabel* addit_ldflags_label;
//  addit_ldflags_label=new QLabel(ldflags_group,"addit_ldflags_label");
//  grid3->addWidget(addit_ldflags_label,2,0);
//  addit_ldflags_label->setText(i18n("additional flags:"));

//  addit_ldflags=new QLineEdit(ldflags_group,"addit_ldflags");
//  grid3->addWidget(addit_ldflags,2,1);
//  ldflags=ldflags.stripWhiteSpace();
//  addit_ldflags->setText(ldflags);
//  grid2->addWidget(ldflags_group,0,0);

//  QString ldflagsMsg = i18n("Insert other linker options here "
//              "to invoke the linker with by setting the "
//              "LDFLAGS-environment variable.");
//  QWhatsThis::add(addit_ldflags_label, ldflagsMsg);
//  QWhatsThis::add(addit_ldflags, ldflagsMsg);

  QGroupBox* libs_group;
  libs_group=new QGroupBox(linkerOptions,"libs_group");

  QGridLayout *grid4 = new QGridLayout(libs_group,5,4,15,7,"grid-i");
  libs_group->setTitle(i18n("Libraries"));
  QWhatsThis::add(libs_group, i18n("Choose your libraries here."));

  l_X11=new QCheckBox(libs_group,"l_X11");
  grid4->addWidget(l_X11,0,0);
  l_X11->setText("X11");
  l_X11->setChecked(ldadd.find(" -lX11 ") != -1);
  if (l_X11->isChecked())
    ldadd = ldadd.replace( QRegExp(" -lX11"), "" );
  QWhatsThis::add(l_X11, i18n("X11 basics "));

  l_Xext=new QCheckBox(libs_group,"l_Xext");
  grid4->addWidget(l_Xext,1,0);
  l_Xext->setText("Xext");
  l_Xext->setChecked(ldadd.find(" -lXext ") != -1);
  if (l_Xext->isChecked())
    ldadd = ldadd.replace( QRegExp(" -lXext"), "" );
  QWhatsThis::add(l_Xext, i18n("X11 extensions "));

  l_qt=new QCheckBox(libs_group,"l_qt");
  grid4->addWidget(l_qt,2,0);
  l_qt->setText("qt");
  l_qt->setChecked((ldadd.find(" -lqt ") != -1) || (ldadd.find(" $(LIB_QT) ") != -1));
  if (l_qt->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lqt"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_QT\\)"), "" );
  }
  QWhatsThis::add(l_qt, i18n("Qt"));

  l_kdecore=new QCheckBox(libs_group,"l_kdecore");
  grid4->addWidget(l_kdecore,3,0);
  l_kdecore->setText("kdecore");
  l_kdecore->setChecked((ldadd.find(" -lkdecore ")  != -1) || (ldadd.find(" $(LIB_KDECORE) ") != -1));
  if (l_kdecore->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lkdecore"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_KDECORE\\)"), "" );
  }
  QWhatsThis::add(l_kdecore, i18n("KDE basics"));

  l_kdeui=new QCheckBox(libs_group,"l_kdeui");
  grid4->addWidget(l_kdeui,0,1);
  l_kdeui->setText("kdeui");
  l_kdeui->setChecked((ldadd.find(" -lkdeui ")  != -1)|| (ldadd.find(" $(LIB_KDEUI) ") != -1));
  if (l_kdeui->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lkdeui"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_KDEUI\\)"), "" );
  }
  QWhatsThis::add(l_kdeui, i18n("KDE user interface"));

  l_khtml=new QCheckBox(libs_group,"l_khtml");
  grid4->addWidget(l_khtml,1,1);
  l_khtml->setText("khtml");
  l_khtml->setChecked((ldadd.find(" -lkhtml ")  != -1) || (ldadd.find(" $(LIB_KHTML) ") != -1));
  if (l_khtml->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lkhtml"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_KHTML\\)"), "" );
  }
  QWhatsThis::add(l_khtml, i18n("KDE HTML widget"));

  l_kfm=new QCheckBox(libs_group,"l_kfm");
  grid4->addWidget(l_kfm,2,1);
  l_kfm->setText("kfm");
  l_kfm->setChecked(ldadd.find(" -lkfm ") != -1);
  if (l_kfm->isChecked())
    ldadd = ldadd.replace( QRegExp(" -lkfm"), "" );
  // TODO add note that this is KDE-1 only, or remove checkbox if project isn't a KDE-1 project
  QWhatsThis::add(l_kfm, i18n("KDE kfm functionality"));

  l_kfile=new QCheckBox(libs_group,"l_kfile");
  grid4->addWidget(l_kfile,3,1);
  l_kfile->setText("kfile");
  l_kfile->setChecked((ldadd.find(" -lkfile ") != -1) || (ldadd.find(" $(LIB_KFILE) ") != -1));
  if (l_kfile->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lkfile"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_KFILE\\)"), "" );
  }
  QWhatsThis::add(l_kfile, i18n("KDE file handling"));

  l_kparts=new QCheckBox(libs_group,"l_kparts");
  grid4->addWidget(l_kparts,3,2);
  l_kparts->setText("kparts");
  l_kparts->setChecked((ldadd.find(" -lkparts ") != -1) || (ldadd.find(" $(LIB_KPARTS) ") != -1));
  if (l_kparts->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lkparts"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_KPARTS\\)"), "" );
  }
  QWhatsThis::add(l_kparts, i18n("KDE component architecture"));

  l_kspell=new QCheckBox(libs_group,"l_kspell");
  grid4->addWidget(l_kspell,0,2);
  l_kspell->setText("kspell");
  l_kspell->setChecked((ldadd.find(" -lkspell ") != -1) || (ldadd.find(" $(LIB_KSPELL) ") != -1));
  if (l_kspell->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lkspell"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_KSPELL\\)"), "" );
  }
  QWhatsThis::add(l_kspell, i18n("KDE Spell checking"));

  l_kab=new QCheckBox(libs_group,"l_kab");
  grid4->addWidget(l_kab,1,2);
  l_kab->setText("kab");
  l_kab->setChecked((ldadd.find(" -lkab ") != -1) || (ldadd.find(" $(LIB_KAB) ") != -1));
  if (l_kab->isChecked())
  {
    ldadd = ldadd.replace( QRegExp(" -lkab"), "" );
    ldadd = ldadd.replace( QRegExp(" \\$\\(LIB_KAB\\)"), "" );
  }
  QWhatsThis::add(l_kab, i18n("KDE addressbook"));

  l_math=new QCheckBox(libs_group,"l_math");
  grid4->addWidget (l_math,0,3);
  l_math->setText("math");
  l_math->setChecked(l_khtml->isChecked() || (ldadd.find(" -lm ") != -1));
  if (l_math->isChecked())
    ldadd = ldadd.replace( QRegExp(" -lm"), "" );
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
  grid2->addWidget(libs_group,0,0);
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
  m_print_debug_info->setText(i18n("Print debug information"));
  m_print_debug_info->setAutoRepeat( FALSE );

  m_cont_after_error = new QCheckBox( makeOptions, "m_cont_after_error" );
  grid1->addWidget(  m_cont_after_error,0,1);
  m_cont_after_error->setFocusPolicy( QWidget::TabFocus );
  m_cont_after_error->setBackgroundMode( QWidget::PaletteBackground );
  m_cont_after_error->setText(i18n("Continue after errors"));
  m_cont_after_error->setAutoRepeat( FALSE );

  m_print_data_base = new QCheckBox( makeOptions, "m_print_data_base" );
  grid1->addWidget( m_print_data_base,0,2);
  m_print_data_base->setFocusPolicy( QWidget::TabFocus );
  m_print_data_base->setBackgroundMode( QWidget::PaletteBackground );
  m_print_data_base->setText(i18n("Print the data base"));
  m_print_data_base->setAutoRepeat( FALSE );

  m_env_variables = new QCheckBox( makeOptions, "m_env_variables" );
  grid1->addWidget( m_env_variables,1,0);
  m_env_variables->setFocusPolicy( QWidget::TabFocus );
  m_env_variables->setBackgroundMode( QWidget::PaletteBackground );
  m_env_variables->setText(i18n("Environment variables"));
  m_env_variables->setAutoRepeat( FALSE );

  m_no_rules = new QCheckBox( makeOptions, "m_no_rules" );
  grid1->addWidget( m_no_rules,1,1);
  m_no_rules->setFocusPolicy( QWidget::TabFocus );
  m_no_rules->setBackgroundMode( QWidget::PaletteBackground );
  m_no_rules->setText(i18n("No built-in rules"));
  m_no_rules->setAutoRepeat( FALSE );

  m_touch_files = new QCheckBox( makeOptions, "m_touch_files" );
  grid1->addWidget(m_touch_files ,1,2);
  m_touch_files->setFocusPolicy( QWidget::TabFocus );
  m_touch_files->setBackgroundMode( QWidget::PaletteBackground );
  m_touch_files->setText(i18n("Touch files"));
  m_touch_files->setAutoRepeat( FALSE );

  m_ignor_errors = new QCheckBox( makeOptions, "m_ignor_errors" );
  grid1->addWidget(m_ignor_errors ,2,0);
  m_ignor_errors->setFocusPolicy( QWidget::TabFocus );
  m_ignor_errors->setBackgroundMode( QWidget::PaletteBackground );
  m_ignor_errors->setText(i18n("Ignore all errors"));
  m_ignor_errors->setAutoRepeat( FALSE );

  m_silent_operation = new QCheckBox( makeOptions, "m_silent_operation" );
  grid1->addWidget(m_silent_operation ,2,1);
  m_silent_operation->setFocusPolicy( QWidget::TabFocus );
  m_silent_operation->setBackgroundMode( QWidget::PaletteBackground );
  m_silent_operation->setText(i18n("Silent operation"));
  m_silent_operation->setAutoRepeat( FALSE );

  m_print_work_dir = new QCheckBox( makeOptions, "m_print_work_dir" );
  grid1->addWidget( m_print_work_dir ,2,2);
  m_print_work_dir->setFocusPolicy( QWidget::TabFocus );
  m_print_work_dir->setBackgroundMode( QWidget::PaletteBackground );
  m_print_work_dir->setText(i18n("Print working directory"));
  m_print_work_dir->setAutoRepeat( FALSE );

  QGridLayout *grid2 = new QGridLayout(0,1,4,15,7, "grid-k");
  grid1->addMultiCellLayout(grid2,3,3,0,2);

  m_job_number_label = new QLabel( makeOptions, "m_job_number_label" );
  grid2->addWidget(m_job_number_label,0,0);
  m_job_number_label->setFocusPolicy( QWidget::NoFocus );
  m_job_number_label->setBackgroundMode( QWidget::PaletteBackground );
  m_job_number_label->setText(i18n("job number"));
  m_job_number_label->setAlignment( 289 );
  m_job_number_label->setMargin( -1 );

  m_job_number = new QSpinBox( makeOptions, "m_job_number" );
  grid2->addWidget(m_job_number,0,1);
  m_job_number->setFocusPolicy( QWidget::StrongFocus );
  m_job_number->setBackgroundMode( QWidget::PaletteBackground );
#if (QT_VERSION < 300)
  m_job_number->setFrameStyle( 50 );
  m_job_number->setLineWidth( 2 );
#endif
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
  m_rebuild_label->setText(i18n("Rebuild behaviour on run/debug:"));
  m_rebuild_label->setAlignment( 289 );
  m_rebuild_label->setMargin( -1 );

  m_rebuild_combo = new QComboBox( makeOptions, "m_rebuild_label" );
  grid2->addWidget(m_rebuild_combo,0,3);
  m_rebuild_combo->setFocusPolicy( QWidget::NoFocus );
  m_rebuild_combo->setBackgroundMode( QWidget::PaletteBase );
  m_rebuild_combo->insertItem(i18n("warning on modification"));
  m_rebuild_combo->insertItem(i18n("only on modification"));
  m_rebuild_combo->insertItem(i18n("always rebuild"));

  grid2 = new QGridLayout(0,3,3,15,7,"grid-l");
  grid1->addMultiCellLayout(grid2,4,6,0,2);

  m_set_modify_label = new QLabel( makeOptions, "m_set_modify_label" );
  grid2->addWidget(m_set_modify_label,0,0);
  m_set_modify_label->setFocusPolicy( QWidget::NoFocus );
  m_set_modify_label->setBackgroundMode( QWidget::PaletteBackground );
  m_set_modify_label->setText(i18n("set modified"));
  m_set_modify_label->setAlignment( 289 );
  m_set_modify_label->setMargin( -1 );

  m_set_modify_line = new QLineEdit( makeOptions, "m_set_modify_line" );
  grid2->addWidget(m_set_modify_line,0,1);
  m_set_modify_line->setFocusPolicy( QWidget::StrongFocus );
  m_set_modify_line->setBackgroundMode( QWidget::PaletteBase );
  m_set_modify_line->setText("");
  m_set_modify_line->setMaxLength( 32767 );
  m_set_modify_line->setEchoMode( QLineEdit::Normal );
  m_set_modify_line->setFrame( TRUE );

  m_set_modify_dir = new QPushButton( makeOptions, "m_set_modify_dir" );
  grid2->addWidget(m_set_modify_dir,0,2);
  m_set_modify_dir->setFocusPolicy( QWidget::TabFocus );
  m_set_modify_dir->setBackgroundMode( QWidget::PaletteBackground );
  m_set_modify_dir->setPixmap(SmallIcon("fileopen"));
  m_set_modify_dir->setAutoRepeat( FALSE );

  m_optional_label = new QLabel( makeOptions, "m_optional_label" );
  m_optional_label->setFocusPolicy( QWidget::NoFocus );
  m_optional_label->setBackgroundMode( QWidget::PaletteBackground );
  m_optional_label->setText(i18n("additional options"));
  m_optional_label->setAlignment( 289 );
  m_optional_label->setMargin( -1 );
  grid2->addWidget(m_optional_label,1,0);

  m_optional_line = new QLineEdit( makeOptions, "m_optional_line" );
  grid2->addMultiCellWidget( m_optional_line,1,1,1,2);
  m_optional_line->setFocusPolicy( QWidget::StrongFocus );
  m_optional_line->setBackgroundMode( QWidget::PaletteBase );
  m_optional_line->setText("");
  m_optional_line->setMaxLength( 32767 );
  m_optional_line->setEchoMode( QLineEdit::Normal );
  m_optional_line->setFrame( TRUE );

  m_makestartpoint_label = new QLabel( makeOptions, "m_makestartpoint_label" );
  grid2->addWidget(m_makestartpoint_label,2,0);
  m_makestartpoint_label->setFocusPolicy( QWidget::NoFocus );
  m_makestartpoint_label->setBackgroundMode( QWidget::PaletteBackground );
  m_makestartpoint_label->setText(i18n("Run make in"));
  m_makestartpoint_label->setAlignment( 289 );
  m_makestartpoint_label->setMargin( -1 );

  m_makestartpoint_line = new QLineEdit( makeOptions, "m_makestartpoint_line" );
  grid2->addWidget(m_makestartpoint_line,2,1);
  m_makestartpoint_line->setFocusPolicy( QWidget::StrongFocus );
  m_makestartpoint_line->setBackgroundMode( QWidget::PaletteBase );
  m_makestartpoint_line->setText("");
  m_makestartpoint_line->setMaxLength( 32767 );
  m_makestartpoint_line->setEchoMode( QLineEdit::Normal );
  m_makestartpoint_line->setFrame( TRUE );

  m_makestartpoint_dir = new QPushButton( makeOptions, "m_makestartpoint_dir" );
  grid2->addWidget(m_makestartpoint_dir,2,2);
  m_makestartpoint_dir->setFocusPolicy( QWidget::TabFocus );
  m_makestartpoint_dir->setBackgroundMode( QWidget::PaletteBackground );
  m_makestartpoint_dir->setPixmap(SmallIcon("fileopen"));
  m_makestartpoint_dir->setAutoRepeat( FALSE );

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

  m_print_debug_info->setChecked(settings->readBoolEntry("PrintDebugInfo"));
  m_print_data_base->setChecked(settings->readBoolEntry("PrintDataBase"));
  m_no_rules->setChecked(settings->readBoolEntry("NoRules"));
  m_env_variables->setChecked(settings->readBoolEntry("EnvVariables"));
  m_cont_after_error->setChecked(settings->readBoolEntry("ContAfterError"));
  m_touch_files->setChecked(settings->readBoolEntry("TouchFiles"));
  m_print_work_dir->setChecked(settings->readBoolEntry("PrintWorkDir"));
  m_silent_operation->setChecked(settings->readBoolEntry("SilentOperation"));
  m_ignor_errors->setChecked(settings->readBoolEntry("IgnorErrors"));

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
  binary_box->setTitle(i18n("Binary Details"));

  QGridLayout *grid1 = new QGridLayout( binary_box ,2,2,15,7,"grid-n");
  QLabel* binary = new QLabel(binary_box,"binary_label");
  binary->setText(i18n("Path and Filename of binary:"));
  grid1->addWidget(binary,0,0);
  binary_edit= new QLineEdit(binary_box,"binary_edit");
  grid1->addWidget(binary_edit,1,0);

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
  libtool_box->setTitle(i18n("Libtool Details"));

  QGridLayout *grid2 = new QGridLayout( libtool_box ,2,2,15,7,"grid-o");
  QLabel* libtool = new QLabel(libtool_box,"libtool_label");
  libtool->setText(i18n("Path of libtool:"));
  grid2->addWidget(libtool,0,0);
  libtool_edit= new QLineEdit(libtool_box,"libtool_edit");
  grid2->addWidget(libtool_edit,1,0);

  QString libtoolDir = prj_info->getLibtoolDir();
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
    binaryOptions->setEnabled(false);
}

void CPrjOptionsDlg::ok()
{
  QString text,text2;
  QStrList short_info;
  int i,n;

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
  compdlg->slotSettingsChanged();

  if (old_cxxflags !=  prj_info->getCXXFLAGS().stripWhiteSpace())
    need_makefile_generation = true;

  //**********linker options*************
//  text=addit_ldflags->text();
//
//  if (l_remove_symbols->isChecked())
//    text+=" -s ";
//  if (l_static->isChecked())
//    text+=" -static";
//  prj_info->setLDFLAGS(text);
//  if(old_ldflags != prj_info->getLDFLAGS().stripWhiteSpace())
//    need_makefile_generation = true;
  text= addit_ldadd->text();

  if (l_math->isChecked() && !l_khtml->isChecked())
    text+=" -lm";

  if(prj_info->isKDE2Project())
  {
    if (l_kab->isChecked())
      text+=" $(LIB_KAB)";
    if (l_kspell->isChecked())
      text+=" $(LIB_KSPELL)";
    if (l_kparts->isChecked())
      text+=" $(LIB_KPARTS)";
    if (l_kfile->isChecked())
      text+=" $(LIB_KFILE)";
    if (l_khtml->isChecked())
      text+=" $(LIB_KHTML)";
    if (l_kdeui->isChecked())
      text+=" $(LIB_KDEUI)";
    if (l_kdecore->isChecked())
      text+=" $(LIB_KDECORE)";
  }
  else
  {
    if (l_kab->isChecked())
      text+=" -lkab";
    if (l_kspell->isChecked())
      text+=" -lkspell";
    if (l_kparts->isChecked())
      text+=" -lkparts";
    if (l_kfile->isChecked())
      text+=" -lkfile";
    if (l_khtml->isChecked())
      text+=" -lkhtml";
    if (l_kdeui->isChecked())
      text+=" -lkdeui";
    if (l_kdecore->isChecked())
      text+=" -lkdecore";
  }

  if (prj_info->isKDE2Project() || prj_info->isQt2Project())
  {
    if (l_qt->isChecked())
        text+=" $(LIB_QT)";
  }
  else
  {
    if (l_qt->isChecked())
        text+=" -lqt";
  }

  if (l_kfm->isChecked())
    text+=" -lkfm";
  if (l_Xext->isChecked())
    text+=" -lXext";
  if (l_X11->isChecked())
    text+=" -lX11";
  prj_info->setLDADD(text);
  if(old_ldadd != prj_info->getLDADD().stripWhiteSpace()){
    need_makefile_generation = true;
  }

  //**********make options*************
  settings = KGlobal::config();
  settings->setGroup("MakeOptionsSettings");

  settings->writeEntry("PrintDebugInfo",m_print_debug_info->isChecked());
  settings->writeEntry("PrintDataBase",m_print_data_base->isChecked());
  settings->writeEntry("NoRules",m_no_rules->isChecked());
  settings->writeEntry("EnvVariables",m_env_variables->isChecked());
  settings->writeEntry("ContAfterError",m_cont_after_error->isChecked());
  settings->writeEntry("TouchFiles", m_touch_files->isChecked());
  settings->writeEntry("PrintWorkDir", m_print_work_dir->isChecked());
  settings->writeEntry("SilentOperation", m_silent_operation->isChecked());
  settings->writeEntry("IgnorErrors", m_ignor_errors->isChecked());

  settings->writeEntry("RebuildType", m_rebuild_combo->currentItem());
  settings->writeEntry("SetModifyLine", m_set_modify_line->text());
  settings->writeEntry("OptionalLine", m_optional_line->text());
  settings->writeEntry("JobNumber", m_job_number->text());
  settings->sync();
  // reject();

  text = "";

  if (m_print_debug_info->isChecked())
    text+=" -d";
  if (m_print_data_base->isChecked())
    text+=" -p";
  if (m_no_rules->isChecked())
    text+=" -r";
  if (m_env_variables->isChecked())
    text+=" -e";
  if (m_cont_after_error->isChecked())
    text+=" -k";
  if (m_touch_files->isChecked())
    text+=" -t";
  if (m_print_work_dir->isChecked())
    text+=" -w";
  if (m_silent_operation->isChecked())
    text+=" -s";
  if (m_ignor_errors->isChecked())
    text+=" -i";

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
  if (version_edit->text() != old_version || prjname_edit->text() != old_name)
    need_configure_in_update = true;

  // check now for modifications inside configure.in(.in)
  if (addOptsDlg && addOptsDlg->changed())
  {
    addOptsDlg->modifyConfigureIn();
    need_configure_in_update = true;
  }
}

// connection to set_modify_dir
void CPrjOptionsDlg::slotFileDialogClicked()
{
  QString file,dir;
  dir = prj_info->getProjectDir();
  file = KFileDialog::getOpenFileName(dir,"*",0,"File");
  m_set_modify_line->setText(file);
}


bool CPrjOptionsDlg::needConfigureInUpdate()
{
  return  need_configure_in_update;
}

void CPrjOptionsDlg::slotBinaryClicked()
{
  QString dir;
  dir = KFileDialog::getOpenFileName(prj_info->getProjectDir() + prj_info->pathToBinPROGRAM() + "/" + prj_info->getBinPROGRAM());
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

void CPrjOptionsDlg::slotFileDialogMakeStartPointClicked()
{
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
