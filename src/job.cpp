/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "job.h"

// plugin
#include "parsers/clangtidyparser.h"
// KF
#include <KLocalizedString>
#include <KMessageBox>
// Qt
#include <QApplication>
#include <QFile>
#include <QRegularExpression>

namespace ClangTidy
{

QString inlineYaml(const Job::Parameters& params)
{
    QString result;

    result.append(QLatin1String("{Checks: '") + params.enabledChecks + QLatin1Char('\''));

    if (!params.headerFilter.isEmpty()) {
        result.append(QLatin1String(", HeaderFilterRegex: '") + params.headerFilter + QLatin1Char('\''));
    }
    result.append(QLatin1Char('}'));

    return result;
}

Job::Job(const Parameters& params, QObject* parent)
    : KDevelop::OutputExecuteJob(parent)
    , m_parameters(params)
{
    setJobName(i18n("Clang-Tidy Analysis"));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput);

    *this << params.executablePath;

    *this << QLatin1String("-p=") + params.buildDir;
    *this << params.filePath;

    // don't add statistics we are not interested in to parse anyway
    *this << QStringLiteral("-quiet");

    if (!params.additionalParameters.isEmpty()) {
        *this << params.additionalParameters;
    }
    if (params.checkSystemHeaders) {
        *this << QStringLiteral("--system-headers");
    }

    if (!params.useConfigFile) {
        *this << QLatin1String("--config=") + inlineYaml(params);
    }

    qCDebug(KDEV_CLANGTIDY) << "checking path" << params.filePath;
}

Job::~Job()
{
    doKill();
}

void Job::processStdoutLines(const QStringList& lines)
{
    m_standardOutput << lines;
}

void Job::processStderrLines(const QStringList& lines)
{
    static const auto xmlStartRegex = QRegularExpression(QStringLiteral("\\s*<"));

    for (const QString& line : lines) {
        // unfortunately sometime clangtidy send non-XML messages to stderr.
        // For example, if we pass '-I /missing_include_dir' to the argument list,
        // then stderr output will contains such line (tested on clangtidy 1.72):
        //
        // (information) Couldn't find path given by -I '/missing_include_dir'
        //
        // Therefore we must 'move' such messages to m_standardOutput.

        if (line.indexOf(xmlStartRegex) != -1) { // the line contains XML
            m_xmlOutput << line;
        } else {
            m_standardOutput << line;
        }
    }
}

void Job::postProcessStdout(const QStringList& lines)
{
    processStdoutLines(lines);

    KDevelop::OutputExecuteJob::postProcessStdout(lines);
}

void Job::postProcessStderr(const QStringList& lines)
{
    processStderrLines(lines);

    KDevelop::OutputExecuteJob::postProcessStderr(lines);
}

void Job::start()
{
    m_standardOutput.clear();
    m_xmlOutput.clear();

    qCDebug(KDEV_CLANGTIDY) << "executing:" << commandLine().join(QLatin1Char(' '));

    KDevelop::OutputExecuteJob::start();
}

QVector<KDevelop::IProblem::Ptr> Job::problems() const
{
    return m_problems;
}

void Job::childProcessError(QProcess::ProcessError processError)
{
    QString message;

    switch (processError) {
    case QProcess::FailedToStart: {
        const auto binaryPath = commandLine().value(0);
        if (binaryPath.isEmpty()) {
            message = i18n("Failed to find clang-tidy binary.");
        } else {
            message = i18n("Failed to start clang-tidy from %1.", binaryPath);
        }
        break;
    }

    case QProcess::Crashed:
        message = i18n("Clang-tidy crashed.");
        break;

    case QProcess::Timedout:
        message = i18n("Clang-tidy process timed out.");
        break;

    case QProcess::WriteError:
        message = i18n("Write to Clang-tidy process failed.");
        break;

    case QProcess::ReadError:
        message = i18n("Read from Clang-tidy process failed.");
        break;

    case QProcess::UnknownError:
        // current clangtidy errors will be displayed in the output view
        // don't notify the user
        break;
    }

    if (!message.isEmpty()) {
        KMessageBox::error(qApp->activeWindow(), message, i18n("Clang-tidy Error"));
    }

    KDevelop::OutputExecuteJob::childProcessError(processError);
}

void Job::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KDEV_CLANGTIDY) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    if (exitCode != 0) {
        qCDebug(KDEV_CLANGTIDY) << "clangtidy failed, standard output: ";
        qCDebug(KDEV_CLANGTIDY) << m_standardOutput.join(QLatin1Char('\n'));
        qCDebug(KDEV_CLANGTIDY) << "clangtidy failed, XML output: ";
        qCDebug(KDEV_CLANGTIDY) << m_xmlOutput.join(QLatin1Char('\n'));
    } else {
        ClangTidyParser parser;
        parser.addData(m_standardOutput);
        parser.parse();
        m_problems = parser.problems();
    }

    KDevelop::OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}
} // namespace ClangTidy
