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


RegisteredFile::RegisteredFile(QString rel_name,bool dist,bool install,QString install_file){
  m_file = rel_name;
  m_dist= dist;
  m_install = install;
  m_install_file = install_file;
}
RegisteredFile::~RegisteredFile(){
}

void RegisteredFile::setRelativeFile(QString rel_name){
  m_file = rel_name;
}

QString RegisteredFile::getRelativeFile(){
  return m_file;
}

void RegisteredFile::setInstallFile(QString abs_file){
  m_install_file = abs_file;
}
QString RegisteredFile::getInstallFile(){
  return m_install_file;
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


void RegisteredFile::writeConfig(KConfig* config){
  config->setGroup(m_file);
  config->writeEntry("dist",m_dist);
  config->writeEntry("install",m_install);
  config->writeEntry("install_file",m_install_file);
}
void RegisteredFile::readConfig(KConfig* config){
  config->setGroup(m_file);
  m_dist = config->readBoolEntry("dist",false);
  m_install = config->readBoolEntry("install",false);
  m_install_file = config->readEntry("install_file","");
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
  fileElement.setAttribute("installFile",m_install_file);
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
  m_install_file = fileElement.attribute("installFile");
  
}
