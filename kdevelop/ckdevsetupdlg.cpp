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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qfileinfo.h>

#include <kmsgbox.h>
#include <klocale.h>

// SETUP DIALOG
CKDevSetupDlg::CKDevSetupDlg(KAccel* accel_pa, QWidget *parent, const char *name )
  : QTabDialog( parent, name,TRUE )
{
  accel = accel_pa;
  wantsTreeRefresh=false;

  setCaption( i18n("KDevelop Setup" ));
  config=kapp->getConfig();
  
  // ****************** the General Tab ********************
  w1 = new QWidget( this, "general" );
  
  
  QButtonGroup* makeGroup;
  makeGroup = new QButtonGroup( w1, "makeGroup" );
  makeGroup->setGeometry( 10, 10, 400, 60 );
  makeGroup->setFrameStyle( 49 );
  makeGroup->setTitle(i18n( "Make-Command" ));
  makeGroup->setAlignment( 1 );
  makeGroup->lower();

  QLabel* makeSelectLabel;
  makeSelectLabel = new QLabel( w1, "makeSelectLabel" );
  makeSelectLabel->setGeometry( 20, 30, 210, 25 );
  makeSelectLabel->setText(i18n("Select Make-Command:"));
  makeSelectLabel->setAlignment( 289 );
  makeSelectLabel->setMargin( -1 );

  config->setGroup("General Options");
  QString make_cmd=config->readEntry("Make","make");

  makeSelectLineEdit = new QLineEdit( w1, "makeSelectLineEdit" );
  makeSelectLineEdit->setGeometry( 270, 30, 130, 25 );
  makeSelectLineEdit->setText(make_cmd);

  KQuickHelp::add(makeGroup,
		  KQuickHelp::add(makeSelectLabel,
		  KQuickHelp::add(makeSelectLineEdit,i18n("Make-Command\n\n"
							  "Select your system's make-command.\n"
							  "Usually, this is make, FreeBSD users\n"
							  "may use gmake. Mind that you can also\n"
							  "add option parameters to your make-binary\n"
					  "as well."))));
  
  bool autoSave=config->readBoolEntry("Autosave",true);
  
  QButtonGroup* autosaveGroup;
  autosaveGroup = new QButtonGroup( w1, "autosaveGroup" );
  autosaveGroup->setGeometry( 10, 90, 400, 90 );
  autosaveGroup->setFrameStyle( 49 );
  autosaveGroup->setTitle( i18n("Autosave") );
  autosaveGroup->setAlignment( 1 );
  //  autosaveGroup->insert( autoSaveCheck );
  autosaveGroup->lower();
  
  autoSaveCheck = new QCheckBox( w1, "autoSaveCheck" );
  autoSaveCheck->setGeometry( 20, 110, 210, 30 );
  autoSaveCheck->setText(i18n("enable Autosave"));
  autoSaveCheck->setAutoRepeat( FALSE );
  autoSaveCheck->setAutoResize( FALSE );
  autoSaveCheck->setChecked(autoSave);
  
  QLabel* autosaveTimeLabel;
  autosaveTimeLabel = new QLabel( w1, "autosaveTimeLabel" );
  autosaveTimeLabel->setGeometry( 20, 140, 210, 25 );
  autosaveTimeLabel->setText(i18n("Select Autosave time-interval:"));
  autosaveTimeLabel->setAlignment( 289 );
  autosaveTimeLabel->setMargin( -1 );
  autosaveTimeLabel->setEnabled(autoSave);
  
  autosaveTimeCombo = new QComboBox( FALSE, w1, "autosaveTimeCombo" );
  autosaveTimeCombo->setGeometry( 270, 140, 130, 25 );
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

  KQuickHelp::add(autosaveTimeLabel,	
		  KQuickHelp::add(autosaveTimeCombo,	
				  KQuickHelp::add(autoSaveCheck,	
						  KQuickHelp::add(autosaveGroup,i18n("Autosave\n\n"
										     "If autosave is enabled, your currently\n"
										     "changed files will be saved after the\n"
										     "time-interval selected times out.\n\n"
										     "Please select your timeout-value.\n"
										     "Available are: 3 minutes, 5 minutes,\n"
										     "15 minutes and 30 minutes.")))));
  
  QButtonGroup* autoswitchGroup;
  autoswitchGroup = new QButtonGroup( w1, "autoswitchGroup" );
  autoswitchGroup->setGeometry( 10, 190, 400, 60 );
  autoswitchGroup->setFrameStyle( 49 );
  autoswitchGroup->setTitle(i18n( "Autoswitch") );
  autoswitchGroup->setAlignment( 1 );
  //  autoswitchGroup->insert( autoSwitchCheck );
  autoswitchGroup->lower();
  
  autoSwitchCheck = new QCheckBox( w1, "autoSwitchCheck" );
  autoSwitchCheck->setGeometry( 20, 210, 180, 30 );

  autoSwitchCheck->setText(i18n("enable Autoswitch"));
  autoSwitchCheck->setAutoRepeat( FALSE );
  autoSwitchCheck->setAutoResize( FALSE );
  bool autoSwitch=config->readBoolEntry("Autoswitch",true);
  autoSwitchCheck->setChecked( autoSwitch );

  defaultClassViewCheck = new QCheckBox( w1, "defaultClassViewCheck" );
  defaultClassViewCheck->setGeometry( 220, 210, 180, 30 );
  defaultClassViewCheck->setText(i18n( "use Class View as default"));
  defaultClassViewCheck->setAutoRepeat( FALSE );
  defaultClassViewCheck->setAutoResize( FALSE );
  bool defaultcv=config->readBoolEntry("DefaultClassView",true);
  defaultClassViewCheck->setChecked( defaultcv );
  KQuickHelp::add(defaultClassViewCheck, i18n("use Class View as default\n\n"
					      "If this is enabled, KDevelop\n"
					      "will automatically switch to\n"
					      "the Class Viewer when switching.\n"
					      "When disabled, KDevelop will\n"
					      "use Logical File Viewer for\n"
					      "autoswitching."));

  KQuickHelp::add(autoSwitchCheck,
		  KQuickHelp::add(autoswitchGroup,i18n("Autoswitch\n\n"
						       "If autoswitch is enabled, KDevelop\n"
						       "will open windows in the working\n"
						       "view automatically according to\n"
						       "most needed functionality.\n\n"
						       "Disableing autoswitch means you\n"
						       "will have to switch to windows\n"
						       "yourself, including turning on and\n"
						       "off the outputwindow.")));	

//  connect( autoSwitchCheck, SIGNAL(toggled(bool)),defaultClassViewCheck, SLOT(setEnabled(bool)));
  
  QButtonGroup* startupGroup = new QButtonGroup( w1, "startupGroup" );
  startupGroup->setGeometry( 10, 260, 400, 70 );
  startupGroup->setFrameStyle( 49 );
  startupGroup->setTitle(i18n("Startup"));
  startupGroup->setAlignment( 1 );
  //	startupGroup->insert( logoCheck );
  //	startupGroup->insert( lastProjectCheck );
  startupGroup->lower();
  
  KQuickHelp::add(startupGroup, i18n("Startup\n\n"
	                  "The Startup group offers options for\n"
				     "starting KDevelop"));
  
  config->setGroup("General Options");
  bool logo=config->readBoolEntry("Logo",true);
  bool lastprj=config->readBoolEntry("LastProject",true);
  
  logoCheck = new QCheckBox( w1, "logoCheck" );
  logoCheck->setGeometry( 20, 275, 190, 25 );
  logoCheck->setText(i18n("Startup Logo"));
  logoCheck->setAutoRepeat( FALSE );
  logoCheck->setAutoResize( FALSE );
  logoCheck->setChecked( logo );
  
  KQuickHelp::add(logoCheck, i18n("Startup Logo\n\n"
	                  "If Startup Logo is enabled, KDevelop will show the\n"
	                  "logo picture while it is starting."));
  
  lastProjectCheck = new QCheckBox( w1, "lastProjectCheck" );
  lastProjectCheck->setGeometry( 20, 295, 190, 25 );
  lastProjectCheck->setText(i18n("Load last project"));
  lastProjectCheck->setAutoRepeat( FALSE );
  lastProjectCheck->setAutoResize( FALSE );
  lastProjectCheck->setChecked( lastprj );
  
  KQuickHelp::add(lastProjectCheck, i18n("Load last project\n\n"
                    "If Load last project is enabled, KDevelop will load\n"
                    "the last project used."));

	
  config->setGroup("TipOfTheDay");
  bool tip=config->readBoolEntry("show_tod",true);

	
	tipDayCheck = new QCheckBox( w1, "tipDayCheck" );
	tipDayCheck->setGeometry( 220, 275, 150, 25 );
	tipDayCheck->setText(i18n("Tip of the Day"));
	tipDayCheck->setAutoRepeat( FALSE );
	tipDayCheck->setAutoResize( FALSE );
	tipDayCheck->setChecked( tip );

	KQuickHelp::add(tipDayCheck, i18n("Tip of the Day\n\n"
	                  "If Tip of the Day is enabled, KDevelop will show the\n"
	                  "Tip of the Day every time it starts."));
	
  connect( autoSwitchCheck, SIGNAL(toggled(bool)),parent, SLOT(slotOptionsAutoswitch(bool)) );
  connect( autoSwitchCheck, SIGNAL(toggled(bool)),defaultClassViewCheck, SLOT(setEnabled(bool)));
  connect( autosaveTimeCombo, SIGNAL(activated(int)),parent, SLOT(slotOptionsAutosaveTime(int)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),parent, SLOT(slotOptionsAutosave(bool)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),autosaveTimeLabel, SLOT(setEnabled(bool)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),autosaveTimeCombo, SLOT(setEnabled(bool)) );
  connect( defaultClassViewCheck,SIGNAL(toggled(bool)),parent,SLOT(slotOptionsDefaultCV(bool)));

  // ****************** the Keys Tab ***************************
  
  dict = new QDict<KKeyEntry>( accel->keyDict() );
  //  KKeyChooser* w2 = new KKeyChooser ( dict,this);
  w2 = new QWidget( this, "keys" );
  w21 = new KKeyChooser ( dict,w2,true);
  w21->setGeometry(15,10,395,320);
  
  
  // ****************** the Documentation Tab ********************
  w = new QWidget( this, "documentaion" );
  config->setGroup("Doc_Location");
  
  KQuickHelp::add(w, i18n("Enter the path to your QT and KDE-Libs\n"
				"Documentation for the Documentation Browser.\n"
				"QT usually comes with complete Documentation\n"
				"whereas for KDE you can create the Documentation\n"
				"easily by pressing the Update button below."));
	
  qt_edit = new QLineEdit( w, "qt_edit" );
  qt_edit->setGeometry( 170, 40, 190, 30 );
  qt_doc_path= config->readEntry("doc_qt", QT_DOCDIR);
  qt_edit->setText(qt_doc_path);
  qt_edit->setMaxLength( 32767 );
  
  QPushButton* qt_button;
  qt_button = new QPushButton( w, "qt_button" );
  qt_button->setGeometry( 370, 40, 30, 30 );
  QPixmap pix;
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  qt_button->setPixmap(pix);
  connect(qt_button,SIGNAL(clicked()),SLOT(slotQtClicked()));
  
  QLabel* qt_label;
  qt_label = new QLabel( w, "qt_label" );
  qt_label->setGeometry( 20, 40, 140, 30 );
  qt_label->setText( i18n("Qt-Library-Doc:") );
  
  
  KQuickHelp::add(qt_edit,
  KQuickHelp::add(qt_button,
  KQuickHelp::add(qt_label, i18n("Enter the path to your QT-Documentation\n"
				 "here. To access the path easier please\n"
				 "press the pushbutton on the right to change\n"
				 "directories.\n\n"
				 "Usually the QT-Documentation is\n"
				 "located in <i><blue>$QTDIR/html</i>"))));	
  
  kde_edit = new QLineEdit( w, "kde_edit");
  kde_edit->setGeometry( 170, 90, 190, 30 );
  kde_doc_path=config->readEntry("doc_kde", KDELIBS_DOCDIR);
  kde_edit->setText(kde_doc_path);
  kde_edit->setMaxLength( 32767 );
  kde_edit->setEchoMode( QLineEdit::Normal );
  kde_edit->setFrame( TRUE );
  
  QPushButton* kde_button;
  kde_button = new QPushButton( w, "kde_button" );
  kde_button->setGeometry( 370, 90, 30, 30 );
  kde_button->setPixmap(pix);
  connect(kde_button,SIGNAL(clicked()),SLOT(slotKDEClicked()));
  
  QLabel* kde_label;
  kde_label = new QLabel( w, "kde_label" );
  kde_label->setGeometry( 20, 90, 140, 30 );
  kde_label->setText( i18n("KDE-Libraries-Doc:") );
  
  KQuickHelp::add(kde_edit,
  KQuickHelp::add(kde_button,
  KQuickHelp::add(kde_label,i18n("Enter the path to your KDE-Documentation\n"
				 "here. To access the path easier please\n"
				 "press the pushbutton on the right to change\n"
				 "directories.\n\n"
				 "If you have no kdelibs Documentation installed,\n"
				 "you can create it by selecting the Update button\n"
				 "below."))));
  
  QLabel* update_label;
  update_label = new QLabel( w, "update_label" );
  update_label->setGeometry( 20, 190, 260, 30 );
  update_label->setText(i18n("Update KDE-Documentation :"));
  update_label->setAlignment( 289 );
  update_label->setMargin( -1 );
  
  QPushButton* update_button;
  update_button = new QPushButton( w, "update_button" );
  update_button->setGeometry( 290, 190, 110, 30 );
  connect( update_button, SIGNAL(clicked()), SLOT(slotKDEUpdateReq()) );
  update_button->setText(i18n("Update..."));
  update_button->setAutoRepeat( FALSE );
  update_button->setAutoResize( FALSE );
  
  KQuickHelp::add(update_label,
  KQuickHelp::add(update_button,i18n("Update KDE-Documentation\n\n"
				     "This lets you create or update the\n"
				     "HTML-documentation of the KDE-libs.\n"
				     "Mind that you have kdoc installed to\n"
				     "use this function. Also, the kdelibs\n"
				     "sources have to be available to create\n"
				     "the documentation, as well as the \n"
				     "Qt-Documentation path has to be set to\n"
				     "cross-reference the KDE-Documentation\n"
				     "with the Qt-classes.")));
  
  QLabel* create_label;
  create_label = new QLabel( w, "create_label" );
  create_label->setGeometry( 20, 230, 260, 30 );
  create_label->setText(i18n("Create Search Database :") );
  create_label->setAlignment( 289 );
  create_label->setMargin( -1 );
  
  QPushButton* create_button;	
  create_button = new QPushButton( w, "create_button" );
  create_button->setGeometry( 290, 230, 110, 30 );
  connect( create_button, SIGNAL(clicked()),parent, SLOT(slotOptionsCreateSearchDatabase()) );
  create_button->setText(i18n("Create..."));
  create_button->setAutoRepeat( FALSE );
  create_button->setAutoResize( FALSE );
  
  KQuickHelp::add(create_label,
		  KQuickHelp::add(create_button,i18n("Create Search Database\n\n"
						     "This will create a search database for glimpse\n"
						     "which will be used to look up marked text in\n"
						     "the documentation. We recommend updating the\n"
						     "database each time you've changed the documentation\n"
						     "e.g. after a kdelibs-update or installing a new\n"
						     "Qt-library version.")));
  
#ifdef WITH_KDOC2
  kdocCheck = new QCheckBox( w, "kdocCheck" );
  kdocCheck->setGeometry( 20, 270, 350, 30 );
  kdocCheck->setText(i18n("Create also KDOC-reference of your project"));
  kdocCheck->setAutoRepeat( FALSE );
  kdocCheck->setAutoResize( FALSE );
  bool bCreateKDoc;

  config->setGroup("General Options");
  bCreateKDoc = config->readBoolEntry("CreateKDoc", false);

  kdocCheck->setChecked( bCreateKDoc );

  KQuickHelp::add(kdocCheck, i18n("Create KDOC-reference of your project\n\n"
                    "If this is enabled, on creating the API-Documentation KDoc creates also\n"
                    "a cross reference file of your project into the seleceted kdoc-reference\n"
                    "directory."));
#endif


  QButtonGroup* docOptionsGroup;
  docOptionsGroup = new QButtonGroup( w, "docOptionsGroup" );
#ifdef WITH_KDOC2
  docOptionsGroup->setGeometry( 10, 160, 400, 150 );
#else
  docOptionsGroup->setGeometry( 10, 160, 400, 110 );
#endif
  docOptionsGroup->setFrameStyle( 49 );
  docOptionsGroup->setTitle(i18n("Options"));
  docOptionsGroup->setAlignment( 1 );
  docOptionsGroup->insert( update_button );
  docOptionsGroup->insert( create_button );
#ifdef WITH_KDOC2
  docOptionsGroup->insert( kdocCheck );
#endif
  docOptionsGroup->lower();
  
  QButtonGroup* docGroup;
  docGroup = new QButtonGroup( w, "docGroup" );
  docGroup->setGeometry( 10, 10, 400, 140 );
  docGroup->setFrameStyle( 49 );
  docGroup->setTitle(i18n("Directories"));
  docGroup->setAlignment( 1 );
  docGroup->insert( qt_button );
  docGroup->insert( kde_button );
  docGroup->lower();
  
  // ****************** the Debugger Tab ***************************

  config->setGroup("Debug");
  bool useExternalDbg = config->readBoolEntry("Use external debugger", false);
  QString dbg_cmd=config->readEntry("External debugger program","kdbg");
  bool displayMangledNames = config->readBoolEntry("Display mangled names", false);
  bool displayStaticMembers = config->readBoolEntry("Display static members", false);
  bool setBPsOnLibLoad = config->readBoolEntry("Break on loading libs", false);
  bool dbgFloatingToolbar = config->readBoolEntry("Enable floating toolbar", false);

  w3 = new QWidget( this, "debug" );

  dbgExternalCheck = new QCheckBox( w3, "dbgExternal" );
  dbgExternalCheck->setGeometry( 20, 10, 210, 25 );
  dbgExternalCheck->setText(i18n("Use external debugger"));
  dbgExternalCheck->setAutoRepeat( FALSE );
  dbgExternalCheck->setAutoResize( FALSE );
  dbgExternalCheck->setChecked(useExternalDbg);

  KQuickHelp::add(dbgExternalCheck, i18n("Select internal or external debugger\n\n"
	                  "Choose whether to use an external debugger\n"
	                  "or the internal debugger within kdevelop\n"
                    "The internal debugger is a frontend to gdb"));

  dbgExternalGroup = new QButtonGroup( w3, "dbgExternalGroup" );
  dbgExternalGroup->setGeometry( 10, 40, 400, 70 );
  dbgExternalGroup->setFrameStyle( 49 );
  dbgExternalGroup->setTitle(i18n( "External" ));
  dbgExternalGroup->setAlignment( 1 );
  dbgExternalGroup->lower();

  dbgSelectCmdLabel = new QLabel( w3, "dbgSelectLabel" );
  dbgSelectCmdLabel->setGeometry( 20, 70, 210, 25 );
  dbgSelectCmdLabel->setText(i18n("Select debug command:"));
  dbgSelectCmdLabel->setAlignment( 289 );
  dbgSelectCmdLabel->setMargin( -1 );

  dbgExternalSelectLineEdit = new QLineEdit( w3, "dbgExternalSelectLineEdit" );
  dbgExternalSelectLineEdit->setGeometry( 270, 70, 130, 25 );
  dbgExternalSelectLineEdit->setText(dbg_cmd);

  KQuickHelp::add(dbgSelectCmdLabel,
    KQuickHelp::add(dbgExternalSelectLineEdit,
        i18n("Identify the external debugger\n\n"
	                  "Enter the program name you wish to run\n"
	                  "as your debugger")));

  dbgInternalGroup = new QButtonGroup( w3, "dbgInternalGroup" );
  dbgInternalGroup->setGeometry( 10, 130, 400, 150 );
  dbgInternalGroup->setFrameStyle( 49 );
  dbgInternalGroup->setTitle(i18n( "Internal" ));
  dbgInternalGroup->setAlignment( 1 );
  dbgInternalGroup->lower();

  dbgMembersCheck = new QCheckBox( w3, "dbgMembers" );
  dbgMembersCheck->setGeometry( 20, 150, 210, 25 );
  dbgMembersCheck->setText(i18n("Display static members"));
  dbgMembersCheck->setAutoRepeat( FALSE );
  dbgMembersCheck->setAutoResize( FALSE );
  dbgMembersCheck->setChecked(displayStaticMembers);
  KQuickHelp::add(dbgMembersCheck, i18n("Display static members\n\n"
	                  "Displaying static members makes gdb slower in\n"
                    "producing data within kde and qt.\n"
                    "It may change the \"signature\" of the data\n"
                    "which QString and friends rely on.\n"
                    "But if you need to debug into these values then\n"
                    "check this option" ));

  dbgAsmCheck = new QCheckBox( w3, "dbgMembers" );
  dbgAsmCheck->setGeometry( 20, 180, 210, 25 );
  dbgAsmCheck->setText(i18n("Display mangled names"));
  dbgAsmCheck->setAutoRepeat( FALSE );
  dbgAsmCheck->setAutoResize( FALSE );
  dbgAsmCheck->setChecked(displayMangledNames);
  KQuickHelp::add(dbgAsmCheck, i18n("Display mangled names\n\n"
	                  "When displaying the disassembled code you\n"
	                  "can select to see the methods mangled names\n"
                    "However, non-mangled names are easier to read." ));

  dbgLibCheck = new QCheckBox( w3, "dbgMembers" );
  dbgLibCheck->setGeometry( 20, 210, 310, 25 );
  dbgLibCheck->setText(i18n("Try setting BPs on lib load"));
  dbgLibCheck->setAutoRepeat( FALSE );
  dbgLibCheck->setAutoResize( FALSE );
  dbgLibCheck->setChecked(setBPsOnLibLoad);
  KQuickHelp::add(dbgLibCheck, i18n("Set pending breakpoints on loading a library\n\n"
	                  "If GDB hasn't seen a library that will be loaded via\n"
	                  "\"dlopen\" then it'll refuse to set a breakpoint in that code.\n"
                    "We can get gdb to stop on a library load and hence\n"
                    "try to set the pending breakpoints. See docs for more\n"
                    "details and a \"gotcha\" relating to this behaviour.\n\n"
                    "If you are not \"dlopen\"ing libs leave this off." ));

  dbgFloatCheck = new QCheckBox( w3, "dbgFloatToolbar" );
  dbgFloatCheck->setGeometry( 20, 240, 310, 25 );
  dbgFloatCheck->setText(i18n("Enable floating toolbar"));
  dbgFloatCheck->setAutoRepeat( FALSE );
  dbgFloatCheck->setAutoResize( FALSE );
  dbgFloatCheck->setChecked(dbgFloatingToolbar);
  KQuickHelp::add(dbgFloatCheck, i18n("Enable flaoting toolbar\n\n"
	                  "Use the floating toolbar. This toolbar always stays\n"
                    "on top of all windows so that if the app covers KDevelop\n"
                    "you have control of the app though the small toolbar\n"
                    "Also this toolbar can be docked to the panel\n"
                    "This toolbar is in addition to the toolbar in KDevelop" ));

  slotSetDebug();
  connect( dbgExternalCheck, SIGNAL(toggled(bool)), SLOT(slotSetDebug()));

//************************** QT-2 directory select *************************//
  w4 = new QWidget( this, "pat" );
  config->setGroup("QT2");

  QGroupBox* kde2_box= new QGroupBox(w4,"NoName");
  kde2_box->setGeometry(10,10,400,190);
  kde2_box->setMinimumSize(0,0);
  kde2_box->setTitle(i18n("Qt 2.x / KDE path"));

  QLabel* qt2= new QLabel(w4,"NoName");
  qt2->setGeometry(30,40,300,30);
  qt2->setMinimumSize(0,0);
  qt2->setText(i18n("Qt 2.x directory:"));

  qt2_edit= new QLineEdit(w4,"NoName");
  qt2_edit->setGeometry(30,70,300,30);
  qt2_edit->setMinimumSize(0,0);
  qt2_edit->setMaxLength( 32767 );

  QString qt2_path= config->readEntry("qt2dir");
  qt2_edit->setText(qt2_path);

  QPushButton* qt2_button= new QPushButton(w4,"NoName");
  qt2_button->setGeometry(340,70,30,30);
  qt2_button->setMinimumSize(0,0);
  qt2_button->setPixmap(pix);

  KQuickHelp::add(qt2_edit,
  KQuickHelp::add(qt2_button,
  KQuickHelp::add(qt2,i18n("Set the root directory path leading to your Qt 2.x path, e.g. /usr/lib/qt-2.0"))));


  QLabel* kde2= new QLabel(w4,"NoName");
  kde2->setGeometry(30,110,300,30);
  kde2->setMinimumSize(0,0);
  kde2->setText(i18n("KDE 2.x directory:"));

  kde2_edit= new QLineEdit(w4,"NoName");
  kde2_edit->setGeometry(30,150,300,30);
  kde2_edit->setMinimumSize(0,0);

  QString kde2_path= config->readEntry("kde2dir");
  kde2_edit->setText(kde2_path);

  QPushButton* kde2_button= new QPushButton(w4,"NoName");
  kde2_button->setGeometry(340,150,30,30);
  kde2_button->setMinimumSize(0,0);
	kde2_button->setPixmap(pix);

  KQuickHelp::add(kde2_edit,
	KQuickHelp::add(kde2_button,
	KQuickHelp::add(kde2,i18n("Set the root directory path leading to your KDE 2 includes/libraries, e.g. /opt/kde2"))));

  connect(qt2_button,SIGNAL(clicked()),SLOT(slotQt2Clicked()));
  connect(kde2_button, SIGNAL(clicked()),SLOT(slotKDE2Clicked()));



  QGroupBox* ppath_box= new QGroupBox(w4,"NoName");
  ppath_box->setGeometry(10,210,400,100);
  ppath_box->setMinimumSize(0,0);
  ppath_box->setTitle(i18n("Default Project Path"));

// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
  config->setGroup("General Options");
  QLabel* ppath= new QLabel(w4,"NoName");
  ppath->setGeometry(30,230,300,30);
  ppath->setMinimumSize(0,0);
  ppath->setText(i18n("Project Path:"));

  ppath_edit= new QLineEdit(w4,"NoName");
  ppath_edit->setGeometry(30,260,300,30);
  ppath_edit->setMinimumSize(0,0);

  QString project_path= config->readEntry("ProjectDefaultDir", QDir::homeDirPath());
  ppath_edit->setText(project_path);

  QPushButton* ppath_button= new QPushButton(w4,"NoName");
  ppath_button->setGeometry(340,260,30,30);
  ppath_button->setMinimumSize(0,0);
	ppath_button->setPixmap(pix);
  connect(ppath_button, SIGNAL(clicked()),SLOT(slotPPathClicked()));
// ---

  KQuickHelp::add(ppath_edit,
	KQuickHelp::add(ppath_button,
	KQuickHelp::add(ppath,i18n("Set the start directory where to create/load projects here"))));



  // *********** tabs ****************
  addTab(w1, i18n("General"));
  addTab(w2, i18n("Keys"));
  addTab( w, i18n("Documentation" ));
  addTab(w3, i18n("Debugger" ));
  addTab(w4, i18n("Path"));

  // **************set the button*********************
  setDefaultButton(i18n("Default"));
  setOkButton(i18n("OK"));
  setCancelButton(i18n("Cancel"));
  connect( this, SIGNAL(defaultButtonPressed()), SLOT(slotDefault()) );
  connect( this, SIGNAL(applyButtonPressed()), SLOT(slotOkClicked()) );
  connect( this, SIGNAL(applyButtonPressed()),parent, SLOT(slotOptionsMake()) );
  resize(440,420);
}

