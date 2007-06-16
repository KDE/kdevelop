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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "processlinemaker.h"

#include <QtCore/QProcess>
#include <QtCore/QStringList>

class ProcessLineMakerPrivate
{
public:
    ProcessLineMakerPrivate( ProcessLineMaker* maker )
        : p(maker)
    {}
    QByteArray m_stdout_rest;
    QByteArray m_stderr_rest;
    QString stdoutbuf;
    QString stderrbuf;
    ProcessLineMaker* p;
    QProcess* m_proc;

    void slotProcessFinished()
    {
        QString out = QString::fromLocal8Bit(m_stdout_rest+m_proc->readAll());
        m_stdout_rest.clear();
        processStdOut( out );

        m_proc->setReadChannel( QProcess::StandardError );
        out = QString::fromLocal8Bit(m_stderr_rest+m_proc->readAll());
        m_stderr_rest.clear();
        m_proc->setReadChannel( QProcess::StandardOutput );
        processStdErr( out );
    }

    void slotReadyReadStdout()
    {
        QString out;
        while( m_proc->canReadLine() )
        {
            out += QString::fromLocal8Bit( m_stdout_rest + m_proc->readLine() );
            m_stdout_rest.clear();
        }
        m_stdout_rest = m_proc->readAll();
        processStdOut( out );
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
            lineList << line;
            stdoutbuf.remove(0, pos+1);
        }
        emit p->receivedStdoutLines(lineList);
    }

    void slotReadyReadStderr()
    {
        QString out;
        m_proc->setReadChannel( QProcess::StandardError );
        while( m_proc->canReadLine() )
        {
            out += QString::fromLocal8Bit( m_stderr_rest + m_proc->readLine() );
            m_stderr_rest.clear();
        }
        m_stderr_rest = m_proc->readAll();
        m_proc->setReadChannel( QProcess::StandardOutput );
        processStdErr( out );
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
            lineList << line;
            stderrbuf.remove(0, pos+1);
        }
        emit p->receivedStderrLines(lineList);
    }
};

ProcessLineMaker::ProcessLineMaker()
    : d( new ProcessLineMakerPrivate( this ) )
{
}

ProcessLineMaker::ProcessLineMaker( QProcess* proc )
    : d( new ProcessLineMakerPrivate( this ) )
{
    d->m_proc = proc;
    d->m_proc->setTextModeEnabled( true );
    connect(proc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReadyReadStdout()) );

    connect(proc, SIGNAL(readyReadStandardError()),
            this, SLOT(slotReadyReadStderr()) );
    connect(proc, SIGNAL(finished(int, QPRocess::ExitStatus)),
            this, SLOT(slotProcessFinished()));
}

void ProcessLineMaker::slotReceivedStdout( const QString& s )
{
    d->processStdOut( s );
}

void ProcessLineMaker::slotReceivedStdout( const char* buffer )
{
    d->processStdOut( QString::fromLocal8Bit( buffer ) );
}

void ProcessLineMaker::slotReceivedStderr( const QString& s )
{
    d->processStdErr( s );
}

void ProcessLineMaker::slotReceivedStderr( const char* buffer )
{
    d->processStdErr( QString::fromLocal8Bit( buffer ) );
}

void ProcessLineMaker::clearBuffers( )
{
    d->stderrbuf = "";
    d->stdoutbuf = "";
}

#include "processlinemaker.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
