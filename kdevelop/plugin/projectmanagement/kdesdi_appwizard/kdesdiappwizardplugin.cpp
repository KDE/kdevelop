/***************************************************************************
                          kdeSDIappwizardplugin.cpp  -  description
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

#include "kdesdiappwizardplugin.h"

#include <iostream.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <qregexp.h>
#include <kglobal.h>
#include <kprocess.h>
#include <qfile.h>
#include <registeredfile.h>

KDESDIAppWizardPlugin::KDESDIAppWizardPlugin(QObject* parent,const char* name){
		
	KStandardDirs* std_dirs = KGlobal::dirs();
		
	// init, every plugin should do this
	// Plugin infos
	m_plugin_author = "KDevelop Team";
	m_plugin_name = "KDE SDI";
	m_plugin_copyright = "(C) 2000 by KDevelop Team";
	m_plugin_version = "0.1";
	m_plugin_description = "KDE <b>SDI</b> Application";
	m_plugin_homepage = "http://www.kdevelop.org";
	m_plugin_icon = DesktopIcon("java_src");
	
	// AppWizardPlugin infos	
	m_project_space_name = "KDE";
	m_application_picture = std_dirs->findResource("data","kdevelop/appwizard_pics/normalApp.bmp");	
	m_project_template = std_dirs->findResource("data","kdevelop/project_templates/kdesdi.tar.gz");
	
	// project infos
	m_projecttype_name = "KDEBinaryProject";
}
KDESDIAppWizardPlugin::~KDESDIAppWizardPlugin(){	
}

void KDESDIAppWizardPlugin::init(bool new_projectspace,ProjectSpace* projectspace,QString projectname){
  AppWizard::init(new_projectspace,projectspace,projectname); // the default one, generate a new project
  
  //add a new page (fileprops)
  m_sdi_fileprops_page = new FilePropsPage(this,"fileprops");
  QList<ClassFileProp>* props_temp = new QList<ClassFileProp>;
  ClassFileProp* prop = new ClassFileProp();
  prop->m_classname = projectname + "App";
  prop->m_headerfile = projectname.lower() + "app.h";
  prop->m_implfile = projectname.lower() + "app.cpp";
  prop->m_baseclass = "KMainWindow";
  prop->m_description = "The base class for the application window. It sets up the main window and reads the config file as well as providing a menubar, toolbar and statusbar. An instance of the View creates your center view, which is connected to the window's Doc object.";
  prop->m_change_baseclass = false;
  prop->m_key = "App";
  props_temp->append(prop);
  
  prop = new ClassFileProp();
  prop->m_classname = projectname + "View";
  prop->m_headerfile = projectname.lower() + "view.h";
  prop->m_implfile = projectname.lower() + "view.cpp";
  prop->m_baseclass = "QWidget";
  prop->m_description = "The View class provides the view widget for the App instance. The View instance inherits QWidget as a base class and represents the view object of a KMainWindow. As View is part of the document-view model, it needs a reference to the document object connected with it by the App class to manipulate and display the document structure provided by the Doc class.";
  prop->m_change_baseclass = true;
  prop->m_key = "View";
  props_temp->append(prop);

  prop = new ClassFileProp();
  prop->m_classname = projectname + "Doc";
  prop->m_headerfile = projectname.lower() + "doc.h";
  prop->m_implfile = projectname.lower() + "doc.cpp";
  prop->m_baseclass = "QObject";
  prop->m_description = "The Doc class provides a document object that can be used in conjunction with the classes App and View to create a document-view model for standard KDE applications based on KApplication and KMainWindow. Doc contains the methods for serialization of the document data from and to files";
  prop->m_change_baseclass = true;
  prop->m_key = "Doc";
  props_temp->append(prop);

  m_sdi_fileprops_page->setClassFileProps(*props_temp);
  addPage(m_sdi_fileprops_page,"Class/File Properties");
  
  
  setFinishEnabled (m_sdi_fileprops_page, true);
}

/** generates default files/app, properties from configwidgets set in AppWizard*/
void KDESDIAppWizardPlugin::generateDefaultFiles(){
   KShellProcess proc("/bin/sh");
  
  // create the directories
  proc << "mkdirhier";
  proc << m_project->getAbsolutePath();
  proc.start(KProcess::Block,KProcess::AllOutput);
  
  // untar/unzip the project to a tmp dir
  KStandardDirs *dirs =  KGlobal::dirs();
  QString tmp_location = dirs->saveLocation("data","kdevelop/appwizard/");
  proc.clearArguments();
  QString args = "xzvf " + m_project_template + " -C " + tmp_location;
  cerr << "KDESDIAppWizardPlugin::generateDefaultFiles():" << args;
  proc << "tar";
  proc << args;
  proc.start(KProcess::Block,KProcess::AllOutput);


  QList<ClassFileProp> props = m_sdi_fileprops_page->getClassFileProps();
  ClassFileProp* prop;
  for ( prop=props.first(); prop != 0; prop=props.next() ){
    if (prop->m_key == "App"){
      m_app_prop = prop;
    }
    if (prop->m_key == "View"){
      m_view_prop = prop;
    }
    if (prop->m_key == "Doc"){
      m_doc_prop = prop;
    }
  }
 
  generateFile(tmp_location + "kbase.h",m_project->getAbsolutePath() + "/" + m_app_prop->m_headerfile);
  generateFile(tmp_location + "kbase.cpp",m_project->getAbsolutePath() + "/" + m_app_prop->m_implfile);  
  generateFile(tmp_location + "kbaseview.h",m_project->getAbsolutePath() + "/" + m_view_prop->m_headerfile);
  generateFile(tmp_location + "kbaseview.cpp",m_project->getAbsolutePath() + "/" + m_view_prop->m_implfile);
  generateFile(tmp_location + "kbasedoc.h",m_project->getAbsolutePath() + "/" + m_doc_prop->m_headerfile);
  generateFile(tmp_location + "kbasedoc.cpp",m_project->getAbsolutePath() + "/" + m_doc_prop->m_implfile);
  generateFile(tmp_location + "main.cpp",m_project->getAbsolutePath() 
	       + "/" + m_project->getName().lower() + ".cpp");
  generateFile(tmp_location + "kbaseui.rc",m_project->getAbsolutePath() 
	       + "/" + m_project->getName().lower() + "ui.rc");	 

  // add the files to the project
  m_project->addFile(m_project->getAbsolutePath() + "/" + m_app_prop->m_headerfile);
  m_project->addFile(m_project->getAbsolutePath() + "/" + m_app_prop->m_implfile);
  m_project->addFile(m_project->getAbsolutePath() + "/" + m_view_prop->m_headerfile);
  m_project->addFile(m_project->getAbsolutePath() + "/" + m_view_prop->m_implfile);
  m_project->addFile(m_project->getAbsolutePath() + "/" + m_doc_prop->m_headerfile);
  m_project->addFile(m_project->getAbsolutePath() + "/" + m_doc_prop->m_implfile);
  m_project->addFile(m_project->getAbsolutePath() + "/" + m_project->getName().lower() + ".cpp");
  
  // the ui.rc  file
  RegisteredFile* file = new RegisteredFile(m_project->getName().lower() + "ui.rc");
  file->setInstall(true);
  file->setInstallFile("$(kde_datadir)/" + m_project->getName() + "/" 
		       + m_project->getName().lower() + "ui.rc");
  m_project->addFile(file);
  
}


