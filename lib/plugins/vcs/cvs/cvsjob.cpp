/***************************************************************************
 *   This file was partly taken from cervisia's cvsservice                 *
 *   Copyright (C) 2002-2003 Christian Loose <christian.loose@hamburg.de>  *
 *                                                                         *
 *   Adapted for KDevelop                                                  *
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsjob.h"

#include <QFile>
#include <QList>

#include <QStringList>
#include <KDebug>
#include <KLocale>
#include <K3Process>


struct CvsJob::Private
{
    Private() : isRunning(false)
    {
        childproc = new K3Process;
        childproc->setUseShell(true, "/bin/sh");
    }
    ~Private() { delete childproc; }

    K3Process*   childproc;
    QString     server;
    QString     rsh;
    QString     directory;
    bool        isRunning;
    QString     outputLines;
};


CvsJob::CvsJob(QObject* parent)
    : KJob(parent), d(new Private)
{
}

CvsJob::~CvsJob()
{
    kDebug()<<  k_funcinfo <<endl;
    delete d;
}

void CvsJob::clear()
{
    d->childproc->clearArguments();
    d->outputLines.clear();
}


void CvsJob::setRSH(const QString& rsh)
{
    d->rsh = rsh;
}


void CvsJob::setServer(const QString& server)
{
    d->server = server;
}


void CvsJob::setDirectory(const QString& directory)
{
    d->directory = directory;
}


bool CvsJob::isRunning() const
{
    return d->isRunning;
}


CvsJob& CvsJob::operator<<(const QString& arg)
{
    *d->childproc << arg;
    return *this;
}


CvsJob& CvsJob::operator<<(const char* arg)
{
    *d->childproc << arg;
    return *this;
}


CvsJob& CvsJob::operator<<(const QStringList& args)
{
    *d->childproc << args;
    return *this;
}


QString CvsJob::cvsCommand() const
{
    QString command;

    const QList<QByteArray>& args(d->childproc->args());
    foreach (QByteArray arg, args)
    {
        if (!command.isEmpty())
            command += ' ';

        command += QFile::decodeName(arg);
    }

    return command;
}


QString CvsJob::output() const
{
    return d->outputLines;
}


void CvsJob::start()
{
    if( !d->rsh.isEmpty() )
        d->childproc->setEnvironment("CVS_RSH", d->rsh);

    if( !d->server.isEmpty() )
        d->childproc->setEnvironment("CVS_SERVER", d->server);

    if( !d->directory.isEmpty() ) {
        kDebug() << "Working directory: " << d->directory << endl;
        d->childproc->setWorkingDirectory(d->directory);
    }

    connect(d->childproc, SIGNAL(processExited(K3Process*)),
        SLOT(slotProcessExited(K3Process*)));
    connect(d->childproc, SIGNAL(receivedStdout(K3Process*, char*, int)),
        SLOT(slotReceivedStdout(K3Process*, char*, int)));
    connect(d->childproc, SIGNAL(receivedStderr(K3Process*, char*, int)),
        SLOT(slotReceivedStderr(K3Process*, char*, int)) );

    kDebug() << "Execute cvs command: " << cvsCommand() << endl;

    d->outputLines.clear();
    d->isRunning = true;
    d->childproc->start(K3Process::NotifyOnExit, K3Process::AllOutput);
}


void CvsJob::cancel()
{
    d->childproc->kill();
}


void CvsJob::slotProcessExited(K3Process* proc)
{
    Q_UNUSED(proc);

    kDebug()<<  k_funcinfo <<endl;

    // disconnect all connections to childproc's signals
    d->childproc->disconnect();

    d->isRunning = false;

    if (!d->childproc->normalExit()) {
        setError( d->childproc->exitStatus() );
        setErrorText( i18n("Process exited with status %1", d->childproc->exitStatus()) );
    }
    emitResult();
}


void CvsJob::slotReceivedStdout(K3Process* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);

    // accumulate output
    d->outputLines += output;
    kDebug()<<  k_funcinfo <<"received output: "<<endl;
    kDebug()<<output<<endl;
}


void CvsJob::slotReceivedStderr(K3Process* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);

    // accumulate output
    d->outputLines += output;

    kDebug()<<  k_funcinfo <<"received error: "<<endl;
    kDebug()<<output<<endl;
}

#include "cvsjob.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
