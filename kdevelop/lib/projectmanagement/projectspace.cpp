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

QString ProjectSpace::getRelativePath(QString source_dir,QString dest_dir){
  kdDebug(9000) << "source_dir:" << source_dir <<endl;
  kdDebug(9000) << "dest_dir:" << dest_dir <<endl;

  // a special case , the dir are equals
  if (source_dir == dest_dir){
    kdDebug(9000) << "rel_dir:./" <<endl;
    return "./";
  }
  dest_dir.remove(0,1); // remove beginning /
  source_dir.remove(0,1); // remove beginning /
  bool found = true;
  int slash_pos=0;
  

  do {
    slash_pos = dest_dir.find('/');
    if (dest_dir.left(slash_pos) == source_dir.left(slash_pos)){
      dest_dir.remove(0,slash_pos+1);
      source_dir.remove(0,slash_pos+1);
    }
    else {
      found = false;
    }
  }
  while(found == true);

  int slashes = source_dir.contains('/');
  int i;
  for(i=0;i < slashes;i++){
    dest_dir.prepend("../");
  }

  kdDebug(9000) << "rel_dir:" << dest_dir <<endl;
  return dest_dir;
}

QString ProjectSpace::getAbsolutePath(QString source_dir, QString rel_path){
  QDir dir(source_dir);
  if(!dir.cd(rel_path)){
    kdDebug(9000) << "Error in ProjectSpace::getAbsolutePath, directory doesn't exists" << endl;
    return "";
  }
  return dir.absPath() + "/";
}
