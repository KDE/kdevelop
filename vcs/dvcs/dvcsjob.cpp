/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS                                                      *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "dvcsjob.h"

#include <QFile>
#include <QList>

#include <QStringList>
#include <KDebug>
#include <KLocale>

#include <util/processlinemaker.h>
#include <interfaces/iplugin.h>

struct DVCSjobPrivate
{
    DVCSjobPrivate() : isRunning(false), commMode(KProcess::SeparateChannels), vcsplugin(0)
    {
        childproc = new KProcess;
        lineMaker = new KDevelop::ProcessLineMaker( childproc );
        failed = false;
    }

    ~DVCSjobPrivate() {
        if (lineMaker) delete lineMaker;
        if (childproc) delete childproc;
    }

    KDevelop::ProcessLineMaker* lineMaker;

    KProcess*   childproc;
    QStringList command;
    QString     server;
    QString     directory;
    bool        isRunning;
    QStringList outputLines;
    KProcess::OutputChannelMode commMode;
    KDevelop::IPlugin* vcsplugin;
    bool        failed;
};

DVCSjob::DVCSjob(KDevelop::IPlugin* parent)
    : VcsJob(parent), d(new DVCSjobPrivate)
{
    d->vcsplugin = parent;
}

DVCSjob::~DVCSjob()
{
    delete d;
}

void DVCSjob::clear()
{
    d->childproc->clearEnvironment();
    d->command.clear();
    d->outputLines.clear();
}

void DVCSjob::setServer(const QString& server)
{
    d->server = server;
}

void DVCSjob::setDirectory(const QString& directory)
{
    d->directory = directory;
}

void DVCSjob::setStandardInputFile(const QString &fileName)
{
    d->childproc->setStandardInputFile(fileName);
}

QString DVCSjob::getDirectory()
{
    return d->directory;
}

bool DVCSjob::isRunning() const
{
    return d->isRunning;
}

DVCSjob& DVCSjob::operator<<(const QString& arg)
{
    d->command.append( arg );
    return *this;
}

DVCSjob& DVCSjob::operator<<(const char* arg)
{
    d->command.append( arg );
    return *this;
}

DVCSjob& DVCSjob::operator<<(const QStringList& args)
{
    d->command << args;
    return *this;
}

QString DVCSjob::dvcsCommand() const
{
    return d->command.join(" ");
}

QString DVCSjob::output() const
{
    return d->outputLines.join("\n");
}

void DVCSjob::start()
{
    if( !d->directory.isEmpty() ) {
        kDebug(9509) << "Working directory:" << d->directory;
        d->childproc->setWorkingDirectory(d->directory);
    }

    connect(d->childproc, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(slotProcessExited(int, QProcess::ExitStatus)));
    connect(d->childproc, SIGNAL(error( QProcess::ProcessError )),
            SLOT(slotProcessError(QProcess::ProcessError)));

    connect(d->lineMaker, SIGNAL(receivedStdoutLines(const QStringList&)),
            SLOT(slotReceivedStdout(const QStringList&)));
    connect(d->lineMaker, SIGNAL(receivedStderrLines(const QStringList&)),
            SLOT(slotReceivedStderr(const QStringList&)) );

    kDebug(9509) << "Execute dvcs command:" << dvcsCommand();

    d->outputLines.clear();
    d->isRunning = true;
    d->childproc->setOutputChannelMode( d->commMode );
    d->childproc->setProgram( d->command );
    d->childproc->setEnvironment(QProcess::systemEnvironment());
    d->childproc->start();
}

void DVCSjob::setCommunicationMode(KProcess::OutputChannelMode comm)
{
    d->commMode = comm;
}

void DVCSjob::cancel()
{
    d->childproc->kill();
}

void DVCSjob::slotProcessError( QProcess::ProcessError err )
{
    Q_UNUSED(err)
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    d->failed = true;
    setError( d->childproc->exitCode() );
    setErrorText( i18n("Process exited with status %1", d->childproc->exitCode() ) );
    kDebug(9509) << "oops, found an error while running" << dvcsCommand() << ":" << err << d->childproc->exitCode();
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}

void DVCSjob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        d->failed = true;
        setError( exitCode );
        setErrorText( i18n("Process exited with status %1", exitCode) );
    }
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}

void DVCSjob::slotReceivedStdout(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug(9509)<<"received output:";
    kDebug(9509)<<output.join("\n");
}

void DVCSjob::slotReceivedStderr(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug(9509)<<"received error:";
    kDebug(9509)<<output.join("\n");
}

QVariant DVCSjob::fetchResults()
{
    return output();
}


KDevelop::VcsJob::JobStatus DVCSjob::status() const
{
    if (d->isRunning)
        return KDevelop::VcsJob::JobRunning;

    if (d->failed)
        return KDevelop::VcsJob::JobFailed;

    return KDevelop::VcsJob::JobSucceeded;
}

KDevelop::IPlugin* DVCSjob::vcsPlugin() const
{
    return d->vcsplugin;
}
