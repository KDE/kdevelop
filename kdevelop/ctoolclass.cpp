/***************************************************************************
                 ctoolclass.cpp - some methods,that would be normal global,but I
		              hate global function :-)
                             -------------------    

    version              :                                   
    begin                : 20 Jan 1999                        
    copyright            : (C) 1999 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                         
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "ctoolclass.h"
#include <iostream.h>
#include <qfile.h>
#include <kmsgbox.h>

bool CToolClass::searchProgram(QString name){
  StringTokenizer tokener;
  bool found=false;
  QString file;
  QString complete_path = getenv("PATH");
  
  tokener.tokenize(complete_path,":");
  
  while(tokener.hasMoreTokens()){
    file = QString(tokener.nextToken()) + "/" + name;
    if(QFile::exists(file)){
      found = true;
      break;
    }
  }
  if(!found){
    KMsgBox::message(0,"Program not found!","KDevelop needs \""+name+"\" to work properly.\n\tPlease install it!",KMsgBox::EXCLAMATION);
  }
  return found;
}

// this is for the installation process
bool CToolClass::searchInstProgram(QString name){
  StringTokenizer tokener;
  bool found=false;
  QString file;
  QString complete_path = getenv("PATH");

  tokener.tokenize(complete_path,":");

  while(tokener.hasMoreTokens()){
    file = QString(tokener.nextToken()) + "/" + name;
    if(QFile::exists(file)){
      found = true;
      break;
    }
  }
  return found;
}

