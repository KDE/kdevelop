/***************************************************************************
                     kdevsetup.cpp - the setup dialog for KDevelop
                             -------------------

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

#include "ckdevsetupdlg.h"

#include "ckdevelop.h"
#include "resource.h"
#include "ccompletionopts.h"
#include "ccodetemplateopts.h"
#include "partselectwidget.h"

#include <kmessagebox.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kfiledialog.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qgrid.h>
#include <qvbox.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qtabwidget.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

CKDevSetupDlg::CKDevSetupDlg(KAccel* accel, QWidget *parent, const char *name, int mdiMode ) :
  KDialogBase ( IconList,                 // dialogFace
                i18n("KDevelop Setup"),   // caption
                Ok|Default|Cancel,        // buttonMask
                Ok,                       // defaultButton
                parent,
                name,
                true,                     // modal
                true)                     // separator
  ,wantsTreeRefresh(false)
  ,config(KGlobal::config())
  ,m_accel(accel)
        ,m_mdiMode(mdiMode)
{

  addGeneralTab();
  addKeysTab();
  addEnhancedCodingTab();
  addDocTab();
  addCompilerTab();
  addDebuggerTab();
  addQT2Tab();
  addUserInterfaceTab();
  addPartSelectTab();

//  connect( this, SIGNAL(defaultButtonPressed()), SLOT(slotDefault()) );
  connect( this, SIGNAL(okClicked()), SLOT(slotOkClicked()) );
  connect( this, SIGNAL(okClicked()), parent, SLOT(slotOptionsMake()) );
}

void CKDevSetupDlg::addGeneralTab()
{
  // ****************** the General Tab ********************
  generalPage = addPage(i18n("General"),i18n("KDevelop General Configuration"),
  KGlobal::instance()->iconLoader()->loadIcon( "gear", KIcon::NoGroup, KIcon::SizeMedium ));
  QGridLayout *grid = new QGridLayout(generalPage,4,1,15,7);

  QButtonGroup* makeGroup;
  makeGroup = new QButtonGroup( generalPage, "makeGroup" );
  //makeGroup->setGeometry( 10, 10, 400, 60 );
  grid->addWidget(makeGroup,0,0);
  makeGroup->setFrameStyle( 49 );
  makeGroup->setTitle(i18n( "Make-Command" ));
  makeGroup->setAlignment( 1 );
  makeGroup->lower();

  QLabel* makeSelectLabel;
  QGridLayout *grid2 = new QGridLayout(makeGroup,1,2,15,7);
  makeSelectLabel = new QLabel( makeGroup, "makeSelectLabel" );
  grid2->addWidget(makeSelectLabel,0,0);
  makeSelectLabel->setText(i18n("Select Make-Command:"));
  makeSelectLabel->setAlignment( 289 );
  makeSelectLabel->setMargin( -1 );

  config->setGroup("General Options");
  QString make_cmd=config->readEntry("Make","make");

  makeSelectLineEdit = new QLineEdit( makeGroup, "makeSelectLineEdit" );
  grid2->addWidget(makeSelectLineEdit,0,1);
  makeSelectLineEdit->setText(make_cmd);

  QString makeSelectMsg = i18n("Make-Command\n\n"
                                                          "Select your system's make-command.\n"
                                                          "Usually, this is make, FreeBSD users\n"
                                                          "may use gmake. Mind that you can also\n"
                                                          "add option parameters to your make-binary\n"
                                          "as well.");
  QWhatsThis::add(makeGroup, makeSelectMsg);
  QWhatsThis::add(makeSelectLabel, makeSelectMsg);
  QWhatsThis::add(makeSelectLineEdit, makeSelectMsg);

  bool autoSave=config->readBoolEntry("Autosave",true);

  QButtonGroup* autosaveGroup;
  autosaveGroup = new QButtonGroup( generalPage, "autosaveGroup" );
  grid2 = new QGridLayout(autosaveGroup,2,2,15,7);

  autosaveGroup->setFrameStyle( 49 );
  autosaveGroup->setTitle( i18n("Autosave") );
  autosaveGroup->setAlignment( 1 );
  //  autosaveGroup->insert( autoSaveCheck );
  autosaveGroup->lower();

  autoSaveCheck = new QCheckBox( autosaveGroup, "autoSaveCheck" );
  grid2->addWidget(autoSaveCheck,0,0);
  autoSaveCheck->setText(i18n("Enable autosave"));
  autoSaveCheck->setAutoRepeat( FALSE );
  autoSaveCheck->setAutoResize( FALSE );
  autoSaveCheck->setChecked(autoSave);

  QLabel* autosaveTimeLabel;
  autosaveTimeLabel = new QLabel( autosaveGroup, "autosaveTimeLabel" );
  grid2->addWidget(autosaveTimeLabel,1,0);
  autosaveTimeLabel->setText(i18n("Select Autosave time-interval:"));
  autosaveTimeLabel->setAlignment( 289 );
  autosaveTimeLabel->setMargin( -1 );
  autosaveTimeLabel->setEnabled(autoSave);

  autosaveTimeCombo = new QComboBox( FALSE, autosaveGroup, "autosaveTimeCombo" );
  grid2->addWidget(autosaveTimeCombo,1,1);
  autosaveTimeCombo->setSizeLimit( 10 );
  autosaveTimeCombo->setAutoResize( FALSE );
  autosaveTimeCombo->insertItem(i18n("3 min"),0 );
  autosaveTimeCombo->insertItem(i18n("5 min"),1 );
  autosaveTimeCombo->insertItem(i18n("15 min"),2 );
  autosaveTimeCombo->insertItem(i18n("30 min"),3 );
  autosaveTimeCombo->setEnabled(autoSave);
  int configTime=config->readNumEntry("Autosave Timeout",3*60*1000);
  if(configTime==3*60*1000)
    autosaveTimeCombo->setCurrentItem(0);
  if(configTime==5*60*1000)
    autosaveTimeCombo->setCurrentItem(1);
  if(configTime==15*60*1000)
    autosaveTimeCombo->setCurrentItem(2);
  if(configTime==30*60*1000)
    autosaveTimeCombo->setCurrentItem(3);

  QString autosaveMsg = i18n("Autosave\n\nIf autosave is enabled, your currently\n"
                                "changed files will be saved after the\n"
                                "time-interval selected times out.\n\n"
                                "Please select your timeout-value.\n"
                                "Available are: 3 minutes, 5 minutes,\n"
                                "15 minutes and 30 minutes.");
  QWhatsThis::add(autosaveTimeLabel,autosaveMsg);
  QWhatsThis::add(autosaveTimeCombo, autosaveMsg);
  QWhatsThis::add(autoSaveCheck, autosaveMsg);
  QWhatsThis::add(autosaveGroup, autosaveMsg);
  grid->addWidget(autosaveGroup,1,0);

  QButtonGroup* autoswitchGroup;
  autoswitchGroup = new QButtonGroup( generalPage, "autoswitchGroup" );

  autoswitchGroup->setFrameStyle( 49 );
  autoswitchGroup->setTitle(i18n( "Autoswitch") );
  autoswitchGroup->setAlignment( 1 );
  //  autoswitchGroup->insert( autoSwitchCheck );
  autoswitchGroup->lower();
  grid2 = new QGridLayout(autoswitchGroup,1,2,15,7);
  autoSwitchCheck = new QCheckBox( autoswitchGroup, "autoSwitchCheck" );
  grid2->addWidget(autoSwitchCheck,0,0);
  autoSwitchCheck->setText(i18n("Enable autoswitch"));
  autoSwitchCheck->setAutoRepeat( FALSE );
  autoSwitchCheck->setAutoResize( FALSE );
  bool autoSwitch=config->readBoolEntry("Autoswitch",true);
  autoSwitchCheck->setChecked( autoSwitch );

  defaultClassViewCheck = new QCheckBox( autoswitchGroup, "defaultClassViewCheck" );
  grid2->addWidget(defaultClassViewCheck,0,1);
  defaultClassViewCheck->setText(i18n( "Use class view as default"));
  defaultClassViewCheck->setAutoRepeat( FALSE );
  defaultClassViewCheck->setAutoResize( FALSE );
  bool defaultcv=config->readBoolEntry("DefaultClassView",true);
  defaultClassViewCheck->setChecked( defaultcv );

  startupEditingCheck = new QCheckBox( autoswitchGroup, "autoStartupCheck" );
  grid2->addWidget(startupEditingCheck,1,0);
  startupEditingCheck->setText(i18n("Start editing \"main\"-source file"));
  startupEditingCheck->setAutoRepeat( FALSE );
  startupEditingCheck->setAutoResize( FALSE );
  bool startupEditing=config->readBoolEntry("StartupEditing",true);
  startupEditingCheck->setChecked( startupEditing );


  QWhatsThis::add(startupEditingCheck, i18n("startup with editing main.cpp or main.c\n\n"
                                              "If this is enabled, KDevelop\n"
                                              "will try to load main.cpp or main.c\n"
                                              "after project creation.\n"
                                              "When disabled, KDevelop doesn't load\n"
                                              "a source file after project creation."));

  QWhatsThis::add(defaultClassViewCheck, i18n("use Class View as default\n\n"
                                              "If this is enabled, KDevelop\n"
                                              "will automatically switch to\n"
                                              "the Class Viewer when switching.\n"
                                              "When disabled, KDevelop will\n"
                                              "use Logical File Viewer for\n"
                                              "autoswitching."));

  QString autoswitchMsg = i18n("Autoswitch\n\n"
                                                       "If autoswitch is enabled, KDevelop\n"
                                                       "will open windows in the working\n"
                                                       "view automatically according to\n"
                                                       "most needed functionality.\n\n"
                                                       "Disableing autoswitch means you\n"
                                                       "will have to switch to windows\n"
                                                       "yourself, including turning on and\n"
                                                       "off the outputwindow.");
  QWhatsThis::add(autoSwitchCheck, autoswitchMsg);
  QWhatsThis::add(autoswitchGroup, autoswitchMsg);
  grid->addWidget(autoswitchGroup,2,0);
//  connect( autoSwitchCheck, SIGNAL(toggled(bool)),defaultClassViewCheck, SLOT(setEnabled(bool)));

  QButtonGroup* startupGroup = new QButtonGroup( generalPage, "startupGroup" );
  //startupGroup->setGeometry( 10, 260, 400, 70 );

  grid2 = new QGridLayout(startupGroup,2,2,15,7);
  startupGroup->setFrameStyle( 49 );
  startupGroup->setTitle(i18n("Startup"));
  startupGroup->setAlignment( 1 );
  //        startupGroup->insert( logoCheck );
  //        startupGroup->insert( lastProjectCheck );
  startupGroup->lower();

  QWhatsThis::add(startupGroup, i18n("Startup\n\n"
                          "The Startup group offers options for\n"
                                     "starting KDevelop"));

  config->setGroup("General Options");
  bool logo=config->readBoolEntry("Logo",true);
  bool lastprj=config->readBoolEntry("LastProject",true);

  logoCheck = new QCheckBox( startupGroup, "logoCheck" );
  grid2->addWidget(logoCheck,0,0);
  logoCheck->setText(i18n("Startup Logo"));
  logoCheck->setAutoRepeat( FALSE );
  logoCheck->setAutoResize( FALSE );
  logoCheck->setChecked( logo );

  QWhatsThis::add(logoCheck, i18n("Startup Logo\n\n"
                          "If Startup Logo is enabled, KDevelop will show the\n"
                          "logo picture while it is starting."));

  lastProjectCheck = new QCheckBox( startupGroup, "lastProjectCheck" );
  grid2->addWidget(lastProjectCheck,1,0);
  lastProjectCheck->setText(i18n("Load last project"));
  lastProjectCheck->setAutoRepeat( FALSE );
  lastProjectCheck->setAutoResize( FALSE );
  lastProjectCheck->setChecked( lastprj );

  QWhatsThis::add(lastProjectCheck, i18n("Load last project\n\n"
                    "If Load last project is enabled, KDevelop will load\n"
                    "the last project used."));


  config->setGroup("TipOfDay");
  bool tip=config->readBoolEntry("RunOnStart",true);

        tipDayCheck = new QCheckBox( startupGroup, "tipDayCheck" );
                grid2->addWidget(tipDayCheck,0,1);
        tipDayCheck->setText(i18n("Tip of the Day"));
        tipDayCheck->setAutoRepeat( FALSE );
        tipDayCheck->setAutoResize( FALSE );
        tipDayCheck->setChecked( tip );

        QWhatsThis::add(tipDayCheck, i18n("Tip of the Day\n\n"
                          "If Tip of the Day is enabled, KDevelop will show the\n"
                          "Tip of the Day every time it starts."));
  grid->addWidget(startupGroup,3,0);
  connect( autoSwitchCheck, SIGNAL(toggled(bool)),parent(), SLOT(slotOptionsAutoswitch(bool)) );
  connect( startupEditingCheck, SIGNAL(toggled(bool)),parent(), SLOT(slotOptionsStartupEditing(bool)) );
  connect( autoSwitchCheck, SIGNAL(toggled(bool)),defaultClassViewCheck, SLOT(setEnabled(bool)));
  connect( autosaveTimeCombo, SIGNAL(activated(int)),parent(), SLOT(slotOptionsAutosaveTime(int)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),parent(), SLOT(slotOptionsAutosave(bool)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),autosaveTimeLabel, SLOT(setEnabled(bool)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),autosaveTimeCombo, SLOT(setEnabled(bool)) );
  connect( defaultClassViewCheck,SIGNAL(toggled(bool)),parent(),SLOT(slotOptionsDefaultCV(bool)));
}

//
// ****************** the Keys Tab ***************************
//
void CKDevSetupDlg::addKeysTab()
{

#if QT_VERSION < 300
  keyMap = m_accel->keyDict();
#endif
  keysPage = addPage(i18n("Keys"),i18n("Configuration of KDevelop Hot-Keys"),
  KGlobal::instance()->iconLoader()->loadIcon( "key_bindings", KIcon::NoGroup, KIcon::SizeMedium ));
  QGridLayout *grid = new QGridLayout(keysPage,2,1,15,7);

#if QT_VERSION < 300
  keyChooser = new KKeyChooser ( &keyMap, keysPage, true);
#else
  keyChooser = new KKeyChooser ( m_accel, keysPage, true);
#endif
  grid->addWidget(keyChooser,0,0);
}

void CKDevSetupDlg::addDocTab()
{
  // ****************** the Documentation Tab ********************

  QFrame* docPage = addPage(i18n("Documentation"),i18n("Path to KDE 2/Qt 2 API Documentation"),
  KGlobal::instance()->iconLoader()->loadIcon( "contents", KIcon::NoGroup, KIcon::SizeMedium ));

  QGridLayout *grid = new QGridLayout(docPage,2,1,15,7);

  config->setGroup("Doc_Location");

  QWhatsThis::add(docPage, i18n("Enter the path to your Qt and KDE-Libs\n"
                                "Documentation for the Documentation Browser.\n"
                                "Qt usually comes with complete documentation,\n"
                                "whereas for KDE you can create the documentation\n"
                                "easily by pressing the Update button below."));

  QButtonGroup* docGroup;

  docGroup = new QButtonGroup( docPage, "docGroup" );
  grid->addWidget(docGroup,0,0);
  docGroup->setFrameStyle( 49 );
  docGroup->setTitle(i18n("Directories"));
  docGroup->setAlignment( 1 );
  docGroup->lower();
  QGridLayout *grid2 = new QGridLayout(docGroup,2,3,15,7);

  qt_edit = new QLineEdit( docGroup, "qt_edit" );
  grid2->addWidget(qt_edit,0,1);
  qt_doc_path= config->readEntry("doc_qt", QT_DOCDIR);
  qt_edit->setText(qt_doc_path);
  qt_edit->setMaxLength( 32767 );

  QPushButton* qt_button;
  qt_button = new QPushButton( docGroup, "qt_button" );
  grid2->addWidget(qt_button,0,2);
  qt_button->setPixmap(SmallIcon("fileopen"));
  connect(qt_button,SIGNAL(clicked()),SLOT(slotQtClicked()));

  QLabel* qt_label;
  qt_label = new QLabel( docGroup, "qt_label" );
  grid2->addWidget(qt_label,0,0);
  qt_label->setText( i18n("Qt-Library-Doc:") );

  QString qtdocMsg = i18n("Enter the path to your Qt documentation\n"
                                 "here. To access the path more easily,\n"
                                 "press the button on the right to change\n"
                                 "directories.\n\n"
                                 "The Qt documentation is usually\n"
                                 "located in <i><blue>$QTDIR/html</i>");

  QWhatsThis::add(qt_edit, qtdocMsg);
  QWhatsThis::add(qt_button, qtdocMsg);
  QWhatsThis::add(qt_label, qtdocMsg);

  kde_edit = new QLineEdit( docGroup, "kde_edit");
  grid2->addWidget(kde_edit,1,1);
  kde_doc_path=config->readEntry("doc_kde", KDELIBS_DOCDIR);
  kde_edit->setText(kde_doc_path);
  kde_edit->setMaxLength( 32767 );
  kde_edit->setEchoMode( QLineEdit::Normal );
  kde_edit->setFrame( TRUE );

  QPushButton* kde_button;
  kde_button = new QPushButton(  docGroup, "kde_button" );
  grid2->addWidget(kde_button,1,2);
  kde_button->setPixmap(SmallIcon("fileopen"));
  connect(kde_button,SIGNAL(clicked()),SLOT(slotKDEClicked()));

  QLabel* kde_label;
  kde_label = new QLabel(  docGroup, "kde_label" );
  grid2->addWidget(kde_label,1,0);
  kde_label->setText( i18n("KDE-Libraries-Doc:") );

  QString kdedocMsg = i18n("Enter the path to your KDE documentation\n"
                                 "here. To access the path more easily,\n"
                                 "press the button on the right to change\n"
                                 "directories.\n\n"
                                 "If you have no kdelibs documentation installed,\n"
                                 "you can create it by pressing the Update button\n"
                                 "below.");
  QWhatsThis::add(kde_edit, kdedocMsg);
  QWhatsThis::add(kde_button, kdedocMsg);
  QWhatsThis::add(kde_label, kdedocMsg);



  QButtonGroup* docOptionsGroup;
  docOptionsGroup = new QButtonGroup( docPage, "docOptionsGroup" );
  grid->addWidget(docOptionsGroup,1,0);
  docOptionsGroup->setFrameStyle( 49 );
  docOptionsGroup->setTitle(i18n("Options"));
  docOptionsGroup->setAlignment( 1 );
  grid2 = new QGridLayout(docOptionsGroup,4,2,15,7);
  QLabel* update_label;
  update_label = new QLabel( docOptionsGroup, "update_label" );
  grid2->addWidget(update_label,0,0);
  update_label->setText(i18n("Update KDE documentation:"));
  update_label->setAlignment( 289 );
  update_label->setMargin( -1 );

  QPushButton* update_button;
  update_button = new QPushButton( docOptionsGroup, "update_button" );
  grid2->addWidget(update_button,0,1);
  connect( update_button, SIGNAL(clicked()), SLOT(slotKDEUpdateReq()) );
  update_button->setText(i18n("Update..."));
  update_button->setAutoRepeat( FALSE );
  update_button->setAutoResize( FALSE );

  QString updateMsg = i18n("Update KDE documentation\n\n"
                                     "This lets you create or update the\n"
                                     "HTML documentation of the KDE-libs.\n"
                                     "To use this function, ensure that"
				     "you have kdoc installed, the kdelibs\n"
                                     "sources available to create the\n"
				     "documentation, and the path set for\n"
                                     "the Qt documentation.\n"
                                     "The path must be set in order to\n"
                                     "cross-reference the KDE documentation\n"
                                     "with the Qt classes.");
  QWhatsThis::add(update_label, updateMsg);
  QWhatsThis::add(update_button, updateMsg);

  QLabel* create_label;
  create_label = new QLabel( docOptionsGroup, "create_label" );
  grid2->addWidget(create_label,1,0);
  create_label->setText(i18n("Create Search Database:") );
  create_label->setAlignment( 289 );
  create_label->setMargin( -1 );

  QPushButton* create_button;
  create_button = new QPushButton( docOptionsGroup, "create_button" );
  grid2->addWidget(create_button,1,1);
  connect( create_button, SIGNAL(clicked()),parent(), SLOT(slotOptionsCreateSearchDatabase()) );
  create_button->setText(i18n("Create..."));
  create_button->setAutoRepeat( FALSE );
  create_button->setAutoResize( FALSE );

  QString createMsg = i18n("Create Search Database\n\n"
                                                     "This will create a search database for glimpse\n"
                                                     "which will be used to look up marked text in\n"
                                                     "the documentation. We recommend updating the\n"
                                                     "database each time you've changed the documentation\n"
                                                     "e.g. after a kdelibs-update or installing a new\n"
                                                     "Qt-library version.");

  QWhatsThis::add(create_label, createMsg);
  QWhatsThis::add(create_button, createMsg);


  useCTags = new QCheckBox( docOptionsGroup, "useCTags" );
  grid2->addWidget(useCTags,2,0);
  useCTags->setText(i18n("Enable CTags database"));
  useCTags->setAutoRepeat( FALSE );
  useCTags->setAutoResize( FALSE );
  bool bUseCTags;
  config->setGroup("General Options");
  bUseCTags = config->readBoolEntry("use_ctags", false);
  useCTags->setChecked( bUseCTags );
  QWhatsThis::add(useCTags,
    i18n("Enable CTags based browsing and searching.\n\n"
         "If this is enabled, you can browse the source code of your"
         "project much easier. You can jump to declarations"
         "and definitions of functions, classes and methods"
         "using the right mouse button.\n"));

  QPushButton* load_ctags_button;
  load_ctags_button = new QPushButton( docOptionsGroup, "load_ctags_button" );
  grid2->addWidget(load_ctags_button,2,1);
  connect( load_ctags_button, SIGNAL(clicked()),parent(), SLOT(slotProjectLoadTags()) );
  load_ctags_button->setText(i18n("Load"));
  load_ctags_button->setAutoRepeat( FALSE );
  load_ctags_button->setAutoResize( FALSE );

  kdocCheck = new QCheckBox( docOptionsGroup, "kdocCheck" );
  grid2->addWidget(kdocCheck,3,0);
  kdocCheck->setText(i18n("Create also KDOC-reference of your project"));
  kdocCheck->setAutoRepeat( FALSE );
  kdocCheck->setAutoResize( FALSE );
  bool bCreateKDoc;

  config->setGroup("General Options");
  bCreateKDoc = config->readBoolEntry("CreateKDoc", false);

  kdocCheck->setChecked( bCreateKDoc );

  QWhatsThis::add(kdocCheck, i18n("Create KDOC-reference of your project\n\n"
                    "If this is enabled, on creating the API-Documentation KDoc creates also\n"
                    "a cross reference file of your project into the selected kdoc-reference\n"
                    "directory."));

}

/** adds the compiler page for setting up the compile environment */
void CKDevSetupDlg::addCompilerTab(){
  compilerPage = addPage(i18n("Compiler"),i18n("Compiler Settings"),
  KGlobal::instance()->iconLoader()->loadIcon( "configure", KIcon::NoGroup, KIcon::SizeMedium ));
  compdlg = new CCompConf(compilerPage);
  QGridLayout *grid = new QGridLayout(compilerPage);
  grid->addWidget(compdlg,0,0);
}

