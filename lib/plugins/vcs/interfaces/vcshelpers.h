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
    Added           /**<File was added to the repository but not committed.*/,
    Uptodate        /**<File was updated or it is already at up to date version.*/,
    Modified        /**<File was modified locally.*/,
    Conflict        /**<Local version has conflicts that need to be resolved before commit.*/,
    Deleted         /**<File or Directory is scheduled to be deleted. */,
};

/**
 * Class that tells you what happened to a given repository location in a
 * specific revision.
 *
 * Combinations of some of the flags are possible, for example Add|Modified,
 * Copy|Modified or Merge|Modified, or when returned from VcsEvent::actions().
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
 * Encapsulates a vcs revision number, date or range of revisions
 */
class VcsRevision
{
public:
    /**
     * @note Not all VCS's support both FileNumber and GlobalNumber. For those
     * that don't, asking for one may give you the other, therefore you should
     * check which is returned. For example, CVS does not support GlobalNumber,
     * and Subversion does not support FileNumber, while Perforce supports both.
     */
    enum RevisionType
    {
        Special         /**<One of the special versions in RevisionSpecialType.*/,
        GlobalNumber    /**<Global repository version when item was last changed.*/,
        FileNumber      /**<Item's independent version number.*/,
        Date,
    };
    enum RevisionSpecialType
    {
        Head            /**<Latest revision in the repository.*/,
        Working         /**<The local copy (including any changes made).*/,
        Base            /**<The repository source of the local copy.*/,
        Previous        /**<The version prior the other one (only valid in functions that take two revisions).*/,
    };

    /**
     * Set the value of this revision
     */
    bool setRevisionValue( const QVariant& rev, RevisionType );

    /**
     * returns the type of the revision
     */
    RevisionType revisionType() const;

    /**
     * return the value of this revision
     */
    QString revisionValue() const;
};

/**
 * Small container class that has a mapping of
 * repository-location -> local location including a recursion flag
 *
 * Just a convenient API around QMap\<KUrl, QPair\<KUrl, MappingFlags\>\>
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

    void addMapping( const KUrl& sourceLocation,
                     const KUrl& destinationLocation,
                     MappingFlags recursion );
    void removeMapping( const KUrl& sourceLocation);
    KUrl::List sourceLocations();
    KUrl destinationLocation( const KUrl& sourceLocation );
    MappingFlags mappingFlags( const KUrl& sourceLocation ) const;
};

/**
 * Small container class that contains information about a history event of a
 * single repository item.
 */
class VcsItemEvent
{
public:
    KUrl repositoryLocation();
    KUrl repositoryCopySourceLocation(); // may be empty
    VcsRevision repositoryCopySourceRevision(); // may be invalid, even if rCSL is not
    VcsRevision revision(); // the FileNumber revision, may be the same as the GlobalNumber
    VcsActions actions();
};

/**
 * Small container class that contains information about a single revision.
 *
 * @note log() only returns information about the specific item that was asked
 * about. When working with a VCS that supports atomic commits (i.e. where a
 * revision might affect more than one item), use change() to retrieve
 * information about all items affected by a particular revision.
 */
class VcsEvent
{
public:
    VcsRevision revision(); // the GlobalNumber revision
    QString author();
    QDate date();
    QString message();
    VcsActions actions();
    QList<VcsItemEvent> items();
};

/**
 * This class encapsulates running a vcs job. It allows to start a job and
 * retrieve results
 *
 * @TODO: Should this stay an interface or do we just subclass KJob?
 */
class VcsJob
{
    /**
     * To easily check which type of job this is
     *
     * @TODO: Check how this can be extended via plugins, maybe use QFlag? (not
     * QFlags!)
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
        Annotate,
        Clone
    };

    /**
     * Simple enum to define how the job finished
     */
    enum FinishStatus
    {
        Succeeded,
        Canceled,
        Failed
    };

public:
    /**
     * This method will return all new results of the job. The actual data
     * type that is wrapped in the QVariant depends on the type of job.
     *
     * @note Results returned by a previous call to fetchResults are not
     * returned.
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
     * status. If there is no error, or the job is not finished, this is an
     * empty string.
     */
    QString errorMessage();

