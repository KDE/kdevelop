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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>

#include "ckdevsetupdlg.h"
#include "resource.h"

// SETUP DIALOG
CKDevSetupDlg::CKDevSetupDlg( QWidget *parent, const char *name,KAccel* accel_pa )
  : QTabDialog( parent, name,TRUE )
{
  accel = accel_pa;
  
  setCaption( i18n("KDevelop Setup" ));
  KApplication *app=KApplication::getKApplication();
  config=app->getConfig();
  
  // ****************** the General Tab ********************
  QWidget *w1 = new QWidget( this, "general" );
  
  config->setGroup("General Options");
  
  QLabel* makeSelectLabel;
  makeSelectLabel = new QLabel( w1, "makeSelectLabel" );
  makeSelectLabel->setGeometry( 20, 30, 210, 25 );
  makeSelectLabel->setText(i18n("Select Make-Command:"));
  makeSelectLabel->setAlignment( 289 );
  makeSelectLabel->setMargin( -1 );
  
  makeSelectCombo = new QComboBox( FALSE, w1, "makeSelectCombo" );
  makeSelectCombo->setGeometry( 270, 30, 130, 25 );
  connect( makeSelectCombo, SIGNAL(activated(int)),parent, SLOT(slotOptionsMake(int)) );
  makeSelectCombo->setSizeLimit( 10 );
  makeSelectCombo->setAutoResize( FALSE );
  makeSelectCombo->insertItem( "make", 0 );
  makeSelectCombo->insertItem( "gmake", 1 );
  makeSelectCombo->insertItem( "dmake", 2 );
  QString make_conf=config->readEntry("Make","make");
  if(make_conf=="make")
    makeSelectCombo->setCurrentItem(0);
  if(make_conf=="gmake")
    makeSelectCombo->setCurrentItem(1);
  if(make_conf=="dmake")
    makeSelectCombo->setCurrentItem(2);
  
  QButtonGroup* makeGroup;
  makeGroup = new QButtonGroup( w1, "makeGroup" );
  makeGroup->setGeometry( 10, 10, 400, 60 );
  makeGroup->setFrameStyle( 49 );
  makeGroup->setTitle(i18n( "Make-Command" ));
  makeGroup->setAlignment( 1 );
  makeGroup->lower();
  
  KQuickHelp::add(makeGroup,
		  KQuickHelp::add(makeSelectLabel,
				  KQuickHelp::add(makeSelectCombo,i18n("Make-Command\n\n"
								       "Select your system's make-command.\n"
								       "At the moment, make, gmake and dmake\n"
								       "are available."))));
  
  bool autoSave=config->readBoolEntry("Autosave",true);
  QLabel* autosaveTimeLabel;
  autosaveTimeLabel = new QLabel( w1, "autosaveTimeLabel" );
  autosaveTimeLabel->setGeometry( 20, 140, 210, 25 );
  autosaveTimeLabel->setText(i18n("Select Autosave time-intervall:"));
  autosaveTimeLabel->setAlignment( 289 );
  autosaveTimeLabel->setMargin( -1 );
  autosaveTimeLabel->setEnabled(autoSave);
  
  autosaveTimeCombo = new QComboBox( FALSE, w1, "autosaveTimeCombo" );
  autosaveTimeCombo->setGeometry( 270, 140, 130, 25 );
  connect( autosaveTimeCombo, SIGNAL(activated(int)),parent, SLOT(slotOptionsAutosaveTime(int)) );
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

  autoSaveCheck = new QCheckBox( w1, "autoSaveCheck" );
  autoSaveCheck->setGeometry( 20, 110, 210, 30 );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),parent, SLOT(slotOptionsAutosave(bool)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),autosaveTimeLabel, SLOT(setEnabled(bool)) );
  connect( autoSaveCheck, SIGNAL(toggled(bool)),autosaveTimeCombo, SLOT(setEnabled(bool)) );
  autoSaveCheck->setText(i18n("enable Autosave"));
  autoSaveCheck->setAutoRepeat( FALSE );
  autoSaveCheck->setAutoResize( FALSE );
  autoSaveCheck->setChecked(autoSave);
  
  QButtonGroup* autosaveGroup;
  autosaveGroup = new QButtonGroup( w1, "autosaveGroup" );
  autosaveGroup->setGeometry( 10, 90, 400, 90 );
  autosaveGroup->setFrameStyle( 49 );
  autosaveGroup->setTitle( i18n("Autosave") );
  autosaveGroup->setAlignment( 1 );
  autosaveGroup->insert( autoSaveCheck );
  autosaveGroup->lower();
  
  KQuickHelp::add(autosaveTimeLabel,	
	KQuickHelp::add(autosaveTimeCombo,	
	KQuickHelp::add(autoSaveCheck,	
	KQuickHelp::add(autosaveGroup,i18n("Autosave\n\n"
	                                "If autosave is enabled, your currently\n"
	                                "changed files will be saved after the\n"
	                                "time-intervall selected times out.\n\n"
	                                "Please select your timeout-value.\n"
	                                "Available are: 3 minutes, 5 minutes,\n"
	                                "15 minutes and 30 minutes.")))));
  
  autoSwitchCheck = new QCheckBox( w1, "autoSwitchCheck" );
  autoSwitchCheck->setGeometry( 20, 210, 210, 30 );
  connect( autoSwitchCheck, SIGNAL(toggled(bool)),parent, SLOT(slotOptionsAutoswitch(bool)) );
  autoSwitchCheck->setText(i18n("enable Autoswitch"));
  autoSwitchCheck->setAutoRepeat( FALSE );
  autoSwitchCheck->setAutoResize( FALSE );
  bool autoSwitch=config->readBoolEntry("Autoswitch",true);
  autoSwitchCheck->setChecked( autoSwitch );
  
  QButtonGroup* autoswitchGroup;
  autoswitchGroup = new QButtonGroup( w1, "autoswitchGroup" );
  autoswitchGroup->setGeometry( 10, 190, 400, 60 );
  autoswitchGroup->setFrameStyle( 49 );
  autoswitchGroup->setTitle(i18n( "Autoswitch") );
  autoswitchGroup->setAlignment( 1 );
  autoswitchGroup->insert( autoSwitchCheck );
  autoswitchGroup->lower();
  
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
	
	
	lastProjectCheck = new QCheckBox( w1, "lastProjectCheck" );
	lastProjectCheck->setGeometry( 20, 295, 190, 25 );
	lastProjectCheck->setText(i18n("Load last project"));
	lastProjectCheck->setAutoRepeat( FALSE );
	lastProjectCheck->setAutoResize( FALSE );
	lastProjectCheck->setChecked( lastprj );
	
	KQuickHelp::add(lastProjectCheck, i18n("Load last project\n\n"
                    "If Load last project is enabled, KDevelop will load\n"
                    "the last project used."));

	QButtonGroup* startupGroup = new QButtonGroup( w1, "startupGroup" );
	startupGroup->setGeometry( 10, 260, 400, 70 );
	startupGroup->setFrameStyle( 49 );
	startupGroup->setTitle(i18n("Startup"));
	startupGroup->setAlignment( 1 );
	startupGroup->insert( logoCheck );
	startupGroup->insert( lastProjectCheck );
	startupGroup->lower();
	
	KQuickHelp::add(startupGroup, i18n("Startup\n\n"
	                  "The Startup group offers options for\n"
	                  "starting KDevelop"));

  // ****************** the Keys Tab ***************************

  dict = new QDict<KKeyEntry>( accel->keyDict() );