void CKDevSetupDlg::addDebuggerTab()
{
  // ****************** the Debugger Tab ***************************

  config->setGroup("Debug");
  bool useExternalDbg       = config->readBoolEntry("Use external debugger", false);
  QString dbg_cmd           = config->readEntry("External debugger program","kdbg");
  bool displayMangledNames  = config->readBoolEntry("Display mangled names", false);
  bool displayStaticMembers = config->readBoolEntry("Display static members", false);
  bool setBPsOnLibLoad      = config->readBoolEntry("Break on loading libs", true);
  bool dbgFloatingToolbar   = config->readBoolEntry("Enable floating toolbar", false);
  bool dbgTerminal          = config->readBoolEntry("Debug on separate tty console", false);

  debuggerPage = addPage(i18n("Debugger"),i18n("Debugger Settings"),
  KGlobal::instance()->iconLoader()->loadIcon( "debugger", KIcon::NoGroup, KIcon::SizeMedium ));
  QGridLayout *grid = new QGridLayout(debuggerPage,3,1,15,7);
  dbgExternalCheck = new QCheckBox( debuggerPage, "dbgExternal" );
  grid->addWidget(dbgExternalCheck,0,0);
  dbgExternalCheck->setText(i18n("Use external debugger"));
  dbgExternalCheck->setAutoRepeat( FALSE );
  dbgExternalCheck->setAutoResize( FALSE );
  dbgExternalCheck->setChecked(useExternalDbg);

  QWhatsThis::add(dbgExternalCheck, i18n("Select internal or external debugger\n\n"
                          "Choose whether to use an external debugger\n"
                          "or the internal debugger within kdevelop\n"
                    "The internal debugger is a frontend to gdb"));

  dbgExternalGroup = new QButtonGroup( debuggerPage, "dbgExternalGroup" );
  QGridLayout *grid2 = new QGridLayout(dbgExternalGroup,1,2,15,7);
  dbgExternalGroup->setFrameStyle( 49 );
  dbgExternalGroup->setTitle(i18n("External debugger settings", "External" ));
  dbgExternalGroup->setAlignment( 1 );
  dbgExternalGroup->lower();

  dbgSelectCmdLabel = new QLabel( dbgExternalGroup, "dbgSelectLabel" );
  grid2->addWidget(dbgSelectCmdLabel,0,0);
  dbgSelectCmdLabel->setText(i18n("Select debug command:"));
  dbgSelectCmdLabel->setAlignment( 289 );
  dbgSelectCmdLabel->setMargin( -1 );

  dbgExternalSelectLineEdit = new QLineEdit( dbgExternalGroup, "dbgExternalSelectLineEdit" );
  grid2->addWidget(dbgExternalSelectLineEdit,0,1);
  dbgExternalSelectLineEdit->setText(dbg_cmd);
  grid->addWidget(dbgExternalGroup,1,0);

  QString dbgSelectCmdMsg = i18n("Identify the external debugger\n\n"
                          "Enter the program name you wish to run\n"
                          "as your debugger");
  QWhatsThis::add(dbgSelectCmdLabel, dbgSelectCmdMsg);
  QWhatsThis::add(dbgExternalSelectLineEdit, dbgSelectCmdMsg);

  dbgInternalGroup = new QButtonGroup( debuggerPage, "dbgInternalGroup" );
  grid->addWidget(dbgInternalGroup,2,0);
  grid2 = new QGridLayout(dbgInternalGroup,5,1,15,7);
  dbgInternalGroup->setFrameStyle( 49 );
  dbgInternalGroup->setTitle(i18n( "Internal debugger settings", "Internal" ));
  dbgInternalGroup->setAlignment( 1 );
  dbgInternalGroup->lower();

  dbgMembersCheck = new QCheckBox( dbgInternalGroup, "dbgMembers" );
  grid2->addWidget( dbgMembersCheck,0,0);
  dbgMembersCheck->setText(i18n("Display static members"));
  dbgMembersCheck->setAutoRepeat( FALSE );
  dbgMembersCheck->setAutoResize( FALSE );
  dbgMembersCheck->setChecked(displayStaticMembers);
  QWhatsThis::add(dbgMembersCheck, i18n("Display static members\n\n"
                          "Displaying static members makes gdb slower in\n"
                    "producing data within kde and qt.\n"
                    "It may change the \"signature\" of the data\n"
                    "which QString and friends rely on.\n"
                    "But if you need to debug into these values then\n"
                    "check this option" ));

  dbgAsmCheck = new QCheckBox( dbgInternalGroup, "dbgMembers" );
  grid2->addWidget( dbgAsmCheck,1,0);
  dbgAsmCheck->setText(i18n("Display mangled names"));
  dbgAsmCheck->setAutoRepeat( FALSE );
  dbgAsmCheck->setAutoResize( FALSE );
  dbgAsmCheck->setChecked(displayMangledNames);
  QWhatsThis::add(dbgAsmCheck, i18n("Display mangled names\n\n"
                          "When displaying the disassembled code you\n"
                          "can select to see the methods mangled names\n"
                    "However, non-mangled names are easier to read." ));

  dbgLibCheck = new QCheckBox( dbgInternalGroup, "dbgMembers" );
  grid2->addWidget( dbgLibCheck,2,0);
  dbgLibCheck->setText(i18n("Try setting breakpoints on lib load"));
  dbgLibCheck->setAutoRepeat( FALSE );
  dbgLibCheck->setAutoResize( FALSE );
  dbgLibCheck->setChecked(setBPsOnLibLoad);
  QWhatsThis::add(dbgLibCheck, i18n("Set pending breakpoints on loading a library\n\n"
                          "If GDB hasn't seen a library that will be loaded via\n"
                          "\"dlopen\" then it'll refuse to set a breakpoint in that code.\n"
                    "We can get GDB to stop on a library load and hence\n"
                    "try to set the pending breakpoints. See docs for more\n"
                    "details and a \"gotcha\" relating to this behavior.\n\n"
                    "If you are not \"dlopen\"ing libs leave this off." ));

  dbgFloatCheck = new QCheckBox( dbgInternalGroup, "dbgFloatToolbar" );
  //dbgFloatCheck->setGeometry( 20, 240, 310, 25 );
  grid2->addWidget( dbgFloatCheck,3,0);
  dbgFloatCheck->setText(i18n("Enable floating toolbar"));
  dbgFloatCheck->setAutoRepeat( FALSE );
  dbgFloatCheck->setAutoResize( FALSE );
  dbgFloatCheck->setChecked(dbgFloatingToolbar);
  QWhatsThis::add(dbgFloatCheck, i18n("Enable floating toolbar\n\n"
                          "Use the floating toolbar. This toolbar always stays\n"
                    "on top of all windows so that if the app covers KDevelop\n"
                    "you have control of the app though the small toolbar\n"
                    "Also this toolbar can be docked to the panel\n"
                    "This toolbar is in addition to the toolbar in KDevelop" ));

  dbgTerminalCheck = new QCheckBox( dbgInternalGroup, "dbgTerminalCheck" );
  grid2->addWidget( dbgTerminalCheck,4,0);
  dbgTerminalCheck->setText(i18n("Enable separate terminal for application i/o"));
  dbgTerminalCheck->setAutoRepeat( FALSE );
  dbgTerminalCheck->setAutoResize( FALSE );
  dbgTerminalCheck->setChecked(dbgTerminal);
  QWhatsThis::add(dbgTerminalCheck, i18n("Enable separate terminal for application i/o\n\n"
                    "This allows you to enter terminal input when your\n"
                    "application contains terminal input code (eg cin, fgets etc.) \n"
                    "If you use terminal input in your app, then tick this option.\n"
                    "Otherwise leave this off." ));

  slotSetDebug();
  connect( dbgExternalCheck, SIGNAL(toggled(bool)), SLOT(slotSetDebug()));
}