void CKDevSetupDlg::slotDefault(){

  // General tab
  if(w1->isVisible()){
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
  if(w2->isVisible())
    w21->allDefault();
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
  answer=1;
  if(!QFileInfo(qt_testfile).exists())
  {
    answer=KMsgBox::yesNo(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "Qt-library documentation. Do you really want to save\n"
                                                              "this value?"));
  }

  if (answer==1)
  {
     config->writeEntry("doc_qt",text);
     wantsTreeRefresh |= (qt_doc_path != text);
  }
  answer=1;    // simulate again ok...
  text = kde_edit->text();
  if(text.right(1) != "/" ){
    text = text + "/";
  }
  QString kde_testfile=text+"kdecore/index.html"; // test if the path really is the qt-doc path
  if(!QFileInfo(kde_testfile).exists())
  {
    answer=KMsgBox::yesNo(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "KDE-library documentation. Do you really want to save\n"
                                                              "this value?"));
  }

  if (answer==1)
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

  bool defaultcv=defaultClassViewCheck->isChecked();
  config->writeEntry("DefaultClassView",defaultcv);

  config->writeEntry("Make",makeSelectLineEdit->text());

  bool logo=logoCheck->isChecked();
  config->writeEntry("Logo",logo);

#ifdef WITH_KDOC2
  bool kdoc=kdocCheck->isChecked();
  config->writeEntry("CreateKDoc",kdoc);
