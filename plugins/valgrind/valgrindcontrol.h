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

#ifndef KDEV_VALGRIND_CONTROL_H
#define KDEV_VALGRIND_CONTROL_H

#include <QObject>
#include <QProcess>

#include <irun.h>

class KProcess;
class QXmlInputSource;
class QXmlSimpleReader;
class ValgrindModel;
class QTcpServer;
class QTcpSocket;
class ValgrindPlugin;

class ValgrindControl : public QObject
{
  Q_OBJECT

public:
    ValgrindControl(ValgrindPlugin* parent);

    ValgrindPlugin* plugin() const;

    bool run(const KDevelop::IRun& run);
    void stop();

private slots:
    void newValgrindConnection();
    void readFromValgrind();

    void readyReadStandardOutput();
    void readyReadStandardError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrored(QProcess::ProcessError);

private:
    KProcess* m_process;
    int m_currentPid;

    QXmlInputSource* m_inputSource;
    QXmlSimpleReader* m_xmlReader;

    QTcpServer* m_server;
    QTcpSocket* m_connection;

    ValgrindModel* m_model;
};

#endif
