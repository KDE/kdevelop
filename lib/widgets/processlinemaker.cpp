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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
}

void ProcessLineMaker::slotReceivedStdout( const QString& s )
{
    // Flush stderr buffer
    if (!stderrbuf.isEmpty()) {
        emit receivedStderrLine(stderrbuf);
        stderrbuf = "";
    }
    
    stdoutbuf += s;
    int pos;
    while ( (pos = stdoutbuf.find('\n')) != -1) {
        QString line = stdoutbuf.left(pos);
        emit receivedStdoutLine(line);
        stdoutbuf.remove(0, pos+1);
    }
}

void ProcessLineMaker::slotReceivedStdout( KProcess*, char *buffer, int buflen )
{
    slotReceivedStdout( QString::fromLocal8Bit( buffer, buflen ) );
}

void ProcessLineMaker::slotReceivedStdout( const char* buffer )
{
    slotReceivedStdout( QString::fromLocal8Bit( buffer ) );
}

void ProcessLineMaker::slotReceivedStderr( const QString& s )
{
    // Flush stdout buffer
    if (!stdoutbuf.isEmpty()) {
        emit receivedStdoutLine(stdoutbuf);
        stdoutbuf = "";
    }
    
    stderrbuf += s;
    int pos;
    while ( (pos = stderrbuf.find('\n')) != -1) {
        QString line = stderrbuf.left(pos);
        emit receivedStderrLine(line);
        stderrbuf.remove(0, pos+1);
    }
}

void ProcessLineMaker::slotReceivedStderr( KProcess*, char *buffer, int buflen )
{
    slotReceivedStderr( QString::fromLocal8Bit( buffer, buflen ) );
}

void ProcessLineMaker::slotReceivedStderr( const char* buffer )
{
    slotReceivedStderr( QString::fromLocal8Bit( buffer ) );
}

void ProcessLineMaker::clearBuffers( )
{
	stderrbuf = "";
	stdoutbuf = "";
}