//
//************************** QT-2 directory select *************************
//
void CKDevSetupDlg::addQT2Tab()
{

  QFrame* QT2Page = addPage(i18n("Path"),i18n("Path to KDE 2/Qt 2 installation"),
  KGlobal::instance()->iconLoader()->loadIcon( "fileopen", KIcon::NoGroup, KIcon::SizeMedium ));

  config->setGroup("QT2");
  QGridLayout *grid = new QGridLayout(QT2Page,2,1,15,7);

  QGroupBox* kde2_box= new QGroupBox(QT2Page,"NoName");
  grid->addWidget(kde2_box,0,0);
  kde2_box->setTitle(i18n("Qt 2.x/KDE path"));
  QGridLayout *grid2 = new QGridLayout(kde2_box,4,2,15,7);

  QLabel* qt2= new QLabel(kde2_box,"NoName");
  qt2->setText(i18n("Qt 2.x directory:"));
  grid2->addWidget(qt2,0,0);

  qt2_edit= new QLineEdit(kde2_box,"NoName");
  qt2_edit->setMaxLength( 32767 );
  grid2->addWidget(qt2_edit,1,0);

  QString qt2_path= config->readEntry("qt2dir");
  qt2_edit->setText(qt2_path);

  QPushButton* qt2_button= new QPushButton(kde2_box,"NoName");
  qt2_button->setPixmap(SmallIcon("fileopen"));
  grid2->addWidget(qt2_button,1,1);

  QString qt2Msg = i18n("Set the root directory path leading to your Qt 2.x path, e.g. /usr/lib/qt-2.0");
  QWhatsThis::add(qt2_edit, qt2Msg);
  QWhatsThis::add(qt2_button, qt2Msg);
  QWhatsThis::add(qt2, qt2Msg);


  QLabel* kde2= new QLabel(kde2_box,"NoName");
  kde2->setText(i18n("KDE 2.x directory:"));
  grid2->addWidget(kde2,2,0);

  kde2_edit= new QLineEdit(kde2_box,"NoName");
  grid2->addWidget(kde2_edit,3,0);

  QString kde2_path= config->readEntry("kde2dir");
  kde2_edit->setText(kde2_path);


  QPushButton* kde2_button= new QPushButton(kde2_box,"NoName");
  kde2_button->setPixmap(SmallIcon("fileopen"));
  grid2->addWidget(kde2_button,3,1);

  QString kde2Msg = i18n("Set the root directory path leading to your KDE 2 includes/libraries, e.g. /opt/kde2");
  QWhatsThis::add(kde2_edit, kde2Msg);
  QWhatsThis::add(kde2_button, kde2Msg);
  QWhatsThis::add(kde2, kde2Msg);

  connect(qt2_button,SIGNAL(clicked()),SLOT(slotQt2Clicked()));
  connect(kde2_button, SIGNAL(clicked()),SLOT(slotKDE2Clicked()));

  QGroupBox* ppath_box= new QGroupBox(QT2Page,"NoName");
  grid->addWidget( ppath_box,1,0);
  grid2 = new QGridLayout(ppath_box,4,2,15,7);
  ppath_box->setTitle(i18n("Default Project Path"));

// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
  config->setGroup("General Options");
  QLabel* ppath= new QLabel(ppath_box,"NoName");
  ppath->setText(i18n("Project Path:"));
  grid2->addWidget(ppath,0,0);

  ppath_edit= new QLineEdit(ppath_box,"NoName");
  grid2->addWidget(ppath_edit,1,0);

  QString project_path= config->readEntry("ProjectDefaultDir", QDir::homeDirPath());
  ppath_edit->setText(project_path);

  QPushButton* ppath_button= new QPushButton(ppath_box,"NoName");
  ppath_button->setPixmap(SmallIcon("fileopen"));
  grid2->addWidget(ppath_button,1,1);
  connect(ppath_button, SIGNAL(clicked()),SLOT(slotPPathClicked()));
// ---

  QString ppathMsg = i18n("Set the start directory where to create/load projects here");
  QWhatsThis::add(ppath_edit, ppathMsg);
  QWhatsThis::add(ppath_button, ppathMsg);
  QWhatsThis::add(ppath, ppathMsg);
}

