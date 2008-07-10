/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Adapted for Hg                                                        *
 *   Copyright 2008 Tom Burdick <thomas.burdick@gmail.com>                 *
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

#include "hgjob.h"

#include <QFile>
#include <QList>

#include <QStringList>
#include <KDebug>
#include <KLocale>

#include "processlinemaker.h"
#include <iplugin.h>

struct HgJob::Private
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
    QString     directory;
    bool        isRunning;
    QStringList outputLines;
    KProcess::OutputChannelMode commMode;
    KDevelop::IPlugin* vcsplugin;
};

HgJob::HgJob(KDevelop::IPlugin* parent)
    : VcsJob(parent), d(new Private)
{
    d->vcsplugin = parent;
}

HgJob::~HgJob()
{
    delete d;
}

void HgJob::clear()
{
    d->childproc->clearEnvironment();
    d->command.clear();
    d->outputLines.clear();
}

void HgJob::setServer(const QString& server)
{
    d->server = server;
}

void HgJob::setDirectory(const QString& directory)
{
    d->directory = directory;
}

QString HgJob::getDirectory()
{
    return d->directory;
}

bool HgJob::isRunning() const
{
    return d->isRunning;
}

HgJob& HgJob::operator<<(const QString& arg)
{
    d->command.append( arg );
    return *this;
}

HgJob& HgJob::operator<<(const char* arg)
{
    d->command.append( arg );
    return *this;
}

HgJob& HgJob::operator<<(const QStringList& args)
{
    d->command.append( args.join(" ") );
    return *this;
}

QString HgJob::hgCommand() const
{
    return d->command.join(" ");
}

QString HgJob::output() const
{
    return d->outputLines.join("\n");
}

void HgJob::start()
{
    if( !d->directory.isEmpty() ) {
        kDebug(9500) << "Working directory:" << d->directory;
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

    kDebug(9500) << "Execute hg command:" << hgCommand();

    d->outputLines.clear();
    d->isRunning = true;
    d->childproc->setOutputChannelMode( d->commMode );
    d->childproc->setProgram( d->command );
    d->childproc->start();
}

void HgJob::setCommunicationMode(KProcess::OutputChannelMode comm)
{
    d->commMode = comm;
}

void HgJob::cancel()
{
    d->childproc->kill();
}

void HgJob::slotProcessError( QProcess::ProcessError err )
{
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    setError( d->childproc->exitCode() );
    setErrorText( i18n("Process exited with status %1", d->childproc->exitCode() ) );
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}

void HgJob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
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

void HgJob::slotReceivedStdout(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug(9500)<<"received output:";
    kDebug(9500)<<output.join("\n");
}

void HgJob::slotReceivedStderr(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug(9500)<<"received error:";
    kDebug(9500)<<output.join("\n");
}

QVariant HgJob::fetchResults()
{
    return output();
}


KDevelop::VcsJob::JobStatus HgJob::status() const
{
    if (d->isRunning)
        return KDevelop::VcsJob::JobRunning;

    if (d->childproc->exitCode() != 0)
        return KDevelop::VcsJob::JobFailed;

    return KDevelop::VcsJob::JobSucceeded;
}

KDevelop::IPlugin* HgJob::vcsPlugin() const
{
    return d->vcsplugin;
}

#include "hgjob.moc"
