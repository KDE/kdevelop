/***************************************************************************
                 ctoolclass.cpp - some methods,that would be normal global,but I
		              hate global function :-)

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
#include <kapp.h>

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
    KMsgBox::message(0,i18n("Program not found!"),i18n("KDevelop needs \"")+name+i18n("\" to work properly.\n\tPlease install it!"),KMsgBox::EXCLAMATION);
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

QString CToolClass::findProgram(QString name){
  StringTokenizer tokener;
  QString file;
  QString complete_path = getenv("PATH");

  tokener.tokenize(complete_path,":");

  while(tokener.hasMoreTokens()){
    file = QString(tokener.nextToken()) + "/" + name;
    if(QFile::exists(file)){
      return file;
    }
  }
  return "";
}

QString CToolClass::getRelativePath(QString source_dir,QString dest_dir){
  // a special case , the dir are equals
  if (source_dir == dest_dir){
    return "./";
  }
  dest_dir.remove(0,1); // remove beginning /
  source_dir.remove(0,1); // remove beginning /
  bool found = true;
  int slash_pos=0;


  do {
    slash_pos = dest_dir.find('/');
    if (dest_dir.left(slash_pos) == source_dir.left(slash_pos)){
      dest_dir.remove(0,slash_pos+1);
      source_dir.remove(0,slash_pos+1);
    }
    else {
      found = false;
    }
  }
  while(found == true);

  int slashes = source_dir.contains('/');
  int i;
  for(i=0;i < slashes;i++){
    dest_dir.prepend("../");
  }

  return dest_dir;
}
