/***************************************************************************
                          project.cpp  -  description
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

#include "project.h"
#include <iostream.h>
#include "registeredfile.h"
#include <kdebug.h>

Project::Project(QObject * parent, const char* name) :  QObjectPlugin(parent,name){
  m_files = new QValueList<RegisteredFile>();
}
Project::~Project(){
}
/*____some get methods_____ */

QString Project::getProjectFile(){
  return m_project_file;
}
QString Project::getUserProjectFile(){
  return m_user_project_file;
}
/** */
QStringList Project::getAllSources(){
}

/** returns all files*/
QStringList Project::getAllFileNames(){
}
RegisteredFile Project::getFileProperties(QString filename){
}
QString Project::getVersion(){
  return m_version;
}
QString Project::getName(){
  return m_name;
}
QString Project::getAbsolutePath(){
  return m_abs_path;
}

//////////////////////////////// some set methods//////////////////////

/** store the project version. */
void Project::setVersion(QString version){
  m_version = version;
}
void Project::setName(QString name){
  m_name = name;
}
void Project::setAbsolutePath(QString path){
  m_abs_path = path;
  m_project_file = m_abs_path + "/" + m_name + ".kdevprj2";
  m_user_project_file = m_abs_path + "/." + m_name + ".kdevprj2";
}
/** generate/modifiy the Makefile*/
void Project::updateMakefile(){
}
void Project::registerFile(RegisteredFile file){
  m_files->append(RegisteredFile(file));
}
void Project::unRegisterFile(RegisteredFile file){
  //	  m_files->remove(file);
}
void Project::showAllFiles(){
  QValueList<RegisteredFile>::Iterator it;
  cerr << "show all registered Files for" << m_name << "\n";
  for(it = m_files->begin(); it != m_files->end(); ++it )
    cerr << "\nFilename:" << (*it).getFilename() << "\n";
}
bool Project::readConfig(QString filename){
  m_project_file = filename;
  return true;
}
bool Project::readGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  return true;
}
bool Project::readUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  return true;
}


bool Project::writeConfig(){
  kdDebug(9000) << "enter Project::writeConfig "  << endl;
  // the "global" one
  kdDebug(9000)  << "project_filename:" << m_project_file << endl;
  KSimpleConfig* config = new KSimpleConfig(m_project_file);
  writeGeneralConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  
  // the "local/user" one
  kdDebug(9000)  << "project_filename:" << m_user_project_file << endl;
  config = new KSimpleConfig(m_user_project_file);
  writeUserConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  
}
bool Project::writeGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("name",m_name);
  config->writeEntry("plugin_name", m_plugin_name); // the projectspacetype name
  QStringList file_list;
  QValueList<RegisteredFile>::Iterator it;
  for(it = m_files->begin(); it != m_files->end(); ++it ){
    file_list.append((*it).getFilename());
  }
  config->writeEntry("files",file_list);
  return true;
}

bool Project::writeUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("test","test");
  return true;
}

