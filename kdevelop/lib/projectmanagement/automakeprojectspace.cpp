/***************************************************************************
                          automakeprojectspace.cpp  -  description
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

#include "automakeprojectspace.h"

AutomakeProjectSpace::AutomakeProjectSpace(QObject* parent,const char* name)
  : ProjectSpace(parent,name){
}
AutomakeProjectSpace::~AutomakeProjectSpace(){
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
