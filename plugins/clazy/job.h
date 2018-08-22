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

#ifndef KDEVCLAZY_JOB_H
#define KDEVCLAZY_JOB_H

#include <interfaces/iproblem.h>
#include <outputview/outputexecutejob.h>

class QElapsedTimer;

namespace Clazy
{

class ChecksDB;
class JobParameters;

class Job : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    Job(const JobParameters& params, QSharedPointer<const ChecksDB> db);
    ~Job() override;

    void start() override;

Q_SIGNALS:
    void problemsDetected(const QVector<KDevelop::IProblem::Ptr>& problems);

protected Q_SLOTS:
    void postProcessStdout(const QStringList& lines) override;
    void postProcessStderr(const QStringList& lines) override;

    void childProcessExited(int exitCode, QProcess::ExitStatus exitStatus) override;
    void childProcessError(QProcess::ProcessError processError) override;

protected:
    /// Empty constructor which creates invalid Job instance. Used only for testing
    Job();

    int m_totalCount = 0;
    int m_finishedCount = 0;

private:
    QString buildMakefile(const JobParameters& params);

private:
    QSharedPointer<const ChecksDB> m_db;
    QScopedPointer<QElapsedTimer> m_timer;

    QStringList m_standardOutput;
    QStringList m_stderrOutput;
};

}
#endif
