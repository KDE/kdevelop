/***************************************************************************

                    kappwizard.cpp - the kde-application-wizard
                             -------------------                                         

    begin                : 9 Sept 1998                                        
    copyright            : (C) 1998 by Stefan Heidrich                         
    email                : sheidric@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ckappwizard.h"
#include "debug.h"

CKAppWizard::CKAppWizard(QWidget* parent,const char* name) : KWizard(parent,name,true){
  
  gen_prj = false;
setCaption(i18n("Application Wizard"));
  init();
  initPages();
  slotDefaultClicked();    
}

void CKAppWizard::init(){
  
  setFixedSize(400,400);
  setCancelButton();
  cancelButton = new QButton();
  cancelButton = getCancelButton();
  QToolTip::add(cancelButton,i18n("exit the CKAppWizard"));
  connect(this,SIGNAL(cancelclicked()),SLOT(slotAppEnd()));
  setOkButton(i18n("Create"));
  okButton = new QButton();
  okButton = getOkButton();
  QToolTip::add(okButton,i18n("creating the project"));
  connect(this,SIGNAL(okclicked()),SLOT(slotOkClicked()));
  setDefaultButton();
  defaultButton = new QButton();
  defaultButton = getDefaultButton();
  QToolTip::add(defaultButton,i18n("set all changes back"));
  connect(this,SIGNAL(defaultclicked(int)),SLOT(slotDefaultClicked()));
  setEnableArrowButtons(true);
}

void CKAppWizard::initPages(){
  
  // create the first page
  page0 = new KWizardPage;
  widget0 = new QWidget(this);
  page0->w = widget0;
  page0->title = (i18n("KWizard"));
  addPage(page0);
  
  QPixmap iconpm;    
  QPixmap minipm;
  QPixmap pm;
  pm.load(KApplication::kde_datadir() + "/kdevelop/pics/kAppWizard.bmp");
  
  // create a widget and paint a picture on it
  widget1a = new QWidget(widget0);
  widget1a->setGeometry(0,0,400,140);
  widget1a->setBackgroundPixmap(pm);
  // create another widget for a picture
  widget1b = new QWidget(widget0);
  widget1b->setGeometry(230,150,140,140);
  separator0 = new KSeparator (widget0);
  separator0->setGeometry(0,140,400,5);
  // create the RadioButtonGroup with all buttons, bubble-help and connections
  bgroup = new QButtonGroup(i18n("Application"),widget0);
  bgroup->setGeometry(20,150,180,140);
  kna = new QRadioButton (i18n("KDE-Application"),widget0);
  kna->setGeometry(30,168,145,30);
//  QToolTip::add(kna,i18n("use for a KDE-Application with toolbar ect."));
  KQuickHelp::add(kna,
	i18n("Select this to create a complete framework for\n"
		"your new application. It will contain three main\n"
		"classes: one for the mainwindow, one for the\n"
		"main view and one for the application's documents\n"
  		"The main window offers a menubar, toolbar and\n"
		"a statusbar including statusbar help and status\n"
		"signaling."));
 
  connect(kna,SIGNAL(clicked()),SLOT(slotAppClicked()));
  kma = new QRadioButton (i18n("KDE-Mini-Application"),widget0);
  kma->setGeometry(30,196,150,30);
//  QToolTip::add(kma,i18n("use for a minimal KDE-Application"));
  KQuickHelp::add(kma,
	i18n("Select this to create a mini-Application.\n"
	 	"This will only contain the main widget\n"
		"class which inherits QWidget until now."));

  connect(kma,SIGNAL(clicked()),SLOT(slotMiniClicked()));

  qta = new QRadioButton (i18n("Qt-Application"),widget0);
  qta->setGeometry(30,224,150,30);
  KQuickHelp::add(qta,
	i18n("Select this to create a Qt-Application."));

  connect(qta,SIGNAL(clicked()),SLOT(slotQtClicked()));
  ta = new QRadioButton (i18n("C/C++-Application"),widget0);
  ta->setGeometry(30,251,150,30);
  //  QToolTip::add(ta,i18n("use for a minimal C++-Application"));
  KQuickHelp::add(ta,
	i18n("Select this to create a C/C++-Terminal-Application."));

  connect(ta,SIGNAL(clicked()),SLOT(slotCPPClicked()));
  bgroup->insert( kna );
  bgroup->insert( kma );
  bgroup->insert( qta );
  bgroup->insert ( ta );
    
  /************************************************************/
  
  // create the second page
  page1 = new KWizardPage;
  widget1 = new QWidget(this);
  page1->w = widget1;
  page1->title = (i18n("Names"));
  addPage(page1);  
  
  // create Labels with LineEdit and a Button
  name = new QLabel(i18n("Projectname:"),widget1);
  name->move(20,10);
  directory = new QLabel(i18n("Projectdirectory:"),widget1);
  directory->move(20,70);
  versionnumber = new QLabel(i18n("Versionsnumber:"),widget1);
  versionnumber->move(20,130);
  separator2 = new KSeparator (widget1);
  separator2->setGeometry(0,172,400,10);
  authorname = new QLabel(i18n("Author:"),widget1);
  authorname->move(20,195);
  email = new QLabel(i18n("Email:"),widget1);
  email->move(20,255);
  nameline = new QLineEdit(widget1);
  nameline->setGeometry(120,10,200,30);
