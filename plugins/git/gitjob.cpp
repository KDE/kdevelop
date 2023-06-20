/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gitjob.h"

#include <QDir>

#include <KProcess>

GitJob::GitJob(const QDir& workingDir, KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : DVcsJob(workingDir, parent, verbosity)
{
    setType(VcsJob::UserType);
    process()->setEnv(QStringLiteral("GIT_OPTIONAL_LOCKS"), QStringLiteral("0"));
}

#include "moc_gitjob.cpp"
