/***************************************************************************
                          automakeprojectspace.cpp  -  description
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

#include "automakeprojectspace.h"
#include <qfile.h>
#include <qtextstream.h>
#include <iostream.h>

AutomakeProjectSpace::AutomakeProjectSpace(QObject* parent,const char* name)
  : ProjectSpace(parent,name){
}
AutomakeProjectSpace::~AutomakeProjectSpace(){
}
void AutomakeProjectSpace::setupGUI(){
  ProjectSpace::setupGUI();
}
void AutomakeProjectSpace::modifyDefaultFiles(){
  ProjectSpace::modifyDefaultFiles();
  QFile file(m_path + "/configure.in");
  if ( file.open(IO_ReadOnly) ) {    // file opened successfully
    QTextStream t( &file );        // use a text stream
    QString text = t.read();
    file.close();
    setInfosInString(text);
    // save
    if ( file.open(IO_WriteOnly) ){
      QTextStream tw( &file );
      tw << text;
      file.close();
    }
    else {
      cerr << "\nERROR! couldn't open file to write:" << file.name();
    }
  }
  else {
    cerr << "\nERROR! couldn't open file to read:" << file.name();
  }
}

/** Store the additional arguments for configure,stored in the *_user files */
void AutomakeProjectSpace::setConfigureArgs(QString args){
  m_configureArgs = args;
}

/** Fetch the commandline additional arguments for configure */
QString AutomakeProjectSpace::configureArgs(){
  return m_configureArgs;
}

  /** add the data to the psElement (Projectspace)*/
bool AutomakeProjectSpace::writeGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  ProjectSpace::writeGlobalConfig(doc,psElement);
  return true;
}
bool AutomakeProjectSpace::writeUserConfig(QDomDocument& doc,QDomElement& psElement){
  ProjectSpace::writeUserConfig(doc,psElement);
  return true;
}

bool AutomakeProjectSpace::readGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  ProjectSpace::readGlobalConfig(doc,psElement);
  return true;
}
bool AutomakeProjectSpace::readUserConfig(QDomDocument& doc,QDomElement& psElement){
  ProjectSpace::readUserConfig(doc,psElement);
  return true;
}


#include "automakeprojectspace.moc"