#endif

  bool lastprj=lastProjectCheck->isChecked();
  config->writeEntry("LastProject",lastprj);
	
	config->setGroup("TipOfTheDay");
  config->writeEntry("show_tod",tipDayCheck->isChecked());

  config->setGroup("Debug");
  config->writeEntry("Use external debugger", dbgExternalCheck->isChecked());
  config->writeEntry("External debugger program", dbgExternalSelectLineEdit->text());
  config->writeEntry("Display mangled names", dbgAsmCheck->isChecked());
  config->writeEntry("Display static members", dbgMembersCheck->isChecked());
  config->writeEntry("Break on loading libs", dbgLibCheck->isChecked());
  config->writeEntry("Enable floating toolbar", dbgFloatCheck->isChecked());

  config->setGroup("QT2");
  config->writeEntry("qt2dir", qt2_edit->text());
  config->writeEntry("kde2dir", kde2_edit->text());

// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
  config->setGroup("General Options");
  config->writeEntry("ProjectDefaultDir", ppath_edit->text());	
// ---

  accel->setKeyDict( *dict);
  accel->writeSettings(config);
  config->sync();
  accept();
}

void CKDevSetupDlg::slotQtClicked(){
  QString dir;
  config->setGroup("Doc_Location");
  dir = KFileDialog::getDirectory(config->readEntry("doc_qt", QT_DOCDIR));
  if (!dir.isEmpty()){
    qt_edit->setText(dir);

    if(dir.right(1) != "/" ){
     dir = dir + "/";
    }
    QString qt_testfile=dir+"classes.html"; // test if the path really is the qt-doc path
    if(!QFileInfo(qt_testfile).exists())
      KMsgBox::message(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "Qt-library documentation. Please choose the\n"
                                                              "correct path."),KMsgBox::EXCLAMATION);
  }
}


