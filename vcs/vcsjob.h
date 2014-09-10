/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_VCSJOB_H
#define KDEVPLATFORM_VCSJOB_H

#include <outputview/outputjob.h>

#include "vcsexport.h"

class QVariant;
class QString;

namespace KDevelop
{

class IPlugin;

/**
 * This class provides an extension of KJob to get various VCS-specific
 * information about the job. This includes the type, the state
 * and the results provided by the job.
 *
 */
class KDEVPLATFORMVCS_EXPORT VcsJob : public OutputJob
{
    Q_OBJECT
public:
    VcsJob( QObject* parent = 0, OutputJobVerbosity verbosity = OutputJob::Verbose);
    virtual ~VcsJob();
    /**
     * To easily check which type of job this is.
     *
     * @TODO: Check how this can be extended via plugins, maybe use QFlag? (not
     * QFlags!)
     */
    enum JobType
    {
        Unknown = -1    /**< Unknown job type (default)*/,
        Add = 0         /**< An add job */,
        Remove = 1      /**< A remove job */,
        Copy = 2        /**< A copy job */,
        Move = 3        /**< A move job */,
        Diff = 4        /**< A diff job */,
        Commit = 5      /**< A commit job */,
        Update = 6      /**< An update job */,
        Merge = 7       /**< A merge job */,
        Resolve = 8     /**< A resolve job */,
        Import = 9      /**< An import job */,
        Checkout = 10   /**< A checkout job */,
        Log = 11        /**< A log job */,
        Push = 12       /**< A push job */,
        Pull = 13       /**< A pull job */,
        Annotate = 14   /**< An annotate job */,
        Clone = 15      /**< A clone job */,
        Status = 16     /**< A status job */,
        Revert = 17     /**< A revert job */,
        Cat = 18        /**< A cat job */,
        UserType = 1000 /**< A custom job */
    };

    /**
     * Simple enum to define how the job finished.
     */
    enum JobStatus
    {
        JobRunning = 0    /**< The job is running */,
        JobSucceeded = 1  /**< The job succeeded */,
        JobCanceled = 2   /**< The job was cancelled */,
        JobFailed = 3     /**< The job failed */,
        JobNotStarted = 4 /**< The job is not yet started */
    };

    /**
     * This method will return all new results of the job. The actual data
     * type that is wrapped in the QVariant depends on the type of job.
     *
     * @note Results returned by a previous call to fetchResults are not
     * returned.
     */
    virtual QVariant fetchResults() = 0;

    /**
     * Find out in which state the job is. It can be running, canceled,
     * failed or finished
     *
     * @return the status of the job
     * @see JobStatus
     */
    virtual JobStatus status() const = 0;

    /**
     * Used to find out about the type of job.
     *
     * @return the type of job
     */
    JobType type() const;

    /**
     * Used to get at the version control plugin. The plugin
     * can be used to get one of the interfaces to execute
     * more vcs actions, depending on this job's results
     * (like getting a diff for an entry in a log)
     */
    virtual KDevelop::IPlugin* vcsPlugin() const = 0;

    /**
     * This can be used to set the type of the vcs job in subclasses.
     */
    void setType( JobType );

Q_SIGNALS:
    /**
     * This signal is emitted when new results are available. Depending on
     * the plugin and the operation, it may be emitted only once when all
     * results are ready, or several times.
     */
    void resultsReady( KDevelop::VcsJob* );

private Q_SLOTS:
    void delayedModelInitialize();

private:
    class VcsJobPrivate* const d;
};

}

#endif