    /**
     * The job is started and the method returns control to the caller only
     * after the job has finished.
     *
     * There's no need to setup signal/slot connections to be able to use
     * this method, all results are available after exec returned.
     *
     * @return the FinishStatus
     */
    FinishStatus exec();

public Q_SLOTS:
    /**
     * The job is not allowed to emit any signals until this method has been
     * called. A plugin may either really start the job here, or it may
     * buffer any signals before start has been called.
     *
     * This method executes the job in an asynchronous way and the job
     * will emit signals.
     */
    void start();

    /**
     * This cancels the job. The job shall not emit any signals except the
     * finished() signal when the cancelling is done and any resources have
     * been cleaned up.
     */
    void cancel();

Q_SIGNALS:
    /**
     * This signal is emitted when new results are available. Depending on
     * the plugin and the operation, it may be emitted only once when all
     * results are ready, or several times.
     */
    void resultsReady( VcsJob* );

    /**
     * This signal is emitted when the job has completed. Depending on
     * @p FinishStatus, this may mean that the job failed, succeeded, or has
     * been fully cancelled.
     *
     * It is safe to delete the job once this signal has been emitted.
     * job.
     */
    void finished( VcsJob*, FinishStatus );
};

/**
 * Annotations for a local file.
 *
 * This class lets the user fetch informations for each line of a local file,
 * including date of last change, author of last change and revision of
 * last change to the line.
 */
class VcsAnnotation
{
public:
    /**
     * @return the local url of the file
     */
    KUrl location();
    /**
     * @return the number of lines in the file
     */
    int lineCount();
    /**
     * @param linenum the number of the line, counting from 0
     * @return the content of the specified line
     */
    QString line( int linenum );
    /**
     * @param linenum the number of the line, counting from 0
     * @return the revision of the last change on the specified line
     */
    VcsRevision revision( int linenum );
    /**
     * @param linenum the number of the line, counting from 0
     * @return the author of the last change on the specified line
     */
    QString author( int linenum );
    /**
     * @param linenum the number of the line, counting from 0
     * @return the date of the last change on the specified line
     */
    QDateTime date( int linenum );
};

class VcsDiff
{
    /**
     * Specify the type of difference the diff() method should create. Note that a
     * request for DiffUnified may not be honored, e.g. if the items being diffed are
     * binary rather than text.
     */
    enum VcsDiffType
    {
        DiffRaw         /**<Request complete copies of both items.*/,
        DiffUnified     /**<Request copy of first item with diff.*/,
        DiffDontCare    /**<Don't care; plugin will return whichever is easiest.*/,
    };

    enum VcsDiffContent
    {
        Binary          /** Binary diff, using the full content of both files.*/,
        Text            /** Textual diff.*/,
    };

    /**
     * @returns the type of diff, i.e. raw or unified
     */
    VcsDiffType type() const;

    /**
     * @returns the content type, i.e. binary or text
     */
    VcsDiffContent contentType() const;

    /**
     * @returns the binary content of the first file of the difference or
     * an empty QByteArray if this is a textual diff
     */
    QByteArray firstBinary() const;

    /**
     * @returns the binary content of the second file of the difference or
     * an empty QByteArray if this is a textual diff
     */
    QByteArray secondBinary() const;

    /**
     * @returns the textual content of the first file of the difference or
     * an empty QString if this is a binary diff
     */
    QString firstText() const;

    /**
     * @returns the textual content of the second file of the difference or
     * an empty QString if this is a unified or binary diff
     */
    QString secondText() const;

    /**
     * @returns the difference between the first and the second file in
     * unified diff format or an empty QString if this is a binary diff
     * or a textual diff using raw format
     */
    QString diff() const;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::MappingFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::VcsActions )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