void CKDevSetupDlg::slotKDEClicked(){
  QString dir;
  config->setGroup("Doc_Location");
  dir = KFileDialog::getDirectory(config->readEntry("doc_kde", KDELIBS_DOCDIR));
  if (!dir.isEmpty()){
    kde_edit->setText(dir);

    if(dir.right(1) != "/" ){
     dir = dir + "/";
    }

    QString kde_testfile=dir+"kdecore/index.html"; // test if the path really is the kde-doc path
    if(!QFileInfo(kde_testfile).exists())
      KMsgBox::message(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "KDE-library documentation. Please choose the\n"
                                                              "correct path or choose 'Update' to create a new\n"
                                                              "documentation"),KMsgBox::EXCLAMATION);
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
}

void CKDevSetupDlg::slotQt2Clicked(){
  QString dir;
  config->setGroup("QT2");
  dir = KFileDialog::getDirectory(config->readEntry("qt2dir"));
  if (!dir.isEmpty()){
    qt2_edit->setText(dir);

  }
  QString qt_testfile=dir+"include/qapp.h"; // test if the path really is the qt2 path
  if(!QFileInfo(qt_testfile).exists())
  	KMsgBox::message(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "Qt-2.x root directory. Please choose the\n"
                                                              "correct path."),KMsgBox::EXCLAMATION);

}
void CKDevSetupDlg::slotKDE2Clicked(){
  QString dir;
  config->setGroup("QT2");
  dir = KFileDialog::getDirectory(config->readEntry("kde2dir"));
  if (!dir.isEmpty()){
    kde2_edit->setText(dir);

  }
  QString kde_testfile=dir+"include/kmessagebox.h"; // test if the path really is the kde2 path
  if(!QFileInfo(kde_testfile).exists())
  	KMsgBox::message(this,i18n("The selected path is not correct!"),i18n("The chosen path does not lead to the\n"
                                                              "KDE-2.x root directory. Please choose the\n"
                                                              "correct path."),KMsgBox::EXCLAMATION);

}

// --- added by Olaf Hartig (olaf@punkbands.de) 22.Feb.2000
void CKDevSetupDlg::slotPPathClicked(){
  QString dir;
  config->setGroup("General Options");
  dir = KFileDialog::getDirectory(config->readEntry("ProjectDefaultDir", QDir::homeDirPath()));
  if (!dir.isEmpty()){
    ppath_edit->setText(dir);
  }
}
// ---
