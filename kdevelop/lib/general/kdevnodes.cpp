/***************************************************************************                                
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "kdevnodes.h"
#include <iostream.h>

KDevNode::KDevNode(QString projectSpaceName,QString projectName){
  m_projectSpaceName = projectSpaceName;
  m_projectName = projectName;
}
void KDevNode::setProjectSpaceName(QString projectSpaceName){
  m_projectSpaceName = projectSpaceName;
}
QString KDevNode::projectSpaceName(){
  return m_projectSpaceName;
}
void KDevNode::setProjectName(QString projectName){
  m_projectName = projectName;
}
QString KDevNode::projectName(){
  return m_projectName;
}
void KDevNode::show(){
  cerr << endl << "KDevNode: " << endl << "ProjectSpaceName:" << m_projectSpaceName;
  cerr << endl << "ProjectName:" << m_projectName;
}


KDevFileNode::KDevFileNode(QString absFileName,QString projectSpaceName,QString projectName)
 : KDevNode(projectSpaceName,projectName){
  m_absFileName = absFileName;
}
void KDevFileNode::setAbsoluteFileName(QString absFileName){
  m_absFileName = absFileName;
}
QString KDevFileNode::absoluteFileName(){
  return m_absFileName;
}
void KDevFileNode::show(){
  KDevNode::show();
  cerr << endl << "FileName:" << m_absFileName;
}


KDevDirectoryNode::KDevDirectoryNode(QString dir,QString projectSpaceName,QString projectName)
 : KDevNode(projectSpaceName,projectName){
  m_dir = dir;
}

void KDevDirectoryNode::setDir(QString dir){
  m_dir = dir;
}
QString KDevDirectoryNode::dir(){
  return m_dir;
}
void KDevDirectoryNode::show(){
  KDevNode::show();
  cerr << endl << "Directory:" << m_dir;
}

