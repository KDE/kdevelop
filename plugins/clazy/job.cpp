/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "job.h"

#include "checksdb.h"
#include "debug.h"
#include "globalsettings.h"
#include "plugin.h"
#include "utils.h"

#include <language/editor/documentrange.h>
#include <shell/problem.h>
// KF
#include <KLocalizedString>
#include <KShell>
// Qt
#include <QMessageBox>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QRegularExpression>

namespace Clazy
{

Job::Job()
    : KDevelop::CompileAnalyzeJob(nullptr)
    , m_db(nullptr)
    , m_timer(nullptr)
{
}

QString commandLineString(const JobParameters& params)
{
    QStringList args;

    args << params.executablePath;

    if (!params.checks.isEmpty()) {
        args << QLatin1String("-checks=") + params.checks;
    }

    if (params.onlyQt) {
        args << QStringLiteral("-only-qt");
    }

    if (params.qtDeveloper) {
        args << QStringLiteral("-qt-developer");
    }

    if (params.qt4Compat) {
        args << QStringLiteral("-qt4-compat");
    }

    if (params.visitImplicitCode) {
        args << QStringLiteral("-visit-implicit-code");
    }

    if (params.ignoreIncludedFiles) {
        args << QStringLiteral("-ignore-included-files");
    }

    if (!params.headerFilter.isEmpty()) {
        args << QLatin1String("-header-filter=") + params.headerFilter;
    }

    if (params.enableAllFixits) {
        args << QStringLiteral("-enable-all-fixits");
    }

    if (params.noInplaceFixits) {
        args << QStringLiteral("-no-inplace-fixits");
    }

    if (!params.extraAppend.isEmpty()) {
        args << QLatin1String("-extra-arg=") + params.extraAppend;
    }

    if (!params.extraPrepend.isEmpty()) {
        args << QLatin1String("-extra-arg-before=%1") + params.extraPrepend;
    }

    if (!params.extraClazy.isEmpty()) {
        args << KShell::splitArgs(params.extraClazy);
    }

    args << QLatin1String("-p=\"") + params.buildDir + QLatin1Char('\"');

//     for (auto it = args.begin(), end = args.end(); it != end; ++it) {
//         QString& commandPart = *it;
//         commandPart = spaceEscapedString(commandPart);
//     }

    return args.join(QLatin1Char(' '));
}

Job::Job(const JobParameters& params, QSharedPointer<const ChecksDB> db)
    : KDevelop::CompileAnalyzeJob(nullptr)
    , m_db(db)
    , m_timer(new QElapsedTimer)
{
    setJobName(i18n("Clazy Analysis (%1)", prettyPathName(params.url)));

    setParallelJobCount(params.parallelJobCount);
    setBuildDirectoryRoot(params.buildDir);
    setCommand(commandLineString(params), params.verboseOutput);
    setToolDisplayName(QStringLiteral("Clazy"));
    setSources(params.filePaths);
}

Job::~Job()
{
}

void Job::processStdoutLines(const QStringList& lines)
{
    m_standardOutput << lines;
}

void Job::processStderrLines(const QStringList& lines)
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

            const int line = match.capturedView(2).toInt() - 1;
            const int column = match.capturedView(3).toInt() - 1;

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
}

void Job::postProcessStdout(const QStringList& lines)
{
    processStdoutLines(lines);

    KDevelop::CompileAnalyzeJob::postProcessStdout(lines);
}

void Job::postProcessStderr(const QStringList& lines)
{
    processStderrLines(lines);

    KDevelop::CompileAnalyzeJob::postProcessStderr(lines);
}

void Job::start()
{
    m_standardOutput.clear();
    m_stderrOutput.clear();

    m_timer->restart();

    KDevelop::CompileAnalyzeJob::start();
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
        QMessageBox::critical(nullptr, i18nc("@title:window", "Clazy Error"), message);
    }

    KDevelop::CompileAnalyzeJob::childProcessError(e);
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

    KDevelop::CompileAnalyzeJob::childProcessExited(exitCode, exitStatus);
}

}

#include "moc_job.cpp"
