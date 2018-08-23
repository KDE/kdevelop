/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "job.h"

#include "checksdb.h"
#include "debug.h"
#include "globalsettings.h"
#include "plugin.h"
#include "utils.h"

#include <language/editor/documentrange.h>
#include <shell/problem.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThread>

namespace Clazy
{

Job::Job()
    : KDevelop::OutputExecuteJob(nullptr)
    , m_db(nullptr)
    , m_timer(nullptr)
{
}

Job::Job(const JobParameters& params, QSharedPointer<const ChecksDB> db)
    : KDevelop::OutputExecuteJob(nullptr)
    , m_db(db)
    , m_timer(new QElapsedTimer)
{
    setJobName(i18n("Clazy Analysis (%1)", prettyPathName(params.checkPath())));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll);

    setProperties(OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(OutputExecuteJob::JobProperty::PostProcessOutput);

    *this << QStringLiteral("make");

    if (GlobalSettings::parallelJobsEnabled()) {
        const int threadsCount =
            GlobalSettings::parallelJobsAutoCount() ?
            QThread::idealThreadCount() :
            GlobalSettings::parallelJobsFixedCount();

        *this << QStringLiteral("-j%1").arg(threadsCount);
    }

    *this << QStringLiteral("-f");
    *this << buildMakefile(params);
}

Job::~Job()
{
    doKill();
}

inline QString spaceEscapedString(const QString& s)
{
    return QString(s).replace(QLatin1Char(' '), QLatin1String("\\ "));
}

QString Job::buildMakefile(const JobParameters& params)
{
    const auto makefilePath = QStringLiteral("%1/kdevclazy.makefile").arg(params.projectBuildPath());

    QFile makefile(makefilePath);
    makefile.open(QIODevice::WriteOnly);

    QTextStream scriptStream(&makefile);

    // Since GNU make (and maybe other make versions) fails on files/paths with whitespaces
    // we should perform space-escaping procedure for all potential strings.

    scriptStream << QStringLiteral("SOURCES =");
    for (const QString& source : params.sources()) {
        scriptStream << QStringLiteral(" %1").arg(spaceEscapedString(source));
    }
    scriptStream << QLatin1Char('\n');

    scriptStream << QStringLiteral("COMMAND =");
    if (!GlobalSettings::verboseOutput()) {
        scriptStream << QLatin1Char('@');
    }
    const auto commandLine = params.commandLine();
    for (const QString& commandPart : commandLine) {
        scriptStream << QStringLiteral(" %1").arg(spaceEscapedString(commandPart));
    }
    scriptStream << QLatin1Char('\n');

    scriptStream << QStringLiteral(".PHONY: all $(SOURCES)\n");
    scriptStream << QStringLiteral("all: $(SOURCES)\n");
    scriptStream << QStringLiteral("$(SOURCES):\n");

    scriptStream << QStringLiteral("\t@echo 'Clazy check started  for $@'\n");
    // Wrap filename ($@) with quotas to handle "whitespaced" file names.
    scriptStream << QStringLiteral("\t$(COMMAND) '$@'\n");
    scriptStream << QStringLiteral("\t@echo 'Clazy check finished for $@'\n");

    makefile.close();

    m_totalCount = params.sources().size();

    return makefilePath;
}

void Job::postProcessStdout(const QStringList& lines)
{
    static const auto startedRegex  = QRegularExpression(QStringLiteral("Clazy check started  for (.+)$"));
    static const auto finishedRegex = QRegularExpression(QStringLiteral("Clazy check finished for (.+)$"));

    for (const QString & line : lines) {
        auto match = startedRegex.match(line);
        if (match.hasMatch()) {
            emit infoMessage(this, match.captured(1));
            continue;
        }

        match = finishedRegex.match(line);
        if (match.hasMatch()) {
            setPercent(++m_finishedCount/(double)m_totalCount * 100);
            continue;
        }
    }

    m_standardOutput << lines;

    if (status() == KDevelop::OutputExecuteJob::JobStatus::JobRunning) {
        OutputExecuteJob::postProcessStdout(lines);
    }
}

void Job::postProcessStderr(const QStringList& lines)
{
    static const auto errorRegex = QRegularExpression(
        QStringLiteral("(.+):(\\d+):(\\d+):\\s+warning:\\s+(.+)\\s+\\[-Wclazy-(.+)\\]$"));

    QVector<KDevelop::IProblem::Ptr> problems;

    for (const QString & line : lines) {
        auto match = errorRegex.match(line);
        if (match.hasMatch()) {
            auto check = m_db ? m_db->checks().value(match.captured(5), nullptr) : nullptr;

            const QString levelName = check ? check->level->displayName : i18n("Unknown Level");
            KDevelop::IProblem::Ptr problem(new KDevelop::DetectedProblem(levelName));

            problem->setSeverity(KDevelop::IProblem::Warning);
            problem->setDescription(match.captured(4));
            if (check) {
                problem->setExplanation(check->description);
            }

            // Sometimes warning/error file path contains "." or ".." elements so we should fix
            // it and take "real" (canonical) path value. But QFileInfo::canonicalFilePath()
            // returns empty string when file does not exists. Unfortunately we can't pass some
            // real file path from unit tests, therefore we should skip canonicalFilePath() step.
            // To detect such testing cases we are check m_timer value, which is not-null only for
            // "real" jobs, created with public constructor.
            const auto document = m_timer.isNull() ? match.captured(1) : QFileInfo(match.captured(1)).canonicalFilePath();

            const int line = match.captured(2).toInt() - 1;
            const int column = match.captured(3).toInt() - 1;

            // TODO add KDevelop::IProblem::FinalLocationMode::ToEnd type ?
            KTextEditor::Range range(line, column, line, 1000);
            KDevelop::DocumentRange documentRange(KDevelop::IndexedString(document), range);
            problem->setFinalLocation(documentRange);
            problem->setFinalLocationMode(KDevelop::IProblem::Range);

            problems.append(problem);
        }
    }
    m_stderrOutput << lines;

    if (problems.size()) {
        emit problemsDetected(problems);
    }

    if (status() == KDevelop::OutputExecuteJob::JobStatus::JobRunning) {
        OutputExecuteJob::postProcessStderr(lines);
    }
}

void Job::start()
{
    m_standardOutput.clear();
    m_stderrOutput.clear();

    qCDebug(KDEV_CLAZY) << "executing:" << commandLine().join(QLatin1Char(' '));

    m_timer->restart();
    setPercent(0);
    m_finishedCount = 0;

    OutputExecuteJob::start();
}

void Job::childProcessError(QProcess::ProcessError e)
{
    QString message;

    switch (e) {
    case QProcess::FailedToStart:
        message = i18n("Failed to start Clazy analysis process.");
        break;

    case QProcess::Crashed:
        if (status() != KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {
            message = i18n("Clazy analysis process crashed.");
        }
        break;

    case QProcess::Timedout:
        message = i18n("Clazy analysis process timed out.");
        break;

    case QProcess::WriteError:
        message = i18n("Write to Clazy analysis process failed.");
        break;

    case QProcess::ReadError:
        message = i18n("Read from Clazy analysis process failed.");
        break;

    case QProcess::UnknownError:
        // errors will be displayed in the output view ?
        // don't notify the user
        break;
    }

    if (!message.isEmpty()) {
        KMessageBox::error(qApp->activeWindow(), message, i18n("Clazy Error"));
    }

    KDevelop::OutputExecuteJob::childProcessError(e);
}

void Job::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KDEV_CLAZY) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    setPercent(100);
    postProcessStdout({QStringLiteral("Elapsed time: %1 s.").arg(m_timer->elapsed()/1000.0)});

    if (exitCode != 0) {
        qCDebug(KDEV_CLAZY) << "clazy failed";
        qCDebug(KDEV_CLAZY) << "stdout output: ";
        qCDebug(KDEV_CLAZY) << m_standardOutput.join(QLatin1Char('\n'));
        qCDebug(KDEV_CLAZY) << "stderr output: ";
        qCDebug(KDEV_CLAZY) << m_stderrOutput.join(QLatin1Char('\n'));
    }

    KDevelop::OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}

}
