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
	
	cerr << "\nKDESDIAppWizardPlugin:: I'm a plugin";	
}
KDESDIAppWizardPlugin::~KDESDIAppWizardPlugin(){	
}

void KDESDIAppWizardPlugin::init(bool new_projectspace,ProjectSpace* projectspace){
  AppWizard::init(new_projectspace,projectspace); // the default one
  
  //add a new page
  m_sdi_fileprops_page = new FilePropsPage(this,"fileprops");
  QList<ClassFileProp>* props_temp = new QList<ClassFileProp>;
  ClassFileProp* prop = new ClassFileProp();
  prop->m_classname = "HelloApp";
  prop->m_headerfile = "helloapp.h";
  prop->m_implfile = "helloapp.cpp";
  prop->m_baseclass = "QMainWindow";
  prop->m_description = "foo <b>bla</b> foo bar";
  prop->m_change_baseclass = false;
  props_temp->append(prop);
  
  prop = new ClassFileProp();
  prop->m_classname = "HelloView";
  prop->m_headerfile = "helloview.h";
  prop->m_implfile = "helloview.cpp";
  prop->m_baseclass = "QButton";
  prop->m_description = "mammy bla mimmy";
  prop->m_change_baseclass = true;
  props_temp->append(prop);

  m_sdi_fileprops_page->setClassFileProps(*props_temp);
  addPage(m_sdi_fileprops_page,"Class/File Properties");
  
  // general
  m_sdi_general_page = new QWidget(this,"plugin_general");
  m_text = new QLabel("I'm a prop widget from a plugin",m_sdi_general_page);
  m_text->resize(200,100);
  addPage(m_sdi_general_page,"General");
  setFinishEnabled (m_sdi_general_page, true);
}

/** generates default files/app, properties from configwidgets set in AppWizard*/
void KDESDIAppWizardPlugin::generateDefaultFiles(){
  AppWizard::generateDefaultFiles(); // the default one
};
