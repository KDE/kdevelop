/* This file is part of KDevelop
   Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
   Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
   Copyright 2011 Lionel Duc <lionel.data@gmail.com>
   Copyright 2011 Sebastien Rannou <mxs@sbrk.org>
   Copyright 2011 Lucas Sarie <lucas.sarie@gmail.com>
   Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
   Copyright 2002 Harald Fernengel <harry@kdevelop.org>
   Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "job.h"

#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QApplication>
#include <QBuffer>
#include <QFileInfo>

#include <KLocalizedString>
#include <KMessageBox>
#include <KShell>

#include <util/processlinemaker.h>
#include <outputview/outputmodel.h>
#include <util/environmentgrouplist.h>
#include <util/path.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <execute/iexecuteplugin.h>
#include <assert.h>

#include <QDir>

#include "cppcheckparser.h"

#include "debug.h"
#include "plugin.h"

namespace cppcheck
{


Job::Job(const Parameters &params, QObject* parent)
    : KDevelop::OutputJob(parent)
    , m_process(new QProcess(this))
    , m_pid(0)
    , m_parser(new cppcheck::CppcheckParser())
    , m_applicationOutput(new KDevelop::ProcessLineMaker(this))
    , m_killed(false)
    , parameters(params)
{
    setCapabilities(KJob::Killable);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_process,  SIGNAL(readyReadStandardOutput()),
            SLOT(readyReadStandardOutput()));
    connect(m_process,  SIGNAL(readyReadStandardError()),
            SLOT(readyReadStandardError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)),
            SLOT(processErrored(QProcess::ProcessError)));

}

Job::~Job()
{
    doKill();
    delete m_process;
    delete m_applicationOutput;
    delete m_parser;
}


void Job::readyReadStandardError()
{
    QString stderr_content_local = m_process->readAllStandardError();
    stderr_output += stderr_content_local;
    m_applicationOutput->slotReceivedStderr(stderr_content_local.toLocal8Bit());
}

void Job::readyReadStandardOutput()
{
    QString stdout_content_local = m_process->readAllStandardOutput();
    stdout_output += stdout_content_local;
    m_applicationOutput->slotReceivedStdout(stdout_content_local.toLocal8Bit());
}

QStringList Job::buildCommandLine() const
{
    QStringList     args;

    args.append("--force");
    args.append("--xml-version=2");

    /* extra parameters */
    QString cppcheckParameters(parameters.parameters);
    if (!cppcheckParameters.isEmpty())
        args.append(KShell::splitArgs(cppcheckParameters));

    if (parameters.checkStyle)
        args.append("--enable=style");

    if (parameters.checkPerformance)
        args.append("--enable=performance");


    if (parameters.checkPortability)
        args.append("--enable=portability");

    if (parameters.checkInformation)
        args.append("--enable=information");

    if (parameters.checkUnusedFunction)
        args.append("--enable=unusedFunction");

    if (parameters.checkMissingInclude)
        args.append("--enable=missingInclude");

    qCDebug(KDEV_CPPCHECK) << "checking paht" << parameters.path;
    args.append(parameters.path);

    return args;
}

