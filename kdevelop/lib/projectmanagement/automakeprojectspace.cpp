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
}

/** Fetch the commandline additional arguments for configure */
QString AutomakeProjectSpace::getConfigureArgs(){
}

bool AutomakeProjectSpace::readGeneralConfig(KSimpleConfig* config){
  config->setGroup("AutomakeProjectSpace Config");
  
  ProjectSpace::readGeneralConfig(config);
}

bool AutomakeProjectSpace::readUserConfig(KSimpleConfig* config){
  config->setGroup("AutomakeProjectSpace Config");
  
  ProjectSpace::readUserConfig(config);
}

bool AutomakeProjectSpace::writeGeneralConfig(KSimpleConfig* config){
  config->setGroup("AutomakeProjectSpace Config");
  config->writeEntry("test_general","value");
  
  ProjectSpace::writeGeneralConfig(config);
}

bool AutomakeProjectSpace::writeUserConfig(KSimpleConfig* config){
  config->setGroup("AutomakeProjectSpace Config");
  config->writeEntry("test_user","value");
  
  ProjectSpace::writeUserConfig(config);
}

#include "automakeprojectspace.moc"
