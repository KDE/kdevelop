/***************************************************************************
                          appwizard.cpp  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appwizard.h"
#include "pluginloader.h"
#include <qfiledialog.h>
#include <iostream.h>
#include <kprocess.h>
#include <kdebug.h>


AppWizard::AppWizard(QWidget* parent, const char* obj_name) : QWizard(parent,obj_name,true){
  // conntects
  m_project=0;
  
}
AppWizard::~AppWizard(){
}

void AppWizard::init(bool new_projectspace,ProjectSpace* projectspace){
  kdDebug(9000) << "enter AppWizard::init" << endl;
  m_new_projectspace = new_projectspace;
  m_projectspace = projectspace;
  getProject();
  
  initDefaultPages(); // initPages
  // add pages
  addPage(m_general_page,"first");
  if(m_new_projectspace){
    addPage(m_fileheader_page,"second");
  }
  resize(519,513);
}

void AppWizard::accept(){
  kdDebug(9000) << "start generation" << endl;
  if(m_new_projectspace){ // only if a new one was selected
    m_projectspace->generateDefaultFiles();
  }
  m_projectspace->addProject(m_project);
  m_projectspace->writeConfig();
  generateDefaultFiles();
  QWizard::accept();
}

QString  AppWizard::getProjectSpaceName(){
  return m_project_space_name;
}
QString  AppWizard::getPreviewPicture(){
  return m_application_picture;
}
Project* AppWizard::getProject(){
  if(m_project == 0){ // no project instance
    m_project = PluginLoader::getNewProject(m_projecttype_name);
  }
  return m_project;
}

void AppWizard::generateDefaultFiles(){
  KShellProcess proc("/bin/sh");
  
  // create the directories
  proc << "mkdirhier";
  proc << m_project->getAbsolutePath();
  proc.start(KProcess::Block,KProcess::AllOutput);
  
  // untar/unzip the project
  proc.clearArguments();
  QString args = "xzvf " + m_project_template + " -C " + m_project->getAbsolutePath();
  cerr << "AppWizard::generateDefaultFiles():" << args;
  proc << "tar";
  proc << args;
  proc.start(KProcess::Block,KProcess::AllOutput);
}
void AppWizard::initDefaultPages(){
	
  m_general_page = new QWidget (this,"general");


  versionnumber = new QLabel( i18n("Version number:"), m_general_page, "versionnumber" );
  versionnumber->setGeometry( 30, 90, 100, 30 );
  versionnumber->setAlignment( 289 );

  versionline = new QLineEdit( m_general_page, "versionline" );
  versionline->setGeometry( 140, 90, 290, 30 );

  authorname = new QLabel( i18n("Author:"), m_general_page, "authorname" );
  authorname->setGeometry( 30, 130, 100, 30 );
  authorname->setAlignment( 289 );

  authorline = new QLineEdit( m_general_page, "authorline" );
  authorline->setGeometry( 140, 130, 290, 30 );

  email = new QLabel( i18n("Email"), m_general_page, "email" );
  email->setGeometry( 30, 170, 100, 30 );
  email->setAlignment( 289 );

  emailline = new QLineEdit( m_general_page, "emailline" );
  emailline->setGeometry( 140, 170, 290, 30 );

    gnufiles = new QCheckBox( i18n("GNU-Standard-Files (INSTALL,README,COPYING...)" ),
                            m_general_page, "gnufiles" );
  gnufiles->setGeometry( 30, 270, 440, 30 );

  lsmfile = new QCheckBox( i18n("lsm-File - Linux Software Map"),
                           m_general_page, "lsmfile" );
   lsmfile->setGeometry( 30, 360, 340, 30 );


		m_general_page->resize(400,300);
		
 // second page (fileheader)
  m_fileheader_page = new QWidget (this,"general");
  	
  fheader = new QCheckBox(  i18n("headertemplate for your files"), m_fileheader_page, "hheader" );
  fheader->setGeometry( 20, 20, 230, 30 );

  fload = new QPushButton( i18n("Load..."), m_fileheader_page, "hload" );
  fload->setGeometry( 260, 20, 100, 30 );

  fnew = new QPushButton( i18n("New"), m_fileheader_page, "hnew" );
  fnew->setGeometry( 380, 20, 100, 30 );

  fedit = new KEdit( m_fileheader_page );
  QFont f("fixed",10);
  fedit->setFont(f);
  fedit->setGeometry( 20, 70, 460, 350 );

  QToolTip::add(fload,i18n("Press this button to select an\n"
  													"existing header template file"));
  QToolTip::add(fnew,i18n("Clears the pre-set headertemplate"));
  QToolTip::add(fedit,i18n("Edit your headertemplate here"));

  QWhatsThis::add(fheader, i18n("Use a standard\n"
				"headertemplate for your headerfiles"));

}
