 /***************************************************************************
                          cnewprojectdlg.cpp  -  description
                             -------------------
    begin                : Tue Feb 9 1999
    copyright            : (C) 1999 by Steen Rabol, Ralf Nolden
    email                : rabol@get2net.dk, Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>

#include <kfiledialog.h>
#include <kmsgbox.h>
#include <kquickhelp.h>
#include <kprocess.h>

#include "cnewprojectdlg.h"

CNewProjectDlg::CNewProjectDlg(QWidget* parent, const char* name):QDialog(parent,name,true)
{
	setCaption(i18n("New Project"));
	
	m_pNameLabel = new QLabel( this, "name_label" );
	m_pNameLabel->setGeometry( 20, 30, 160, 25 );
	m_pNameLabel->setText(i18n("Project Name:"));
	m_pNameLabel->setAlignment( 289 );
	m_pNameLabel->setMargin( -1 );

	m_pNameEdit = new QLineEdit( this, "name_edit" );
	m_pNameEdit->setGeometry( 180, 30, 230, 25 );
	m_pNameEdit->setText( "" );
	m_pNameEdit->setMaxLength( 100 );
	m_pNameEdit->setEchoMode( QLineEdit::Normal );
	m_pNameEdit->setFrame( TRUE );

	KQuickHelp::add(m_pNameLabel,
	KQuickHelp::add(m_pNameEdit,i18n("Insert the name for the new project here.")));
	
	m_pVersionLabel = new QLabel( this, "version_label" );
	m_pVersionLabel->setGeometry( 20, 60, 160, 25 );
	m_pVersionLabel->setText(i18n("Version:") );
	m_pVersionLabel->setAlignment( 289 );
	m_pVersionLabel->setMargin( -1 );

	m_pVersionEdit = new QLineEdit( this, "version_edit" );
	m_pVersionEdit->setGeometry( 180, 60, 230, 25 );
	m_pVersionEdit->setText( "" );
	m_pVersionEdit->setMaxLength( 100 );
	m_pVersionEdit->setEchoMode( QLineEdit::Normal );
	m_pVersionEdit->setFrame( TRUE );
	
	KQuickHelp::add(m_pVersionLabel,
	KQuickHelp::add(m_pVersionEdit,i18n("Set the version number for the new project here.")));
	
	m_pDirLabel = new QLabel( this, "dir_label" );
	m_pDirLabel->setGeometry( 20, 100, 150, 25 );
	m_pDirLabel->setText(i18n("Project Directory:"));
	m_pDirLabel->setAlignment( 289 );
	m_pDirLabel->setMargin( -1 );
	
	m_pDirEdit = new QLineEdit( this, "dir_edit" );
	m_pDirEdit->setGeometry( 180, 100, 180, 25 );
	m_pDirEdit->setText(QDir::homeDirPath()+ QString("/"));
	m_pDirEdit->setMaxLength( 32767 );
	m_pDirEdit->setEchoMode( QLineEdit::Normal );
	m_pDirEdit->setFrame( TRUE );

	m_pDirSelect = new QPushButton( this, "dir_select" );
	m_pDirSelect->setGeometry( 370, 100, 40, 25 );
	m_pDirSelect->setText( "..." );
	m_pDirSelect->setAutoRepeat( FALSE );
	m_pDirSelect->setAutoResize( FALSE );

	KQuickHelp::add(m_pDirLabel,
	KQuickHelp::add(m_pDirEdit,
	KQuickHelp::add(m_pDirSelect,i18n("Select the main project directory here."))));
	
	m_pAuthorLabel = new QLabel( this, "author_label" );
	m_pAuthorLabel->setGeometry( 20, 140, 150, 25 );
	m_pAuthorLabel->setText(i18n("Author:"));
	m_pAuthorLabel->setAlignment( 289 );
	m_pAuthorLabel->setMargin( -1 );

	m_pAuthorEdit = new QLineEdit( this, "author_edit" );
	m_pAuthorEdit->setGeometry( 180, 140, 230, 25 );
	m_pAuthorEdit->setText( "" );
	m_pAuthorEdit->setMaxLength( 32767 );
	m_pAuthorEdit->setEchoMode( QLineEdit::Normal );
	m_pAuthorEdit->setFrame( TRUE );

	KQuickHelp::add(m_pAuthorLabel,
	KQuickHelp::add(m_pAuthorEdit,i18n("Insert your Name or your team name here.")));
	
	m_pEmailLabel = new QLabel( this, "email_label" );
	m_pEmailLabel->setGeometry( 20, 170, 120, 25 );
	m_pEmailLabel->setText(i18n("Email:"));
	m_pEmailLabel->setAlignment( 289 );
	m_pEmailLabel->setMargin( -1 );

	m_pEmailEdit = new QLineEdit( this, "email_edit" );
	m_pEmailEdit->setGeometry( 180, 170, 230, 25 );
	m_pEmailEdit->setText( "" );
	m_pEmailEdit->setMaxLength( 32767 );
	m_pEmailEdit->setEchoMode( QLineEdit::Normal );
	m_pEmailEdit->setFrame( TRUE );

  KQuickHelp::add(m_pEmailLabel,
  KQuickHelp::add(m_pEmailEdit, i18n("Insert your E-mail address here.")));

	m_pTypeKDE = new QRadioButton( this, "type_KDE" );
	m_pTypeKDE->setGeometry( 20, 240, 200, 25 );
	m_pTypeKDE->setText(i18n("KDE-Application"));
	m_pTypeKDE->setAutoRepeat( FALSE );
	m_pTypeKDE->setAutoResize( FALSE );
	m_pTypeKDE->setChecked( TRUE );

	KQuickHelp::add(m_pTypeKDE,i18n("Checking KDE-Application will execute the\n"
	                                "following actions:\n\n"
	                                "-creating the project directory and projectfile\n"
	                                "-creating GNU files\n"
	                                "-creating project subdirectories for po-files,\n"
	                                " and docs.\n\n"
	                                "Configure.in and Makefile.am's will be created\n"
	                                "with default values for KDE-applications."));
	
	m_pTypeKOM = new QRadioButton( this, "type_KOM" );
	m_pTypeKOM->setGeometry( 20, 270, 200, 25 );
	m_pTypeKOM->setText(i18n("KOM-Application"));
	m_pTypeKOM->setAutoRepeat( FALSE );
	m_pTypeKOM->setAutoResize( FALSE );
  m_pTypeKOM->setEnabled(false);
	
	KQuickHelp::add(m_pTypeKOM,i18n("Checking KOM-Application will execute the\n"
	                                "following actions:\n\n"
	                                "-creating the project directory and projectfile\n"
	                                "-creating GNU files\n"
	                                "-creating project subdirectories for po-files,\n"
	                                " and docs.\n\n"
	                                "Configure.in and Makefile.am's will be created\n"
	                                "with default values for KDE-applications\n"
	                                "using KOM (KDE Object Modell)."));


	m_pTypeQt = new QRadioButton( this, "type_Qt" );
	m_pTypeQt->setGeometry( 20, 300, 200, 25 );
	m_pTypeQt->setText(i18n("Qt-Application"));
	m_pTypeQt->setAutoRepeat( FALSE );
	m_pTypeQt->setAutoResize( FALSE );
  m_pTypeQt->setEnabled(false);
	
	KQuickHelp::add(m_pTypeQt,i18n("Checking Qt-Application will execute the\n"
	                                "following actions:\n\n"
	                                "-creating the project directory and projectfile\n"
	                                "-creating GNU files\n"
	                                "-creating project subdirectories for po-files,\n"
	                                " and docs.\n\n"
	                                "Configure.in and Makefile.am's will be created\n"
	                                "with default values for Qt-applications."));
	
	m_pTypeCplus = new QRadioButton( this, "type_Cplus" );
	m_pTypeCplus->setGeometry( 20, 330, 200, 25 );
	m_pTypeCplus->setText(i18n("C++-Application"));
	m_pTypeCplus->setAutoRepeat( FALSE );
	m_pTypeCplus->setAutoResize( FALSE );
	KQuickHelp::add(m_pTypeCplus,i18n("Checking C++-Application will execute the\n"
	                                "following actions:\n\n"
	                                "-creating the project directory and projectfile\n"
	                                "-creating GNU files\n"
	                                "-creating project subdirectory.\n\n"
	                                "Configure.in and Makefile.am's will be created\n"
	                                "with default values for Qt-applications."));
	
	m_pTypeShared = new QRadioButton( this, "type_shared" );
	m_pTypeShared->setGeometry( 230, 240, 180, 25 );
	m_pTypeShared->setText(i18n("Shared Library"));
	m_pTypeShared->setAutoRepeat( FALSE );
	m_pTypeShared->setAutoResize( FALSE );
  m_pTypeShared->setEnabled(false);
	
	KQuickHelp::add(m_pTypeShared,i18n("Checking Shared Library will execute the\n"
	                                "following actions:\n\n"
	                                "-creating the project directory and projectfile\n"
	                                "-creating GNU files\n"
	                                "-creating project subdirectory.\n\n"
	                                "Configure.in and Makefile.am's will be created\n"
	                                "with default values for construction of a shared\n"
	                                "libraries."));


	m_pTypeStatic = new QRadioButton( this, "type_static" );
	m_pTypeStatic->setGeometry( 230, 270, 180, 25 );
	m_pTypeStatic->setText(i18n("Static Library"));
	m_pTypeStatic->setAutoRepeat( FALSE );
	m_pTypeStatic->setAutoResize( FALSE );
  m_pTypeStatic->setEnabled(false);
	KQuickHelp::add(m_pTypeStatic,i18n("Checking Shared Library will execute the\n"
	                                "following actions:\n\n"
	                                "-creating the project directory and projectfile\n"
	                                "-creating GNU files\n"
	                                "-creating project subdirectory.\n\n"
	                                "Configure.in and Makefile.am's will be created\n"
	                                "with default values for construction of a static\n"
	                                "libraries."));


	m_pInfoGroup = new QButtonGroup( this, "info" );
	m_pInfoGroup->setGeometry( 10, 10, 420, 200 );
	m_pInfoGroup->setFrameStyle( 49 );
	m_pInfoGroup->setTitle(i18n("Project Info"));
	m_pInfoGroup->setAlignment( 1 );
	m_pInfoGroup->insert( m_pDirSelect );
	m_pInfoGroup->lower();
	
	m_pTypeGroup = new QButtonGroup( this, "type_group" );
	m_pTypeGroup->setGeometry( 10, 220, 420, 150 );
	m_pTypeGroup->setFrameStyle( 49 );
	m_pTypeGroup->setTitle(i18n("Project Type:"));
	m_pTypeGroup->setAlignment( 1 );
	m_pTypeGroup->insert( m_pTypeKDE );
	m_pTypeGroup->insert( m_pTypeKOM );
	m_pTypeGroup->insert( m_pTypeQt );
	m_pTypeGroup->insert( m_pTypeCplus );
	m_pTypeGroup->insert( m_pTypeShared );
	m_pTypeGroup->insert( m_pTypeStatic );
	m_pTypeGroup->lower();


	m_pOK = new QPushButton( this, "button_ok" );
	m_pOK->setGeometry( 30, 390, 120, 30 );
	m_pOK->setText( i18n("OK") );
	m_pOK->setAutoRepeat( FALSE );
	m_pOK->setAutoResize( FALSE );

	m_pCancel = new QPushButton( this, "button_cancel" );
	m_pCancel->setGeometry( 290, 390, 120, 30 );
	m_pCancel->setText(i18n("Cancel") );
	m_pCancel->setAutoRepeat( FALSE );
	m_pCancel->setAutoResize( FALSE );
	
	resize( 440, 440 );

	projectDir=QDir::homeDirPath()+ QString("/");
  success=false;
  connect(m_pNameEdit,SIGNAL(textChanged(const char*)),SLOT(slotNameChanged()));

	connect(m_pDirSelect,SIGNAL(clicked()),SLOT(slotDirSelect()));
  connect(m_pOK,SIGNAL(clicked()),SLOT(slotOK()));
  connect(m_pCancel,SIGNAL(clicked()),SLOT(reject()));

}


CNewProjectDlg::~CNewProjectDlg(){
}

QString CNewProjectDlg::getNewProjectFile(){
  return genProjectFile;
}

bool CNewProjectDlg::generatedNewProject(){
  return success;
}

void CNewProjectDlg::slotNameChanged(){
  QString name=m_pNameEdit->text();
  m_pDirEdit->setText(projectDir + name.lower());

}
void CNewProjectDlg::slotDirSelect(){
  KDirDialog* dirdialog = new KDirDialog(projectDir,this,"Directory");
  dirdialog->setCaption(i18n("Select New Project Directory"));
  dirdialog->show();
  QString projname = m_pNameEdit->text();
  projectDir = dirdialog->dirPath() + projname.lower();
  m_pDirEdit->setText(projectDir);
  projectDir = dirdialog->dirPath();
  delete (dirdialog);
}

void CNewProjectDlg::slotOK(){

  if(createdProject()){
    success=true;
    accept();
  }
  else{
		KMsgBox::message(0,i18n("KDevelop"),i18n("Creation of new project failed !"));
  	return;
  }
}


bool CNewProjectDlg::createdProject(){
  QString tmpPrjName=m_pNameEdit->text();
  QString tmpPrjVersion=m_pVersionEdit->text();
  QString tmpPrjDir=m_pDirEdit->text();
  QString tmpAuthor=m_pAuthorEdit->text();
  QString tmpEmail=m_pEmailEdit->text();
  if(tmpPrjName.isEmpty()){
		KMsgBox::message(0,i18n("KDevelop"),i18n("You need to select a name for the new project"));
		return false;
  }

  // First create the project directory
  QDir* dir=new QDir();
  dir->mkdir(tmpPrjDir);
  dir->setCurrent(tmpPrjDir);
  dir->mkdir(tmpPrjName.lower());

  // Then create the .kdevprj project
  QString prj_str = tmpPrjDir + "/" + tmpPrjName.lower() + ".kdevprj";
  newPrj = new CProject(prj_str);
  newPrj->readProject();
  newPrj->setKDevPrjVersion("0.2");
  newPrj->setProjectName(tmpPrjName);
  newPrj->setSubDir(tmpPrjName.lower() + "/");
  newPrj->setAuthor(tmpAuthor);
  newPrj->setEmail(tmpEmail);
  newPrj->setVersion(tmpPrjVersion);
  newPrj->setBinPROGRAM(tmpPrjName.lower());


  // copy GNU file templates
  KProcess* p = new KProcess();

  QString templ_path = KApplication::kde_datadir()+"/kdevelop/templates/";
  *p << "cp" << templ_path+"AUTHORS_template" <<"AUTHORS";
  p->start(KProcess::Block,KProcess::AllOutput);
  p->clearArguments();
  *p << "cp" << templ_path+"COPYING_template" <<"COPYING";
  p->start(KProcess::Block,KProcess::AllOutput);
  p->clearArguments();
  *p << "cp" << templ_path+"ChangeLog_template" <<"ChangeLog";
  p->start(KProcess::Block,KProcess::AllOutput);
  p->clearArguments();
  *p << "cp" << templ_path+"INSTALL_template" <<"INSTALL";
  p->start(KProcess::Block,KProcess::AllOutput);
  p->clearArguments();
  *p << "cp" << templ_path+"README_template" <<"README";
  p->start(KProcess::Block,KProcess::AllOutput);
  p->clearArguments();
  *p << "cp" << templ_path+"TODO_template" <<"TODO";
  p->start(KProcess::Block,KProcess::AllOutput);
  p->clearArguments();
  *p << "cp" << templ_path+"lsm_template" <<tmpPrjName.lower()+".lsm";
  p->start(KProcess::Block,KProcess::AllOutput);

  // now create specific files
  if(m_pTypeKDE->isChecked())
    if(!createdKDE())
      return false;
  if(m_pTypeKOM->isChecked())
    if(!createdKOM())
      return false;
  if(m_pTypeQt->isChecked())
    if(!createdQt())
      return false;
  if(m_pTypeShared->isChecked())
    if(!createdShared())
      return false;
  if(m_pTypeStatic->isChecked())
    if(!createdStatic())
      return false;
  dir->setCurrent(tmpPrjDir);
  newPrj->writeProject();
  genProjectFile=prj_str;
  return true;
}


bool CNewProjectDlg::createdKDE(){
  QString tmpPrjName=m_pNameEdit->text();
  QString tmpPrjVersion=m_pVersionEdit->text();
  QString tmpPrjDir=m_pDirEdit->text();
  QString tmpAuthor=m_pAuthorEdit->text();
  QString tmpEmail=m_pEmailEdit->text();
  QDir* dir=new QDir();
  dir->setCurrent(tmpPrjDir);
  dir->mkdir("po",false);
  newPrj->setProjectType("normal_kde");
  return true;
}
bool CNewProjectDlg::createdKOM(){
  return true;
}
bool CNewProjectDlg::createdQt(){
  return true;
}
bool CNewProjectDlg::createdCplus(){
  newPrj->setProjectType("normal_cpp");
  return true;
}
bool CNewProjectDlg::createdShared(){
  return true;
}
bool CNewProjectDlg::createdStatic(){
  return true;
}
















