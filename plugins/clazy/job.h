/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_JOB_H
#define KDEVCLAZY_JOB_H

// CompileAnalyzer
#include <compileanalyzejob.h>
// KDevPlatform
#include <interfaces/iproblem.h>
#include <outputview/outputexecutejob.h>

class QElapsedTimer;

namespace Clazy
{

class ChecksDB;

class JobParameters
{
public:
    QString executablePath;
    QUrl url;
    QStringList filePaths;
    QString buildDir;

    QString checks;

    bool onlyQt = false;
    bool qtDeveloper = false;
    bool qt4Compat = false;
    bool visitImplicitCode = false;
    bool ignoreIncludedFiles = false;

    QString headerFilter;

    bool enableAllFixits = false;
    bool noInplaceFixits = false;

    QString extraAppend;
    QString extraPrepend;
    QString extraClazy;

    bool verboseOutput = false;
    int parallelJobCount = 1;
};

class Job : public KDevelop::CompileAnalyzeJob
{
    Q_OBJECT

protected:
    /// Empty constructor which creates invalid Job instance. Used only for testing
    Job();

public:
    Job(const JobParameters& params, QSharedPointer<const ChecksDB> db);
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

private:
    QSharedPointer<const ChecksDB> m_db;
    QScopedPointer<QElapsedTimer> m_timer;

    QStringList m_standardOutput;
    QStringList m_stderrOutput;
};

}
#endif