//  QToolTip::add(nameline,i18n("enter the projectname"));
  connect(nameline,SIGNAL(textChanged(const char*)),SLOT(slotProjectnameEntry()));
  directoryline = new QLineEdit(widget1);
  directoryline->setGeometry(120,70,200,30);
//  QToolTip::add(directoryline,i18n("the directory, in which your project is"));
  directoryload = new QPushButton ("...",widget1);
  directoryload->setGeometry(330,70,30,30);
//  QToolTip::add(directoryload,i18n("you can browse to a directory here"));
  connect(directoryload,SIGNAL(clicked()),SLOT(slotDirDialogClicked()));
  versionline = new QLineEdit(widget1);
  versionline->setGeometry(120,130,200,30);
//  QToolTip::add(versionline,i18n("the versionsnumber, with which your project should start"));
  authorline = new QLineEdit(widget1);
  authorline->setGeometry(120,195,200,30);
//  QToolTip::add(authorline,i18n("enter your name"));
  emailline = new QLineEdit(widget1);
  emailline->setGeometry(120,255,200,30);
//  QToolTip::add(emailline,i18n("enter your email-address"));
  KQuickHelp::add(name,
  KQuickHelp::add(nameline,
	i18n("Enter the Project's name here. This is\n"
		"also the directory name under which your Project\n"
		"will be created.")));
  KQuickHelp::add(directory,
  KQuickHelp::add(directoryline,
  KQuickHelp::add(directoryload,
	i18n("Enter the toplevel-directory the project\n"
 		"will be stored. This <b>must</b> be an\n"
		"<b>existing directory !!"))));
  KQuickHelp::add(versionnumber,
  KQuickHelp::add(versionline,
	i18n("Enter the Version number here.\n"
		"The number will be used in the about-dialog\n"
		"and in the main widget's topbar.")));
  KQuickHelp::add(authorname,
  KQuickHelp::add(authorline,
	i18n("Enter your name here for the about dialog.\n"
		"You may also consider using the Team-name.")));
  KQuickHelp::add(email,
  KQuickHelp::add(emailline,
  	i18n("Enter your email adress here, so users\n"
		"can send you bug reports easiely ;-))")));

  /************************************************************/
  
  // create the third page
  page2 = new KWizardPage;
  widget2 = new QWidget(this);
  page2->w = widget2;
  page2->title = (i18n("Projectfiles & Desktopgoodies"));
  addPage(page2);
  
  // create CheckBoxes  with help
  apidoc = new QCheckBox (i18n("API-Documentation"), widget2);
  apidoc->setGeometry(20,5,170,30);
//  QToolTip::add(apidoc,i18n("activate, if you wish the API-Documentationfiles"));
  KQuickHelp::add(apidoc, i18n("Check this if you wish to have your API\n"
				"Documentation generated automatically"));

  userdoc = new QCheckBox (i18n("User-Documentation"), widget2);
  userdoc->setGeometry(20,45,200,30);
//  QToolTip::add(userdoc,i18n("activate, if you wish the User-Documentationfiles"));
  KQuickHelp::add(userdoc, i18n("Check this if you wish a general User\n"
				"Documentation to be generated automatically."));

  lsmfile = new QCheckBox (i18n("lsm-File - Linux Software Map"), widget2);
  lsmfile->setGeometry(20,85,300,30);
//  QToolTip::add(lsmfile,i18n("activate, if you wish the lsm-File"));
  KQuickHelp::add(lsmfile, i18n("Check this to create a lsm-File"));

  gnufiles = new QCheckBox (i18n("GNU-Standard-Files (INSTALL,README,COPYING...)"), widget2);
  gnufiles->setGeometry(20,125,350,30);
//  QToolTip::add(gnufiles,i18n("activate, if you wish the GNU-Standard-Files"));
  KQuickHelp::add(gnufiles, i18n("Check this if you wish to have\n"
				"the GNU-Standard files created like\n"
				"INSTALL and README"));

  separator1 = new KSeparator (widget2);
  separator1->setGeometry(0,165,400,10);
  progicon = new QCheckBox (i18n("Program-Icon"), widget2);
  progicon->setGeometry(20,190,140,30);
  connect(progicon,SIGNAL(clicked()),SLOT(slotProgIconClicked()));   
//  QToolTip::add(progicon,i18n("activate, if you want an icon for your program"));
  KQuickHelp::add(progicon, i18n("Check this to create a sample icon for\n"
				"your program. We suggest to do so and just\n"
				"change it's view by using KIconEdit."));

  miniicon = new QCheckBox (i18n("Mini-Icon"), widget2);
  miniicon->setGeometry(20,250,140,30);
  connect(miniicon,SIGNAL(clicked()),SLOT(slotMiniIconClicked()));   
//  QToolTip::add(miniicon,i18n("activate, if you want a mini-icon for your program"));
  KQuickHelp::add(miniicon, i18n("Check this to create a sample mini-icon\n"
				"for your program. We suggest to do so and just\n"
				"change it's view by using KIconEdit."));
 
  datalink = new QCheckBox (i18n(".kdelnk-File"), widget2);
  datalink->setGeometry(240,190,140,30);