//  KKeyChooser* w2 = new KKeyChooser ( dict,this);
  QWidget *w2 = new QWidget( this, "keys" );
  KKeyChooser* w21 = new KKeyChooser ( dict,w2,true);
  w21->setGeometry(15,10,395,320);


  // ****************** the Documentation Tab ********************
  QWidget *w = new QWidget( this, "documentaion" );
  config->setGroup("Doc_Location");
  
  
  KQuickHelp::add(w, i18n("Enter the path to your QT and KDE-Libs\n"
		          "Documentation for the Documentation Browser.\n"
                          "QT usually comes with complete Documentation\n"
			  "whereas for KDE you can create the Documentation\n"
			  "easiely by switching to menuentry"
			  "<i>Documentation -> \n Update KDE-Documentation</i>."
			  "Please use the Update dialog whenever you update \n"
			  "your KDE Libs."));
  
  qt_edit = new QLineEdit( w, "qt_edit" );
  qt_edit->setGeometry( 170, 40, 190, 30 );
  qt_edit->setText( config->readEntry("doc_qt"));
  qt_edit->setMaxLength( 32767 );
  
  QPushButton* qt_button;
  qt_button = new QPushButton( w, "qt_button" );
  qt_button->setGeometry( 370, 40, 30, 30 );
  qt_button->setText( "..." );
  connect(qt_button,SIGNAL(clicked()),SLOT(slotQtClicked()));
  
  QLabel* qt_label;
  qt_label = new QLabel( w, "qt_label" );
  qt_label->setGeometry( 20, 40, 140, 30 );
  qt_label->setText( i18n("Qt-library:") );
  
  
  KQuickHelp::add(qt_edit,
  KQuickHelp::add(qt_button,
		  KQuickHelp::add(qt_label,
				  i18n("Enter the path to your QT-Documentation\n"
				       "here. To access the path easier please\n"
				       "press the pushbutton on the right to change\n"
				       "directories.\n\nUsually the QT-Documentation is\n"
				       "located in <i><blue>$QTDIR/html</i>"))));	
  
  kde_edit = new QLineEdit( w, "kde_edit");
  kde_edit->setGeometry( 170, 90, 190, 30 );
  kde_edit->setText(config->readEntry("doc_kde"));
  kde_edit->setMaxLength( 32767 );
  kde_edit->setEchoMode( QLineEdit::Normal );
  kde_edit->setFrame( TRUE );
  
  QPushButton* kde_button;
  kde_button = new QPushButton( w, "kde_button" );
  kde_button->setGeometry( 370, 90, 30, 30 );
  kde_button->setText( "..." );
  connect(kde_button,SIGNAL(clicked()),SLOT(slotKDEClicked()));
  
  QLabel* kde_label;
  kde_label = new QLabel( w, "kde_label" );
  kde_label->setGeometry( 20, 90, 140, 30 );
  kde_label->setText( i18n("KDE-libraries:") );
  
  KQuickHelp::add(kde_edit,
		  KQuickHelp::add(kde_button,
  KQuickHelp::add(kde_label,
		i18n("Enter the path to your KDE-Documentation\n"
		     "here. To access the path easier please\n"
		     "press the pushbutton on the right to change\n"
		     "directories.\n\n"
		     "If you have no kdelibs Documentation installed,\n"
		     "switch to menuentry <blue><i>Documentation ->\n"
		     "Update KDE-Documentation</i><black> to create it."))));  

  QLabel* update_label;
  update_label = new QLabel( w, "update_label" );
  update_label->setGeometry( 20, 190, 260, 30 );
  update_label->setText(i18n("Create KDE-Documentation :"));
  update_label->setAlignment( 289 );
  update_label->setMargin( -1 );
  
  QPushButton* update_button;
  update_button = new QPushButton( w, "update_button" );
  update_button->setGeometry( 290, 190, 110, 30 );
  connect( update_button, SIGNAL(clicked()),parent, SLOT(slotDocUpdateKDEDocumentation()) );
  update_button->setText(i18n("Update..."));
  update_button->setAutoRepeat( FALSE );
  update_button->setAutoResize( FALSE );
  
  KQuickHelp::add(update_label,
		  KQuickHelp::add(update_button,i18n("Create KDE-Documentation\n\n"
						     "This lets you create or update the\n"
						     "HTML-documentation of the KDE-libs.\n"
						     "Mind that you have kdoc installed to\n"
						     "use this function. Also, the kdelibs\n"
						     "sources have to be available to create\n"
						     "the documentation.")));
  
  QLabel* create_label;
  create_label = new QLabel( w, "create_label" );
  create_label->setGeometry( 20, 230, 260, 30 );
  create_label->setText(i18n("Create Search Database :") );
  create_label->setAlignment( 289 );
  create_label->setMargin( -1 );
  
  QPushButton* create_button;	
  create_button = new QPushButton( w, "create_button" );
  create_button->setGeometry( 290, 230, 110, 30 );
  connect( create_button, SIGNAL(clicked()),parent, SLOT(slotCreateSearchDatabase()) );
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
	
  QButtonGroup* docOptionsGroup;
  docOptionsGroup = new QButtonGroup( w, "docOptionsGroup" );
  docOptionsGroup->setGeometry( 10, 160, 400, 110 );
  docOptionsGroup->setFrameStyle( 49 );
  docOptionsGroup->setTitle(i18n("Options"));
  docOptionsGroup->setAlignment( 1 );
  docOptionsGroup->insert( update_button );
  docOptionsGroup->insert( create_button );
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
  
  addTab(w1, i18n("General"));
  addTab(w2, i18n("Keys"));
  addTab( w, i18n("Documentation" ));
  
  
  // **************set the button*********************
  setOkButton(i18n("OK"));
  setCancelButton(i18n("Cancel"));
  connect( this, SIGNAL(applyButtonPressed()), SLOT(ok()) );
  resize(440,420);
  
}


