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
#include "misc.h"


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
    KMessageBox::error(0, i18n("KDevelop needs \"%1\" to work properly.\n\tPlease install it!").arg(name));
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


QString CToolClass::locatehtml(const QString &filename)
{

    QString path = locate("html", KGlobal::locale()->language() + '/' + filename);
    if (path.isNull())
       path = locate("html", "default/" + filename);
	cerr << path;
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

