/* This file is part of the KDE project
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>

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

#include "processlinemaker.h"

#include <QProcess>

ProcessLineMaker::ProcessLineMaker()
{
}

ProcessLineMaker::ProcessLineMaker( const QProcess* proc )
{
    connect(proc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReceivedStdout()) );
    
    connect(proc, SIGNAL(readyReadStandardError()),
            this, SLOT(slotReceivedStderr()) );

    connect(&stdouttimer, SIGNAL(timeout()),
            this, SLOT(slotTimeoutStdout()) );

    connect(&stderrtimer, SIGNAL(timeout()),
            this, SLOT(slotTimeoutStderr()) );
}

void ProcessLineMaker::slotReceivedStdout( const QByteArray& buffer )
{
    stdouttimer.stop();
    stdoutbuf += buffer;
    int pos;
    while ( (pos = stdoutbuf.find('\n')) != -1) {
        QString line = QString::fromLocal8Bit(stdoutbuf, pos);
        emit receivedStdoutLine(line);
        stdoutbuf.remove(0, pos+1);
    }
    if (!stdoutbuf.isEmpty())
        stdouttimer.start(100, true);
}

void ProcessLineMaker::slotReceivedStdout()
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    Q_ASSERT(process);
    QByteArray input = process->readAllStandardOutput();
    slotReceivedStdout(input); // It is zero-terminated
}

void ProcessLineMaker::slotTimeoutStdout()
{
    emit receivedStdoutLine(QString::fromLocal8Bit(stdoutbuf));
    stdoutbuf.truncate(0);
}

void ProcessLineMaker::slotReceivedStderr( const QByteArray& buffer )
{
    stderrtimer.stop();
    stderrbuf += buffer;
    int pos;
    while ( (pos = stderrbuf.find('\n')) != -1) {
        QString line = QString::fromLocal8Bit(stderrbuf, pos);
        emit receivedStderrLine(line);
        stderrbuf.remove(0, pos+1);
    }
    if (!stderrbuf.isEmpty())
        stderrtimer.start(100, true);
}

void ProcessLineMaker::slotReceivedStderr()
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    Q_ASSERT(process);
    QByteArray input = process->readAllStandardError();
    slotReceivedStderr(input); // It is zero-terminated
}

void ProcessLineMaker::slotTimeoutStderr()
{
    emit receivedStderrLine(QString::fromLocal8Bit(stderrbuf));
    stderrbuf.truncate(0);
}

void ProcessLineMaker::clearBuffers( )
{
    stderrbuf.truncate(0);
    stdoutbuf.truncate(0);
    stdouttimer.stop();
    stderrtimer.stop();
}

#include "processlinemaker.moc"
             