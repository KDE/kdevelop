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

#include <iostream.h>
#include <stdlib.h>
#include <qfile.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstddirs.h>
#include "ctoolclass.h"
#include <kdebug.h>
#include <qdir.h>


bool CToolClass::searchProgram(QString name){
  bool found=false;
  QString file;
  QString path = getenv("PATH");

  QStringList pathlist = QStringList::split(path, ":");

  QStringList::ConstIterator it;
  for (it = pathlist.begin(); it != pathlist.end(); ++it) {
    file = (*it) + "/" + name;
    if(QFile::exists(file)){
      found = true;
      break;
    }
  }
  
  if(!found){
    KMessageBox::error(0, i18n("KDevelop needs \"%1\" to work properly.\n\tPlease install it!").arg(name));
  }
  return found;
}


// this is for the installation process
bool CToolClass::searchInstProgram(QString name){
  bool found=false;
  QString file;
  QString path = getenv("PATH");

  QStringList pathlist = QStringList::split(path, ":");

  QStringList::ConstIterator it;
  for (it = pathlist.begin(); it != pathlist.end(); ++it) {
    file = (*it) + "/" + name;
    if(QFile::exists(file)){
      found = true;
      break;
    }
  }

  return found;
}


QString CToolClass::findProgram(QString name){
  QString file;
  QString path = getenv("PATH");

  QStringList pathlist = QStringList::split(path, ":");

  QStringList::ConstIterator it;
  for (it = pathlist.begin(); it != pathlist.end(); ++it) {
    file = (*it) + "/" + name;
    if(QFile::exists(file)){
      return file;
      break;
    }
  }

  return "";
}


QString CToolClass::locatehtml(const QString &filename)
{

    QString path = locate("html", KGlobal::locale()->language() + '/' + filename);
    if (path.isNull())
       path = locate("html", "default/" + filename);

    return path;
}


/*---------------------------------- CToolClass::escapetext()
 * escapetext()
 *    converts a string to a regular expression.
 *    you will need the function if you want to search even this text.
 *    chars which are special chars in a regular expression are escaped with '\\'
 *
 * Parameters:
 *   szOldText   the search text you want to convert
 *   bForGrep    set this true if you will change the special chars from grep
 *
 * Returns:
 *   a string object with the escaped string
 *-----------------------------------------------------------------*/
QString CToolClass::escapetext(const char *szOldText, bool bForGrep)
{
  QString sRegExpString="";
  char ch;
  int i;
  bool bFound;
  char *szChangingChars= (bForGrep) ? (char*)"[]\\^$" : (char*)"$^*[]|()\\;,#<>-.~{}" ;

  if (szOldText)
  {
    while ((ch=*szOldText++)!='\0')
     {
       bFound=false;
       for (i=0; !bFound && szChangingChars[i]!='\0';)
      {
         if (szChangingChars[i]==ch)
             bFound=true;
         else
             i++;
      }
      if (bFound)
          sRegExpString+='\\';
      sRegExpString+=ch;
    }
  }
  return sRegExpString;
}

QString CToolClass::getRelativePath(QString source_dir,QString dest_dir){
  kdDebug(9000) << "source_dir:" << source_dir <<endl;
  kdDebug(9000) << "dest_dir:" << dest_dir <<endl;

  // a special case , the dir are equals
  if (source_dir == dest_dir){
    kdDebug(9000) << "rel_dir:./" <<endl;
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

  kdDebug(9000) << "rel_dir:" << dest_dir <<endl;
  return dest_dir;
}

QString CToolClass::getAbsolutePath(QString source_dir, QString rel_path){
  QDir dir(source_dir);
  if(!dir.cd(rel_path)){
    kdDebug(9000) << "Error in CToolClass::getAbsolutePath, directory doesn't exists" << endl;
    return "";
  }
  return dir.absPath() + "/";
}
QString CToolClass::getAbsoluteFile(QString sourceDir, QString relFile){
   QDir dir(sourceDir);
   QFileInfo fileInfo(relFile);
   QString dirPath = fileInfo.dirPath();
   if(!dir.cd(dirPath)){
    kdDebug(9000) << "Error in CToolClass::getAbsoluteFile, directory doesn't exists" << endl;
    return "";
  }
   return dir.absPath() + "/" + fileInfo.fileName();
}
