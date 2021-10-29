/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_JOB_H
#define CLANGTIDY_JOB_H

// plugin
#include "parsers/clangtidyparser.h"
#include <debug.h>
// CompileAnalyzer
#include <compileanalyzejob.h>
// KDevPlatform
#include <interfaces/iproblem.h>

namespace ClangTidy
{
/**
 * \class
 * \brief specializes a KJob for running clang-tidy.
 */
class Job : public KDevelop::CompileAnalyzeJob
{
    Q_OBJECT

public:
    /**
     * \class
     * \brief command line parameters.
     */
    struct Parameters {
        QString projectRootDir;
        QString executablePath;
        QStringList filePaths;
        QString buildDir;
        QString additionalParameters;
        QString enabledChecks;
        bool useConfigFile = false;
        QString headerFilter;
        bool checkSystemHeaders = false;
        int parallelJobCount = 1;
    };

    explicit Job(const Parameters& params, QObject* parent = nullptr);
    ~Job() override;

public: // KJob API
    void start() override;

protected Q_SLOTS:
    void postProcessStdout(const QStringList& lines) override;
    void postProcessStderr(const QStringList& lines) override;

    void childProcessExited(int exitCode, QProcess::ExitStatus exitStatus) override;
    void childProcessError(QProcess::ProcessError processError) override;

protected:
    void processStdoutLines(const QStringList& lines);
    void processStderrLines(const QStringList& lines);

protected:
    ClangTidyParser m_parser;
    QStringList m_standardOutput;
    QStringList m_xmlOutput;
    const Job::Parameters m_parameters;

    QVector<KDevelop::IProblem::Ptr> m_problems;
};

}

#endif
