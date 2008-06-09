/* This file is part of KDevelop
   Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEV_VALGRIND_CONTROL_H
#define KDEV_VALGRIND_CONTROL_H

#include <QObject>
#include <QProcess>
#include <QTcpSocket>

#include <irun.h>

class KJob;
class KProcess;
class QXmlInputSource;
class QXmlSimpleReader;
class ValgrindModel;
class QTcpServer;
class QTcpSocket;
class ValgrindPlugin;
class QBuffer;

namespace KDevelop { class ProcessLineMaker; }

class ValgrindControl : public QObject
{
  Q_OBJECT

public:
    ValgrindControl(ValgrindPlugin* parent);

    ValgrindModel* model() const;

    ValgrindPlugin* plugin() const;

    bool run(const KDevelop::IRun& run, KJob* job);
    void stop();

private slots:
    void newValgrindConnection();
    void socketError(QAbstractSocket::SocketError err);
    void readFromValgrind();

    void readyReadStandardOutput();
    void readyReadStandardError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrored(QProcess::ProcessError);

    void applicationOutput(const QStringList& lines);
    void applicationOutputStdErr(const QStringList &lines);

private:
    KProcess* m_process;
    int m_currentPid;
    KJob* m_job;

    QTcpServer* m_server;
    QTcpSocket* m_connection;

    ValgrindModel* m_model;

    KDevelop::ProcessLineMaker* m_applicationOutput;
};

#endif
