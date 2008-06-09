/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
    Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#include <outputjob.h>

#include <QString>

namespace KDevelop {
class ProjectBaseItem;
class CommandExecutor;
}

class MakeBuilder;

class MakeJob: public KDevelop::OutputJob
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
        InvalidBuildDirectoryError,
        BuildCommandError,
        FailedError
    };

    MakeJob( MakeBuilder* builder, KDevelop::ProjectBaseItem* item, CommandType command, const QString& overrideTarget = QString() );

    virtual void start();

    KDevelop::ProjectBaseItem* item() const;
    CommandType commandType();
    const QString& customTarget() const;
    
protected:
    bool doKill();

private Q_SLOTS:
    void slotFailed();
    void slotCompleted();

private:
    QStringList computeBuildCommand() const;
    KUrl computeBuildDir(KDevelop::ProjectBaseItem* item) const;
    QMap<QString, QString> environmentVars() const;

    MakeBuilder* m_builder;
    KDevelop::ProjectBaseItem* m_item;
    CommandType m_command;
    QString m_overrideTarget;
    KDevelop::CommandExecutor* m_executor;
    bool m_killed;
};

#endif // MAKEJOB_H

