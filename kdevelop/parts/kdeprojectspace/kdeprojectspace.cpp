/***************************************************************************
                          kdeprojectspace.cpp
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdeprojectspace.h"
#include "main.h"
#include <kinstance.h>
#include <iostream.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kaction.h>

KDEProjectSpace::KDEProjectSpace(QObject* parent,const char* name)
		: AutomakeProjectSpace(parent,name){
	cerr << "\nKDEProjectSpace created\n";
	
	setInstance(KDEProjectSpaceFactory::instance());

	KStandardDirs* std_dirs = KGlobal::dirs();
	
	// init, every plugin should do this
	m_plugin_author = "KDevelop Team";
	m_plugin_name = "KDE";
	m_plugin_copyright = "(C) 2000 by KDevelop Team";
	m_plugin_version = "0.1";
	m_plugin_description = "kk";
	m_plugin_homepage = "http://www.kdevelop.org";
	m_plugin_icon = DesktopIcon("buttons");
	m_plugin_libraryname = "libkde_projectspace";
	// projectspace
	m_projectspace_template = std_dirs->findResource("data","kdevelop/projectspaces/kde_projectspace.tar.gz");
	m_language = "C++";


	//	setXMLFile("kdevkdeprojectspaceui.rc");
	setXMLFile("/home/kde2/kdevelop/kdevelop/parts/kdeprojectspace/kdevkdeprojectspace.rc"); //test
}
KDEProjectSpace::~KDEProjectSpace(){
}

void KDEProjectSpace::setupGUI(){
  cerr << "Building KDEProjectSpace GUI" << endl;
  KAction *pAction;
  pAction = new KAction( i18n("Add new &Translation File..."), "locale", 0, 
			 this, SLOT( slotProjectAddNewTranslationFile() ),
			 actionCollection(), "project_add_translation");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Adds a new language for internationalization to the project") );
}

// slots

void KDEProjectSpace::slotProjectAddNewTranslationFile(){
  cerr << "\n Add new &Translation File (KDE)... actived";
}
void KDEProjectSpace::modifyDefaultFiles(){
  AutomakeProjectSpace::modifyDefaultFiles();
}

 /** add the data to the psElement (Projectspace)*/
bool KDEProjectSpace::writeGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::writeGlobalConfig(doc,psElement);
  QDomElement kdeElement = psElement.appendChild(doc.createElement("KDEProjectSpace")).toElement();
  return true;
}
bool KDEProjectSpace::writeUserConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::writeUserConfig(doc,psElement);
  QDomElement kdeElement = psElement.appendChild(doc.createElement("KDEProjectSpace")).toElement();
  return true;
}

bool KDEProjectSpace::readGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::readGlobalConfig(doc,psElement);
  QDomElement kdeElement = psElement.namedItem("KDEProjectSpace").toElement();
  if(kdeElement.isNull()){
    cerr << "\nKDEProjectSpace::readGlobalConfig not \"KDEProjectSpace\" tag found!";
    return false;
  }
  else{
    cerr << "\nKDEProjectSpace::readGlobalConfig  \"KDEProjectSpace\" tag found!";
  }
  return true;
}
bool KDEProjectSpace::readUserConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::readUserConfig(doc,psElement);
  QDomElement kdeElement = psElement.namedItem("KDEProjectSpace").toElement();
  if(kdeElement.isNull()){
    cerr << "\nKDEProjectSpace::readUserConfig not \"KDEProjectSpace\" tag found!";
    return false;
  }
  else{
    cerr << "\nKDEProjectSpace::readUserConfig  \"KDEProjectSpace\" tag found!";
  }
  return true;
}


