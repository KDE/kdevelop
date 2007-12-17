/* This file is part of KDevelop
   Copyright 2006 Hamish Rodda <rodda@kde.org>
   Copyright 2002 Harald Fernengel <harry@kdevelop.org>

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

#include <k3process.h>
#include <kprocess.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "valgrindmodel.h"
#include "valgrindplugin.h"

ValgrindControl::ValgrindControl(ValgrindPlugin* parent)
    : QObject(parent)
    , m_process(new KProcess(this))
    , m_inputSource(0)
    , m_xmlReader(new QXmlSimpleReader)
    , m_server(0)
    , m_connection(0)
    , m_model(new ValgrindModel(this))
{
    m_process->setOutputChannelMode( KProcess::SeparateChannels );

    connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(readyReadStandardOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), SLOT(readyReadStandardError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), SLOT(processErrored(QProcess::ProcessError)));
}

bool ValgrindControl::run(const KDevelop::IRun& run)
{
    Q_ASSERT(m_process->state() != QProcess::Running);

    int port = 38462;
    if (!m_server) {
        m_server = new QTcpServer(this);
        connect(m_server, SIGNAL(newConnection()), SLOT(readFromValgrind()));

        // Try an arbitrary port range for now
        while (!m_server->listen(QHostAddress::LocalHost, port) && port < 38482)
            ++port;

        if (!m_server->isListening())
            kWarning() << "Could not open TCP socket for communication with Valgrind." ;
        else
            kDebug() << "Opened TCP socket" << port << "for communication with Valgrind.";
    }

    QStringList arguments;
    arguments << QString("--tool=%1").arg(run.instrumentor());
    arguments << run.instrumentorArguments();
    arguments << "--xml=yes";
    arguments << QString("--log-socket=localhost:%1").arg(port);
    arguments << run.executable().path();
    arguments << run.arguments();

    m_process->setProgram(plugin()->valgrindExecutable().path(), arguments);

    m_process->start();

    m_xmlReader->setContentHandler(m_model);

    return true;
}

void ValgrindControl::stop()
{
    m_process->kill();
}

void ValgrindControl::readFromValgrind( )
{
}

void ValgrindControl::newValgrindConnection( )
{
    QTcpSocket* sock = m_server->nextPendingConnection();
    kDebug() << sock;
    if (sock && !m_connection) {
        m_connection = sock;
        delete m_inputSource;
        m_inputSource = new QXmlInputSource(sock);
        m_xmlReader->parse(m_inputSource, true);
        connect(sock, SIGNAL(readyRead()), SLOT(slotReadFromValgrind()));
    }
}

ValgrindPlugin * ValgrindControl::plugin() const
{
    return static_cast<ValgrindPlugin*>(const_cast<QObject*>(parent()));
}

void ValgrindControl::processErrored(QProcess::ProcessError )
{
}

void ValgrindControl::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    //core()->running( this, false );

    m_server->close();
    delete m_connection;
    m_connection = 0L;

    /*if (kcInfo.runKc)
    {
        KProcess kcProc;
//        kcProc.setWorkingDirectory(kcInfo.kcWorkDir);
        kcProc << kcInfo.kcPath;
        kcProc << QString("cachegrind.out.%1").arg(p->pid());
        kcProc.startDetached();
    }*/
}

void ValgrindControl::readyReadStandardError()
{
}

void ValgrindControl::readyReadStandardOutput()
{
}

#include "valgrindcontrol.moc"
