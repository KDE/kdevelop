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

#include "kdejavasdiappwizardplugin.h"

#include <iostream.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <qregexp.h>
#include <kglobal.h>
#include <kprocess.h>
#include <qfile.h>
#include <registeredfile.h>

KDEJavaSDIAppWizardPlugin::KDEJavaSDIAppWizardPlugin(QObject*,const char*) {
		
	KStandardDirs* std_dirs = KGlobal::dirs();
		
	
	// AppWizardPlugin infos	
	m_projectspaceName = "KDE";
	m_applicationPicture = std_dirs->findResource("data","kdevelop/appwizard_pics/normalApp.bmp");	
	m_projectTemplate = std_dirs->findResource("data","kdevelop/project_templates/kdejavasdi.tar.gz");
	m_applicationDescription = "<b>Java SDI</b>";	
	// project infos
	m_projecttypeName = "KDEBinaryProject";
	m_pAboutData=0;
}
KDEJavaSDIAppWizardPlugin::~KDEJavaSDIAppWizardPlugin(){	
}

void KDEJavaSDIAppWizardPlugin::init(bool new_projectspace,ProjectSpace* projectspace,QString projectname, QString absProjectLocation){
  AppWizard::init(new_projectspace,projectspace,projectname,absProjectLocation); // the default one, generate a new project

  //add a new page (fileprops)
  m_sdi_fileprops_page = new FilePropsPage(this,"fileprops");
  QList<ClassFileProp>* props_temp = new QList<ClassFileProp>;
  ClassFileProp* prop = new ClassFileProp();
  prop->m_classname = projectname + "App";
  prop->m_implfile = projectname + "App.java";
  prop->m_baseclass = "KMainWindow";
  prop->m_description = "The base class for the application window. It sets up the main window and reads the config file as well as providing a menubar, toolbar and statusbar. An instance of the View creates your center view, which is connected to the window's Doc object.";
  prop->m_change_baseclass = false;
  prop->m_key = "App";
  props_temp->append(prop);

  prop = new ClassFileProp();
  prop->m_classname = projectname + "View";
  prop->m_implfile = projectname + "View.java";
  prop->m_baseclass = "QWidget";
  prop->m_description = "The View class provides the view widget for the App instance. The View instance inherits QWidget as a base class and represents the view object of a KMainWindow. As View is part of the document-view model, it needs a reference to the document object connected with it by the App class to manipulate and display the document structure provided by the Doc class.";
  prop->m_change_baseclass = true;
  prop->m_key = "View";
  props_temp->append(prop);

  prop = new ClassFileProp();
  prop->m_classname = projectname + "Doc";
  prop->m_implfile = projectname + "Doc.java";
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
void KDEJavaSDIAppWizardPlugin::generateDefaultFiles(){
   KShellProcess proc("/bin/sh");

  // create the directories
  proc << "mkdirhier";
  proc << m_pProject->absolutePath();
  proc.start(KProcess::Block,KProcess::AllOutput);

  // untar/unzip the project to a tmp dir
  KStandardDirs *dirs =  KGlobal::dirs();
  QString tmp_location = dirs->saveLocation("data","kdevelop/appwizard/");
  proc.clearArguments();
  QString args = "xzvf " + m_projectTemplate + " -C " + tmp_location;
  cerr << "KDEJavaSDIAppWizardPlugin::generateDefaultFiles():" << args;
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

  generateFile(tmp_location + "KBaseApp.java",m_pProject->absolutePath() + "/" + m_app_prop->m_implfile);
  generateFile(tmp_location + "KBaseView.java",m_pProject->absolutePath() + "/" + m_view_prop->m_implfile);
  generateFile(tmp_location + "KBaseDoc.java",m_pProject->absolutePath() + "/" + m_doc_prop->m_implfile);
  generateFile(tmp_location + "kbaseui.rc",m_pProject->absolutePath()
	       + "/" + m_pProject->name().lower() + "ui.rc");	

  // add the files to the project
  m_pProject->addFile(m_pProject->absolutePath() + "/" + m_app_prop->m_implfile);
  m_pProject->addFile(m_pProject->absolutePath() + "/" + m_view_prop->m_implfile);
  m_pProject->addFile(m_pProject->absolutePath() + "/" + m_doc_prop->m_implfile);

  // the ui.rc  file
  RegisteredFile* file = new RegisteredFile(m_pProject->name().lower() + "ui.rc");
  file->setInstall(true);
  file->setInstallFile("$(kde_datadir)/" + m_pProject->name() + "/"
		       + m_pProject->name().lower() + "ui.rc");
  m_pProject->addFile(file);

}


void KDEJavaSDIAppWizardPlugin::setInfosInString(QString& text){

  AppWizard::setInfosInString(text); // defaults like |VERSION|

  text.replace(QRegExp("|APPCLASS|"),m_app_prop->m_classname);
  text.replace(QRegExp("|DOCCLASS|"),m_doc_prop->m_classname);
  text.replace(QRegExp("|VIEWCLASS|"),m_view_prop->m_classname);
}

KAboutData* KDEJavaSDIAppWizardPlugin::aboutPlugin(){
  if (m_pAboutData == 0){
    m_pAboutData= new KAboutData( "KDE Java SDI", I18N_NOOP("KDE Java SDI Appwizard"),
				  "0.1", "desc",
				  KAboutData::License_GPL,
				  "(c) 1998-2000, The KDevelop Team",
				  "text",
				  "http://www.kdevelop.org");

    m_pAboutData->addAuthor("Richard Dale",I18N_NOOP("Developer"), "Richard_Dale@tipitina.demon.co.uk");

  }
  return m_pAboutData;
}
#include "kdejavasdiappwizardplugin.moc"
