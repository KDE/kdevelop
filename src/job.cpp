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

// plugin
#include "parsers/clangtidyparser.h"
#include <qtcompat_p.h>
// KF
#include <KLocalizedString>
#include <KMessageBox>
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
        parameters.executablePath,
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
    : KDevelop::OutputExecuteJob(parent)
    , m_parameters(params)
{
    setJobName(i18n("Clang-Tidy Analysis"));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStdout |
                  KDevelop::OutputExecuteJob::JobProperty::DisplayStderr |
                  KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput);

    m_totalCount = params.filePaths.size();

    // TODO: check success of creation
    generateMakefile();

    *this << QStringList{
        QStringLiteral("make"),
        QStringLiteral("-f"),
        m_makeFilePath,
    };

    qCDebug(KDEV_CLANGTIDY) << "checking files" << params.filePaths;
}

Job::~Job()
{
    doKill();

    if (!m_makeFilePath.isEmpty()) {
        QFile::remove(m_makeFilePath);
    }
}

void Job::generateMakefile()
{
    m_makeFilePath = m_parameters.buildDir + QLatin1String("/kdevclangtidy.makefile");

    QFile makefile(m_makeFilePath);
    makefile.open(QIODevice::WriteOnly);

    QTextStream scriptStream(&makefile);

    scriptStream << QStringLiteral("SOURCES =");
    for (const auto& source : qAsConst(m_parameters.filePaths)) {
        // TODO: how to escape " in a filename, for those people who like to go extreme?
        scriptStream << QLatin1String(" \\\n\t\"") + source + QLatin1Char('\"');
    }
    scriptStream << QLatin1Char('\n');

    scriptStream << QStringLiteral("COMMAND =");
    const auto commandLine = commandLineArgs(m_parameters);
    for (const auto& commandPart : commandLine) {
        scriptStream << QLatin1Char(' ') << commandPart;
    }
    scriptStream << QLatin1Char('\n');

    scriptStream << QStringLiteral(".PHONY: all $(SOURCES)\n");
    scriptStream << QStringLiteral("all: $(SOURCES)\n");
    scriptStream << QStringLiteral("$(SOURCES):\n");

    scriptStream << QStringLiteral("\t@echo 'Clang-Tidy check started  for $@'\n");
    scriptStream << QStringLiteral("\t$(COMMAND) $@\n");
    scriptStream << QStringLiteral("\t@echo 'Clang-Tidy check finished for $@'\n");

    makefile.close();
}

void Job::processStdoutLines(const QStringList& lines)
{
    static const auto startedRegex  = QRegularExpression(QStringLiteral("Clang-Tidy check started  for (.+)$"));
    static const auto finishedRegex = QRegularExpression(QStringLiteral("Clang-Tidy check finished for (.+)$"));

    for (const auto& line : lines) {
        auto match = startedRegex.match(line);
        if (match.hasMatch()) {
            emit infoMessage(this, match.captured(1));
            continue;
        }

        match = finishedRegex.match(line);
        if (match.hasMatch()) {
            ++m_finishedCount;
            setPercent(static_cast<double>(m_finishedCount)/m_totalCount * 100);
            continue;
        }
    }

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

    setPercent(0);
    m_finishedCount = 0;

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
        message = i18n("Failed to start Clang-Tidy process.");
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

    setPercent(100);

    if (exitCode != 0) {
        qCDebug(KDEV_CLANGTIDY) << "clang-tidy failed, standard output: ";
        qCDebug(KDEV_CLANGTIDY) << m_standardOutput.join(QLatin1Char('\n'));
        qCDebug(KDEV_CLANGTIDY) << "clang-tidy failed, XML output: ";
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