//
//************************** QT-2 directory select *************************
//
void CKDevSetupDlg::addUserInterfaceTab()
{
  QFrame* UserInterfacePage = addPage(i18n("User interface"),i18n("Type of user interface"),
  KGlobal::instance()->iconLoader()->loadIcon( "window_list", KIcon::NoGroup, KIcon::SizeMedium ));

  QGridLayout* grid = new QGridLayout(UserInterfacePage,3,1,15,7);
  QLabel* label = new QLabel(i18n("What kind of user interface do you want?"),UserInterfacePage);
  grid->addWidget(label,0,0);

  bg = new QButtonGroup(UserInterfacePage);
  grid->addWidget(bg,1,0);
  QGridLayout* innerGrid = new QGridLayout(bg,3,2,15,7);
  QPixmap pm;

  QRadioButton* childframe = new QRadioButton( i18n("Childframe mode"), bg );
  innerGrid->addWidget(childframe,0,0);
  QLabel* pictureLabelCF = new QLabel(bg);
  pm.load(locate("appdata", "pics/childfrm.png"));
  pictureLabelCF->setPixmap(pm);
  innerGrid->addWidget(pictureLabelCF,0,1);

  QRadioButton* toplevel = new QRadioButton( i18n("Toplevel mode"), bg );
  innerGrid->addWidget(toplevel,1,0);
  QLabel* pictureLabelTL = new QLabel(bg);
  pm.load(locate("appdata", "pics/toplevel.png"));
  pictureLabelTL->setPixmap(pm);
  innerGrid->addWidget(pictureLabelTL,1,1);

  QRadioButton* tabpage = new QRadioButton( i18n("Tab page mode"), bg );
  innerGrid->addWidget(tabpage,2,0);
  QLabel* pictureLabelTP = new QLabel(bg);
  pm.load(locate("appdata", "pics/tabpage.png"));
  pictureLabelTP->setPixmap(pm);
  innerGrid->addWidget(pictureLabelTP,2,1);

  childframe->setChecked(false);
  toplevel->setChecked(false);
  tabpage->setChecked(false);
//  tabpage->setEnabled(false);//until it is usuable

  switch (m_mdiMode) {
  case QextMdi::ToplevelMode:
    toplevel->setChecked(true);
    break;
  case QextMdi::ChildframeMode:
    childframe->setChecked(true);
    break;
  case QextMdi::TabPageMode:
    tabpage->setChecked(true);
    break;
  default:
    break;
  }

  bg->setFrameStyle(QFrame::Raised|QFrame::Box);
  bg->setMargin(8);
  bg->setFixedHeight(bg->sizeHint().height());

  grid->setRowStretch(2,1);

  QString cfTxt = i18n("All tool views are initially docked to the mainframe.\nEditor and browser views will live within a view area of the mainframe.");
  QWhatsThis::add(childframe, cfTxt);
  QString tlTxt = i18n("All editor, browser and tool views will be toplevel windows (directly on desktop).");
  QWhatsThis::add(toplevel, tlTxt);
  QString tpTxt = i18n("All tool views are initially docked to the mainframe.\nEditor and browser views will be stacked in a tab window.");
  QWhatsThis::add(tabpage, tpTxt);
}

