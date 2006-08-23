/* This file is part of KDevelop
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "valgrindcontrol.h"

ValgrindControl::ValgrindControl( QObject * parent )
{
}

void ValgrindControl::run( ValgrindModel * model, const QString & executable, const QString & parameters, const QString & valgrindExecutable, const QString & valgrindParameters )
{
  if ( m_process->isRunning() ) {
    KMessageBox::sorry( 0, i18n( "There is already an instance of valgrind running." ) );
    return;
    /// @todo - ask for forced kill
  }

  int port = 38462;
  if (!m_valgrindServer) {
    m_valgrindServer = new QTcpServer(this);
    connect(m_valgrindServer, SIGNAL(newConnection()), SLOT(readFromValgrind()));

    // Try an arbitrary port range for now
    while (!m_valgrindServer->listen(QHostAddress::LocalHost, port) && port < 38482)
      ++port;

    if (!m_valgrindServer->isListening())
      kWarning() << "Could not open TCP socket for communication with Valgrind." << endl;
    else
      kDebug() << "Opened TCP socket " << port << " for communication with Valgrind." << endl;
  }

  m_process->clearArguments();
  *m_process << valExec << valParams  << "--xml=yes" << QString("--log-socket=localhost:%1").arg(port) << exec << params;
  m_process->start( KProcess::NotifyOnExit, KProcess::AllOutput );
  mainWindow()->raiseView( m_treeView );
  core()->running( this, true );

  _lastExec = exec;
  _lastParams = params;

  m_xmlReader->setContentHandler(m_model);
}

void ValgrindControl::m_processessExited( KProcess* p )
{
  if ( p == m_process ) {
    core()->running( this, false );

    m_valgrindServer->close();
    delete m_valgrindConnection;
    m_valgrindConnection = 0L;

    if (kcInfo.runKc)
    {
        KProcess *kcProc = new KProcess;
//        kcProc->setWorkingDirectory(kcInfo.kcWorkDir);
        *kcProc << kcInfo.kcPath;
        *kcProc << QString("cachegrind.out.%1").arg(p->pid());
        kcProc->start(KProcess::DontCare);
    }
  }
}

void ValgrindControl::readFromValgrind( )
{
}

void ValgrindControl::newValgrindConnection( )
{
}

#include "valgrindcontrol.moc"
