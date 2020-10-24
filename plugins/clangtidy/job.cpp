/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

// KDevPlatform
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
// KF
#include <KLocalizedString>
// Qt
#include <QApplication>
#include <QFile>
#include <QRegularExpression>

namespace ClangTidy
{

// uses ' for quoting
QString inlineYaml(const Job::Parameters& parameters)
{
    QString result;

    result.append(QLatin1String("{Checks: '") + parameters.enabledChecks + QLatin1Char('\''));

    if (!parameters.headerFilter.isEmpty()) {
        // TODO: the regex might need escpaing for potential quotes of all kinds
        result.append(QLatin1String(", HeaderFilterRegex: '") + parameters.headerFilter + QLatin1Char('\''));
    }
    result.append(QLatin1Char('}'));

    return result;
}

// uses " for quoting
QStringList commandLineArgs(const Job::Parameters& parameters)
{
    QStringList args{
        QLatin1Char('\"') + parameters.executablePath + QLatin1Char('\"'),
        QLatin1String("-p=\"") + parameters.buildDir + QLatin1Char('\"'),
        // don't add statistics we are not interested in to parse anyway
        QStringLiteral("-quiet"),
    };
    if (!parameters.additionalParameters.isEmpty()) {
        args << parameters.additionalParameters;
    }
    if (parameters.checkSystemHeaders) {
        args << QStringLiteral("--system-headers");
    }

    if (!parameters.useConfigFile) {
        args << QLatin1String("--config=\"") + inlineYaml(parameters) + QLatin1Char('\"');
    }

    return args;
}


Job::Job(const Parameters& params, QObject* parent)
    : KDevelop::CompileAnalyzeJob(parent)
    , m_parameters(params)
{
    setJobName(i18n("Clang-Tidy Analysis"));

    setParallelJobCount(m_parameters.parallelJobCount);
    setBuildDirectoryRoot(m_parameters.buildDir);
    const auto commandLine = commandLineArgs(m_parameters);
    setCommand(commandLine.join(QLatin1Char(' ')), false);
    setToolDisplayName(QStringLiteral("Clang-Tidy"));
    setSources(m_parameters.filePaths);

    connect(&m_parser, &ClangTidyParser::problemsDetected,
            this, &Job::problemsDetected);

    qCDebug(KDEV_CLANGTIDY) << "checking files" << params.filePaths;
}

Job::~Job()
{
}

void Job::processStdoutLines(const QStringList& lines)
{
    m_parser.addData(lines);
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
    m_xmlOutput.clear();

    KDevelop::CompileAnalyzeJob::start();
}

void Job::childProcessError(QProcess::ProcessError processError)
{
    QString messageText;

    switch (processError) {
    case QProcess::FailedToStart: {
        messageText = i18n("Failed to start Clang-Tidy process.");
        break;
    }

    case QProcess::Crashed:
        messageText = i18n("Clang-Tidy crashed.");
        break;

    case QProcess::Timedout:
        messageText = i18n("Clang-Tidy process timed out.");
        break;

    case QProcess::WriteError:
        messageText = i18n("Write to Clang-Tidy process failed.");
        break;

    case QProcess::ReadError:
        messageText = i18n("Read from Clang-Tidy process failed.");
        break;

    case QProcess::UnknownError:
        // current clangtidy errors will be displayed in the output view
        // don't notify the user
        break;
    }

    if (!messageText.isEmpty()) {
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        KDevelop::ICore::self()->uiController()->postMessage(message);
    }

    KDevelop::CompileAnalyzeJob::childProcessError(processError);
}

void Job::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0) {
        qCDebug(KDEV_CLANGTIDY) << "clang-tidy failed, standard output: ";
        qCDebug(KDEV_CLANGTIDY) << m_standardOutput.join(QLatin1Char('\n'));
        qCDebug(KDEV_CLANGTIDY) << "clang-tidy failed, XML output: ";
        qCDebug(KDEV_CLANGTIDY) << m_xmlOutput.join(QLatin1Char('\n'));
    }

    KDevelop::CompileAnalyzeJob::childProcessExited(exitCode, exitStatus);
}

} // namespace ClangTidy
