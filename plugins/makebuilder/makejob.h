/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAKEJOB_H
#define MAKEJOB_H

#include <outputview/outputexecutejob.h>

#include <QString>

#include "imakebuilder.h"

namespace KDevelop {
class ProjectBaseItem;
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
    };

    MakeJob( QObject* parent, KDevelop::ProjectBaseItem* item,
             CommandType command, const QStringList& overrideTargets = QStringList(),
             const MakeVariables& variables = MakeVariables() );
    ~MakeJob() override;

    void start() override;

    KDevelop::ProjectBaseItem* item() const;
    CommandType commandType() const;
    QStringList customTargets() const;


    // This returns the build directory for registered item.
    QUrl workingDirectory() const override;

    // This returns the "make" command line.
    QStringList commandLine() const override;

    // This returns the configured privileged execution command (if specified by user).
    QStringList privilegedExecutionCommand() const override;

    // This returns the configured global environment profile.
    QString environmentProfile() const override;

private:
    static bool isNMake(const QString& makeBin);
    
    QPersistentModelIndex m_idx;
    CommandType m_command;
    QStringList m_overrideTargets;
    MakeVariables m_variables;
};

#endif // MAKEJOB_H