//  QToolTip::add(datalink,i18n("activate, if you want a link in the K-Menu"));
  KQuickHelp::add(datalink, i18n("Check this if you wish to have\n"
				"a .kdelnk for you application in the\n"
				"K-Menu.  The default is the Applications-\n"
				"submenu. You can change this to other menus\n"
				"by changing the file properties afterwards."));

  // create PushButtons for the icons with help and connections
  iconload = new QPushButton(widget2);
  iconload->setGeometry(140,183,55,55);
//  QToolTip::add(iconload,i18n("you can select another icon here"));
  KQuickHelp::add(iconload, i18n("Select this if you wish another\n"
				"already created icon to be used."));

  connect(iconload,SIGNAL(clicked()),SLOT(slotIconButtonClicked()));
  miniload = new QPushButton(widget2);
  miniload->setGeometry(165,250,30,30);
//  QToolTip::add(miniload,i18n("you can select another mini-icon here"));
  KQuickHelp::add(miniload, i18n("Select this if you wish another\n"
 				"already created mini-icon to be used."));
					
  connect(miniload,SIGNAL(clicked()),SLOT(slotMiniIconButtonClicked()));
  
  /************************************************************/
  
  // create the fourth page
  page3 = new KWizardPage;
  widget3 = new QWidget(this);
  page3->w = widget3;
  page3->title = (i18n("Headertemplate for .h-files"));
  addPage(page3);
  
  // create a CheckBox, two PushButtons and a KEdit for the Headertemplate
  hheader = new QCheckBox (i18n("headertemplate for .h-files"), widget3);
  hheader->setGeometry(20,10,190,30);
  connect(hheader,SIGNAL(clicked()),SLOT(slotHeaderHeaderClicked()));   
//  QToolTip::add(hheader,i18n("activate, if you want a headertemplate for your headerfiles"));
  KQuickHelp::add(hheader, i18n("Check this if you want a standard\n"
				"headertemplate for your headerfiles"));

  hload = new QPushButton (i18n("Load..."),widget3);
  hload->setGeometry(210,10,60,30);
  QToolTip::add(hload,i18n("you can load another headertemplate here"));
  connect(hload,SIGNAL(clicked()),SLOT(slotHeaderDialogClicked()));
  hnew = new QPushButton (i18n("New"),widget3);
  hnew->setGeometry(300,10,60,30);
  QToolTip::add(hnew,i18n("you can clear the headertemplate here"));
  connect(hnew,SIGNAL(clicked()),SLOT(slotNewHeaderButtonClicked()));
  hedit = new KEdit(kapp,widget3);
  QFont f("courier",10);
  hedit->setFont(f);
  hedit->setGeometry(20,60,340,230);
  QToolTip::add(hedit,i18n("you can edit your headertemplate here"));
  
  /************************************************************/
  
  // create the fifth page
  page4 = new KWizardPage;
  widget4 = new QWidget(this);
  page4->w = widget4;
  page4->title = (i18n("Headertemplate for .cpp-files"));
  addPage(page4);
  
  // create a CheckBox, two PushButtons and a KEdit for the Cpptemplate
  cppheader = new QCheckBox (i18n("headertemplate for .cpp-files"), widget4);
  cppheader->setGeometry(20,10,190,30);
  connect(cppheader,SIGNAL(clicked()),SLOT(slotCppHeaderClicked()));   
//  QToolTip::add(cppheader,i18n("activate, if you want a cpptemplate for your cppfiles"));
  KQuickHelp::add(cppheader, i18n("Check this if you want a standard\n"
				"cpp-Template for your cpp-files."));

  cppload = new QPushButton (i18n("Load..."),widget4);
  cppload->setGeometry(210,10,60,30);
  QToolTip::add(cppload,i18n("you can load another cpptemplate here"));
  connect(cppload,SIGNAL(clicked()),SLOT(slotCppDialogClicked()));
  cppnew = new QPushButton (i18n("New"),widget4);
  cppnew->setGeometry(300,10,60,30);
  QToolTip::add(cppnew,i18n("you can clear the cpptemplate here"));
  connect(cppnew,SIGNAL(clicked()),SLOT(slotNewCppButtonClicked()));   
  cppedit = new KEdit(kapp,widget4);
  cppedit->setFont(f);
  cppedit->setGeometry(20,60,340,230);
  QToolTip::add(cppedit,i18n("you can edit your cpptemplate here"));
  
  /************************************************************/
  
  // create the sixth page
  page5 = new KWizardPage;
  widget5 = new QWidget(this);
  page5->w = widget5;
  page5->title = (i18n("Processes"));
  addPage(page5);
  
  // create a MultiLineEdit for the processes of kAppWizard
  output = new QMultiLineEdit(widget5);
  output->setGeometry(5,5,375,225);
  output->setReadOnly(false);
  QFont font("helvetica",10);
  output->setFont(font);
  QToolTip::add(output,i18n("you can see the normal outputs here"));
  errOutput = new QMultiLineEdit(widget5);
  errOutput->setGeometry(5,230,375,65);
  errOutput->setReadOnly(false);
  errOutput->setFont(font);
  QToolTip::add(errOutput,i18n("you can see all warnings and errormessages here"));
  // go to page 2 then to page 1
  gotoPage(1);  
  gotoPage(0);    
}

// connection to directoryload
void CKAppWizard::slotDirDialogClicked() {
  QString projname;
  dirdialog = new KDirDialog(dir,this,"Directory");
  dirdialog->setCaption (i18n("Directory"));
  dirdialog->show();
  projname = nameline->text();
  dir = dirdialog->dirPath() + projname.lower();
  directoryline->setText(dir);
  dir = dirdialog->dirPath();
  delete (dirdialog);
}

