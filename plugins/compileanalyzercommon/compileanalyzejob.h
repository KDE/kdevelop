/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPILEANALYZER_COMPILEANALYZEJOB_H
#define COMPILEANALYZER_COMPILEANALYZEJOB_H

// lib
#include <compileanalyzercommonexport.h>
// KDevPlatform
#include <interfaces/iproblem.h>
#include <outputview/outputexecutejob.h>
// Qt
#include <QRegularExpression>

namespace KDevelop
{

class KDEVCOMPILEANALYZERCOMMON_EXPORT CompileAnalyzeJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

protected: // API to implement
    static QString spaceEscapedString(const QString& s);

public:
    explicit CompileAnalyzeJob(QObject* parent = nullptr);
    ~CompileAnalyzeJob() override;

public: // KJob API
    void start() override;

public:
    void setParallelJobCount(int parallelJobCount);
    void setBuildDirectoryRoot(const QString& buildDir);
    void setCommand(const QString& commandcommand, bool verboseOutput = true);
    void setToolDisplayName(const QString& toolDisplayName);
    void setSources(const QStringList& sources);

Q_SIGNALS:
    void problemsDetected(const QVector<KDevelop::IProblem::Ptr>& problems);

protected Q_SLOTS:
    void postProcessStdout(const QStringList& lines) override;
    void childProcessExited(int exitCode, QProcess::ExitStatus exitStatus) override;

protected:
    void parseProgress(const QStringList& lines);

private:
    void generateMakefile();

private:
    QString m_makeFilePath;
    QString m_buildDir;
    QString m_command;
    QString m_toolDisplayName;
    QStringList m_sources;
    int m_parallelJobCount = 1;
    bool m_verboseOutput = true;

    int m_finishedCount = 0;
    int m_totalCount = 0;

    QRegularExpression m_fileStartedRegex;
    QRegularExpression m_fileFinishedRegex;
};

}

#endif
