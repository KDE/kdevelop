/***************************************************************************
                          project.cpp  -  description
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

#include "project.h"
#include <iostream.h>

Project::Project(QObject * parent, const char* name) :  QObjectPlugin(parent,name){
  m_files = new QValueList<RegisteredFile>();
}
Project::~Project(){
}
/*____some get methods_____ */

QString Project::getProjectFile(){
  return m_abs_path + "/" + m_name + ".kdevprj2";
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

/** store the project version. */
  void Project::setVersion(QString version){
    m_version = version;
  }

void Project::setName(QString name){
  m_name = name;
}
QString Project::getName(){
  return m_name;
}

void Project::setAbsolutePath(QString path){
  m_abs_path = path;
}
QString Project::getAbsolutePath(){
  return m_abs_path;
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
    cerr << "\nFilename:" << (*it).filename << "\n";
}

/** read the projectfile */
bool Project::readConfig(){
}
/** write the projectfile to the disk or db*/
bool Project::writeConfig(){
}
