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
	m_plugin_description = "KDE SDI Application";
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
