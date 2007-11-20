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
#include "processlinemaker.moc"

#include <kprocess.h>

ProcessLineMaker::ProcessLineMaker()
{
}

ProcessLineMaker::ProcessLineMaker( const KProcess* proc )
{
    connect(proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
            this, SLOT(slotReceivedStdout(KProcess*,char*,int)) );
    
    connect(proc, SIGNAL(receivedStderr(KProcess*,char*,int)),
            this, SLOT(slotReceivedStderr(KProcess*,char*,int)) );

    connect(&stdouttimer, SIGNAL(timeout()),
            this, SLOT(slotTimeoutStdout()) );

    connect(&stderrtimer, SIGNAL(timeout()),
            this, SLOT(slotTimeoutStderr()) );
}

void ProcessLineMaker::slotReceivedStdout( const char *buffer )
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

void ProcessLineMaker::slotReceivedStdout( KProcess*, char *buffer, int )
{
    slotReceivedStdout(buffer); // It is zero-terminated
}

void ProcessLineMaker::slotTimeoutStdout()
{
    emit receivedStdoutLine(QString::fromLocal8Bit(stdoutbuf));
    stdoutbuf.truncate(0);
}

void ProcessLineMaker::slotReceivedStderr( const char *buffer )
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

void ProcessLineMaker::slotReceivedStderr( KProcess*, char *buffer, int )
{
    slotReceivedStderr(buffer); // It is zero-terminated
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