void CKDevSetupDlg::slotDefault(){

  // General tab
  if(generalPage->isVisible()){
    makeSelectLineEdit->setText("make");

    autoSaveCheck->setChecked(true);
    autosaveTimeCombo->setCurrentItem(1);

    autoSwitchCheck->setChecked(true);
    defaultClassViewCheck->setChecked(false);
    logoCheck->setChecked(true);
    lastProjectCheck->setChecked(true);
    tipDayCheck->setChecked(true);
  }
  // keychooser tab
  if(keysPage->isVisible())
    keyChooser->allDefault();

  if(debuggerPage->isVisible())
  {
    dbgExternalCheck->setChecked(false);
//    QLineEdit* dbgExternalSelectLineEdit;

    // Dbg internal options
    dbgMembersCheck->setChecked(false);
    dbgAsmCheck->setChecked(false);
    dbgLibCheck->setChecked(true);
    dbgFloatCheck->setChecked(false);
    dbgTerminalCheck->setChecked(false);
  }
}

void CKDevSetupDlg::slotOkClicked(){
  QString text;
  int answer;

  // check now the documentation locations
  config->setGroup("Doc_Location");
  wantsTreeRefresh=false;

  text = qt_edit->text();
  if(text.right(1) != "/" ){
    text = text + "/";
  }
  QString qt_testfile=text+"classes.html"; // test if the path really is the qt-doc path
  answer=KMessageBox::Yes;
  if(!QFileInfo(qt_testfile).exists())
  {
    answer=KMessageBox::questionYesNo(this,i18n("The chosen path does not lead to the\n"
                                                "Qt-library documentation. Do you really want to save\n"
                                                "this value?"),
                                            i18n("The Selected Path is not Correct!"));
  }

  if (answer==KMessageBox::Yes)
  {
     config->writeEntry("doc_qt",text);
     wantsTreeRefresh |= (qt_doc_path != text);
  }
  answer=KMessageBox::Yes;    // simulate again ok...
  text = kde_edit->text();
  if(text.right(1) != "/" ){
    text = text + "/";
  }
  QString kde_testfile=text+"kdecore/index.html"; // test if the path really is the qt-doc path
  if(!QFileInfo(kde_testfile).exists())
  {
    answer=KMessageBox::questionYesNo(this,i18n("The chosen path does not lead to the\n"
                                                "KDE-library documentation. Do you really want to save\n"
                                                "this value?"),
                                            i18n("The Selected Path is not Correct!"));
  }

  if (answer==KMessageBox::Yes)
  {
     config->writeEntry("doc_kde",text);
     wantsTreeRefresh |= (kde_doc_path != text);
  }

  config->setGroup("General Options");

  bool autosave=autoSaveCheck->isChecked();
  config->writeEntry("Autosave",autosave);

  int timeCurrent=autosaveTimeCombo->currentItem();
  if(timeCurrent==0)
    config->writeEntry("Autosave Timeout",3*60*1000);
  if(timeCurrent==1)
    config->writeEntry("Autosave Timeout",5*60*1000);
  if(timeCurrent==2)
    config->writeEntry("Autosave Timeout",15*60*1000);
  if(timeCurrent==3)
    config->writeEntry("Autosave Timeout",30*60*1000);

  bool autoswitch=autoSwitchCheck->isChecked();
  config->writeEntry("Autoswitch",autoswitch);

  bool startupEditing=startupEditingCheck->isChecked();
  config->writeEntry("StartupEditing",startupEditing);

  bool defaultcv=defaultClassViewCheck->isChecked();
  config->writeEntry("DefaultClassView",defaultcv);

  config->writeEntry("Make",makeSelectLineEdit->text());

  bool logo=logoCheck->isChecked();
  config->writeEntry("Logo",logo);

  bool kdoc=kdocCheck->isChecked();
  config->writeEntry("CreateKDoc",kdoc);

  bool bCTags=useCTags->isChecked();
  config->writeEntry("use_ctags",bCTags);

  bool lastprj=lastProjectCheck->isChecked();
  config->writeEntry("LastProject",lastprj);

        config->setGroup("TipOfDay");
    config->writeEntry("RunOnStart",tipDayCheck->isChecked());

  config->setGroup("Debug");
  config->writeEntry("Use external debugger", dbgExternalCheck->isChecked());
  config->writeEntry("External debugger program", dbgExternalSelectLineEdit->text());
  config->writeEntry("Display mangled names", dbgAsmCheck->isChecked());
  config->writeEntry("Display static members", dbgMembersCheck->isChecked());
  config->writeEntry("Break on loading libs", dbgLibCheck->isChecked());
  config->writeEntry("Enable floating toolbar", dbgFloatCheck->isChecked());
  config->writeEntry("Debug on separate tty console", dbgTerminalCheck->isChecked());

  config->setGroup("QT2");
  config->writeEntry("qt2dir", qt2_edit->text());
  config->writeEntry("kde2dir", kde2_edit->text());

// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
  config->setGroup("General Options");
  config->writeEntry("ProjectDefaultDir", ppath_edit->text());
// ---

  // user interface
  config->setGroup("General Options");
  switch (bg->id(bg->selected())) {
  case 0:
          m_mdiMode = QextMdi::ChildframeMode;
    break;
  case 1:
          m_mdiMode = QextMdi::ToplevelMode;
    break;
  case 2:
          m_mdiMode = QextMdi::TabPageMode;
    break;
  default:
    break;
  }

  // code completion
  completionOptsDlg->slotSettingsChanged();
  // code template
  codeTemplateOptsDlg->slotSettingsChanged();
  partSelectWidget->accept();

#if QT_VERSION < 300
  m_accel->setKeyDict(keyMap);
#else
  keyChooser->commitChanges();
#endif
  m_accel->writeSettings(config);
  config->sync();
  accept();
}

