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