// connection of hload
void CKAppWizard::slotHeaderDialogClicked() {
  headerdialog = new KFileDialog(QDir::homeDirPath(),"*",this,"Headertemplate",true,true);
  headerdialog->setCaption (i18n("Header"));
  headerdialog->show();
  hedit->loadFile(headerdialog->selectedFile(),cppedit->OPEN_READWRITE);
  delete (headerdialog);
}

// connection of cppload
void CKAppWizard::slotCppDialogClicked() {
  cppdialog = new KFileDialog(QDir::homeDirPath(),"*",this,"Cpptemplate",true,true);
  cppdialog->setCaption("Cpp");
  cppdialog->show();
  cppedit->loadFile(cppdialog->selectedFile(),cppedit->OPEN_READWRITE);
  delete (cppdialog);
}

// connection of hnew
void CKAppWizard::slotNewHeaderButtonClicked() {
  hedit->clear();
}

// connection of cppnew
void CKAppWizard::slotNewCppButtonClicked() {
  cppedit->clear();
}

// connection of this (defaultButton)
void CKAppWizard::slotOkClicked() {
  cancelButton->setFixedWidth(75);
  cancelButton->setEnabled(false);
  defaultButton->setEnabled(false);
  setCancelButton(i18n("Exit"));
  errOutput->clear();
  output->clear();
  QDir kdevelop;
  kdevelop.mkdir(QDir::homeDirPath() + "/.kde/share/apps/kdevelop");
  cppedit->setName(QDir::homeDirPath() + "/.kde/share/apps/kdevelop/cpp");
  cppedit->toggleModified(true);
  cppedit->doSave();
  hedit->setName(QDir::homeDirPath() + "/.kde/share/apps/kdevelop/header");
  hedit->toggleModified(true);
  hedit->doSave();
  ofstream entries (QDir::homeDirPath() + "/.kde/share/apps/kdevelop/entries");
  entries << "APPLICATION\n";
  if (kna->isChecked()) 
    entries << "standard\n";
  else 
    if (kma->isChecked())
      entries << "mini\n";
    else if (qta->isChecked()) {
      entries << "qt\n";
    }
    else 
      entries << "terminal\n";
  entries << "NAME\n";
  entries << nameline->text() << "\n";
  entries << "DIRECTORY\n";
  if(QString(directoryline->text()).right(1) == "/"){
    entries << directoryline->text() << "\n";
  }
  else{
    entries << directoryline->text() << "/" << "\n";
  }
  
  
  entries << "AUTHOR\n";
  entries << authorline->text() << "\n";
  entries << "EMAIL\n";
  entries << emailline->text() << "\n";
  entries << "API\n";
  if (apidoc->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "USER\n";
  if (userdoc->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "LSM\n";
  if (lsmfile->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "GNU\n";
  if (gnufiles->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "PROGICON\n";
  if (progicon->isChecked()) {
    entries << name1 << "\n";
  }
  else entries << "no\n";
  entries << "MINIICON\n";
  if (miniicon->isChecked())
    entries << name2 << "\n";
  else entries << "no\n";
  entries << "KDELNK\n";
  if (datalink->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "HEADER\n";
  if (hheader->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "CPP\n";
  if (cppheader->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "VERSION\n";
  entries << versionline->text() << "\n";
  
  namelow = nameline->text();
  namelow = namelow.lower();
  QDir directory;
  directory.mkdir(directoryline->text() + QString("/"));
  p = new KProcess();
  QString copysrc;
  QString copydes = directoryline->text() + QString ("/");
  if (kma->isChecked()) { 
    copysrc = KApplication::kde_datadir() + "/kdevelop/templates/mini.tar.gz";
    *p << "cp";
    *p << copysrc;
    *p << copydes;
    p->start(KProcess::Block,KProcess::AllOutput);
  } 
  else if (kna->isChecked()) {
    copysrc = KApplication::kde_datadir() + "/kdevelop/templates/normal.tar.gz";
    *p << "cp";
    *p << copysrc;
    *p << copydes;
    p->start(KProcess::Block,KProcess::AllOutput);
  }
  else if (qta->isChecked()) {
    copysrc = KApplication::kde_datadir() + "/kdevelop/templates/qt.tar.gz";
    *p << "cp";
    *p << copysrc;
    *p << copydes;
    p->start(KProcess::Block,KProcess::AllOutput);
  }
  else {
    copysrc = KApplication::kde_datadir() + "/kdevelop/templates/cpp.tar.gz";
    *p << "cp";
    *p << copysrc;
    *p << copydes;
    p->start(KProcess::Block,KProcess::AllOutput);
  }
  p->clearArguments();
  connect(p,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));
  connect(p,SIGNAL(receivedStdout(KProcess *, char *, int)),
          this,SLOT(slotPerlOut(KProcess *, char *, int)));
  connect(p,SIGNAL(receivedStderr(KProcess *, char *, int)),
          this,SLOT(slotPerlErr(KProcess *, char *, int)));
  QString path = kapp->kde_datadir()+"/kdevelop/tools/";
  *p << "perl" << path + "processes.pl";
  p->start(KProcess::NotifyOnExit, KProcess::AllOutput);
  okButton->setEnabled(false);
  gotoPage(5);
  int i;
  for (i=0;i<5;i++) {
    setPageEnabled(i,false);
  }
  kma->setEnabled(false);
  kna->setEnabled(false);
  ta->setEnabled(false);
  apidoc->setEnabled(false);
  lsmfile->setEnabled(false);
  cppheader->setEnabled(false);
  hheader->setEnabled(false);
  datalink->setEnabled(false);
  miniicon->setEnabled(false);
  progicon->setEnabled(false);
  gnufiles->setEnabled(false);
  userdoc->setEnabled(false);
  directoryline->setEnabled(false);
  nameline->setEnabled(false);
  okButton->setEnabled(false);
  miniload->setEnabled(false);
  iconload->setEnabled(false);
  cppedit->setEnabled(false);
  hedit->setEnabled(false);
  authorline->setEnabled(false);
  emailline->setEnabled(false);  
  versionline->setEnabled(false);
  hnew->setEnabled(false);
  hload->setEnabled(false);
  cppnew->setEnabled(false);
  cppload->setEnabled(false);

}

// connection of this (cancelButton)
void CKAppWizard::slotAppEnd() {
  nametext = nameline->text();
  if ((!(okButton->isEnabled())) && (nametext.length() >= 1)) {
    delete (p);
    delete (q);
    delete (project);
  }
  delete (errOutput);
  delete (output);
  delete (cppedit);
  delete (cppnew);
  delete (cppload);
  delete (cppheader);
  delete (hedit);
  delete (hnew);
  delete (hload);
  delete (hheader);
  delete (miniload);
  delete (iconload);
  delete (datalink);
  delete (miniicon);
  delete (progicon);
  delete (separator1);
  delete (gnufiles);
  delete (lsmfile);
  delete (userdoc);
  delete (apidoc);
  delete (emailline);
  delete (authorline);
  delete (versionline);
  delete (directoryload);
  delete (email);
  delete (authorname);
  delete (separator2);
  delete (versionnumber);
  delete (directory);
  delete (name);
  delete (ta);
  delete (kma);
  delete (kna);
  delete (bgroup);
  delete (separator0);
  delete (widget1b);
  delete (widget1a);
  reject();
}

// connection of this (okButton)
void CKAppWizard::slotPerlOut(KProcess*,char* buffer,int buflen) {
  QString str(buffer,buflen);
  output->append(str);
  output->setCursorPosition(output->numLines(),0);
}

// connection of this (okButton)
void CKAppWizard::slotPerlErr(KProcess*,char* buffer,int buflen) {
  QString str(buffer,buflen);
  errOutput->append(str);
  errOutput->setCursorPosition(errOutput->numLines(),0);
}

// connection of kna
void CKAppWizard::slotAppClicked() {
  pm.load(KApplication::kde_datadir() +"/kdevelop/pics/normalApp.bmp");
  widget1b->setBackgroundPixmap(pm);
  apidoc->setEnabled(true);
  apidoc->setChecked(true);
  datalink->setEnabled(true);
  datalink->setChecked(true);
  progicon->setEnabled(true);
  progicon->setChecked(true);
  miniicon->setEnabled(true);
  miniicon->setChecked(true);
  miniload->setEnabled(true);
  iconload->setEnabled(true);
}

// connection of kma
void CKAppWizard::slotMiniClicked() {
  pm.load(KApplication::kde_datadir() + "/kdevelop/pics/miniApp.bmp");
  widget1b->setBackgroundPixmap(pm);
  apidoc->setEnabled(true);
  apidoc->setChecked(true);
  datalink->setEnabled(true);
  datalink->setChecked(true);
  progicon->setEnabled(true);
  progicon->setChecked(true);
  miniicon->setEnabled(true);
  miniicon->setChecked(true);
  miniload->setEnabled(true);
  iconload->setEnabled(true);
}

// connection of qta
void CKAppWizard::slotQtClicked() {
  pm.load(KApplication::kde_datadir() +"/kdevelop/pics/qtApp.bmp");
  widget1b->setBackgroundPixmap(pm);
  apidoc->setEnabled(true);
  apidoc->setChecked(true);
  datalink->setEnabled(true);
  datalink->setChecked(true);
  progicon->setEnabled(true);
  progicon->setChecked(true);
  miniicon->setEnabled(true);
  miniicon->setChecked(true);
  miniload->setEnabled(true);
  iconload->setEnabled(true);
}

// connection of ta
void CKAppWizard::slotCPPClicked() {
  pm.load(KApplication::kde_datadir() + "/kdevelop/pics/terminalApp.bmp");
  widget1b->setBackgroundPixmap(pm);
  apidoc->setEnabled(false);
  apidoc->setChecked(false);
  datalink->setEnabled(false);
  datalink->setChecked(false);
  progicon->setEnabled(false);
  progicon->setChecked(false);
  miniicon->setEnabled(false);
  miniicon->setChecked(false);
  miniload->setEnabled(false);
  iconload->setEnabled(false);
}

// connection of this
void CKAppWizard::slotDefaultClicked() {
  kma->setChecked(false);
  kna->setChecked(true);
  ta->setChecked(false);
  pm.load(KApplication::kde_datadir() +"/kdevelop/pics/normalApp.bmp");
  widget1b->setBackgroundPixmap(pm);
  apidoc->setChecked(true);
  lsmfile->setChecked(true);
  cppheader->setChecked(true);
  hheader->setChecked(true);
  datalink->setChecked(true);
  miniicon->setChecked(true);
  progicon->setChecked(true);
  gnufiles->setChecked(true);
  userdoc->setChecked(true);
  miniload->setEnabled(true);
  iconload->setEnabled(true);
  directoryline->setText(QDir::homeDirPath()+ QString("/"));
  dir = QDir::homeDirPath()+ QString("/");
  nameline->setText(0);
  okButton->setEnabled(false);
  miniload->setPixmap(QPixmap(KApplication::kde_icondir() + "/mini/application_settings.xpm"));
  iconload->setPixmap(QPixmap(KApplication::kde_icondir() + "/edit.xpm"));
  cppedit->loadFile(KApplication::kde_datadir() + "/kdevelop/templates/cpp_template",cppedit->OPEN_READWRITE);
  hedit->loadFile(KApplication::kde_datadir() + "/kdevelop/templates/header_template",hedit->OPEN_READWRITE);
  authorline->setText(0);
  emailline->setText(0);
  versionline->setText(0);
}

// connection of nameline
void CKAppWizard::slotProjectnameEntry() {
  nametext = nameline->text();
  nametext = nametext.stripWhiteSpace();
  if (nametext.length() == 1) {
    QRegExp regexp ("[a-zA-Z]");
    if (regexp.match(nametext) == -1) {
      nametext = "";
    }
    else {
      nametext = nametext.upper();
    }
  }
  nameline->setText(nametext);
  directoryline->setText(dir + nametext.lower());
  nametext == "" ? okButton->setEnabled(false) : okButton->setEnabled(true);
}

// connection of iconload
void CKAppWizard::slotIconButtonClicked() {
  QStrList iconlist;
  KIconLoaderDialog iload;
  iconlist.append (KApplication::kde_icondir());
  iconlist.append (KApplication::localkdedir()+"/share/icons");
  iload.setDir(&iconlist);
  iload.selectIcon(name1,"*");
  if (!name1.isNull() )   
    iconload->setPixmap(kapp->getIconLoader()->loadIcon(name1));
}

// connection of miniload
void CKAppWizard::slotMiniIconButtonClicked() {
  QStrList miniiconlist;
  KIconLoaderDialog  mload;
  miniiconlist.append (KApplication::kde_icondir()+"/mini");
  miniiconlist.append (KApplication::localkdedir()+"/share/icons/mini");
  mload.setDir(&miniiconlist);
  mload.selectIcon(name2,"*");
  if (!name2.isNull() )     
    miniload->setPixmap(kapp->getIconLoader()->loadMiniIcon(name2));
}

// activate and deactivate the iconbutton
void CKAppWizard::slotProgIconClicked() {
  if (progicon->isChecked()) {
    iconload->setEnabled(true);
  }
  else {
    iconload->setEnabled(false);
  }
}

// activate and deactivate the miniiconbutton
void CKAppWizard::slotMiniIconClicked() {
  if (miniicon->isChecked()) {
    miniload->setEnabled(true);
  }
  else {
    miniload->setEnabled(false);
  }
}

// activate and deactivate the headerloadbutton, headernewbutton, headeredit
void CKAppWizard::slotHeaderHeaderClicked() {
  if (hheader->isChecked()) {
    hload->setEnabled(true);
    hnew->setEnabled(true);
    hedit->setEnabled(true);
  }
  else {
    hload->setEnabled(false);
    hnew->setEnabled(false);
    hedit->setEnabled(false);
  }
}

// activate and deactivate the cpploadbutton, cppnewbutton, cppedit
void CKAppWizard::slotCppHeaderClicked() {
  if (cppheader->isChecked()) {
    cppload->setEnabled(true);
    cppnew->setEnabled(true);
    cppedit->setEnabled(true);
  }
  else {
    cppload->setEnabled(false);
    cppnew->setEnabled(false);
    cppedit->setEnabled(false);
  }
}

void CKAppWizard::slotProcessExited() {

  QString directory = directoryline->text();
  QString prj_str = directory + "/" + namelow + ".kdevprj";
  project = new CProject(prj_str);
  project->readProject();
  project->setKDevPrjVersion("0.3");
  if (ta->isChecked()) {
    project->setProjectType("normal_cpp");
  }
  else if (kma->isChecked()) {
    project->setProjectType("mini_kde");  
  }
  else if (kna->isChecked()) {
    project->setProjectType("normal_kde");
  }
  else {
    project->setProjectType("normal_qt");
  }
  project->setProjectName (nameline->text());
  project->setSubDir (namelow + "/");
  project->setAuthor (authorline->text());
  project->setEmail (emailline->text());
  project->setVersion (versionline->text());
  if (userdoc->isChecked()) {
    project->setSGMLFile ("index.sgml");
  }
  project->setBinPROGRAM (namelow);
  project->setLDFLAGS (" ");
  project->setCXXFLAGS ("-O0 -g -Wall");
  
  if (kna->isChecked()) {
    project->setLDADD (" -lkfile -lkfm -lkdeui -lkdecore -lqt -lXext -lX11");
  }
  else if (kma->isChecked()) {
    project->setLDADD (" -lkdeui -lkdecore -lqt -lXext -lX11");
  }
  else if (qta->isChecked()) {
    project->setLDADD (" -lqt -lXext -lX11");
  }
  
  QStrList sub_dir_list;
  TMakefileAmInfo makeAmInfo;
  makeAmInfo.rel_name = "Makefile.am";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.rel_name.data());
  makeAmInfo.type = "normal";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.type.data());
  sub_dir_list.append(namelow);
  makeAmInfo.sub_dirs = sub_dir_list;
  project->writeMakefileAmInfo (makeAmInfo);
  project->addMakefileAmToProject (makeAmInfo.rel_name);

  makeAmInfo.rel_name =  namelow + "/Makefile.am";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.rel_name.data());
  makeAmInfo.type = "prog_main";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.type.data());
  sub_dir_list.clear();
  if (userdoc->isChecked()) {
    //    sub_dir_list.append("docs");
  }
  makeAmInfo.sub_dirs = sub_dir_list;
  project->writeMakefileAmInfo (makeAmInfo);
  project->addMakefileAmToProject (makeAmInfo.rel_name);
  
  makeAmInfo.rel_name =  namelow + "/docs/Makefile.am";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.rel_name.data());
  makeAmInfo.type = "normal";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.type.data());
  sub_dir_list.clear();
  //  sub_dir_list.append("en");
  makeAmInfo.sub_dirs = sub_dir_list;
  project->writeMakefileAmInfo (makeAmInfo);
  project->addMakefileAmToProject (makeAmInfo.rel_name);

  makeAmInfo.rel_name =  namelow + "/docs/en/Makefile.am";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.rel_name.data());
  makeAmInfo.type = "normal";
  KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.type.data());
  sub_dir_list.clear();
  makeAmInfo.sub_dirs = sub_dir_list;
  project->writeMakefileAmInfo (makeAmInfo);
  project->addMakefileAmToProject (makeAmInfo.rel_name);
  
  if (!(ta->isChecked() || qta->isChecked())) {
    makeAmInfo.rel_name = "po/Makefile.am";
    KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.rel_name.data());
    makeAmInfo.type = "normal";
    KDEBUG1(KDEBUG_INFO,CKAPPWIZARD,"%s",makeAmInfo.type.data());
    sub_dir_list.clear();
    makeAmInfo.sub_dirs = sub_dir_list;
    project->writeMakefileAmInfo (makeAmInfo);
    project->addMakefileAmToProject (makeAmInfo.rel_name);
  }
  TFileInfo fileInfo;
  if (gnufiles->isChecked()) {
  project->addFileToProject ("AUTHORS");
  fileInfo.rel_name = "AUTHORS";
  fileInfo.type = "DATA";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);

  project->addFileToProject ("COPYING");
  fileInfo.rel_name = "COPYING";
  fileInfo.type = "DATA";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  project->addFileToProject ("ChangeLog");
  fileInfo.rel_name = "ChangeLog";
  fileInfo.type = "DATA";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);

  project->addFileToProject ("INSTALL");
  fileInfo.rel_name = "INSTALL";
  fileInfo.type = "DATA";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  project->addFileToProject ("README");
  fileInfo.rel_name = "README";
  fileInfo.type = "DATA";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);

  project->addFileToProject ("TODO");
  fileInfo.rel_name = "TODO";
  fileInfo.type = "DATA";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  }

  if (lsmfile->isChecked()) {
    project->addFileToProject (namelow + ".lsm");
    fileInfo.rel_name = namelow + ".lsm";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->writeFileInfo (fileInfo);
  }

  project->addFileToProject (namelow + "/main.cpp");
  fileInfo.rel_name = namelow + "/main.cpp";
  fileInfo.type = "SOURCE";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  if (!(ta->isChecked())) {
  project->addFileToProject (namelow + "/" + namelow + ".cpp");
  fileInfo.rel_name = namelow + "/" + namelow + ".cpp";
  fileInfo.type = "SOURCE";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  project->addFileToProject (namelow + "/" + namelow + ".h");
  fileInfo.rel_name = namelow + "/" + namelow + ".h";
  fileInfo.type = "HEADER";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  }

  if (kna->isChecked() || qta->isChecked()) {
  project->addFileToProject (namelow + "/" + namelow + "doc.cpp");
  fileInfo.rel_name = namelow + "/" + namelow + "doc.cpp";
  fileInfo.type = "SOURCE";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  project->addFileToProject (namelow + "/" + namelow + "doc.h");
  fileInfo.rel_name = namelow + "/" + namelow + "doc.h";
  fileInfo.type = "HEADER";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  project->addFileToProject (namelow + "/" + namelow + "view.cpp");
  fileInfo.rel_name = namelow + "/" + namelow + "view.cpp";
  fileInfo.type = "SOURCE";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  project->addFileToProject (namelow + "/" + namelow + "view.h");
  fileInfo.rel_name = namelow + "/" + namelow + "view.h";
  fileInfo.type = "HEADER";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  project->addFileToProject (namelow + "/resource.h");
  fileInfo.rel_name = namelow + "/resource.h";
  fileInfo.type = "HEADER";
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->writeFileInfo (fileInfo);
  }

  if (datalink->isChecked()) {
    project->addFileToProject (namelow + "/" + namelow + ".kdelnk");
    fileInfo.rel_name = namelow + "/" + namelow + ".kdelnk";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_appsdir)/Applications/" + namelow + ".kdelnk";
    project->writeFileInfo (fileInfo);
  }
  
  if (progicon->isChecked()) {
    project->addFileToProject (namelow + "/" + namelow + ".xpm");
    fileInfo.rel_name = namelow + "/" + namelow + ".xpm";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_icondir)/" + namelow + ".xpm";
    project->writeFileInfo (fileInfo);
  }

  if (miniicon->isChecked()) {
    project->addFileToProject (namelow + "/mini-" + namelow + ".xpm");
    fileInfo.rel_name = namelow + "/mini-" + namelow + ".xpm";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
    fileInfo.install = true;
    QString icon = namelow + ".xpm";
    fileInfo.install_location = "$(kde_minidir)/" + icon;
    project->writeFileInfo (fileInfo);
  }
  
  if (userdoc->isChecked()) {
    project->addFileToProject (namelow + "/docs/en/index-1.html");
    fileInfo.rel_name = namelow + "/docs/en/index-1.html";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
   if (!(ta->isChecked())) {
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-1.html";
   }
    project->writeFileInfo (fileInfo);

    project->addFileToProject (namelow + "/docs/en/index-2.html");
    fileInfo.rel_name = namelow + "/docs/en/index-2.html";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
   if (!(ta->isChecked())) {
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-2.html";
   }
    project->writeFileInfo (fileInfo);
    project->addFileToProject (namelow + "/docs/en/index-3.html");
    fileInfo.rel_name = namelow + "/docs/en/index-3.html";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
   if (!(ta->isChecked())) {
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-3.html";
   }
    project->writeFileInfo (fileInfo);
    project->addFileToProject (namelow + "/docs/en/index-4.html");
    fileInfo.rel_name = namelow + "/docs/en/index-4.html";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
   if (!(ta->isChecked())) {
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-4.html";
   }
    project->writeFileInfo (fileInfo);
    project->addFileToProject (namelow + "/docs/en/index-5.html");
    fileInfo.rel_name = namelow + "/docs/en/index-5.html";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
   if (!(ta->isChecked())) { 
   fileInfo.install = true;
    fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-5.html";
   }
    project->writeFileInfo (fileInfo);
    project->addFileToProject (namelow + "/docs/en/index-6.html");
    fileInfo.rel_name = namelow + "/docs/en/index-6.html";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
   if (!(ta->isChecked())) {
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-6.html";
   }
    project->writeFileInfo (fileInfo);
    project->addFileToProject (namelow + "/docs/en/index.html");
    fileInfo.rel_name = namelow + "/docs/en/index.html";
    fileInfo.type = "DATA";
    fileInfo.dist = true;
   if (!(ta->isChecked())) {
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index.html";
   }
    project->writeFileInfo (fileInfo);
  }
  
  
  QStrList group_filters;
  group_filters.append("*");
  project->addLFVGroup ("Others","");
  project->setFilters("Others",group_filters);

  
  if (gnufiles->isChecked()) {
  group_filters.clear();
  group_filters.append("AUTHORS");
  group_filters.append("COPYING");
  group_filters.append("ChangeLog");
  group_filters.append("INSTALL");
  group_filters.append("README");
  group_filters.append("TODO");
  group_filters.append("NEWS");
  project->addLFVGroup ("GNU","");
  project->setFilters("GNU",group_filters);
  }

  group_filters.clear();
  group_filters.append("*.cpp");
  group_filters.append("*.c");
  group_filters.append("*.cc");
  group_filters.append("*.C");
  project->addLFVGroup ("Sources","");
  project->setFilters("Sources",group_filters);
 
  group_filters.clear();
  group_filters.append("*.h");
  project->addLFVGroup ("Header","");
  project->setFilters("Header",group_filters);

  

  project->writeProject ();
  project->updateMakefilesAm ();

  q = new KProcess();
  connect(q,SIGNAL(processExited(KProcess *)),this,SLOT(slotMakeEnd()));
  connect(q,SIGNAL(receivedStdout(KProcess *, char *, int)),
          this,SLOT(slotPerlOut(KProcess *, char *, int)));
  connect(q,SIGNAL(receivedStderr(KProcess *, char *, int)),
          this,SLOT(slotPerlErr(KProcess *, char *, int)));
  QString path1 = kapp->kde_datadir()+"/kdevelop/tools/";
  *q << "perl" << path1 + "processesend.pl";
  q->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

// enable cancelbutton if everything is done
void CKAppWizard::slotMakeEnd() {
  cancelButton->setEnabled(true);
  gen_prj = true;
}

// return the directory with the projectfile
QString CKAppWizard::getProjectFile() {
  nametext = nameline->text();
  nametext = nametext.lower();
  directorytext = directoryline->text();
  if(QString(directoryline->text()).right(1) == "/"){
    directorytext = directorytext + nametext + ".kdevprj";
  }
  else{
    directorytext = directorytext + "/" + nametext + ".kdevprj";
  }
  delete (directoryline);
  delete (nameline);
  delete (defaultButton);
  delete (okButton);
  delete (cancelButton);
  delete (widget0);
  delete (widget1);
  delete (widget2);
  delete (widget3);
  delete (widget4);
  delete (widget5);
  return directorytext;
}

// return TRUE if a poject is generated
bool CKAppWizard::generatedProject(){
  return gen_prj;
}
