/*
 * This file is part of KDevelop
 *
 * Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>
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
