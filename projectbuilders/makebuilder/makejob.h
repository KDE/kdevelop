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

#include <outputview/outputjob.h>

#include <QString>
#include <QProcess>
#include <QMap>
#include "imakebuilder.h"

namespace KDevelop {
class ProjectBaseItem;
class ProcessLineMaker;
}

class KUrl;
class KProcess;
class MakeBuilder;
class MakeOutputModel;

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
        ItemNoLongerValidError,
        InvalidBuildDirectoryError,
        BuildCommandError,
        FailedError = FailedShownError
    };

    MakeJob( MakeBuilder* builder, KDevelop::ProjectBaseItem* item, CommandType command, const QStringList& overrideTargets = QStringList(),
             const MakeVariables& variables = MakeVariables() );
    virtual ~MakeJob();

    virtual void start();

    KDevelop::ProjectBaseItem* item() const;
    CommandType commandType();
    const QStringList& customTargets() const;

    MakeOutputModel* model() const;

    void setItem( KDevelop::ProjectBaseItem* item );

public slots:
    void addStandardOutput( const QStringList& );
protected:
    bool doKill();

private Q_SLOTS:
    void procError( QProcess::ProcessError error );
    void procFinished( int code, QProcess::ExitStatus status );

private:
    QStringList computeBuildCommand() const;
    KUrl computeBuildDir(KDevelop::ProjectBaseItem* item) const;
    QStringList environmentVars() const;

    MakeBuilder* m_builder;
    KDevelop::ProjectBaseItem* m_item;
    CommandType m_command;
    QStringList m_overrideTargets;
    MakeVariables m_variables;
    KDevelop::ProcessLineMaker* m_lineMaker;
    KProcess* m_process;
    bool m_killed;
    bool firstError;
};

#endif // MAKEJOB_H

