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
#include <QtCore/QDir>
#include <QtGui/QStandardItemModel>

#include <KDE/KDebug>
#include <KDE/KLocale>

#include <interfaces/iplugin.h>

struct DVcsJobPrivate
{
    DVcsJobPrivate() : childproc(new KProcess), commMode(KProcess::SeparateChannels), vcsplugin(0)
    {
        isRunning = wasStarted = false;
    }

    ~DVcsJobPrivate() {
        delete childproc;
    }

    KProcess*   childproc;
    bool        isRunning;
    bool        wasStarted;
    QByteArray  output;
    KProcess::OutputChannelMode commMode;
    KDevelop::IPlugin* vcsplugin;
    
    QVariant results;
};

DVcsJob::DVcsJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity), d(new DVcsJobPrivate)
{
    d->vcsplugin = parent;

    connect(d->childproc, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(slotProcessExited(int, QProcess::ExitStatus)));
    connect(d->childproc, SIGNAL(error( QProcess::ProcessError )),
            SLOT(slotProcessError(QProcess::ProcessError)));

    connect(d->childproc, SIGNAL(readyReadStandardOutput()),
                SLOT(slotReceivedStdout()));
}

DVcsJob::~DVcsJob()
{
    delete d;
}

void DVcsJob::setDirectory(const QDir& directory)
{
    const QString workingDirectory = directory.absolutePath();
    kDebug() << "Working directory:" << workingDirectory;
    d->childproc->setWorkingDirectory(workingDirectory);
}

QDir DVcsJob::getDirectory() const
{
    return QDir(d->childproc->workingDirectory());
}

bool DVcsJob::isRunning() const
{
    return d->isRunning;
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
    d->results = res;
}

QVariant DVcsJob::fetchResults()
{
    return d->results;
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

    kDebug() << "Execute dvcs command:" << dvcsCommand();

    d->output.clear();
    d->isRunning = true;
    d->childproc->setOutputChannelMode( d->commMode );
    d->childproc->setEnvironment(QProcess::systemEnvironment());
    //the started() and error() signals may be delayed! It causes crash with deferred deletion!!!
    d->childproc->start();
    
    displayOutput(getDirectory().path() + "> " + dvcsCommand().join(" "));
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
    d->isRunning = false;

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
    
    displayOutput(i18n("Command finnished with error %1.", errorValue));
    jobIsReady();
}

void DVcsJob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    d->isRunning = false;

    if (exitStatus != QProcess::NormalExit || exitCode != 0)
        slotProcessError(QProcess::UnknownError);

    displayOutput(i18n("Command exited with value %1.", exitCode));
    jobIsReady();
}

void DVcsJob::displayOutput(const QString& data)
{
    static_cast<QStandardItemModel*>(model())->appendRow(new QStandardItem(data));
}

void DVcsJob::slotReceivedStdout()
{
    QByteArray output = d->childproc->readAllStandardOutput();
    
    // accumulate output
    d->output.append(output);
    
    displayOutput(output);
}

KDevelop::VcsJob::JobStatus DVcsJob::status() const
{
    if (!d->wasStarted)
        return KDevelop::VcsJob::JobNotStarted;
    if (d->isRunning)
        return KDevelop::VcsJob::JobRunning;
    if(error()!=0)
        return KDevelop::VcsJob::JobFailed;
    
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
