/***************************************************************************
			 projectspace.cpp  -  description
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

#include "projectspace.h"
#include <kprocess.h>
#include <kstddirs.h>
#include <iostream.h>
#include "plugin.h"
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <qdir.h>
#include <ctoolclass.h>
#include <qfileinfo.h>


ProjectSpace::ProjectSpace(QObject* parent,const char* name,QString file) : KDevComponent(parent,name){
  m_projects = new QList<Project>;
  if (file != ""){
    readConfig(file);
  }
}
ProjectSpace::~ProjectSpace(){
}

void ProjectSpace::addProject(QString file){
  
}
void ProjectSpace::addProject(Project* prj){
  m_projects->append (prj);
  m_current_project = prj;
}
void ProjectSpace::setCurrentProject(Project* prj){
  m_current_project = prj;
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
  kdDebug(9000)  << "ProjectSpace::generateDefaultFiles():" << endl << args << endl;
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

/** writes a NAME.kdevpsp and .NAME.kdevpsp
    NAME.kdevpsp contains options for all users, like cvs system
    .NAME.kdevpsp contains options from the local user:
*/
bool ProjectSpace::readConfig(QString abs_filename){
  kdDebug(9000) << "enter ProjectSpace::readConfig" << endl;
  QFileInfo file_info(abs_filename);
  m_path = file_info.dirPath();
  m_projectspace_file = abs_filename;
  // the "global" one
  kdDebug(9000)  << "project_space_filename:" << m_projectspace_file << endl;
  KSimpleConfig* config = new KSimpleConfig(m_projectspace_file);
  readGeneralConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;

  m_user_projectspace_file = m_path + "/." + m_name + ".kdevpsp";
  // the "local/user" on
  kdDebug(9000) << "user_projectspace_filename:" << m_user_projectspace_file << endl;
  config = new KSimpleConfig(m_user_projectspace_file);
  writeUserConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  return true;
}

bool ProjectSpace::readGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  m_name = config->readEntry("name","");
  m_path = config->readEntry("path","");
  m_plugin_name = config->readEntry("plugin_name","");
  

  // read all projects
  QStringList files = config->readListEntry("projectfiles");
  QStringList::Iterator it;
  QString filename;
  QFileInfo fileinfo;
  Project* prj;
  for(it = files.begin(); it != files.end(); ++it){
    fileinfo.setFile(*it);
    filename = fileinfo.fileName();
    prj = new Project(this,"",CToolClass::getAbsolutePath(m_path,fileinfo.dirPath()) + filename);
    addProject(prj);
  }
  return true;
}

bool ProjectSpace::readUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  return true;
}


bool ProjectSpace::writeConfig(){
  kdDebug(9000) << "enter ProjectSpace::writeConfig" << endl;
  // the "global" one
  QString filename = m_path + "/" + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename:" << filename << endl;
  KSimpleConfig* config = new KSimpleConfig(filename);
  writeGeneralConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;

  // write all projectfiles
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    prj->writeConfig();
  }
  
  // the "local/user" on
  filename = m_path + "/." + m_name + ".kdevpsp";
  kdDebug(9000) << "filename:" << filename << endl;
  config = new KSimpleConfig(filename);
  writeUserConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  
}

bool ProjectSpace::writeGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("name",m_name);
  config->writeEntry("path",m_path);
  config->writeEntry("plugin_name", m_plugin_name); // the projectspacetype name
  QStringList projectfiles;
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    // add the relative path
    QString file = prj->getProjectFile();
    projectfiles.append(CToolClass::getRelativePath(m_path,file));
  }
  config->writeEntry("projectfiles",projectfiles);
  return true;
}

bool ProjectSpace::writeUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("user", m_plugin_libraryname);
  return true;
}

