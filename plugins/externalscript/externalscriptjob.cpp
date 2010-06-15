/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2010 Milian Wolff <mail@milianw.de>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "externalscriptjob.h"

#include "externalscriptitem.h"

#include <QFileInfo>

#include <KProcess>
#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>

#include <outputview/outputmodel.h>
#include <util/processlinemaker.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <QApplication>

ExternalScriptJob::ExternalScriptJob( ExternalScriptItem* item, QObject* parent )
    : KDevelop::OutputJob( parent ), m_proc( new KProcess( this ) ),
    m_lineMaker( new KDevelop::ProcessLineMaker( m_proc, this ) )
{
  kDebug() << "creating external script job";

  setCapabilities( Killable );
  setStandardToolView( KDevelop::IOutputView::RunView );
  setBehaviours( KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
  setModel( new KDevelop::OutputModel(), KDevelop::IOutputView::TakeOwnership );

  KDevelop::IDocument* active = KDevelop::ICore::self()->documentController()->activeDocument();

  QString command = item->command();

  if ( active ) {
    const KUrl url = active->url();
    if ( url.isLocalFile() ) {
      ///TODO: make configurable, use fallback to project dir
      m_proc->setWorkingDirectory( active->url().directory() );
    }
    ///TODO: make those placeholders escapeable
    command.replace( "%u", url.pathOrUrl() );
    ///TODO: does that work with remote files?
    ///TODO: document the available placeholders
    QFileInfo info( url.pathOrUrl() );
    command.replace( "%f", info.filePath() );
    command.replace( "%b", info.baseName() );
    command.replace( "%n", info.fileName() );
    command.replace( "%d", info.path() );
  }

  connect( m_lineMaker, SIGNAL( receivedStdoutLines( QStringList ) ),
           model(), SLOT( appendLines( QStringList ) ) );
  connect( m_proc, SIGNAL( error( QProcess::ProcessError ) ),
           SLOT( processError( QProcess::ProcessError ) ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ),
           SLOT( processFinished( int, QProcess::ExitStatus ) ) );

  // Now setup the process parameters
  kDebug() << "setting command:" << command;

  m_proc->setOutputChannelMode( KProcess::MergedChannels );
  m_proc->setShellCommand( command );

  setObjectName( command );
}

void ExternalScriptJob::start()
{
  kDebug() << "launching?" << m_proc;

  if ( m_proc ) {
    startOutput();
    appendLine( i18n( "Running external script: %1", m_proc->program().join(" ") ) );
    m_proc->start();
  } else {
    kWarning() << "No process, something went wrong when creating the job";
    // No process means we've returned early on from the constructor, some bad error happened
    emitResult();
  }
}

bool ExternalScriptJob::doKill()
{
  if ( m_proc ) {
    m_proc->kill();
    appendLine( i18n( "*** Killed Application ***" ) );
  }

  return true;
}


void ExternalScriptJob::processFinished( int exitCode , QProcess::ExitStatus status )
{
  m_lineMaker->flushBuffers();

  if ( exitCode == 0 && status == QProcess::NormalExit )
    appendLine( i18n( "*** Exited normally ***" ) );
  else
    if ( status == QProcess::NormalExit )
      appendLine( i18n( "*** Exited with return code: %1 ***", QString::number( exitCode ) ) );
    else
      if ( error() == KJob::KilledJobError )
        appendLine( i18n( "*** Process aborted ***" ) );
      else
        appendLine( i18n( "*** Crashed with return code: %1 ***", QString::number( exitCode ) ) );

  kDebug() << "Process done";

  emitResult();
}

void ExternalScriptJob::processError( QProcess::ProcessError error )
{
  if ( error == QProcess::FailedToStart ) {
    setError( -1 );
    QString errmsg =  i18n( "Could not start program '%1'. Make sure that the "
                            "path is specified correctly.", m_proc->program().join( " " ) );
    KMessageBox::error( QApplication::activeWindow(), errmsg, i18n( "Could not start application" ) );
    setErrorText( errmsg );
    emitResult();
  }

  kDebug() << "Process error";
}

void ExternalScriptJob::appendLine( const QString& l )
{
  if ( KDevelop::OutputModel* m = model() ) {
    m->appendLine( l );
  }
}

KDevelop::OutputModel* ExternalScriptJob::model()
{
  return dynamic_cast<KDevelop::OutputModel*>( OutputJob::model() );
}


#include "externalscriptjob.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
