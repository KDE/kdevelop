/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMOIDEXECUTIONJOB_H
#define PLASMOIDEXECUTIONJOB_H

#include <QProcess>
#include <outputview/outputjob.h>

class ExecutePlasmoidPlugin;

namespace KDevelop
{
class ILaunchConfiguration;
class OutputModel;
class CommandExecutor;
}


class PlasmoidExecutionJob : public KDevelop::OutputJob
{
Q_OBJECT
public:
    PlasmoidExecutionJob( ExecutePlasmoidPlugin* parent, KDevelop::ILaunchConfiguration* cfg );
    void start() override;
    bool doKill() override;
    KDevelop::OutputModel* model();

    static QString executable(KDevelop::ILaunchConfiguration* cfg);
    static QStringList arguments(KDevelop::ILaunchConfiguration* cfg);
    static QString workingDirectory(KDevelop::ILaunchConfiguration* cfg);

public Q_SLOTS:
    void slotCompleted(int);
    void slotFailed(QProcess::ProcessError);

private:
    KDevelop::CommandExecutor* m_process;
};

#endif
