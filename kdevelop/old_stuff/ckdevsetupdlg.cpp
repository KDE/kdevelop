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
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>
#include <qdir.h>
#include <klocale.h>
#include <kaccel.h>
#include <kkeydialog.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "ckdevsetupdlg.h"
#include "ckdevelop.h"
#include "resource.h"

#if HAVE_CONFIG_H
#include "../config.h"
#endif


// SETUP DIALOG
CKDevSetupDlg::CKDevSetupDlg( QWidget *parent, KAccel* accel_pa,
  KGuiCmdManager &cmdMngr, const char *name)
  : QTabDialog( parent, name,TRUE )
{
  accel = accel_pa;
  wantsTreeRefresh=false;

  setCaption( i18n("KDevelop Setup" ));
  config=kapp->config();
  
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

  QString text;
  text = i18n("Make-Command\n\n"
              "Select your system's make-command.\n"
              "Usually, this is make, FreeBSD users\n"
              "may use gmake. Mind that you can also\n"
              "add option parameters to your make-binary\n"
              "as well.");
  QWhatsThis::add(makeGroup, text);
  QWhatsThis::add(makeSelectLabel, text);
  QWhatsThis::add(makeSelectLineEdit, text);
  
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

  text = i18n("Autosave\n\n"
              "If autosave is enabled, your currently\n"
              "changed files will be saved after the\n"
              "time-interval selected times out.\n\n"
              "Please select your timeout-value.\n"
              "Available are: 3 minutes, 5 minutes,\n"
              "15 minutes and 30 minutes.");
  QWhatsThis::add(autosaveTimeLabel, text);
  QWhatsThis::add(autosaveTimeCombo, text);
  QWhatsThis::add(autoSaveCheck, text);
  QWhatsThis::add(autosaveGroup, text);
  
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
  QWhatsThis::add(defaultClassViewCheck, i18n("use Class View as default\n\n"
					      "If this is enabled, KDevelop\n"
					      "will automatically switch to\n"
					      "the Class Viewer when switching.\n"
					      "When disabled, KDevelop will\n"
					      "use Logical File Viewer for\n"
					      "autoswitching."));

  text = i18n("Autoswitch\n\n"
              "If autoswitch is enabled, KDevelop\n"
              "will open windows in the working\n"
              "view automatically according to\n"
              "most needed functionality.\n\n"
              "Disableing autoswitch means you\n"
              "will have to switch to windows\n"
              "yourself, including turning on and\n"
              "off the outputwindow.");
  QWhatsThis::add(autoSwitchCheck, text);
  QWhatsThis::add(autoswitchGroup, text);

  connect( autoSwitchCheck, SIGNAL(toggled(bool)),defaultClassViewCheck, SLOT(setEnabled(bool)));
  
  QButtonGroup* startupGroup = new QButtonGroup( w1, "startupGroup" );
  startupGroup->setGeometry( 10, 260, 400, 70 );
  startupGroup->setFrameStyle( 49 );
  startupGroup->setTitle(i18n("Startup"));
  startupGroup->setAlignment( 1 );
  //	startupGroup->insert( logoCheck );
  //	startupGroup->insert( lastProjectCheck );
  startupGroup->lower();
  
  QWhatsThis::add(startupGroup, i18n("Startup\n\n"
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
  
  QWhatsThis::add(logoCheck, i18n("Startup Logo\n\n"
	                  "If Startup Logo is enabled, KDevelop will show the\n"
	                  "logo picture while it is starting."));
  
  lastProjectCheck = new QCheckBox( w1, "lastProjectCheck" );
  lastProjectCheck->setGeometry( 20, 295, 190, 25 );
  lastProjectCheck->setText(i18n("Load last project"));
  lastProjectCheck->setAutoRepeat( FALSE );
  lastProjectCheck->setAutoResize( FALSE );
  lastProjectCheck->setChecked( lastprj );
  
  QWhatsThis::add(lastProjectCheck, i18n("Load last project\n\n"
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

	QWhatsThis::add(tipDayCheck, i18n("Tip of the Day\n\n"
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
  
  QWhatsThis::add(w, i18n("Enter the path to your Qt and KDE-Libs\n"
				"Documentation for the Documentation Browser.\n"
				"Qt usually comes with complete Documentation\n"
				"whereas for KDE you can create the Documentation\n"
				"easiely by pressing the Update button below."));
	
  qt_edit = new QLineEdit( w, "qt_edit" );
  qt_edit->setGeometry( 170, 40, 190, 30 );
  qt_doc_path= config->readEntry("doc_qt", QT_DOCDIR);
  qt_edit->setText(qt_doc_path);
  qt_edit->setMaxLength( 32767 );
  
  QPushButton* qt_button;
  qt_button = new QPushButton( w, "qt_button" );
  qt_button->setGeometry( 370, 40, 30, 30 );
  qt_button->setPixmap(BarIcon("open"));
  connect(qt_button,SIGNAL(clicked()),SLOT(slotQtClicked()));
  
  QLabel* qt_label;
  qt_label = new QLabel( w, "qt_label" );
  qt_label->setGeometry( 20, 40, 140, 30 );
  qt_label->setText( i18n("Qt-Library-Doc:") );
  
  text = i18n("Enter the path to your Qt documentation\n"
              "here. To access the path easier please\n"
              "press the pushbutton on the right to change\n"
              "directories.\n\n"
              "Usually the Qt documentation is\n"
              "located in <i><blue>$QTDIR/html</i>");
  QWhatsThis::add(qt_edit, text);
  QWhatsThis::add(qt_button, text);
  QWhatsThis::add(qt_label, text);
  
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
  kde_button->setPixmap(BarIcon("open"));
  connect(kde_button,SIGNAL(clicked()),SLOT(slotKDEClicked()));
  
  QLabel* kde_label;
  kde_label = new QLabel( w, "kde_label" );
  kde_label->setGeometry( 20, 90, 140, 30 );
  kde_label->setText( i18n("KDE-Libraries-Doc:") );
  
  text = i18n("Enter the path to your KDE documentation\n"
              "here. To access the path easier please\n"
              "press the pushbutton on the right to change\n"
              "directories.\n\n"
              "If you have no kdelibs documentation installed,\n"
              "you can create it by selecting the Update button\n"
              "below.");
  QWhatsThis::add(kde_edit, text);
  QWhatsThis::add(kde_button, text);
  QWhatsThis::add(kde_label, text);
  
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
  
  text = i18n("Update KDE documentation\n\n"
              "This lets you create or update the\n"
              "HTML documentation of the KDE-libs.\n"
              "Mind that you have kdoc installed to\n"
              "use this function. Also, the kdelibs\n"
              "sources have to be available to create\n"
              "the documentation, as well as the \n"
              "Qt documentation path has to be set to\n"
              "cross-reference the KDE documentation\n"
              "with the Qt classes.");
  QWhatsThis::add(update_label, text);
  QWhatsThis::add(update_button, text);
  
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
  
  text = i18n("Create Search Database\n\n"
              "This will create a search database for glimpse\n"
              "which will be used to look up marked text in\n"
              "the documentation. We recommend updating the\n"
              "database each time you've changed the documentation\n"
              "e. g. after a kdelibs update or installing a new\n"
              "Qt library version.");
  QWhatsThis::add(create_label, text);
  QWhatsThis::add(create_button, text);
  
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

  KGuiCmdConfigTab *keys = new KGuiCmdConfigTab(this, &cmdMngr);
  addTab(keys, i18n("KWrite Keys"));

  addTab( w, i18n("Documentation" ));
  
  
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
  answer=KMessageBox::Yes;
  if(!QFileInfo(qt_testfile).exists())
  {
    answer=KMessageBox::questionYesNo(this,i18n("The chosen path does not lead to the\n"
                                         "Qt-library documentation. Do you really want to save\n"
                                         "this value?"), i18n("The selected path is not correct!"));
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
                                        "this value?"), i18n("The selected path is not correct!"));
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

  bool defaultcv=defaultClassViewCheck->isChecked();
  config->writeEntry("DefaultClassView",defaultcv);

  config->writeEntry("Make",makeSelectLineEdit->text());

  bool logo=logoCheck->isChecked();
  config->writeEntry("Logo",logo);

  bool lastprj=lastProjectCheck->isChecked();
  config->writeEntry("LastProject",lastprj);
	
	config->setGroup("TipOfTheDay");
  config->writeEntry("show_tod",tipDayCheck->isChecked());

  accel->setKeyDict( *dict);
  accel->writeSettings(config);
  config->sync();
  accept();
}

void CKDevSetupDlg::slotQtClicked(){
  QString dir;
  config->setGroup("Doc_Location");
  dir = KFileDialog::getExistingDirectory(config->readEntry("doc_qt", QT_DOCDIR));
  if (!dir.isEmpty()){
    qt_edit->setText(dir);

    QString qt_testfile=dir+"classes.html"; // test if the path really is the qt-doc path
    if(!QFileInfo(qt_testfile).exists())
      KMessageBox::information(this, i18n("The chosen path does not lead to the\n"
                                          "Qt library documentation. Please choose the\n"
                                          "correct path."));
  }
}


void CKDevSetupDlg::slotKDEClicked(){
  QString dir;
  config->setGroup("Doc_Location");
  dir = KFileDialog::getExistingDirectory(config->readEntry("doc_kde", KDELIBS_DOCDIR));
  if (!dir.isEmpty()){
    kde_edit->setText(dir);

    QString kde_testfile=dir+"kdecore/index.html"; // test if the path really is the qt-doc path
    if(!QFileInfo(kde_testfile).exists())
      KMessageBox::information(this, i18n("The chosen path does not lead to the\n"
                                          "KDE library documentation. Please choose the\n"
                                          "correct path or choose 'Update' to create a new\n"
                                          "documentation"));
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
