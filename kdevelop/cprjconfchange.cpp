/***************************************************************************
                          cprjconfchange.h  -  description
                             -------------------
    begin                : Wed Nov 21 2001
    copyright            : (C) 2001 by Walter Tasin
    email                : tasin@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cprjconfchange.h"

#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <kregexp.h>
#include <kdebug.h>
#include <iostream.h>

#define COMMENT				"^dnl[\\s]+"
#define COMMENTKDEV		".*[\\s]+dnl[\\s]+__kdevelop__[\\s]*$"
#define OPTIONSUFFIX	" dnl __kdevelop__"


CPrjConfChange::CPrjConfChange(const QString &confDir) : bChanged(false)
{
   setConfDir(confDir);
}

/* sets the directory in which configure.in(.in) should be found
    and read in the content of the file
    if no file is found the member "confFile" is empty.
*/
void CPrjConfChange::setConfDir(const QString &confDir)
{
  QFileInfo cdir(confDir);
  dir=QString::null;
  bChanged=false;
  
  if (!confDir.isEmpty() && cdir.isDir())
  {
    dir=cdir.absFilePath();
    searchConfFile();
    readConfFile();
  }

}

void CPrjConfChange::searchConfFile()
{
  QFileInfo file;
  QDir confDir;
  confFile=QString::null;
  if (!dir.isEmpty())
  {
    confDir.setPath(dir);
    file.setFile(confDir, "configure.in.in");
    if (file.exists())
    {
      confFile=file.absFilePath();
    }
    else
    {
      file.setFile(confDir, "configure.in");
      if (file.exists())
        confFile=file.absFilePath();
    }
  }
}

bool CPrjConfChange::readConfFile()
{
  bool bOk=false;
  if (!confFile.isEmpty())
  {
    QFile file(confFile);
    QTextStream stream(&file);
    QString fullLine;
    QRegExp exp("\\\\[\\s]*$");
    int pos;
    
    cerr << "-- read " << confFile.latin1() << "--" << endl;
    
    content.clear();
    if(file.open(IO_ReadOnly))
    { 
  
      while(!stream.eof())
      {
        QString line=stream.readLine();
        pos=line.find(exp);
        if (pos==-1)
        {
          fullLine+=line;
          cerr << fullLine.latin1() << endl;
          content.append(fullLine);
          fullLine="";
        }  
        else
        {
          // check for \\-escape char
          fullLine+=line.left(pos) + "\n";
        }
      }
      file.close();
      if (!fullLine.isEmpty())
      {
        cerr << fullLine.latin1() << endl;
        content.append(fullLine);
      }
      
      cerr << endl << "--" << endl;  
      bOk=true;
    }
  }
  return bOk;
}

/* writes out the confFile
   this file always ends up with a "\n"
*/
bool CPrjConfChange::writeConfFile()
{
  bool bOk=false;
  if (!confFile.isEmpty() && bChanged)
  {
    QFile file(confFile);
    QTextStream stream(&file);
    QString fullLine;
    
    cerr << "-- write " << confFile.latin1() << "--" << endl;
    
    if(file.open(IO_WriteOnly))
    { 
      for (QStringList::Iterator it = content.begin(); it != content.end(); ++it)
      {
         fullLine=*it;
         fullLine.replace(QRegExp("[\\n]"),"\\\n");
         
         stream << fullLine; 
         cerr << fullLine.latin1();
       
         stream << "\n";
         cerr << endl;
      } 
      file.close();
      bOk=true;
      bChanged=false;
    }
  }
  return bOk;
}

/* This method checks if the desired option is
   available to change inside configure.in(.in)
   e.g. isOptionAvailable("CXXFLAGS=");
   
   - this search is case sensitive
   - remember: the passed string is a regexp!!
*/
bool CPrjConfChange::isOptionAvailable(const QString &option) const
{
  bool bAvail=false;
  QString fullLine;
  QString disabledOption(COMMENT), enabledOption(option);
  
  // prepare the regexps
  disabledOption+=option+QString(COMMENTKDEV);
  enabledOption+=QString(COMMENTKDEV);
  
  for (QStringList::ConstIterator it = content.begin(); it != content.end() && !bAvail; ++it)
  {
    fullLine=*it;
    if (fullLine.find(QRegExp(disabledOption))!=-1)
      bAvail=true;
    if (fullLine.find(QRegExp(enabledOption))!=-1)
      bAvail=true;
      
  }  
  return bAvail;
}

bool CPrjConfChange::setOption(const QString &option, const QString &value) 
{
  bool bFound=false;
  QString fullLine;
  QString disabledOption(COMMENT), enabledOption(option);
  
  // prepare the regexps
  disabledOption+=option+QString(COMMENTKDEV);
  enabledOption+=QString(COMMENTKDEV);
  
  for (QStringList::Iterator it = content.begin(); it != content.end() && !bFound; ++it)
  {
    fullLine=*it;
    if (fullLine.find(QRegExp(disabledOption))!=-1)
      bFound=true;
    if (fullLine.find(QRegExp(enabledOption))!=-1)
      bFound=true;
    
    if (bFound)
    {
      (*it)=option+value+OPTIONSUFFIX;  
      bChanged=true;
    }
  }  
  return bFound;
}

/*
   check if the required option is enabled
*/
bool CPrjConfChange::isOptionEnabled(const QString &option) const
{
  bool bEnabled=false;
  QString fullLine;
  QString enabledOption("^[\\s]*");
  
  // prepare the regexp
  enabledOption+=option+QString(COMMENTKDEV);
  
  for (QStringList::ConstIterator it = content.begin(); it != content.end() && !bEnabled; ++it)
  {
    fullLine=*it;
    if (fullLine.find(QRegExp(enabledOption))!=-1)
      bEnabled=true;
      
  }  
  return bEnabled;
}


// returns the complete configure.in(.in) as a QStringList 
QStringList CPrjConfChange::getConfContent()
{
  return content;
}

// sets the complete configure.in.in content (for future use)
void CPrjConfChange::setConfContent(const QStringList &_content)
{
  if (!confFile.isEmpty())
  {
    content=_content;
    bChanged=true;
  }
}

CPrjConfChange::~CPrjConfChange()
{

}