void KDESDIAppWizardPlugin::setInfosInString(QString& text){

  AppWizard::setInfosInString(text); // defaults like |VERSION|

  QString appheader_big = m_app_prop->m_headerfile.upper();
  appheader_big.replace( QRegExp("\\."),"_");
  QString docheader_big = m_doc_prop->m_headerfile.upper();
  docheader_big.replace( QRegExp("\\."),"_");
  QString viewheader_big = m_view_prop->m_headerfile.upper();
  viewheader_big.replace( QRegExp("\\."),"_");

  text.replace(QRegExp("|APPHEADER|"),m_app_prop->m_headerfile);
  text.replace(QRegExp("|APPHEADERBIG|"),appheader_big);
  text.replace(QRegExp("|VIEWHEADER|"),m_view_prop->m_headerfile);
  text.replace(QRegExp("|VIEWHEADERBIG|"),viewheader_big);
  text.replace(QRegExp("|DOCHEADER|"),m_doc_prop->m_headerfile);
  text.replace(QRegExp("|DOCHEADERBIG|"),docheader_big);

  text.replace(QRegExp("|APPCLASS|"),m_app_prop->m_classname);
  text.replace(QRegExp("|DOCCLASS|"),m_doc_prop->m_classname);
  text.replace(QRegExp("|VIEWCLASS|"),m_view_prop->m_classname);

  
  
}
