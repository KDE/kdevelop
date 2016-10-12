/* This file is part of KDevelop
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2002 Harald Fernengel <harry@kdevelop.org>
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
 * Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CPPCHECK_JOB_H
#define CPPCHECK_JOB_H

#include <interfaces/iproblem.h>
#include <outputview/outputexecutejob.h>

namespace cppcheck
{

class CppcheckParser;

class Job : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    struct Parameters
    {
        QString parameters;
        bool checkStyle = false;
        bool checkPerformance = false;
        bool checkPortability = false;
        bool checkInformation = false;
        bool checkUnusedFunction = false;
        bool checkMissingInclude = false;
        QString path;
        QString executable;
    };

    Job(const Parameters& params, QObject* parent = nullptr);
    ~Job() override;

    void start() override;

Q_SIGNALS:
    void problemsDetected(const QVector<KDevelop::IProblem::Ptr>& problems);

protected slots:
    void postProcessStdout(const QStringList& lines) override;
    void postProcessStderr(const QStringList& lines) override;

    void childProcessExited(int exitCode, QProcess::ExitStatus exitStatus) override;
    void childProcessError(QProcess::ProcessError processError) override;

protected:
    QScopedPointer<CppcheckParser> m_parser;

    QStringList m_standardOutput;
    QStringList m_xmlOutput;
};

}

#endif
