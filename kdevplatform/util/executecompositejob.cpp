/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "executecompositejob.h"
#include "debug.h"

namespace KDevelop {

ExecuteCompositeJob::ExecuteCompositeJob(QObject* parent, const QList<KJob*>& jobs)
    : KDevCoreAddons::KSequentialCompoundJob(parent)
{
    qCDebug(UTIL) << "execute composite" << jobs;
    for (KJob* job : jobs) {
        if (!job) {
            qCWarning(UTIL) << "Added null job!";
            continue;
        }
        addSubjob(job);
        if (objectName().isEmpty())
            setObjectName(job->objectName());
    }
}

ExecuteCompositeJob::~ExecuteCompositeJob() = default;

}

#include "moc_executecompositejob.cpp"
