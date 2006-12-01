
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

#include <kdebug.h>

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