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

#include <KLocale>
#include <KDebug>
#include <KMessageBox>
#include <KProcess>
#include <KShell>

#include <util/processlinemaker.h>
#include <outputview/outputmodel.h>
#include <util/environmentgrouplist.h>
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

#include "cppcheckmodel.h"
#include "cppcheckparser.h"


#include "plugin.h"
#include "modelwrapper.h"

namespace cppcheck
{
/*!
 * Creates a model and a parser according to the specified name and
 * connects the 2 items
 */
class ModelParserFactoryPrivate
{

public:
    void make(const QString& type, cppcheck::Model*& m_model, cppcheck::Parser*& m_parser);

};

void ModelParserFactoryPrivate::make(const QString& tool, cppcheck::Model*& m_model, cppcheck::Parser*& m_parser)
{
    ModelWrapper* modelWrapper = 0;
    if (tool == "cppcheck") {
        m_model = new cppcheck::CppcheckModel();
        modelWrapper = new ModelWrapper(m_model);
        m_parser = new cppcheck::CppcheckParser();
        QObject::connect(m_parser, SIGNAL(newElement(cppcheck::Model::eElementType)),
                         modelWrapper, SLOT(newElement(cppcheck::Model::eElementType)));
        QObject::connect(m_parser, SIGNAL(newData(cppcheck::Model::eElementType, QString, QString, int, QString, QString, QString, QString, QString)),
                         modelWrapper, SLOT(newData(cppcheck::Model::eElementType, QString, QString, int, QString, QString, QString, QString, QString)));
    }

    m_model->setModelWrapper(modelWrapper);
    QObject::connect(m_parser, SIGNAL(reset()), modelWrapper, SLOT(reset()));
    m_model->reset();
}

Job::Job(cppcheck::Plugin* inst, bool allFiles, QObject* parent)
    : KDevelop::OutputJob(parent)
    , m_process(new KProcess(this))
    , m_pid(0)
    , m_model(0)
    , m_parser(0)
    , m_applicationOutput(new KDevelop::ProcessLineMaker(this))
    , m_plugin(inst)
    , m_killed(false)
    , allFiles(allFiles)
{
    setCapabilities(KJob::Killable);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    connect(m_process,  SIGNAL(readyReadStandardOutput()),
            SLOT(readyReadStandardOutput()));
    connect(m_process,  SIGNAL(readyReadStandardError()),
            SLOT(readyReadStandardError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)),
            SLOT(processErrored(QProcess::ProcessError)));

    // create the correct model for each tool
    QString tool = "cppcheck";
    ModelParserFactoryPrivate factory;

    factory.make(tool, m_model, m_parser);
    m_model->getModelWrapper()->job(this);
    m_plugin->incomingModel(m_model);

}

Job::~Job()
{
    doKill();
    if (m_model && m_model->getModelWrapper())
        m_model->getModelWrapper()->job(0);
    delete m_process;
    delete m_applicationOutput;
    delete m_parser;
}

void        Job::processModeArgs(QStringList& out,
                                 const t_cppcheck_cfg_argarray mode_args,
                                 int mode_args_count,
                                 KConfigGroup& cfg) const
{
    // For each option, set the right string in the arguments list
    for (int i = 0; i < mode_args_count; ++i) {
        QString val;
        QString argtype = mode_args[i][2];

        if (argtype == "str")
            val = cfg.readEntry(mode_args[i][0]);
        else if (argtype == "int") {
            int n = cfg.readEntry(mode_args[i][0], 0);
            if (n) {
                val.sprintf("%d", n);
            }
        } else if (argtype == "bool") {
            bool n = cfg.readEntry(mode_args[i][0], false);
            val = n ? "yes" : "no";
        } else if (argtype == "float") {
            int n = cfg.readEntry(mode_args[i][0], 1);
            val.sprintf("%d.0", n);
        }
        if (val.length()) {
            QString argument = QString(mode_args[i][1]) + val;
            out << argument;
        }
    }
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
    static const t_cppcheck_cfg_argarray generic_args = {
    };
    static const int        generic_args_count = sizeof(generic_args) / sizeof(*generic_args);

    KConfig config("kdevcppcheckrc");
    KConfigGroup cfg = config.group("cppcheck");

    QStringList     args;
    args.append(KShell::splitArgs(cfg.readEntry("Cppcheck Arguments", "")));

    args.append("--force");
    args.append("--xml-version=2");

    /* extra parameters */
    QString cppcheckParameters(cfg.readEntry("cppcheckParameters", ""));
    if (!cppcheckParameters.isEmpty())
        args.append(cppcheckParameters);

    bool additionalCheckStyle = cfg.readEntry("AdditionalCheckStyle", false);
    if (additionalCheckStyle)
        args.append("--enable=style");

    bool additionalCheckPerformance = cfg.readEntry("AdditionalCheckPerformance", false);
    if (additionalCheckPerformance)
        args.append("--enable=performance");


    bool additionalCheckPortability = cfg.readEntry("AdditionalCheckPortability", false);
    if (additionalCheckPortability)
        args.append("--enable=portability");
    bool additionalCheckInformation = cfg.readEntry("AdditionalCheckInformation", false);
    if (additionalCheckInformation)
        args.append("--enable=information");

    bool AdditionalCheckUnusedFunction = cfg.readEntry("AdditionalCheckUnusedFunction", false);
    if (AdditionalCheckUnusedFunction)
        args.append("--enable=unusedFunction");

    bool additionalCheckMissingInclude = cfg.readEntry("AdditionalCheckMissingInclude", false);
    if (additionalCheckMissingInclude)
        args.append("--enable=missingInclude");

    if (allFiles == false) {
        kDebug() << "checking file: " << m_plugin->core()->documentController()->activeDocument()->url().toLocalFile() << "(" << "" << ")";
        args.append(m_plugin->core()->documentController()->activeDocument()->url().toLocalFile());
    } else {
        kDebug() << "checking all files";
        // project path
        for (int i = 0; i < KDevelop::ICore::self()->projectController()->projects().count(); i++) {
            args.append(KDevelop::ICore::self()->projectController()->projects().at(i)->folder().toLocalFile());
        }
    }

    QString         tool = cfg.readEntry("Current Tool", "cppcheck");
    processModeArgs(args, generic_args, generic_args_count, cfg);

    addToolArgs(args, cfg);
    return args;
}

