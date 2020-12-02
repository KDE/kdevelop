/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SHELLCHECK_JOB_H
#define SHELLCHECK_JOB_H

#include <interfaces/iproblem.h>
#include <outputexecutejob.h>

namespace shellcheck {
    
class Parameters;
class OutputParser;

/**
 * This job handles calling the shellcheck executable and getting the
 * output parsed
 */
class Job : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    explicit Job(const Parameters& params, QObject* parent = nullptr);
    ~Job() override;

    void start() override;

Q_SIGNALS:
    void problemsDetected(const QVector<KDevelop::IProblem::Ptr>& problems);


protected Q_SLOTS:
    void postProcessStdout(const QStringList& lines) override;
    void postProcessStderr(const QStringList& lines) override;

    void childProcessExited(int exitCode, QProcess::ExitStatus exitStatus) override;
    void childProcessError(QProcess::ProcessError processError) override;
    
private:
    QScopedPointer<OutputParser> m_parser;
    QVector<KDevelop::IProblem::Ptr> m_problems;

    QStringList m_shellCheckOutput;
};


} // end namespace

#endif // SHELLCHECK_JOB_H
