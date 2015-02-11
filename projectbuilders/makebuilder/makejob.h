/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
    Copyright 2008 Hamish Rodda <rodda@kde.org>
    Copyright 2012 Ivan Shapovalov <intelfx100@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef MAKEJOB_H
#define MAKEJOB_H

#include <outputview/outputexecutejob.h>

#include <QString>
#include <QProcess>

#include "imakebuilder.h"

namespace KDevelop {
class OutputModel;
class ProjectBaseItem;
class ProcessLineMaker;
}

class QUrl;

class MakeJob: public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    enum CommandType
    {
        BuildCommand,
        CleanCommand,
        CustomTargetCommand,
        InstallCommand
    };

    enum ErrorTypes
    {
        IncorrectItemError = UserDefinedError,
        ItemNoLongerValidError,
        BuildCommandError,
        FailedError = FailedShownError
    };

    MakeJob( QObject* parent, KDevelop::ProjectBaseItem* item,
             CommandType command, const QStringList& overrideTargets = QStringList(),
             const MakeVariables& variables = MakeVariables() );
    virtual ~MakeJob();

    virtual void start() override;

    KDevelop::ProjectBaseItem* item() const;
    CommandType commandType();
    QStringList customTargets() const;


    // This returns the build directory for registered item.
    virtual QUrl workingDirectory() const override;

    // This returns the "make" command line.
    virtual QStringList commandLine() const override;

    // This returns the configured privileged execution command (if specified by user).
    virtual QStringList privilegedExecutionCommand() const override;

    // This returns the configured global environment profile.
    virtual QString environmentProfile() const override;

private:
    QPersistentModelIndex m_idx;
    CommandType m_command;
    QStringList m_overrideTargets;
    MakeVariables m_variables;
};

#endif // MAKEJOB_H

