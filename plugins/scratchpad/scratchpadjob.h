/*
    SPDX-FileCopyrightText: 2018 Amish K. Naidu <amhndu@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRATCHPADJOB_H
#define SCRATCHPADJOB_H

#include <outputview/outputjob.h>

#include <QProcess>

namespace KDevelop {
class OutputModel;
class ProcessLineMaker;
}
class KProcess;

class ScratchpadJob
    : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    ScratchpadJob(const QString& command, const QString& title, QObject* parent);

    void start() override;
    bool doKill() override;

private Q_SLOTS:
    void processFinished(int exitCode, QProcess::ExitStatus status);
    void processError(QProcess::ProcessError error);

private:
    KDevelop::OutputModel* outputModel() const;

    KProcess* m_process;
    KDevelop::ProcessLineMaker* m_lineMaker;
};

#endif // SCRATCHPADJOB_H
