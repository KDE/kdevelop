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

#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QTcpServer>
#include <QTcpSocket>

#include <kprocess.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "valgrindmodel.h"

ValgrindControl::ValgrindControl( QObject * parent )
  : QObject(parent)
  , m_inputSource(0)
  , m_xmlReader(new QXmlSimpleReader)
  , m_server(0)
  , m_connection(0)
{
  m_process = new KShellProcess();
  connect( m_process, SIGNAL(receivedStdout( KProcess*, char*, int )), SLOT(receivedStdout( KProcess*, char*, int )) );
  connect( m_process, SIGNAL(receivedStderr( KProcess*, char*, int )), SLOT(receivedStderr( KProcess*, char*, int )) );
  connect( m_process, SIGNAL(processExited( KProcess* )), SLOT(processExited( KProcess* )) );
}

void ValgrindControl::run( ValgrindModel * model, const QString & executable, const QString & parameters, const QString & valgrindExecutable, const QString & valgrindParameters )
{
  if ( m_process->isRunning() ) {
    KMessageBox::sorry( 0, i18n( "There is already an instance of valgrind running." ) );
    return;
    /// @todo - ask for forced kill
  }

  int port = 38462;
  if (!m_server) {
    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), SLOT(readFromValgrind()));

    // Try an arbitrary port range for now
    while (!m_server->listen(QHostAddress::LocalHost, port) && port < 38482)
      ++port;

    if (!m_server->isListening())
      kWarning() << "Could not open TCP socket for communication with Valgrind." << endl;
    else
      kDebug() << "Opened TCP socket " << port << " for communication with Valgrind." << endl;
  }

  m_process->clearArguments();
  *m_process << valgrindExecutable << valgrindParameters  << "--xml=yes" << QString("--log-socket=localhost:%1").arg(port) << executable << parameters;
  m_process->start( KProcess::NotifyOnExit, KProcess::AllOutput );
  //mainWindow()->raiseView( m_treeView );
  //KDevCore::core()->running( this, true );

  m_xmlReader->setContentHandler(model);
}

void ValgrindControl::stop()
{
  m_process->kill();
}

void ValgrindControl::processExited( KProcess* p )
{
  if ( p == m_process ) {
    //core()->running( this, false );

    m_server->close();
    delete m_connection;
    m_connection = 0L;

    /*if (kcInfo.runKc)
    {
        KProcess *kcProc = new KProcess;
//        kcProc->setWorkingDirectory(kcInfo.kcWorkDir);
        *kcProc << kcInfo.kcPath;
        *kcProc << QString("cachegrind.out.%1").arg(p->pid());
        kcProc->start(KProcess::DontCare);
    }*/
  }
}

void ValgrindControl::readFromValgrind( )
{
}

void ValgrindControl::newValgrindConnection( )
{
  QTcpSocket* sock = m_server->nextPendingConnection();
  kDebug() << k_funcinfo << sock << endl;
  if (sock && !m_connection) {
    m_connection = sock;
    delete m_inputSource;
    m_inputSource = new QXmlInputSource(sock);
    m_xmlReader->parse(m_inputSource, true);
    connect(sock, SIGNAL(readyRead()), SLOT(slotReadFromValgrind()));
  }
}

void ValgrindControl::receivedStdout( KProcess*, char*, int )
{
}

void ValgrindControl::receivedStderr( KProcess*, char*, int )
{
}

#include "valgrindcontrol.moc"
