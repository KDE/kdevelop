/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

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
    virtual void start() override;
    virtual bool doKill() override;
private slots:
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
    KDevelop::CommandExecutor* exec;
    bool killed;
    bool enabled;
};

#endif 
