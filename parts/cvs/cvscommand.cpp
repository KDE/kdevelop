/***************************************************************************
 *   Copyright (C) 2002 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvscommand.h"

#include <kprocess.h>
#include <kprogress.h>
#include <klocale.h>
#include <kmessagebox.h>

CvsCommand::CvsCommand( const QStringList& args, const QString& workingDir, QObject* parent, const char* name ): 
	QObject( parent, name ), out( "" ) /* make sure out is not QString::null since that would mean "error" */

{
  progressDlg = new KProgressDialog( 0, 0, i18n("CVS command running..."), 
                    i18n("Please wait until the CVS command finishes."), false );
  connect( progressDlg, SIGNAL(cancelClicked()),
           this, SLOT(cancelClicked()) );

  proc = new KProcess();
  proc->setWorkingDirectory( workingDir );
  *proc << "cvs";
  *proc << args;

  connect( proc, SIGNAL(processExited(KProcess*)),
           this, SLOT(processExited()) );
  connect( proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
           this, SLOT(receivedStdout(KProcess*,char*,int)) );
  connect( proc, SIGNAL(receivedStderr(KProcess*,char*,int)),
           this, SLOT(receivedStderr(KProcess*,char*,int)) );

  bool ok = proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );

  if ( !ok ) {
    delete progressDlg;
    progressDlg = 0;
    KMessageBox::error( 0, i18n("Could not invoke CVS. Please make sure it is installed correctly"), i18n("Error invoking CVS") );

    emit finished( QString::null, QString::null );
    deleteLater();
  }
}

void CvsCommand::receivedStdout (KProcess*, char *buffer, int buflen)
{
  out += QString::fromUtf8( buffer, buflen );  
}

void CvsCommand::receivedStderr (KProcess*, char *buffer, int buflen)
{
  err += QString::fromUtf8( buffer, buflen );
}

void CvsCommand::processExited()
{
  delete progressDlg;
  progressDlg = 0;

  emit finished( out, err );
  deleteLater();
}

void CvsCommand::cancelClicked()
{
  delete progressDlg;
  progressDlg = 0;
  proc->kill();

  emit finished( QString::null, QString::null );
  deleteLater();
}

CvsCommand::~CvsCommand()
{
  delete proc;
  delete progressDlg;
}

#include "cvscommand.moc"
