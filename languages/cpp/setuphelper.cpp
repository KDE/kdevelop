
/***************************************************************************
*   Copyright (C) 2006 by Andras Mantia                                   *
*   amantia@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "setuphelper.h"
#include "blockingkprocess.h"
#include "driver.h"
#include <kdebug.h>
#include "ktempfile.h" /* defines [function] KTempDir */
#include "kstandarddirs.h" /* defines [function] locateLocal */
#include "qdir.h" /* defines QDir */
#include <stdio.h>

namespace SetupHelper {
  
QString getGccIncludePath(bool *ok)
{
  *ok = true;
  QString processStdout; 
  BlockingKProcess proc;
  proc << "gcc" ;
  proc << "-print-file-name=include" ;
  if ( !proc.start(KProcess::NotifyOnExit, KProcess::Stdout) ) {
    kdWarning(9007) << "Couldn't start gcc" << endl;
    *ok = false;
    return QString();
  }
  processStdout = proc.stdOut();    

  return processStdout;
}

QString getVerboseGccIncludePath(bool *ok)
{
  *ok = false;
  ///Create temp file
  KTempFile tempFile(locateLocal("tmp", "kdevelop_temp"), ".cpp");
  tempFile.setAutoDelete(true);
  if( tempFile.status() != 0 ) 
    return QString();//Failed to create temp file
  
  QString path = tempFile.name();
  QFileInfo pathInfo( path );

  char fileText[] = "//This source-file is empty";
  fwrite(fileText, strlen(fileText), 1, tempFile.fstream() );
  tempFile.close();
  
  BlockingKProcess proc;
  proc.setUseShell(true);
  proc.setWorkingDirectory(pathInfo.dir(true).path());
  proc << "gcc -v " + pathInfo.fileName() + " 2>&1";
  if ( !proc.start(KProcess::NotifyOnExit, KProcess::Stdout) ) {
    kdWarning(9007) << "Couldn't start gcc" << endl;
    *ok = false;
    return QString();
  }
  *ok = true;
  return proc.stdOut();
}

QStringList getGccMacros(bool *ok)
{
  *ok = true;
  QString processStdout; 
  BlockingKProcess proc;
  proc << "gcc";
  proc << "-E";
  proc << "-dM";
  proc << "-ansi" ;
  proc << "-";
  if ( !proc.start(KProcess::NotifyOnExit, KProcess::Stdout) ) {
    kdWarning(9007) << "Couldn't start gcc" << endl;
    *ok = false;
    return QStringList();
  }
  proc.closeStdin();
  processStdout = proc.stdOut(); 
  QStringList lines = QStringList::split('\n', processStdout);
  return lines;
}

}