void Job::start()
{
    KConfig config("kdevcppcheckrc");
    KConfigGroup grp = config.group("cppcheck");    KDevelop::EnvironmentGroupList l(KGlobal::config());

    KUrl cppcheckExecutable(grp.readEntry("CppcheckExecutable", KUrl("/usr/bin/cppcheck")));

    QString err;

    QString envgrp = "";

    setStandardToolView(KDevelop::IOutputView::DebugView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setModel(new KDevelop::OutputModel());

    m_process->setEnvironment(l.createEnvironment(envgrp, m_process->systemEnvironment()));

    Q_ASSERT(m_process->state() != QProcess::Running);

    // some tools need to initialize stuff before the process starts
    beforeStart();

    QStringList cppcheckArgs;

    cppcheckArgs = buildCommandLine();
    kDebug() << "executing:" << cppcheckExecutable.toLocalFile() << cppcheckArgs;
    m_process->setProgram(cppcheckExecutable.toLocalFile(), cppcheckArgs);

    m_process->start();
    m_pid = m_process->pid();
    //setTitle(QString(i18n("job output (pid=%1)", m_pid)));
    setTitle(QString(i18n("Cppcheck output")));
    startOutput();
    connect(m_applicationOutput, SIGNAL(receivedStdoutLines(QStringList)),
            model(), SLOT(appendLines(QStringList)));
    connect(m_applicationOutput, SIGNAL(receivedStderrLines(QStringList)),
            model(), SLOT(appendLines(QStringList)));



    emit updateTabText(m_model, i18n("job running (pid=%1)",  m_pid));

    processStarted();
}

bool Job::doKill()
{
    if (m_process && m_process->pid()) {
        m_process->kill();
        m_killed = true;
        m_process = 0;
    }
    return true;
}



cppcheck::Plugin* Job::plugin() const
{
    return m_plugin;
}

void Job::processErrored(QProcess::ProcessError e)
{
    switch (e) {
    case QProcess::FailedToStart:
        KMessageBox::error(qApp->activeWindow(), i18n("Failed to start cppcheck from \"%1\".", m_process->property("executable").toString()), i18n("Failed to start Cppcheck"));
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
    kDebug() << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    QString tabname = i18n("cppcheck finished (pid=%1,exit=%2)", m_pid, exitCode);

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

        kDebug() << "cppcheck failed, output: ";
        kDebug() << stdout_output;
        kDebug() << "cppcheck failed, error output: ";
        kDebug() << stderr_output;

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

        emit updateTabText(m_model, tabname);
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

void Job::addToolArgs(QStringList& args, KConfigGroup& cfg) const
{
    static const t_cppcheck_cfg_argarray cppcheck_args = {
        {"Cppcheck Arguments", "", "str"},
    };
    static const int        count = sizeof(cppcheck_args) / sizeof(*cppcheck_args);

    processModeArgs(args, cppcheck_args, count, cfg);
}

/*
 * KProcessOutputToParser implementation
 */
KProcessOutputToParser::KProcessOutputToParser(Parser* parser)
{
    m_parser = parser;
    m_device = new QBuffer();
    m_device->open(QIODevice::WriteOnly);
    m_process = 0L;
}

KProcessOutputToParser::~KProcessOutputToParser()
{
    if (m_device != 0)
        delete m_device;
    if (m_process)
        delete m_process;
}

int KProcessOutputToParser::execute(QString execPath, QStringList args)
{
    m_process = new KProcess();
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->setProgram(execPath, args);
    QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(newDataFromStdOut()));
    QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processEnded(int, QProcess::ExitStatus)));
    //execute and wait the end of the program
    return m_process->execute();
}

void KProcessOutputToParser::processEnded(int , QProcess::ExitStatus status)
{
    kDebug() << status;
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


#include "job.moc"
