/***************************************************************************
                          ckdevelProject.cpp  -  description
                             -------------------
    begin                : Sun Oct 10 1999
    copyright            : (C) 1999 by ian geiser
    email                : geiseri@msoe.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cdevproject.h"
#include <qfile.h>
#include <qregexp.h>
#include <qstring.h>
#include <iostream.h>
#include <qtextstream.h>

ckdevelProject::ckdevelProject(){
}
ckdevelProject::~ckdevelProject(){
}

/** This will load the selected project file and populate the internal data structure. */
bool ckdevelProject::loadProject(QString qsFileName){
  bool bGoodFile = false;
  bool bFlag = false; // States when the [General] tag has been found
  QRegExp regexAuthor( "^author=*", TRUE, FALSE );
  QRegExp regexEmail("^email=*", TRUE, FALSE);
  QRegExp regexInfo("^short_info=*", TRUE, FALSE);
  QRegExp regexProject("^project_name=*", TRUE, FALSE);
  QRegExp regexVersion("^version=*", TRUE, FALSE);
  QRegExp regexConfig("^configure_args=*", TRUE, FALSE);
  QRegExp regexGeneral("[General]", TRUE, FALSE);
  QString newLine;
  QFile newProject( qsFileName );
  QTextStream t(& newProject);
  if (newProject.open( IO_ReadOnly ))
  while( !t.eof())
       {
            newLine = t.readLine();
            if (newLine.contains(regexAuthor) == 1)
              qsAuthor = newLine.mid(7, newLine.length());
            else if (newLine.contains(regexEmail) == 1)
              qsEmailAddress = newLine.mid(6, newLine.length());
            else if (newLine.contains(regexInfo) == 1)
              qsInfo = newLine.mid(11, newLine.length());
            else if (newLine.contains(regexProject) == 1)
              qsProjectName = newLine.mid(13, newLine.length());
            else if (newLine.contains(regexVersion) == 1)
              qsVersion = newLine.mid(8, newLine.length());
            else if (newLine.contains(regexConfig) == 1)
              qsConfig = newLine.mid(15, newLine.length());
            else
              cout << "EXTRA: " << newLine << endl;
            bGoodFile = true;
       }
  else
    bGoodFile = false;
  newProject.close();
  return bGoodFile;
}

/** Return the version number of the current project. */
QString ckdevelProject::getVersion(){
  return qsVersion;
}

/** Returns the author & email of the current project.
It will be formatted as AuthorName <emailaddress> */
QString ckdevelProject::getAuthor(){
  QString tempAuthor;
  tempAuthor += qsAuthor;
  tempAuthor += " <";
  tempAuthor += qsEmailAddress;
  tempAuthor += ">";
  return tempAuthor;
}

/** Returns the short info from the current project. */
QString ckdevelProject::getInfo(){
/*
    We may have to clean up the info.
    Convert the following:
    \, -> ,
    ,  -> \n
*/
  QString retInfo = qsInfo;
  retInfo.replace( QRegExp("[^\\\\]\\,"), "\n");
  retInfo.replace( QRegExp("\\\\,"), "," );
  return retInfo;
}

/** Returns the current project name. */
QString ckdevelProject::getProjectName(){
  return qsProjectName;
}



/** The configure options */
QString ckdevelProject::getConfig(){
  return qsConfig;

}