void CKDevSetupDlg::slotQtClicked(){
  QString dir;
  config->setGroup("Doc_Location");
  dir = KFileDialog::getExistingDirectory(config->readEntry("doc_qt", QT_DOCDIR));
  if (!dir.isEmpty()){
    qt_edit->setText(dir);

    if(dir.right(1) != "/" ){
     dir = dir + "/";
    }
    QString qt_testfile=dir+"classes.html"; // test if the path really is the qt-doc path
    if(!QFileInfo(qt_testfile).exists())
      KMessageBox::error(this,i18n("The chosen path does not lead to the\n"
                                   "Qt-library documentation. Please choose the\n"
                                   "correct path."),
                                i18n("The Selected Path is not Correct!"));
  }
}


void CKDevSetupDlg::slotKDEClicked(){
  QString dir;
  config->setGroup("Doc_Location");
  dir = KFileDialog::getExistingDirectory(config->readEntry("doc_kde", KDELIBS_DOCDIR));
  if (!dir.isEmpty()){
    kde_edit->setText(dir);

    if(dir.right(1) != "/" ){
     dir = dir + "/";
    }

    QString kde_testfile=dir+"kdecore/index.html"; // test if the path really is the kde-doc path
    if(!QFileInfo(kde_testfile).exists())
      KMessageBox::error(this,i18n("The chosen path does not lead to the\n"
                                   "KDE-library documentation. Please choose the\n"
                                   "correct path or choose 'Update' to create a new\n"
                                   "documentation"),
                                i18n("The Selected Path is not Correct!"));
  }
}

