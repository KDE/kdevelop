/***************************************************************************
                          registeredfile.cpp  -  description
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

#include "registeredfile.h"
#include <iostream.h>


RegisteredFile::RegisteredFile(QString relName,bool dist,bool install,QString installFile){
  m_file = relName;
  m_dist= dist;
  m_install = install;
  m_installFile = installFile;
}
RegisteredFile::~RegisteredFile(){
}

void RegisteredFile::setRelativeFile(QString relName){
  m_file = relName;
}

QString RegisteredFile::relativeFile(){
  return m_file;
}

void RegisteredFile::setInstallFile(QString absFile){
  m_installFile = absFile;
}
QString RegisteredFile::getInstallFile(){
  return m_installFile;
}

void RegisteredFile::setToDist(bool yes){
  m_dist = yes;
}
bool RegisteredFile::isInDist(){
  return m_dist;
}

void RegisteredFile::setInstall(bool yes){
  m_install = yes;
}
bool RegisteredFile::shouldBeInstalled(){
  return m_install;
}

void RegisteredFile::writeConfig(QDomDocument& doc, QDomElement& fileElement){
   cerr << "\nenter RegisteredFile::writeConfig";
   fileElement.setAttribute("file",m_file);
   if(m_dist) {
     fileElement.setAttribute("dist","true");
   }
   else{
     fileElement.setAttribute("dist","false");
   }
   if(m_install){
     fileElement.setAttribute("install","true");
   }
   else{
     fileElement.setAttribute("install","false");
   }
  fileElement.setAttribute("installFile",m_installFile);
}
void RegisteredFile::readConfig(QDomElement& fileElement){
  QString dist = fileElement.attribute("dist");
  if(dist == "true"){
    m_dist = true;
  }
  else{
    m_dist = false;
  }
  QString install = fileElement.attribute("install");
  if(install == "true"){
    m_install = true;
  }
  else{
    m_install = false;
  }
  m_file = fileElement.attribute("file");
  m_installFile = fileElement.attribute("installFile");
  
}
