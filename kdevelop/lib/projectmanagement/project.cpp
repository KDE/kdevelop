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
#include <ctoolclass.h>
#include <qfileinfo.h>

Project::Project(QObject * parent, const char* name) :  QObjectPlugin(parent,name){
  m_files = new QList<RegisteredFile>();
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
QString Project::relativePath(){
  return m_relPath;
}
void Project::setRelativePath(QString path){
  m_relPath = path;
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
void Project::addFile(RegisteredFile* file){
  m_files->append(file);
}

void Project::addFile(QString abs_filename){
  QString rel_file = CToolClass::getRelativePath(m_abs_path,abs_filename);
  RegisteredFile* file = new RegisteredFile(rel_file);
  m_files->append(file);
}


void Project::removeFile(RegisteredFile* file){
  //	  m_files->remove(file);
}
void Project::showAllFiles(){
  RegisteredFile* file;
  cerr << endl << "show all registered Files for: " << m_name;
  for(file = m_files->first(); file != 0;file =  m_files->next() ){
    cerr << "\nFilename:" << file->getRelativeFile();
  }
}

void Project::dump(){
  cerr << endl << "Project Name: " << m_name;
  cerr << endl << "relative Path: " << m_relPath;
  cerr << endl << "absolute Path: " << m_abs_path;
  cerr << endl << "Version: " << m_version;
  showAllFiles();
}


bool Project::readUserConfig(QDomDocument& doc,QDomElement& projectElement){
  QDomElement filesElement = projectElement.namedItem("projecttest").toElement();
  if(filesElement.isNull()){
    cerr << "\nProject::readUserConfig no \"projecttest\" tag found!";
    return false;
  }
  return true;
}



bool Project::writeGlobalConfig(QDomDocument& doc,QDomElement& projectElement){
  cerr << "\nenter Project::writeGlobalConfig";
  projectElement.setAttribute("name",m_name);
  projectElement.setAttribute("pluginName",m_plugin_name);
  projectElement.setAttribute("relativePath",m_relPath);

  // Files tag
  QDomElement filesElement = projectElement.appendChild(doc.createElement("Files")).toElement();
  QStringList fileList;
  RegisteredFile* pFile;
  for(pFile = m_files->first(); pFile != 0; pFile= m_files->next() ){
    QDomElement fileElement = filesElement.appendChild(doc.createElement("File")).toElement();
    pFile->writeConfig(doc,fileElement);
  }
  return true;
  
}
bool Project::writeUserConfig(QDomDocument& doc,QDomElement& projectElement){
  QDomElement openFilesElement = projectElement.appendChild(doc.createElement("projecttest")).toElement();
  return true;
}

bool Project::readGlobalConfig(QDomDocument& doc,QDomElement& projectElement){
  m_name = projectElement.attribute("name");
  m_relPath =  projectElement.attribute("relativePath");

  QDomElement filesElement = projectElement.namedItem("Files").toElement();
  if(filesElement.isNull()){
    cerr << "\nProjectSpace::readGlobalConfig no \"Files\" tag found!";
    return false;
  }

  RegisteredFile* file;
  QDomNodeList fileList = filesElement.elementsByTagName("File");
  unsigned int i;
  for (i = 0; i < fileList.count(); ++i){
    QDomElement fileElement = fileList.item(i).toElement();
    file = new RegisteredFile();
    file->readConfig(fileElement);
    m_files->append(file);
  }
  //  showAllFiles();
  return true;
}

#include "project.moc"
