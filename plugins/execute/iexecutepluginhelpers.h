/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_IEXECUTEPLUGIN_HELPERS_H
#define KDEVPLATFORM_PLUGIN_IEXECUTEPLUGIN_HELPERS_H

#include "iexecuteplugin.h"

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <util/executecompositejob.h>

#include <QDebug>

/**
 * @return a job that combines a given job with its dependency, if any
 *
 * @param dependentJob a non-null job that may have a dependency
 * @param execute a plugin to create the dependency job
 * @param config a non-null launch configuration for @p execute to retrieve the dependency job specification from
 *
 * If @p dependentJob has no dependency, the job itself is returned.
 *
 * If @p dependentJob already has an error code set, the dependency job is not created and the job itself is returned.
 * In this case @p dependentJob should fail immediately once started. So running the dependency job before starting
 * @p dependentJob would perform useless work and delay the inevitable failure. The doomed @p dependentJob
 * should still be started in order to emit result (the error) and let RunController display its error message.
 *
 * @note This function is inline so as to be applicable both to the ExecutePlugin and to the ExecutePlasmoidPlugin.
 */
[[nodiscard]] inline KJob* makeJobWithDependency(KJob* dependentJob, const IExecutePlugin& execute,
                                                 KDevelop::ILaunchConfiguration* config)
{
    Q_ASSERT(dependentJob);
    Q_ASSERT(config);

    if (dependentJob->error() != KJob::NoError) {
        return dependentJob;
    }

    auto* const dependency = execute.dependencyJob(config);
    if (!dependency) {
        return dependentJob;
    }

    QObject::connect(dependentJob, &KJob::finished, dependency, [dependentJob, dependency] {
        // The dependent job must have been canceled if it finishes while the dependency is still alive.
        // Kill the no longer needed dependency job. This can also happen if a failure or a
        // cancellation of the dependency aborts the composite job and consequently the unstarted
        // dependent job, in which case killing the already finished dependency here has no effect.
        qDebug() << "dependent job" << dependentJob << "finished, so killing its obsolete dependency" << dependency;
        dependency->kill();
    });

    auto* const compositeJob =
        new KDevelop::ExecuteCompositeJob(KDevelop::ICore::self()->runController(), {dependency, dependentJob});
    // ExecuteCompositeJob() adopts the nonempty name of its first subjob.
    // Give it the name of the primary job instead of the name of the less important dependency job.
    compositeJob->setObjectName(dependentJob->objectName());
    return compositeJob;
}

#endif // KDEVPLATFORM_PLUGIN_IEXECUTEPLUGIN_HELPERS_H
