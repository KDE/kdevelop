/***************************************************************************
                          projectspace.cpp  -  description
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

#include "projectspace.h"
#include <kprocess.h>
#include <kstddirs.h>
#include <iostream.h>
#include "plugin.h"
#include <kstddirs.h>
#include <ksimpleconfig.h>

ProjectSpace::ProjectSpace(QObject* parent,const char* name) : QObjectPlugin(parent,name){
	m_projects = new QList<Project>;
}
ProjectSpace::~ProjectSpace(){
}

void ProjectSpace::addProject(Project* prj){
  m_projects->append (prj);
}
void ProjectSpace::removeProject(QString name){
}
void ProjectSpace::generateDefaultFiles(){
  KShellProcess proc("/bin/sh");
  
  // create the directories
  proc << "mkdirhier";
  proc << m_path;
  proc.start(KProcess::Block,KProcess::AllOutput);
  
  // untar/unzip the projectspace
  proc.clearArguments();
  QString args = "xzvf " + m_projectspace_template + " -C " + m_path;
  cerr << "ProjectSpace::generateDefaultFiles():" << args;
  proc << "tar";
  proc << args;
  proc.start(KProcess::Block,KProcess::AllOutput);
}

/*_____some get methods_____*/

/** returns the name of the projectspace*/
QString ProjectSpace::getName(){
  }

  /** Fetch the name of the version control system */
  QString ProjectSpace::getVCSystem(){
  }

  /** Fetch the authors name. stored in the *_user files*/
  QString ProjectSpace::getAuthor(){
  }

  /** Fetch the authors eMail-address,  stored in the *_user files */
  QString ProjectSpace::getEmail(){
  }

   /** set the projectspace name*/
  void ProjectSpace::setName(QString name){
  	m_name = name;
  }
  void ProjectSpace::setAbsolutePath(QString path){
  	m_path = path;
  }

  /** Store the name of version control system */
  void ProjectSpace::setVCSystem(QString vcsystem){
  }

  /** stored in the *_user files*/
  void ProjectSpace::setAuthor(QString name){
  }

  /** set the email, stored in the *_user file */
  void ProjectSpace::setEmail(QString email){
  }

  /** method to fill up a string template with actual projectspace info
   */
  QString& ProjectSpace::setInfosInString(QString& strtemplate, bool basics){
  }


  /** writes a NAME.kdevpsp and NAME_user.kdevpsp
      NAME.kdevpsp contains options for all users, like cvs system
      NAME_user contains options from the local user:
  */
  bool ProjectSpace::readConfig(QString filename){
  	return true;
  }

bool ProjectSpace::readGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  return true;
}

bool ProjectSpace::readUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  return true;
}


bool ProjectSpace::writeConfig(){
  // the "global" one
  QString filename = m_path + "/" + m_name + ".kdevpsp";
  cerr << "filename:" << filename << "\n";
  KSimpleConfig* config = new KSimpleConfig(filename);
  writeGeneralConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  
  // the "local" on
  filename = m_path + "/." + m_name + ".kdevpsp";
  cerr << "filename:" << filename << "\n";
  config = new KSimpleConfig(filename);
  writeUserConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  
}
bool ProjectSpace::writeGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("name",m_name);
  config->writeEntry("path",m_path);
  config->writeEntry("plugin_libraryname", m_plugin_libraryname);
  QStringList projectfiles;
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    projectfiles.append(prj->getProjectFile());
  }
  config->writeEntry("projectfiles",projectfiles);
  
  return true;
}

bool ProjectSpace::writeUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("user", m_plugin_libraryname);
  return true;
}
