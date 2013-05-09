/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS                                                      *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright Aleix Pol Gonzalez <aleixpol@kde.org>                       *
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
#include <QtCore/QDir>
#include <QtGui/QStandardItemModel>

#include <KDE/KDebug>
#include <KDE/KLocale>
#include <KDE/KUrl>

#include <interfaces/iplugin.h>
#include <outputview/outputmodel.h>

using namespace KDevelop;

struct DVcsJobPrivate
{
    DVcsJobPrivate() : childproc(new KProcess), vcsplugin(0)
    {}

    ~DVcsJobPrivate() {
        delete childproc;
    }

    KProcess*   childproc;
    VcsJob::JobStatus status;
    QByteArray  output;
    QByteArray  errorOutput;
    IPlugin* vcsplugin;
    
    QVariant results;
    OutputModel* model;
};

DVcsJob::DVcsJob(const QDir& workingDir, IPlugin* parent, OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity), d(new DVcsJobPrivate)
{
    Q_ASSERT(workingDir.exists());
    d->status = JobNotStarted;
    d->vcsplugin = parent;
    d->childproc->setWorkingDirectory(workingDir.absolutePath());
    d->model = new OutputModel;
    setModel(d->model);
    setCapabilities(Killable);
    
    connect(d->childproc, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(slotProcessExited(int,QProcess::ExitStatus)));
    connect(d->childproc, SIGNAL(error(QProcess::ProcessError)),
            SLOT(slotProcessError(QProcess::ProcessError)));

    connect(d->childproc, SIGNAL(readyReadStandardOutput()),
                SLOT(slotReceivedStdout()));
    
}

DVcsJob::~DVcsJob()
{
    delete d;
}

QDir DVcsJob::directory() const
{
    return QDir(d->childproc->workingDirectory());
}

DVcsJob& DVcsJob::operator<<(const QString& arg)
{
    *d->childproc << arg;
    return *this;
}

DVcsJob& DVcsJob::operator<<(const char* arg)
{
    *d->childproc << arg;
    return *this;
}

DVcsJob& DVcsJob::operator<<(const QStringList& args)
{
    *d->childproc << args;
    return *this;
}

QStringList DVcsJob::dvcsCommand() const
{
    return d->childproc->program();
}

QString DVcsJob::output() const
{
    QByteArray stdoutbuf = rawOutput();
    int endpos = stdoutbuf.size();
    if (d->status==JobRunning) {    // We may have received only part of a code-point. apol: ASSERT?
        endpos = stdoutbuf.lastIndexOf('\n')+1; // Include the final newline or become 0, when there is no newline
    }

    return QString::fromLocal8Bit(stdoutbuf, endpos);
}

QByteArray DVcsJob::rawOutput() const
{
    return d->output;
}

QByteArray DVcsJob::errorOutput() const
{
    return d->errorOutput;
}

void DVcsJob::setResults(const QVariant &res)
{
    d->results = res;
}

QVariant DVcsJob::fetchResults()
{
    return d->results;
}

void DVcsJob::start()
{
    Q_ASSERT_X(!d->status==JobRunning, "DVCSjob::start", "Another proccess was started using this job class");

    const QDir& workingdir = directory();
    if( !workingdir.exists() ) {
        QString error = i18n( "Working Directory does not exist: %1", d->childproc->workingDirectory() );
        d->model->appendLine(error);
        setError( 255 );
        setErrorText(error);
        d->status = JobFailed;
        emitResult();
        return;
    }
    if( !workingdir.isAbsolute() ) {
        QString error = i18n( "Working Directory is not absolute: %1", d->childproc->workingDirectory() );
        d->model->appendLine(error);
        setError( 255 );
        setErrorText(error);
        d->status = JobFailed;
        emitResult();
        return;
    }

    kDebug() << "Execute dvcs command:" << dvcsCommand();

    QString service;
    if(d->vcsplugin)
        service = d->vcsplugin->objectName();
    setObjectName(service+": "+dvcsCommand().join(" "));
    
    d->status = JobRunning;
    d->childproc->setOutputChannelMode(KProcess::SeparateChannels);
    //the started() and error() signals may be delayed! It causes crash with deferred deletion!!!
    d->childproc->start();
    
    d->model->appendLine(directory().path() + "> " + dvcsCommand().join(" "));
}

void DVcsJob::setCommunicationMode(KProcess::OutputChannelMode comm)
{
    d->childproc->setOutputChannelMode(comm);
}

void DVcsJob::cancel()
{
    d->childproc->kill();
}

void DVcsJob::slotProcessError( QProcess::ProcessError err )
{
    d->status = JobFailed;

    //Do not use d->childproc->exitCode() to set an error! If we have FailedToStart exitCode will return 0,
    //and if exec is used, exec will return true and that is wrong!
    setError(verbosity()==Verbose ? OutputJob::FailedShownError : UserDefinedError);
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
        errorValue = "WriteError";
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
    
    d->errorOutput = d->childproc->readAllStandardError();
    displayOutput(QString::fromLocal8Bit(d->errorOutput));
    d->model->appendLine(i18n("Command finished with error %1.", errorValue));
    
    //Even if it was a silent process we want to provide some feedback to the user about what went wrong
    //so we show the output then.
    if(verbosity()==Silent)
        startOutput();
    emitResult();
}

void DVcsJob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    d->status = JobSucceeded;

    if (exitStatus == QProcess::CrashExit)
        slotProcessError(QProcess::Crashed);

    else if (exitCode != 0)
        slotProcessError(QProcess::UnknownError);

    d->model->appendLine(i18n("Command exited with value %1.", exitCode));
    jobIsReady();
}

void DVcsJob::displayOutput(const QString& data)
{
    d->model->appendLines(data.split('\n'));
}

void DVcsJob::slotReceivedStdout()
{
    QByteArray output = d->childproc->readAllStandardOutput();
    
    // accumulate output
    d->output.append(output);
    
    displayOutput(QString::fromLocal8Bit(output));
}

VcsJob::JobStatus DVcsJob::status() const
{
    return d->status;
}

IPlugin* DVcsJob::vcsPlugin() const
{
    return d->vcsplugin;
}

DVcsJob& DVcsJob::operator<<(const KUrl& url)
{
    *d->childproc << url.toLocalFile();
    return *this;
}

DVcsJob& DVcsJob::operator<<(const QList< KUrl >& urls)
{
    foreach(const KUrl &url, urls)
        operator<<(url);
    return *this;
}

bool DVcsJob::doKill()
{
    if (d->childproc->state() == QProcess::NotRunning) {
        return true;
    }

    static const int terminateKillTimeout = 1000; // ms
    d->childproc->terminate();
    bool terminated = d->childproc->waitForFinished( terminateKillTimeout );
    if( !terminated ) {
        d->childproc->kill();
        terminated = d->childproc->waitForFinished( terminateKillTimeout );
    }
    return terminated;
}

void DVcsJob::jobIsReady()
{
    emit readyForParsing(this); //let parsers to set status
    emitResult(); //KJob
    emit resultsReady(this); //VcsJob
}

KProcess* DVcsJob::process() {return d->childproc;}
