/***************************************************************************
                                                  main.cpp  -  description
                                                        -------------------
      begin                              : Fri Jun 18 13:52:43 CEST 1999

      copyright                      : (C) 1999 by Martin Piskernig
      email                              : martin.piskernig@stuwo.at
 ***************************************************************************/

/***************************************************************************
 *                                                                                                                                                *
 *    This program is free software; you can redistribute it and/or modify  *
 *    it under the terms of the GNU General Public License as published by  *
 *    the Free Software Foundation; either version 2 of the License, or        *
 *    (at your option) any later version.                                                                    *
 *                                                                                                                                                *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qregexp.h>
#include <qstrlist.h>

#if _QT_VERSION < 2
#define QCString         QString
#endif

QString getValues(QFile &file, const QString &sKeyword)
{
  QString sResult, sLine;
  bool bFound=false, bContinues=false;

  file.at(0);
  QTextStream tstream(&file);

  while(!tstream.eof() && !bFound)
  {
      sLine=tstream.readLine();
      sLine=sLine.simplifyWhiteSpace();  // strip whitespaces an simplify it
      if (sLine.find(sKeyword)==0)    // accept only if keyword is now at first position
      {
          bFound=true;
          // take the part w/o the keyword
          sResult=sLine.mid(sKeyword.length(),sLine.length());
          // check the escape char
          if (sResult.right(1)==QCString("\\"))
          {
                bContinues=true;
                sResult=sResult.left(sResult.length()-1);
          }
      }
  }

  while(!tstream.eof() && bContinues)
  {
      sLine=tstream.readLine();
      sLine=sLine.simplifyWhiteSpace();  // strip whitespaces an simplify it

      sResult+=' ';
      sResult+=sLine;

      // check the escape char
      if (sResult.right(1)==QCString("\\"))
      {
            sResult=sResult.left(sResult.length()-1);
      }
      else
            bContinues=false;

  }

  sResult=sResult.simplifyWhiteSpace();  // strip and simplify whitespaces
  return sResult;
}

QStrList makeStringList(const QString &str)
{
  QStrList slResult;
  QString sTemp = str;
  QString sTheRealPart;

  int pos;
  sTemp=sTemp.simplifyWhiteSpace();

  while (!sTemp.isEmpty())
  {
      if ((pos=sTemp.find(' '))!=-1)
      {
          sTheRealPart=sTemp.left(pos);
          sTemp=sTemp.mid(pos+1, sTemp.length());
      }
      else
      {
          sTheRealPart=sTemp;
          sTemp="";
      }

      sTheRealPart=sTheRealPart.stripWhiteSpace();  // make it a little bit secure ;-)
      if (!sTheRealPart.isEmpty())
          slResult.append(sTheRealPart);

  }

  return slResult;
}

void outputInfo(QFile &srcfile, const QString &sName, QFile &destfile)
{
  QString sKeyword,sEntry;
  QStrList sEntryList;


  cout << "---" << endl;
  cout << "Name of project: " << sName << endl;

  sKeyword=QCString("_SOURCES = ");
  sEntry=getValues(srcfile, sName+sKeyword);
  sEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "  sources: " << sEntry <<endl;

  /////////////
  sKeyword=QCString("_METASOURCES = ");
  sEntry=getValues(srcfile, sName+sKeyword);
  sEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "  metasources: " << sEntry <<endl;

  /////////////
  sKeyword=QCString("_LDADD = ");
  sEntry=getValues(srcfile, sName+sKeyword);
  sEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "  ldadd: " << sEntry <<endl;

  /////////////
  sKeyword=QCString("_INCLUDES = ");
  sEntry=getValues(srcfile, sName+sKeyword);
  sEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "  includes: " << sEntry <<endl;

  sKeyword=QCString("APPSDIR = ");
  sEntry=getValues(srcfile, sKeyword);
  sEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "  includes: " << sEntry <<endl;

  cout << "---" << endl;

}

int analyze(QFile &file, const QString &dirstr)
{
 QString sMakefilename, sLine, sEntry;

 QString sSubDirs, sProgs;
 QStrList slSubDirList, slProgList;

 // Construct the Makefile.am string

 sMakefilename =  QCString("/Makefile.am");
 sMakefilename = dirstr + sMakefilename;

 QFile makefile(sMakefilename);

 // If no Makefile.am exists, return to caller
 // Return 1 if error
 // Return 0 if everything's ok

 if(!makefile.exists())
  {
    cerr << "Makefile.am does not exist in directory '" << dirstr << "'." << endl;
    return(1);
  }

 if(!makefile.open(IO_ReadOnly))
  {
    cerr << "Cannot open Makefile.am of '" << dirstr << "'." << endl;
    return(1);
  }

 // Print out directory we are entering

 cout << "Entering directory '" << dirstr << "'" << endl;

 sSubDirs=getValues(makefile, QCString("SUBDIRS = "));
 slSubDirList=makeStringList(sSubDirs);
 // Get subdirs

 sProgs=getValues(makefile, QCString("bin_PROGRAMS = "));
 slProgList=makeStringList(sProgs);


 sEntry=slProgList.first();
 while (!sEntry.isNull())
 {
    if (!sEntry.isEmpty())
    {
        outputInfo(makefile, sEntry, file);
    }
    sEntry=slProgList.next();
 }


 // Close open Makefile.am
 makefile.close();

 // Recursive calls to subdirs
 sEntry=slSubDirList.first();
 while (!sEntry.isNull())
 {
    if (!sEntry.isEmpty())
    {
        analyze(file, dirstr+QCString("/")+sEntry);
    }
    sEntry=slSubDirList.next();
 }

 return(0);
}


////////////////// here is main()
int main(int argc, char** argv)
{
 QDir oldDir;    // get actual dir to restore
 QDir newDir;
 QFile prjFile;
 QString sPrjdir, sFilename;

 if(argc<2)
  {
    cout << "Experimental convert of KDevelop projects" << endl;
    cout << "Usage: import projectdir kdevprjfile" << endl;
    exit(1);
  }

 sPrjdir=QCString(argv[1]);
 sFilename=QCString(argv[2]);
 // Directory already exists?

 newDir.setPath(sPrjdir);
 if(!newDir.exists())
  {
    cerr << "Directory " << sPrjdir << " does not exist!" << endl;
    exit(1);
  }

 // File already exists?

 prjFile.setName(sFilename);
 if(prjFile.exists())
  {
    char c;
    cerr << "File " << sFilename << " already exists!" << endl;
    cerr << "Do you want to overwrite the file? ";
    cin >> c;
    if(toupper(c)!='Y')
      exit(1);
  }

 if (!prjFile.open(IO_ReadWrite))
 {
    cerr << "Cannot open " << sFilename << "! (maybe not enough permissions)"
                 << endl;
    exit(1);
 }

 cout << endl;
 cout << "Analyzing project" << endl;
 cout << "-----------------" << endl << endl;

 // Recurse over SUBDIRS is done in analyze

 analyze(prjFile, sPrjdir);

 // Close open file
 prjFile.close();


 return EXIT_SUCCESS;
}