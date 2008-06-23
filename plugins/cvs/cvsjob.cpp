/***************************************************************************
 *   This file was partly taken from cervisia's cvsservice                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *                                                                         *
 *   Adapted for KDevelop                                                  *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
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

#include "processlinemaker.h"
#include <iplugin.h>

struct CvsJob::Private
{
    Private() : isRunning(false), commMode(KProcess::SeparateChannels), vcsplugin(0)
    {
        childproc = new KProcess;
        lineMaker = new KDevelop::ProcessLineMaker( childproc );
    }

    ~Private() {
        if (lineMaker) delete lineMaker;
        if (childproc) delete childproc;
    }

    KDevelop::ProcessLineMaker* lineMaker;

    KProcess*   childproc;
    QStringList command;
    QString     server;
    QString     rsh;
    QString     directory;
    bool        isRunning;
    QStringList outputLines;
    KProcess::OutputChannelMode commMode;
    KDevelop::IPlugin* vcsplugin;
};


CvsJob::CvsJob(KDevelop::IPlugin* parent)
    : VcsJob(parent), d(new Private)
{
    d->vcsplugin = parent;
}

CvsJob::~CvsJob()
{
    delete d;
}

void CvsJob::clear()
{
    d->childproc->clearEnvironment();
    d->command.clear();
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


QString CvsJob::getDirectory()
{
    return d->directory;
}


bool CvsJob::isRunning() const
{
    return d->isRunning;
}


CvsJob& CvsJob::operator<<(const QString& arg)
{
    d->command.append( arg );
    return *this;
}


CvsJob& CvsJob::operator<<(const char* arg)
{
    d->command.append( arg );
    return *this;
}


CvsJob& CvsJob::operator<<(const QStringList& args)
{
    d->command.append( args.join(" ") );
    return *this;
}


QString CvsJob::cvsCommand() const
{
    return d->command.join(" ");
}


QString CvsJob::output() const
{
    return d->outputLines.join("\n");
}


void CvsJob::start()
{
    if( !d->rsh.isEmpty() )
        d->childproc->setEnv("CVS_RSH", d->rsh);

    if( !d->server.isEmpty() )
        d->childproc->setEnv("CVS_SERVER", d->server);

    if( !d->directory.isEmpty() ) {
        kDebug(9500) << "Working directory:" << d->directory;
        d->childproc->setWorkingDirectory(d->directory);
    }

    connect(d->childproc, SIGNAL(finished(int, QProcess::ExitStatus)),
        SLOT(slotProcessExited(int, QProcess::ExitStatus)));
    connect(d->childproc, SIGNAL(error(QProcess::ProcessError)), 
        SLOT(slotProcessError(QProcess::ProcessError)));

    connect(d->lineMaker, SIGNAL(receivedStdoutLines(const QStringList&)),
        SLOT(slotReceivedStdout(const QStringList&)));
    connect(d->lineMaker, SIGNAL(receivedStderrLines(const QStringList&)),
        SLOT(slotReceivedStderr(const QStringList&)) );

    kDebug(9500) << "Execute cvs command:" << cvsCommand();

    d->outputLines.clear();
    d->isRunning = true;
    d->childproc->setOutputChannelMode( d->commMode );
    d->childproc->setProgram( d->command );
    d->childproc->start();
}


void CvsJob::setCommunicationMode(KProcess::OutputChannelMode comm)
{
    d->commMode = comm;
}


void CvsJob::cancel()
{
    d->childproc->kill();
}

void CvsJob::slotProcessError( QProcess::ProcessError err)
{
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    setError( d->childproc->exitCode() );
    setErrorText( i18n("Process exited with status %1", d->childproc->exitCode()) );
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}


void CvsJob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        setError( exitCode );
        setErrorText( i18n("Process exited with status %1", exitCode) );
    }
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}


void CvsJob::slotReceivedStdout(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug(9500)<<"received output:";
    kDebug(9500)<<output.join("\n");
}


void CvsJob::slotReceivedStderr(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug(9500)<<"received error:";
    kDebug(9500)<<output.join("\n");
}

QVariant CvsJob::fetchResults()
{
    return output();
}

KDevelop::VcsJob::JobStatus CvsJob::status() const
{
    if (d->isRunning)
        return KDevelop::VcsJob::JobRunning;

    if (d->childproc->exitCode() != 0)
        return KDevelop::VcsJob::JobFailed;

    return KDevelop::VcsJob::JobSucceeded;
}

KDevelop::IPlugin* CvsJob::vcsPlugin() const
{
    return d->vcsplugin;
}

#include "cvsjob.moc"
