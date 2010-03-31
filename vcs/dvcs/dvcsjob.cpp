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

#include <interfaces/iplugin.h>
#include <QDir>

struct DVcsJobPrivate
{
    DVcsJobPrivate() : childproc(new KProcess), commMode(KProcess::SeparateChannels), vcsplugin(0)
    {
        isRunning = failed = wasStarted = false;
    }

    ~DVcsJobPrivate() {
        delete childproc;
    }

    KProcess*   childproc;
    QStringList command;
    QString     server;
    QDir        directory;
    bool        isRunning;
    bool        wasStarted;
    bool        failed;
    QByteArray  output;
    KProcess::OutputChannelMode commMode;
    KDevelop::IPlugin* vcsplugin;
};

DVcsJob::DVcsJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity), d(new DVcsJobPrivate)
{
    d->vcsplugin = parent;
}

DVcsJob::~DVcsJob()
{
    delete d;
}

void DVcsJob::clear()
{
    //Do not use KProcess::clearEnvironment() (it sets the environment to kde_dummy).
    //Also DVCSjob can't set it, so it's ok.
    d->command.clear();
    d->output.clear();
    d->server.clear();
    d->directory = QDir::temp();
    d->isRunning = d->failed = d->wasStarted = false;
}

void DVcsJob::setServer(const QString& server)
{
    d->server = server;
}

void DVcsJob::setDirectory(const QDir& directory)
{
    d->directory = directory;
}

void DVcsJob::setStandardInputFile(const QString &fileName)
{
    d->childproc->setStandardInputFile(fileName);
}

const QDir & DVcsJob::getDirectory() const
{
    return d->directory;
}

bool DVcsJob::isRunning() const
{
    return d->isRunning;
}

DVcsJob& DVcsJob::operator<<(const QString& arg)
{
    d->command.append( arg );
    return *this;
}

DVcsJob& DVcsJob::operator<<(const char* arg)
{
    d->command.append( arg );
    return *this;
}

DVcsJob& DVcsJob::operator<<(const QStringList& args)
{
    d->command << args;
    return *this;
}

QString DVcsJob::dvcsCommand() const
{
    return d->command.join(" ");
}

QString DVcsJob::output() const
{
    QByteArray stdoutbuf = rawOutput();
    int endpos = stdoutbuf.size();
    if (isRunning()) {    // We may have received only part of a code-point
        endpos = stdoutbuf.lastIndexOf('\n')+1; // Include the final newline or become 0, when there is no newline
    }

    return QString::fromLocal8Bit(stdoutbuf, endpos);
}

QByteArray DVcsJob::rawOutput() const
{
    return d->output;
}

void DVcsJob::setResults(const QVariant &res)
{
    results = res;
}

QVariant DVcsJob::fetchResults()
{
    return results;
}

void DVcsJob::setExitStatus(const bool exitStatus)
{
    d->failed = exitStatus;
}

void DVcsJob::start()
{
    Q_ASSERT_X(!d->isRunning, "DVCSjob::start", "Another proccess was started using this job class");
    d->wasStarted = true;

#if 0
    //do not allow to run commands in the application's working dir
    //TODO: change directory to KUrl, check if it's a relative path
    if(d->directory.isEmpty() ) 
    {
        kDebug() << "No working directory specified for DVCS command";
        slotProcessError(QProcess::UnknownError);
        return;
    }
#endif
    const QString workingDirectory = d->directory.absolutePath();
    kDebug() << "Working directory:" << workingDirectory;
    d->childproc->setWorkingDirectory(workingDirectory);


    connect(d->childproc, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(slotProcessExited(int, QProcess::ExitStatus)));
    connect(d->childproc, SIGNAL(error( QProcess::ProcessError )),
            SLOT(slotProcessError(QProcess::ProcessError)));

    connect(d->childproc, SIGNAL(readyReadStandardError()),
                SLOT(slotReceivedStderr()));
    connect(d->childproc, SIGNAL(readyReadStandardOutput()),
                SLOT(slotReceivedStdout()));

    kDebug() << "Execute dvcs command:" << dvcsCommand();

    d->output.clear();
    d->isRunning = true;
    d->childproc->setOutputChannelMode( d->commMode );
    d->childproc->setProgram( d->command );
    d->childproc->setEnvironment(QProcess::systemEnvironment());
    //the started() and error() signals may be delayed! It causes crash with deferred deletion!!!
    d->childproc->waitForStarted();
    d->childproc->start();
}

void DVcsJob::setCommunicationMode(KProcess::OutputChannelMode comm)
{
    d->commMode = comm;
}

void DVcsJob::cancel()
{
    d->childproc->kill();
}

void DVcsJob::slotProcessError( QProcess::ProcessError err )
{
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    //NOTE: some DVCS commands can use stderr...
    d->failed = true;

    //Do not use d->childproc->exitCode() to set an error! If we have FailedToStart exitCode will return 0,
    //and if exec is used, exec will return true and that is wrong!
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

void DVcsJob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    // disconnect all connections to childproc's signals; they are no longer needed
    d->childproc->disconnect();

    d->isRunning = false;

    if (exitStatus != QProcess::NormalExit || exitCode != 0)
        slotProcessError(QProcess::UnknownError);

    kDebug() << "process has finished with no errors";
    jobIsReady();
}

void DVcsJob::slotReceivedStdout()
{
    // accumulate output
    d->output.append(d->childproc->readAllStandardOutput());
}

void DVcsJob::slotReceivedStderr()
{
    // accumulate output
    d->output.append(d->childproc->readAllStandardError());
}

KDevelop::VcsJob::JobStatus DVcsJob::status() const
{
    if (!d->wasStarted)
        return KDevelop::VcsJob::JobNotStarted;
    if (d->failed)
        return KDevelop::VcsJob::JobFailed;
    if (d->isRunning)
        return KDevelop::VcsJob::JobRunning;
    return KDevelop::VcsJob::JobSucceeded;
}

KDevelop::IPlugin* DVcsJob::vcsPlugin() const
{
    return d->vcsplugin;
}

void DVcsJob::jobIsReady()
{
    emit readyForParsing(this); //let parsers to set status
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}

KProcess* DVcsJob::getChildproc() {return d->childproc;}
