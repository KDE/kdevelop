/* This file is part of the KDE project
   Copyright 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>
   Copyright 2007 Oswald Buddenhagen <ossi@kde.org>

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

#include <QProcess>
#include <QStringList>

namespace KDevelop {

class ProcessLineMakerPrivate
{
public:
    QByteArray stdoutbuf;
    QByteArray stderrbuf;
    ProcessLineMaker* p;
    QProcess* m_proc;

    explicit ProcessLineMakerPrivate(ProcessLineMaker* maker)
        : p(maker)
    {
    }

    void slotReadyReadStdout()
    {
        stdoutbuf += m_proc->readAllStandardOutput();
        processStdOut();
    }

    static QStringList streamToStrings(QByteArray& data)
    {
        QStringList lineList;
        int pos;
        while ((pos = data.indexOf('\n')) != -1) {
            if (pos > 0 && data.at(pos - 1) == '\r')
                lineList << QString::fromLocal8Bit(data.constData(), pos - 1);
            else
                lineList << QString::fromLocal8Bit(data.constData(), pos);
            data.remove(0, pos + 1);
        }
        return lineList;
    }

    void processStdOut()
    {
        emit p->receivedStdoutLines(streamToStrings(stdoutbuf));
    }

    void slotReadyReadStderr()
    {
        stderrbuf += m_proc->readAllStandardError();
        processStdErr();
    }

    void processStdErr()
    {
        emit p->receivedStderrLines(streamToStrings(stderrbuf));
    }

};

ProcessLineMaker::ProcessLineMaker(QObject* parent)
    : QObject(parent)
    , d_ptr(new ProcessLineMakerPrivate(this))
{
}

ProcessLineMaker::ProcessLineMaker(QProcess* proc, QObject* parent)
    : QObject(parent)
    , d_ptr(new ProcessLineMakerPrivate(this))
{
    Q_D(ProcessLineMaker);

    d->m_proc = proc;
    connect(proc, &QProcess::readyReadStandardOutput,
            this, [this] {
            Q_D(ProcessLineMaker);
            d->slotReadyReadStdout();
        });
    connect(proc, &QProcess::readyReadStandardError,
            this, [this] {
            Q_D(ProcessLineMaker);
            d->slotReadyReadStderr();
        });
}

ProcessLineMaker::~ProcessLineMaker() = default;

void ProcessLineMaker::slotReceivedStdout(const QByteArray& buffer)
{
    Q_D(ProcessLineMaker);

    d->stdoutbuf += buffer;
    d->processStdOut();
}

void ProcessLineMaker::slotReceivedStderr(const QByteArray& buffer)
{
    Q_D(ProcessLineMaker);

    d->stderrbuf += buffer;
    d->processStdErr();
}

void ProcessLineMaker::discardBuffers()
{
    Q_D(ProcessLineMaker);

    d->stderrbuf.truncate(0);
    d->stdoutbuf.truncate(0);
}

void ProcessLineMaker::flushBuffers()
{
    Q_D(ProcessLineMaker);

    if (!d->stdoutbuf.isEmpty())
        emit receivedStdoutLines(QStringList(QString::fromLocal8Bit(d->stdoutbuf)));
    if (!d->stderrbuf.isEmpty())
        emit receivedStderrLines(QStringList(QString::fromLocal8Bit(d->stderrbuf)));
    discardBuffers();
}

}

#include "moc_processlinemaker.cpp"
