/*
    SPDX-FileCopyrightText: 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
    SPDX-FileCopyrightText: 2011 Damien Coppel <damien.coppel@gmail.com>
    SPDX-FileCopyrightText: 2011 Lionel Duc <lionel.data@gmail.com>
    SPDX-FileCopyrightText: 2011 Sebastien Rannou <mxs@sbrk.org>
    SPDX-FileCopyrightText: 2011 Lucas Sarie <lucas.sarie@gmail.com>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2002 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2013 Christoph Thielecke <crissi99@gmx.de>
    SPDX-FileCopyrightText: 2016-2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "job.h"

#include "debug.h"
#include "parser.h"
#include "utils.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
#include <shell/problem.h>
// KF
#include <KLocalizedString>
// Qt
#include <QApplication>
#include <QElapsedTimer>
#include <QRegularExpression>

namespace cppcheck
{

Job::Job(const Parameters& params, QObject* parent)
    : KDevelop::OutputExecuteJob(parent)
    , m_timer(new QElapsedTimer)
    , m_parser(new CppcheckParser)
    , m_showXmlOutput(params.showXmlOutput)
    , m_projectRootPath(params.projectRootPath())
{
    setJobName(i18n("Cppcheck Analysis (%1)", prettyPathName(params.checkPath)));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);

    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput);

    *this << params.commandLine();
    qCDebug(KDEV_CPPCHECK) << "checking path" << params.checkPath;
}

Job::~Job()
{
    doKill();
}

void Job::postProcessStdout(const QStringList& lines)
{
    static const auto fileNameRegex = QRegularExpression(QStringLiteral("Checking ([^:]*)\\.{3}"));
    static const auto percentRegex  = QRegularExpression(QStringLiteral("(\\d+)% done"));

    QRegularExpressionMatch match;

    for (const QString& line : lines) {
        match = fileNameRegex.match(line);
        if (match.hasMatch()) {
            emit infoMessage(this, match.captured(1));
            continue;
        }

        match = percentRegex.match(line);
        if (match.hasMatch()) {
            setPercent(match.capturedView(1).toULong());
            continue;
        }
    }

    m_standardOutput << lines;

    if (status() == KDevelop::OutputExecuteJob::JobStatus::JobRunning) {
        KDevelop::OutputExecuteJob::postProcessStdout(lines);
    }
}

void Job::postProcessStderr(const QStringList& lines)
{
    static const auto xmlStartRegex = QRegularExpression(QStringLiteral("\\s*<"));

    for (const QString & line : lines) {
        // unfortunately sometime cppcheck send non-XML messages to stderr.
        // For example, if we pass '-I /missing_include_dir' to the argument list,
        // then stderr output will contains such line (tested on cppcheck 1.72):
        //
        // (information) Couldn't find path given by -I '/missing_include_dir'
        //
        // Therefore we must 'move' such messages to m_standardOutput.

        if (line.indexOf(xmlStartRegex) != -1) { // the line contains XML
            m_xmlOutput << line;

            m_parser->addData(line);

            m_problems = m_parser->parse();
            emitProblems();
        }
        else {
            KDevelop::IProblem::Ptr problem(new KDevelop::DetectedProblem(i18n("Cppcheck")));

            problem->setSeverity(KDevelop::IProblem::Error);
            problem->setDescription(line);
            problem->setExplanation(QStringLiteral("Check your cppcheck settings"));

            m_problems = {problem};
            emitProblems();

            if (m_showXmlOutput) {
                m_standardOutput << line;
            } else {
                postProcessStdout({line});
            }
        }
    }

    if (status() == KDevelop::OutputExecuteJob::JobStatus::JobRunning && m_showXmlOutput) {
        KDevelop::OutputExecuteJob::postProcessStderr(lines);
    }
}

void Job::start()
{
    m_standardOutput.clear();
    m_xmlOutput.clear();

    qCDebug(KDEV_CPPCHECK) << "executing:" << commandLine().join(QLatin1Char(' '));

    m_timer->restart();
    KDevelop::OutputExecuteJob::start();
}

void Job::childProcessError(QProcess::ProcessError e)
{
    QString messageText;

    switch (e) {
    case QProcess::FailedToStart:
        messageText = i18n("Failed to start Cppcheck from \"%1\".", commandLine()[0]);
        break;

    case QProcess::Crashed:
        if (status() != KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {
            messageText = i18n("Cppcheck crashed.");
        }
        break;

    case QProcess::Timedout:
        messageText = i18n("Cppcheck process timed out.");
        break;

    case QProcess::WriteError:
        messageText = i18n("Write to Cppcheck process failed.");
        break;

    case QProcess::ReadError:
        messageText = i18n("Read from Cppcheck process failed.");
        break;

    case QProcess::UnknownError:
        // current cppcheck errors will be displayed in the output view
        // don't notify the user
        break;
    }

    if (!messageText.isEmpty()) {
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        KDevelop::ICore::self()->uiController()->postMessage(message);
    }

    KDevelop::OutputExecuteJob::childProcessError(e);
}

void Job::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KDEV_CPPCHECK) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    postProcessStdout({QStringLiteral("Elapsed time: %1 s.").arg(m_timer->elapsed()/1000.0)});

    if (exitCode != 0) {
        qCDebug(KDEV_CPPCHECK) << "cppcheck failed, standard output: ";
        qCDebug(KDEV_CPPCHECK) << m_standardOutput.join(QLatin1Char('\n'));
        qCDebug(KDEV_CPPCHECK) << "cppcheck failed, XML output: ";
        qCDebug(KDEV_CPPCHECK) << m_xmlOutput.join(QLatin1Char('\n'));
    }

    KDevelop::OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}

void Job::emitProblems()
{
    if (!m_problems.isEmpty()) {
        emit problemsDetected(m_problems);
    }
}

}

#include "moc_job.cpp"
