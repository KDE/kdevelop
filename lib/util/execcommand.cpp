/* This file is part of the KDE project
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "execcommand.h"

#include <kprocess.h>
#include <kprogress.h>
#include <klocale.h>
#include <kmessagebox.h>

ExecCommand::ExecCommand( const QString& executable, const QStringList& args, 
                          const QString& workingDir, const QStringList& env,
                          QObject* parent):
	QObject( parent ), out( "" ) /* make sure out is not QString::null since that would mean "error" */

{
  progressDlg = 0;

  proc = new KProcess();
  proc->setWorkingDirectory( workingDir );
  for ( QStringList::ConstIterator it = env.begin(); it != env.end(); ++it )
    proc->setEnvironment( (*it).section( '=', 0, 0 ), (*it).section( '=', 1, 1 ) );
  *proc << executable;
  *proc << args;

  connect( proc, SIGNAL(processExited(KProcess*)),
           this, SLOT(processExited()) );
  connect( proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
           this, SLOT(receivedStdout(KProcess*,char*,int)) );
  connect( proc, SIGNAL(receivedStderr(KProcess*,char*,int)),
           this, SLOT(receivedStderr(KProcess*,char*,int)) );

  bool ok = proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );

  if ( !ok ) {
    KMessageBox::error( 0, i18n("Could not invoke \"%1\". Please make sure it is installed correctly").arg( executable ),
                        i18n("Error Invoking Command") );

    emit finished( QString::null, QString::null );
    deleteLater();

  } else {
    progressDlg = new KProgressDialog( 0, 0, i18n("Command running..."),
                      i18n("Please wait until the \"%1\" command finishes.").arg( executable ), false );
    connect( progressDlg, SIGNAL(cancelClicked()),
             this, SLOT(cancelClicked()) );
  }
}

void ExecCommand::receivedStdout (KProcess*, char *buffer, int buflen)
{
  out += QString::fromUtf8( buffer, buflen );  
}

void ExecCommand::receivedStderr (KProcess*, char *buffer, int buflen)
{
  err += QString::fromUtf8( buffer, buflen );
}

void ExecCommand::processExited()
{
  delete progressDlg;
  progressDlg = 0;

  emit finished( out, err );
  deleteLater();
}

void ExecCommand::cancelClicked()
{
  delete progressDlg;
  progressDlg = 0;
  proc->kill();

  emit finished( QString::null, QString::null );
  deleteLater();
}

ExecCommand::~ExecCommand()
{
  delete proc;
  delete progressDlg;
}

#include "execcommand.moc"
