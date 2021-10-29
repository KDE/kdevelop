/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2002 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
    SPDX-FileCopyrightText: 2011 Damien Coppel <damien.coppel@gmail.com>
    SPDX-FileCopyrightText: 2011 Lionel Duc <lionel.data@gmail.com>
    SPDX-FileCopyrightText: 2013 Christoph Thielecke <crissi99@gmx.de>
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CPPCHECK_JOB_H
#define CPPCHECK_JOB_H

#include "parameters.h"

#include <interfaces/iproblem.h>
#include <outputview/outputexecutejob.h>

class QElapsedTimer;

namespace cppcheck
{

class CppcheckParser;

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

protected:
    void emitProblems();

    QScopedPointer<QElapsedTimer> m_timer;

    QScopedPointer<CppcheckParser> m_parser;
    QVector<KDevelop::IProblem::Ptr> m_problems;

    QStringList m_standardOutput;
    QStringList m_xmlOutput;

    bool m_showXmlOutput;

    KDevelop::Path m_projectRootPath;
};

}

#endif
