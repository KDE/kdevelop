/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "compileanalyzejob.h"

// lib
#include <debug.h>
// KF
#include <KLocalizedString>
// Qt
#include <QTemporaryFile>

namespace KDevelop
{

QString CompileAnalyzeJob::spaceEscapedString(const QString& s)
{
    return QString(s).replace(QLatin1Char(' '), QLatin1String("\\ "));
}

CompileAnalyzeJob::CompileAnalyzeJob(QObject* parent)
    : OutputExecuteJob(parent)
{
    setCapabilities(KJob::Killable);
    setStandardToolView(IOutputView::TestView);
    setBehaviours(IOutputView::AutoScroll);
    setProperties(JobProperties(DisplayStdout | DisplayStderr | PostProcessOutput));
}

CompileAnalyzeJob::~CompileAnalyzeJob()
{
    doKill();

    if (!m_makeFilePath.isEmpty()) {
        QFile::remove(m_makeFilePath);
    }
}

void CompileAnalyzeJob::setParallelJobCount(int parallelJobCount)
{
    m_parallelJobCount = parallelJobCount;
}

void CompileAnalyzeJob::setBuildDirectoryRoot(const QString& buildDir)
{
    m_buildDir = buildDir;
}

void CompileAnalyzeJob::setCommand(const QString& command, bool verboseOutput)
{
    m_command = command;
    m_verboseOutput = verboseOutput;
}

void CompileAnalyzeJob::setToolDisplayName(const QString& toolDisplayName)
{
    m_toolDisplayName = toolDisplayName;

    m_fileStartedRegex  = QRegularExpression(m_toolDisplayName + QLatin1String(" check started  for (.+)$"));
    m_fileFinishedRegex = QRegularExpression(m_toolDisplayName + QLatin1String(" check finished for (.+)$"));
}

void CompileAnalyzeJob::setSources(const QStringList& sources)
{
    m_sources = sources;
}

void CompileAnalyzeJob::generateMakefile()
{
    QTemporaryFile makefile(m_buildDir + QLatin1String("/kdevcompileanalyzerXXXXXX.makefile"));
    makefile.setAutoRemove(false);
    makefile.open();
    m_makeFilePath = makefile.fileName();

    QTextStream scriptStream(&makefile);

    scriptStream << QStringLiteral("SOURCES =");
    for (const auto& source : std::as_const(m_sources)) {
        scriptStream << QLatin1String(" \\\n\t") << spaceEscapedString(source);
    }
    scriptStream << QLatin1Char('\n');

    scriptStream << QLatin1String("COMMAND = ");
    if (!m_verboseOutput) {
        scriptStream << QLatin1Char('@');
    }
    scriptStream << m_command << QLatin1Char('\n');

    scriptStream << QLatin1String(".PHONY: all $(SOURCES)\n");
    scriptStream << QLatin1String("all: $(SOURCES)\n");
    scriptStream << QLatin1String("$(SOURCES):\n");

    scriptStream << QLatin1String("\t@echo '") << m_toolDisplayName << QLatin1String(" check started  for $@'\n");
    // Wrap filename ($@) with quotas to handle "whitespaced" file names.
    scriptStream << QLatin1String("\t$(COMMAND) \"$@\"\n");
    scriptStream << QLatin1String("\t@echo '") << m_toolDisplayName << QLatin1String(" check finished for $@'\n");

    makefile.close();
}

void CompileAnalyzeJob::start()
{
    // TODO: check success of creation
    generateMakefile();

    *this << QStringList{
        QStringLiteral("make"),
        QStringLiteral("-j"),
        QString::number(m_parallelJobCount),
        QStringLiteral("-k"), // keep-going
        QStringLiteral("-f"),
        m_makeFilePath,
    };

    qCDebug(KDEV_COMPILEANALYZER) << "executing:" << commandLine().join(QLatin1Char(' '));

    m_finishedCount = 0;
    m_totalCount = m_sources.size();

    setPercent(0);

    KDevelop::OutputExecuteJob::start();
}

void CompileAnalyzeJob::parseProgress(const QStringList& lines)
{
    for (const auto& line : lines) {
        const auto startedMatch = m_fileStartedRegex.match(line);
        if (startedMatch.hasMatch()) {
            emit infoMessage(this, startedMatch.captured(1));
            continue;
        }

        const auto finishedMatch = m_fileFinishedRegex.match(line);
        if (finishedMatch.hasMatch()) {
            ++m_finishedCount;
            setPercent(static_cast<double>(m_finishedCount)/m_totalCount * 100);
            continue;
        }
    }
}

void CompileAnalyzeJob::postProcessStdout(const QStringList& lines)
{
    parseProgress(lines);

    KDevelop::OutputExecuteJob::postProcessStdout(lines);
}

void CompileAnalyzeJob::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KDEV_COMPILEANALYZER) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    setPercent(100);

    KDevelop::OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}

}

#include "moc_compileanalyzejob.cpp"