void Job::start()
{
    QUrl cppcheckExecutable(QUrl::fromLocalFile(parameters.executable));

    QString envgrp = "";
    KDevelop::EnvironmentGroupList l(KSharedConfig::openConfig());

    setStandardToolView(KDevelop::IOutputView::DebugView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setModel(new KDevelop::OutputModel());

    m_process->setEnvironment(l.createEnvironment(envgrp, m_process->systemEnvironment()));

    Q_ASSERT(m_process->state() != QProcess::Running);

    // some tools need to initialize stuff before the process starts
    beforeStart();

    QStringList cppcheckArgs;

    cppcheckArgs = buildCommandLine();
    qCDebug(KDEV_CPPCHECK) << "executing:" << cppcheckExecutable.toLocalFile() << cppcheckArgs;
    m_process->setProgram(cppcheckExecutable.toLocalFile());
    m_process->setArguments(cppcheckArgs);

    m_process->start();
    m_pid = m_process->pid();
    //setTitle(QString(i18n("job output (pid=%1)", m_pid)));
    setTitle(QString(i18n("Cppcheck output")));
    startOutput();
    connect(m_applicationOutput, SIGNAL(receivedStdoutLines(QStringList)),
            model(), SLOT(appendLines(QStringList)));
    connect(m_applicationOutput, SIGNAL(receivedStderrLines(QStringList)),
            model(), SLOT(appendLines(QStringList)));



    processStarted();
}

bool Job::doKill()
{
    if (m_process && m_process->pid()) {
        m_process->kill();
        m_killed = true;
        m_process = nullptr;
    }
    return true;
}

QVector<KDevelop::IProblem::Ptr> Job::problems() const
{
    return m_parser->problems();
}

void Job::processErrored(QProcess::ProcessError e)
{
    switch (e) {
    case QProcess::FailedToStart:
        KMessageBox::error(qApp->activeWindow(), i18n("Failed to start Cppcheck from \"%1\".", m_process->property("executable").toString()), i18n("Failed to start Cppcheck"));
        break;
    case QProcess::Crashed:
        // if the process was killed by the user, the crash was expected
        // don't notify the user
        if (!m_killed)
            KMessageBox::error(qApp->activeWindow(), i18n("Cppcheck crashed."), i18n("Cppcheck Error"));
        break;
    case QProcess::Timedout:
        KMessageBox::error(qApp->activeWindow(), i18n("Cppcheck process timed out."), i18n("Cppcheck Error"));
        break;
    case QProcess::WriteError:
        KMessageBox::error(qApp->activeWindow(), i18n("Write to Cppcheck process failed."), i18n("Cppcheck Error"));
        break;
    case QProcess::ReadError:
        KMessageBox::error(qApp->activeWindow(), i18n("Read from Cppcheck process failed."), i18n("Cppcheck Error"));
        break;
    case QProcess::UnknownError:
        KMessageBox::error(qApp->activeWindow(), i18n("Unknown Cppcheck process error."), i18n("Cppcheck Error"));
        break;
    }
}

void Job::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KDEV_CPPCHECK) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    QString tabname = i18n("Cppcheck finished (pid=%1,exit=%2)", m_pid, exitCode);

    if (exitCode != 0) {
        /*
        ** Here, check if Cppcheck failed (because of bad parameters or whatever).
        ** Because Cppcheck always returns 1 on failure, and the profiled application's return
        ** on success, we cannot know for sure which process returned != 0.
        **
        ** The only way to guess that it is Cppcheck which failed is to check stderr and look for
        ** "cppcheck:" at the beginning of each line, even though it can still be the profiled
        ** process that writes it on stderr. It is, however, unlikely enough to be reliable in
        ** most cases.
        */

        qCDebug(KDEV_CPPCHECK) << "cppcheck failed, output: ";
        qCDebug(KDEV_CPPCHECK) << stdout_output;
        qCDebug(KDEV_CPPCHECK) << "cppcheck failed, error output: ";
        qCDebug(KDEV_CPPCHECK) << stderr_output;

        const QString& s = stdout_output;
        QStringList err = s.split("\n");
        tabname = i18n("job failed");
        model()->appendLines(err);
    } else {
        // success

        string_device = new QBuffer();
        string_device->open(QIODevice::ReadWrite);
        string_device->write(stdout_output.toLocal8Bit());
        m_parser->addData(stderr_output);
        m_parser->parse();
        string_device->close();

    }
    processEnded();
    emitResult();
    emit jobFinished();
}

void Job::beforeStart()
{
}

void Job::processStarted()
{
}

void Job::processEnded()
{
}

KDevelop::OutputModel* Job::model()
{
    return dynamic_cast<KDevelop::OutputModel*>(KDevelop::OutputJob::model());
}

/*
 * KProcessOutputToParser implementation
 */
KProcessOutputToParser::KProcessOutputToParser(Parser* parser)
{
    m_parser = parser;
    m_device = new QBuffer();
    m_device->open(QIODevice::WriteOnly);
    m_process = nullptr;
}

KProcessOutputToParser::~KProcessOutputToParser()
{
    if (m_device != nullptr)
        delete m_device;
    if (m_process)
        delete m_process;
}

bool KProcessOutputToParser::execute(QString execPath, QStringList args)
{
    m_process = new QProcess();
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    m_process->setProgram(execPath);
    m_process->setArguments(args);
    QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(newDataFromStdOut()));
    QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processEnded(int, QProcess::ExitStatus)));
    //execute and wait the end of the program
    m_process->start();
    return m_process->waitForFinished();
}

void KProcessOutputToParser::processEnded(int , QProcess::ExitStatus status)
{
    qCDebug(KDEV_CPPCHECK) << status;
    if (status == QProcess::NormalExit) {

        m_device->close();
    }
    delete m_process;
}

void KProcessOutputToParser::newDataFromStdOut()
{
    m_device->write(m_process->readAllStandardOutput());
}


}
