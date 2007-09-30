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

#ifndef VCSJOB_H
#define VCSJOB_H

#include <kjob.h>

#include <vcsexport.h>

class QVariant;
class QString;
class KUrl;

namespace KDevelop
{

/**
 * This class provides an extension of KJob to get various Vcs
 * specific information about the job. This includes the type, the state
 * and the results provided by the job.
 *
 */
class KDEVPLATFORMVCS_EXPORT VcsJob : public KJob
{
    Q_OBJECT
public:
    VcsJob( QObject* parent = 0);
    virtual ~VcsJob();
    /**
     * To easily check which type of job this is
     *
     * @TODO: Check how this can be extended via plugins, maybe use QFlag? (not
     * QFlags!)
     */
    enum JobType
    {
        Add = 0,
        Remove = 1,
        Copy = 2,
        Move = 3,
        Diff = 4,
        Commit = 5,
        Update = 6,
        Merge = 7,
        Resolve = 8,
        Import = 9,
        Checkout = 10,
        Log = 11,
        Push = 12,
        Pull = 13,
        Annotate = 14,
        Clone = 15,
        Status = 16,
        Revert = 17,
        Cat = 18,
	UserType = 1000
    };

    /**
     * Simple enum to define how the job finished
     */
    enum JobStatus
    {
        JobRunning = 0,
        JobSucceeded = 1,
        JobCanceled = 2 ,
        JobFailed = 3,
        JobNotStarted = 4
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
     * Find out in which state the job is, it can be running, cancelled
     * failed or finished
     *
     * @return the status of the job
     * @see JobStatus
     */
    virtual JobStatus status() const = 0;

    /**
     * Used to find out about the type of job
     *
     * @return the type of job
     */
    JobType type();

protected:
    /**
     * This can be used to set the type of the vcs job in subclasses
     */
    void setType( JobType );

Q_SIGNALS:
    /**
     * This signal is emitted when new results are available. Depending on
     * the plugin and the operation, it may be emitted only once when all
     * results are ready, or several times.
     */
    void resultsReady( VcsJob* );

private:
    class VcsJobPrivate* const d;
};

}

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
