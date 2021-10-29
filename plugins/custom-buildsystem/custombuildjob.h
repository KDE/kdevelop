/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef CUSTOMBUILDJOB_H
#define CUSTOMBUILDJOB_H

#include <outputview/outputjob.h>
#include "custombuildsystemconfig.h"
#include <QProcess>

class CustomBuildSystem;
namespace KDevelop
{
class ProjectBaseItem;
class CommandExecutor;
class OutputModel;
class IProject;
}

class CustomBuildJob : public KDevelop::OutputJob
{
Q_OBJECT
public:
    enum ErrorType {
        UndefinedBuildType = UserDefinedError,
        FailedToStart,
        UnknownExecError,
        Crashed,
        WrongArgs,
        ToolDisabled,
        NoCommand
    };
    
    CustomBuildJob( CustomBuildSystem*, KDevelop::ProjectBaseItem*, CustomBuildSystemTool::ActionType t );
    void start() override;
    bool doKill() override;

    void setInstallPrefix(const QUrl &installPrefix) { this->installPrefix=installPrefix; }

private Q_SLOTS:
    void procFinished(int);
    void procError( QProcess::ProcessError );
private:
    KDevelop::OutputModel* model();
    CustomBuildSystemTool::ActionType type;
    QString projectName;
    QString cmd;
    QString arguments;
    QString environment;
    QString builddir;
    QUrl installPrefix;
    KDevelop::CommandExecutor* exec;
    bool killed;
    bool enabled;
};

#endif 
