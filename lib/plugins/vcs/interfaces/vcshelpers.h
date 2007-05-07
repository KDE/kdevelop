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

#ifndef VCSHELPERS_H
#define VCSHELPERS_H

namespace KDevelop
{

/**
 * Status of a local file
 */
enum VcsState
{
    Unknown         /**<No VCS information about a file is known (or file is not under VCS control).*/,
    Added           /**<File was added to the repository but not commited.*/,
    Uptodate        /**<File was updated or it is already at up to date version.*/,
    Modified        /**<File was modified locally.*/,
    Conflict        /**<Local version has conflicts that need to be resolved before commit.*/,
    Deleted         /**<File or Directory is scheduled to be deleted. */,
};

/**
 * Class that tells you what happened to a given repository location in a
 * specific revision.
 *
 * Combinations of some of the flags are possible, for example add+modify,
 * copy+modify or merge+modify
 */
enum VcsAction
{
    ContentsModified     /**<Directory was not changed (only contents changed).*/,
    Add                  /**<File was added.*/,
    Delete               /**<File was deleted.*/,
    Modified             /**<File was modified, for example by editing.*/,
    Copy                 /**<File was copied.*/,
    Merge                /**<File had changes merged into it.*/,
};
Q_DECLARE_FLAGS( VcsActions, VcsAction )

/**
 * Specify the type of difference the diff() method creates
 */
enum VcsDiffMode
{
    DiffRaw         /**<Request complete copies of both items.*/,
    DiffUnified     /**<Request copy of first item with diff.*/,
    DiffDontCare    /**<Don't care; plugin will return whichever is easiest.*/,
};

/**
 * Encapsulates a vcs revision number, date or range of revisions
 */
class VcsRevision
{
public:
    enum RevisionType
    {
        GlobalNumber    /**<Global repository version when file was last changed./*/,
        FileNumber      /**<File version number, may be the same as GlobalNumber.*/,
        Date,
        Range
    };
    setRevisionValue( const QVariant& rev, RevisionType );
    setSourceRevision( const Revision& );
    setTargetRevision( const Revision& );
    RevisionType revisionType() const;
    QString revisionValue() const;
    Revision sourceRevision() const;
    Revision targetRevision() const;
};

/**
 * Small container class that has a mapping of
 * repository-location -> local location including a recursion flag
 *
 * Just a convenient API around QMap<KUrl, QPair<KUrl, MappingFlags>>
 *
 */
class VcsMapping
{
public:
    enum MappingFlag
    {
        Recursive = 1,
        NonRecursive = 2
    };
    Q_DECLARE_FLAGS( MappingFlags, MappingFlag )

    void addMapping( const KUrl& repositoryLocation,
                     const KUrl& localLocation,
                     MappingFlags recursion );
    void removeMapping( const KUrl& repositoryLocation);
    KUrl::List repositoryLocations();
    KUrl localLocation( const KUrl& repositoryLocation );
    MappingFlags mappingFlags( const KUrl& repositoryLocation ) const;
};

/**
 * Small container class that contains information about a history event of a
 * single repository item. This class contains information that is common to a
 * VcsEvent (which represents a single item) and a VcsChange (which may have
 * information about several items).
 */
class VcsItemEvent
{
public:
    KUrl repositoryLocation();
    KUrl repositoryCopySourceLocation(); // may be NULL
    VcsActions actions();
};

/**
 * Small container class that contains information about a history event of a
 * single change concerning a particular repository item.
 */
class VcsEvent
{
public:
    VcsRevision revision();
    QString user();
    QDate date();
    QString message();
    VcsItemEvent item();
};

/**
 * Small container class that contains information about a single change.
 *
 * The actions() will be the comined actions of all items. This corresponds to
 * a cvs or svn commit for example.
 */
class VcsChange
{
    VcsRevision revision();
    QString user();
    QDate date();
    QString message();
    QList<VcsItemEvent> items();
};

/**
 * This class encapsulates running a vcs job. It allows to start a job and retrieve
 * results
 *
 * @TODO: Should this stay an interface or do we just subclass KJob?
 */
class VcsJob
{
    /**
     * To easily check which type of job this is
     *
     * @TODO: Check how this can be extended via plugins, maybe use QFlag? (not QFlags!)
     */
    enum VcsJobType
    {
        Add,
        Remove,
        Copy,
        Move,
        Diff,
        Commit,
        Update,
        Merge,
        Resolve,
        Import,
        Checkout,
        Log,
        Push,
        Pull,
        Clone
    };

    /**
     * Simple enum to define how the job finished
     */
    enum FinishStatus
    {
        Done,
        Cancelled,
        Error
    };

    public:
        /**
         * This method will return all available results of the job. The actual
         * data type that is wrapped in the QVariant depends on the type of job.
         */
        QVariant fetchResults();

        /**
         * Used to find out about the type of job
         *
         * @return the type of job
         */
        VcsJobType type();

        /**
         * Can be used to obtain an error message if the job exited with an error
         * status. If there's no error or the job is not finished this is an
         * empty string.
         */
        QString errorMessage();

    public Q_SLOTS:
        /**
         * The job is not allowed to emit any signals until this method has been called.
         * A plugin may either really start the job here or it may as well buffer any
         * signals before start has been called.
         */
        void start();

        /**
         * This cancels the job, the job shall not emit any signals excep the finished()
         * signal when the cancelling is done and any resources have been cleaned up
         */
        void cancel();

    Q_SIGNALS:
        /**
         * this signal is emitted whenever new results are available
         */
        void resultsReady( VcsJob* );
        void finished( VcsJob*, FinishStatus );
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::MappingFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::VcsActions )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
