/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SCRIPTAPPJOB_H
#define KDEVPLATFORM_PLUGIN_SCRIPTAPPJOB_H

#include <QProcess>
#include <outputview/outputjob.h>

class ExecuteScriptPlugin;

namespace KDevelop
{
class ILaunchConfiguration;
class ProcessLineMaker;
class OutputModel;
}

class KProcess;

class ScriptAppJob : public KDevelop::OutputJob
{
Q_OBJECT
public:
    ScriptAppJob( ExecuteScriptPlugin* parent, KDevelop::ILaunchConfiguration* cfg );
    void start() override;
    bool doKill() override;
    KDevelop::OutputModel* model();
private Q_SLOTS:
    void processError(QProcess::ProcessError);
    void processFinished(int,QProcess::ExitStatus);
private:
    void appendLine(const QString &l);
    KProcess* proc;
    KDevelop::ProcessLineMaker* lineMaker;
};

#endif
