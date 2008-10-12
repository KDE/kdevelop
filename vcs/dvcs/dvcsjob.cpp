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

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QStringList>

#include <KDE/KDebug>
#include <KDE/KLocale>

#include <util/processlinemaker.h>
#include <interfaces/iplugin.h>

struct DVCSjobPrivate
{
    DVCSjobPrivate() : commMode(KProcess::SeparateChannels), vcsplugin(0)
    {
        childproc = new KProcess;
        lineMaker = new KDevelop::ProcessLineMaker( childproc );
        isRunning = failed = wasStarted = false;
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
    bool        wasStarted;
    bool        failed;
    QStringList outputLines;
    KProcess::OutputChannelMode commMode;
    KDevelop::IPlugin* vcsplugin;
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
    //Do not use KProcess::clearEnvironment() (it sets the environment to kde_dummy.
    //Also DVCSjob can't set it, so it's ok.
    d->command.clear();
    d->outputLines.clear();
    d->server.clear();
    d->directory.clear();
    d->isRunning = d->failed = d->wasStarted = false;
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

void DVCSjob::setResults(const QVariant &res) 
{
    results = res;
}

QVariant DVCSjob::fetchResults()
{
    return results;
}

void DVCSjob::setExitStatus(const bool exitStatus)
{
    d->failed = exitStatus;
}

void DVCSjob::start()
{
    Q_ASSERT_X(!d->isRunning, "DVCSjob::start", "Another proccess was started using this job class");
    d->wasStarted = true;

    //do not allow to run commands in the application's working dir
    //TODO: change directory to KUrl, check if it's a relative path
    if(d->directory.isEmpty() ) 
    {
        kDebug() << "No working directory specified for DVCS command";
        slotProcessError(QProcess::UnknownError);
        return;
    }

    kDebug() << "Working directory:" << d->directory;
    d->childproc->setWorkingDirectory(d->directory);


    connect(d->childproc, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(slotProcessExited(int, QProcess::ExitStatus)));
    connect(d->childproc, SIGNAL(error( QProcess::ProcessError )),
            SLOT(slotProcessError(QProcess::ProcessError)));

    connect(d->lineMaker, SIGNAL(receivedStdoutLines(const QStringList&)),
            SLOT(slotReceivedStdout(const QStringList&)));
    connect(d->lineMaker, SIGNAL(receivedStderrLines(const QStringList&)),
            SLOT(slotReceivedStderr(const QStringList&)) );

    kDebug() << "Execute dvcs command:" << dvcsCommand();

    d->outputLines.clear();
    d->isRunning = true;
    d->childproc->setOutputChannelMode( d->commMode );
    d->childproc->setProgram( d->command );
    d->childproc->setEnvironment(QProcess::systemEnvironment());
    //the started() and error() signals may be delayed! It causes crash with deferred deletion!!!
    d->childproc->waitForStarted();
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
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    //NOTE: some DVCS commands can use stderr...
    d->failed = true;

    //Do not use d->childproc->exitCode() to set an error! If we have FailedToStart exitCode will return 0,
    //and if exec is used, exec will return true and thet is wrong!
    setError(UserDefinedError);
    setErrorText( i18n("Process exited with status %1", d->childproc->exitCode() ) );

    QString errorValue;
    //if trolls add Q_ENUMS for QProcess, then we can use better solution than switch:
    //QMetaObject::indexOfEnumerator(char*), QLatin1String(QMetaEnum::valueToKey())...
    switch (err)
    {
    case QProcess::FailedToStart:
        errorValue = "FailedToStart";
        break;
    case QProcess::Crashed:
        errorValue = "Crashed";
        break;
    case QProcess::Timedout:
        errorValue = "Timedout";
        break;
    case QProcess::WriteError:
        errorValue = "WriteErro";
        break;
    case QProcess::ReadError:
        errorValue = "ReadError";
        break;
    case QProcess::UnknownError:
        errorValue = "UnknownError";
        break;
    }
    kDebug() << "oops, found an error while running" << dvcsCommand() << ":" << errorValue 
                                                     << "Exit code is:" << d->childproc->exitCode();
    jobIsReady();
}

void DVCSjob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    if (exitStatus != QProcess::NormalExit || exitCode != 0)
        slotProcessError(QProcess::UnknownError);

    kDebug() << "process has finished with no errors";
    jobIsReady();
}

void DVCSjob::slotReceivedStdout(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug()<<"received output:";
    kDebug()<<output.join("\n");
}

void DVCSjob::slotReceivedStderr(const QStringList& output)
{
    // accumulate output
    d->outputLines += output;

    kDebug()<<"received error:";
    kDebug()<<output.join("\n");
}

KDevelop::VcsJob::JobStatus DVCSjob::status() const
{
    if (!d->wasStarted)
        return KDevelop::VcsJob::JobNotStarted;
    if (d->failed)
        return KDevelop::VcsJob::JobFailed;
    if (d->isRunning)
        return KDevelop::VcsJob::JobRunning;
    return KDevelop::VcsJob::JobSucceeded;
}

KDevelop::IPlugin* DVCSjob::vcsPlugin() const
{
    return d->vcsplugin;
}

void DVCSjob::jobIsReady()
{
    emit readyForParsing(this); //let parsers to set status
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}

KProcess* DVCSjob::getChildproc() {return d->childproc;}
