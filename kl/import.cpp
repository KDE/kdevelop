/***************************************************************************
                          main.cpp  -  description                              
                             -------------------                                         
    begin                : Fri Jun 18 13:52:43 CEST 1999
                                           
    copyright            : (C) 1999 by Martin Piskernig                         
    email                : martin.piskernig@stuwo.at                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <stdlib.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qregexp.h>
#include <qstrlist.h>

QFile file;
QDir ddir;
int pos;
QString subdirs,appsdir,sources,includes,metasources,name,extradist,noinstheaders,ldadd,toplevel;
QTextStream ts;
QString str;

int analyze(QString dirstr);
QString getString(QString str);
QStrList splitString(QString str);

int main(int argc, char** argv)
{
 if(argc<2)
  {
   cout << "Experimental convert of KDevelop projects" << endl;
   cout << "Usage: import projectdir kdevprjfile" << endl;
   exit(1);
  }

 // Directory already exists?

 ddir.setPath(argv[1]);
 if(!ddir.exists())
  {
   cout << "Directory " << argv[1] << " does not exist!" << endl;
   exit(1);
  }

 // File already exists?

 file.setName(argv[2]);
 if(file.exists())
  {
   char c;
   cout << "File " << argv[2] << " already exists!" << endl;
   cout << "Do you want to overwrite the file? ";
   cin >> c;
   if(c=='y')
    file.open(IO_ReadWrite);
   else
    exit(1);
  }

 // Else open it read write

 else
  file.open(IO_ReadWrite);

 cout << endl;
 cout << "Analyzing project" << endl;
 cout << "-----------------" << endl << endl;

 // Recurse over SUBDIRS is done in analyze

 analyze(argv[1]);
 toplevel = argv[1];

 // Close open file

 file.close();
 return EXIT_SUCCESS;
}

int analyze(QString dirstr)
{
 QDir dir(dirstr);
 QString makefilestr;
 QString temp;
 // Construct the Makefile.am string
 
 makefilestr = dirstr;
 makefilestr += "/Makefile.am";
 QFile makefile(makefilestr);

 // If no Makefile.am exists, return to caller
 // Return 1 if error
 // Return 0 if everything alright

 if(!makefile.open(IO_ReadOnly))
  {
   cout << "Makefile.am does not exist in directory '" << dir.path() << "'." << endl;
   return(1);
  }

 // Print out directory we are entering

 cout << endl << "Entering directory '" << dirstr << "'" << endl;

 ts.setDevice(&makefile);
   
 // Get subdirs
 
 while(!ts.eof())
  {
   str = ts.readLine();
   if(str.find("SUBDIRS = ")!=-1)
    {
     subdirs = getString(str);
     
    }
  }

 // Reset textstream to position 0
 makefile.at(0);
 
 while(!ts.eof())
  {
   str = ts.readLine();
   if(str.find("bin_PROGRAMS = ")!=-1)
    name = getString(str);
   if(name.find(" ")!=-1)
    {
     cout << "Sorry, multiple targets not supported yet." << endl;
     exit(-1);
    }
  }

 makefile.at(0);

 // Get other information like source files, metasources, headers, ...
 while(!ts.eof())
  {
   str = ts.readLine();
   if(str.find(name+"_SOURCES = ")!=-1)
    sources = getString(str);
   else if(str.find(name+"_METASOURCES = ")!=-1)
    metasources = getString(str);
   else if(str.find(name+"_LDADD = ")!=-1)
    ldadd = getString(str);
   else if(str.find(name+"_INCLUDES = ")!=-1)
    includes = getString(str);
   else if(str.find("APPSDIR = ")!=-1)
    appsdir = getString(str);
  }
  
 // Print out collected information

 if(name)
  cout << "Name of project: " << name << endl;
 if(sources)
  cout << name << " source files: " << sources << endl;
 if(metasources)
  cout << name << " metasource files: " << metasources << endl;
 if(subdirs)
  cout << "Subdirs: " << subdirs << endl;
 if(includes)
  cout << "Includes: " << includes << endl;
 if(ldadd)
  cout << "Ldadd: " << ldadd << endl;
 if(appsdir)
  cout << "Appsdir: " << appsdir << endl;
 
 // Recursive calls to subdirs
/* QStrList list = splitString(subdirs);
 QString s = list.first();
 while(s)
  {
   dirstr+=toplevel;
   dir.cd(dirstr);
   analyze(s);
   s = list.next();
  };
*/

 // Close open Makefile.am

 makefile.close();
 return(0);
}

QString getString(QString str)
{
 QString temp;
 pos = str.find("= ");
 str.remove(0,pos+1);

 // Now strip first and last whitespace
 
 str = str.stripWhiteSpace();
 str = str.simplifyWhiteSpace();
 pos = str.find("\\");
 if(pos==-1);
 else
  do
   {
   
    // Read next line in temporary string
    
    QString temp = ts.readLine();
    
    // Strip white spaces in temp string
      
    temp.stripWhiteSpace();
      
    // Append temp to str
      
    str.append(temp);
      
    // Replace \ thru white space
      
    str.replace(QRegExp("\\"),"");
    str = str.stripWhiteSpace();
   }
  while(temp.find("\\")!=-1);
 str = str.stripWhiteSpace();
 str = str.simplifyWhiteSpace();

 // Finally, store in return string
  
 return(str);
}

QStrList splitString(QString str)
{
 QStrList list;
 QString temp = "";
 if(str.find(" ")==-1)
  return(0);
 str+=' ';
 do
  {
   temp = str;
   temp+=' ';
   pos = temp.find(" ");
   temp.truncate(pos);
   str.remove(0,pos);
   str = str.stripWhiteSpace();
   str = str.simplifyWhiteSpace();
   temp = temp.stripWhiteSpace();
   temp = temp.simplifyWhiteSpace();
   list.append(temp);
  }
 while(str!="");
 // Return string list
 return(list);
}

