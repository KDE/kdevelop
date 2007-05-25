/* This file is part of the KDE project
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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

#include <k3process.h>
#include <QStringList>

class ProcessLineMakerPrivate
{
public:
    ProcessLineMakerPrivate( ProcessLineMaker* maker )
        : p(maker)
    {}
    QString stdoutbuf;
    QString stderrbuf;
    ProcessLineMaker* p;

    void slotReceivedStdout( K3Process*, char *buffer, int buflen )
    {
        processStdOut( QString::fromLocal8Bit( buffer, buflen ) );
    }

    void processStdOut( const QString& s )
    {
        // First Flush the opposite buffer
        if (!stderrbuf.isEmpty())
        {
            emit p->receivedStderrLines( QStringList(stderrbuf) );

            stderrbuf = "";
        }


        stdoutbuf += s;
        int pos;
        QStringList lineList;
        while ( (pos = stdoutbuf.indexOf('\n')) != -1)
        {
            QString line = stdoutbuf.left(pos);
            lineList += line;
            stdoutbuf.remove(0, pos+1);
        }
        emit p->receivedStdoutLines(lineList);
    }

    void slotReceivedStderr( K3Process*, char *buffer, int buflen )
    {
        processStdErr( QString::fromLocal8Bit(buffer, buflen) );
    }

    void processStdErr( const QString& s )
    {
        // First Flush the opposite buffer
        if (!stdoutbuf.isEmpty())
        {
            emit p->receivedStdoutLines( QStringList(stdoutbuf) );

            stdoutbuf = "";
        }


        stderrbuf += s;
        int pos;
        QStringList lineList;
        while ( (pos = stderrbuf.indexOf('\n')) != -1)
        {
            QString line = stderrbuf.left(pos);
            lineList += line;
            stderrbuf.remove(0, pos+1);
        }
        emit p->receivedStderrLines(lineList);
    }
};

ProcessLineMaker::ProcessLineMaker()
    : d( new ProcessLineMakerPrivate( this ) )
{
}

ProcessLineMaker::ProcessLineMaker( const K3Process* proc )
    : d( new ProcessLineMakerPrivate( this ) )
{
    connect(proc, SIGNAL(receivedStdout(K3Process*,char*,int)),
            this, SLOT(slotReceivedStdout(K3Process*,char*,int)) );

    connect(proc, SIGNAL(receivedStderr(K3Process*,char*,int)),
            this, SLOT(slotReceivedStderr(K3Process*,char*,int)) );
}

void ProcessLineMaker::clearBuffers( )
{
    d->stderrbuf = "";
    d->stdoutbuf = "";
}

#include "processlinemaker.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
