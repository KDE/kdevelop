#include "cimport.h"

QString CImport::getValues(QFile &file, const QString &sKeyword)
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
          // kill now the follwing =
          sResult=sResult.stripWhiteSpace(); // but first maybe there's a whitespace
          if (sResult.left(1)=="=")
          {
              sResult=sResult.mid(1, sResult.length());
              // check the escape char
              if (sResult.right(1)=="\\")
              {
                    bContinues=true;
                    sResult=sResult.left(sResult.length()-1);
              }
          }
          else
          {
              bFound=false;    // there's no = so continue search
              sResult="";
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
      if (sResult.right(1)=="\\")
      {
            sResult=sResult.left(sResult.length()-1);
      }
      else
            bContinues=false;

  }

  sResult=sResult.simplifyWhiteSpace();  // strip and simplify whitespaces
  return sResult;
}

QStrList CImport::makeStringList(const QString &str)
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

/* void CImport::outputInfo(QFile &srcfile, const QString &sName, QFile &destfile)
{
  QString sEntry;
  QStrList slEntryList;


  cout << "---" << endl;
  cout << "Name of project: " << sName << endl;

  sEntry=getValues(srcfile, sName+"_SOURCES");
  slEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "sources: " << sEntry <<endl;

  /////////////
  sEntry=getValues(srcfile, sName+"_METASOURCES");
  slEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "metasources: " << sEntry <<endl;

  /////////////
  sEntry=getValues(srcfile, sName+"_LDADD");
  slEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "ldadd: " << sEntry <<endl;

  /////////////
  sEntry=getValues(srcfile, sName+"_INCLUDES");
  slEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "includes: " << sEntry <<endl;

  sEntry=getValues(srcfile, "APPSDIR");
  slEntryList=makeStringList(sEntry);

  // here we are having the info either as string or stringlist
  if (!sEntry.isEmpty())
    cout << "includes: " << sEntry <<endl;

  cout << "---" << endl;

}
*/

int CImport::write(QFile &file, const QString &dirstr)
{
 QString sMakefilename, sLine, sEntry;

 QString sSubDirs, sProgs;
 QStrList slSubDirList, slProgList;

 // Construct the Makefile.am string

 sMakefilename = dirstr + "/Makefile.am";

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

 // Print directory we are entering

 cout << "Entering directory '" << dirstr << "'" << endl;

 sSubDirs=getValues(makefile, "SUBDIRS");
 slSubDirList=makeStringList(sSubDirs);
 // Get subdirs

 sProgs=getValues(makefile, "bin_PROGRAMS");
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
        analyze(file, dirstr+"/"+sEntry);
    }
    sEntry=slSubDirList.next();
 }

 return(0);
}