void CKDevSetupDlg::ok(){


  config->setGroup(i18n("General Options"));

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

  int current=makeSelectCombo->currentItem();
  config->writeEntry("Make",makeSelectCombo->text(current));

  bool logo=logoCheck->isChecked();
  config->writeEntry("Logo",logo);

  bool lastprj=lastProjectCheck->isChecked();
  config->writeEntry("LastProject",lastprj);
	
	config->setGroup("TipOfTheDay");
  config->writeEntry("show_tod",tipDayCheck->isChecked());

  QString text;
  config->setGroup("Doc_Location");
  text = qt_edit->text();
  config->writeEntry("doc_qt",text);
  text = kde_edit->text();
  config->writeEntry("doc_kde" , text);
  
  accel->setKeyDict( *dict);

  config->sync();
  close();
}

void CKDevSetupDlg::slotQtClicked(){
  QString dir;
  dir = KFileDialog::getDirectory(config->readEntry("doc_qt"));
  if (!dir.isEmpty()){
  qt_edit->setText(dir);
  config->setGroup("Doc_Location");
  config->writeEntry("doc_qt",dir);
  }
}
void CKDevSetupDlg::slotKDEClicked(){
  QString dir;
  dir = KFileDialog::getDirectory(config->readEntry("doc_kde"));
  if (!dir.isEmpty()){
    kde_edit->setText(dir);
    config->setGroup("Doc_Location");
    config->writeEntry("doc_kde",dir);
  }
}





