void CKDevSetupDlg::slotKDEUpdateReq(){
  QString new_path;
  ((CKDevelop*) parent())->slotOptionsUpdateKDEDocumentation();
  config->setGroup("Doc_Location");
  new_path=config->readEntry("doc_kde", KDELIBS_DOCDIR);

  if (kde_doc_path != new_path)
    kde_edit->setText(new_path);
}

void CKDevSetupDlg::slotSetDebug()
{
  bool externalDbg = dbgExternalCheck->isChecked();

  // external options
  dbgExternalGroup->setEnabled(externalDbg);
  dbgExternalSelectLineEdit->setEnabled(externalDbg);
  dbgSelectCmdLabel->setEnabled(externalDbg);

  // internal options
  dbgInternalGroup->setEnabled(!externalDbg);
  dbgMembersCheck->setEnabled(!externalDbg);
  dbgAsmCheck->setEnabled(!externalDbg);
  dbgLibCheck->setEnabled(!externalDbg);
  dbgFloatCheck->setEnabled(!externalDbg);
  dbgTerminalCheck->setEnabled(!externalDbg);
}

void CKDevSetupDlg::slotQt2Clicked(){
  QString dir;
  config->setGroup("QT2");
  dir = KFileDialog::getExistingDirectory(config->readEntry("qt2dir"));
  if (!dir.isEmpty()){
    qt2_edit->setText(dir);

  }
  QString qt_testfile=dir+"include/qapp.h"; // test if the path really is the qt2 path
  if(!QFileInfo(qt_testfile).exists())
          KMessageBox::error(this,i18n("The chosen path does not lead to the\n"
                                 "Qt-2.x root directory. Please choose the\n"
                                 "correct path."),
                             i18n("The Selected Path is not Correct!"));

}
void CKDevSetupDlg::slotKDE2Clicked(){
  QString dir;
  config->setGroup("QT2");
  dir = KFileDialog::getExistingDirectory(config->readEntry("kde2dir"));
  if (!dir.isEmpty()){
    kde2_edit->setText(dir);

  }
  QString kde_testfile=dir+"include/kmessagebox.h"; // test if the path really is the kde2 path
  if(!QFileInfo(kde_testfile).exists())
          KMessageBox::error(this,i18n("The chosen path does not lead to the\n"
                                 "KDE-2.x root directory. Please choose the\n"
                                 "correct path."),
                             i18n("The Selected Path is not Correct!"));

}

// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
void CKDevSetupDlg::slotPPathClicked(){
  QString dir;
  config->setGroup("General Options");
  dir = KFileDialog::getExistingDirectory(config->readEntry("ProjectDefaultDir", QDir::homeDirPath()));
  if (!dir.isEmpty()){
    ppath_edit->setText(dir);
  }
}
// ---

void CKDevSetupDlg::addEnhancedCodingTab()
{
    QFrame* additionalPage = addPage(i18n("Enhanced Coding"),
                                     i18n("Enhanced Coding Configuration"),
                                     KGlobal::instance()->iconLoader()->loadIcon( "source", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *grid = new QGridLayout( additionalPage );
    QTabWidget* tab = new QTabWidget( additionalPage );
    CKDevelop* pDevelop = (CKDevelop*) parent();
    completionOptsDlg = new CCompletionOpts( pDevelop, additionalPage );
    codeTemplateOptsDlg = new CCodeTemplateOpts( additionalPage );
    tab->addTab( completionOptsDlg, i18n("Code Completion") );
    tab->addTab( codeTemplateOptsDlg, i18n("Code Template") );
    grid->addWidget( tab, 0, 0 );

}

void CKDevSetupDlg::addPartSelectTab()
{
    QFrame* additionalPage = addPage(i18n("KDevelop Parts"),
                                     i18n("KDevelop Parts Configuration"),
                                     KGlobal::instance()->iconLoader()->loadIcon( "source", KIcon::NoGroup, KIcon::SizeMedium ));

    QGridLayout *grid = new QGridLayout( additionalPage );
    CKDevelop* pDevelop = (CKDevelop*) parent();
    partSelectWidget = new PartSelectWidget( additionalPage );
    grid->addWidget( partSelectWidget, 0, 0 );
}

#include "ckdevsetupdlg.moc"
