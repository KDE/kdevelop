/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef IBASICVERSIONCONTROLH_H
#define IBASICVERSIONCONTROLH_H

#include "vcshelpers.h"

namespace KDevelop
{

/**
 * This is the basic interface that all Version Control or Source Code Management
 * plugins need to implement. None of the methods in this interface is optional.
 *
 * This only works on a local checkout from the repository, if your plugin should
 * offer functionality that works solely on the server see the
 * IRepositoryVersionControl interface
 *
 * @TODO: Use KJob instead of int as return value and move wait to KJob and possibly the return-value-retrieval too
 */
class KDEVVCS_EXPORT IBasicVersionControl
{
public:

    enum RecursionMode
    {
        Recursive,
        NonRecursive
    };
    virtual ~IBasicVersionControl(){}

    /**
     * These methods rely on a valid vcs-directory with vcs-metadata in it.
     *
     * revisions can contain a date in format parseable by QDate, a number,
     * or the special words HEAD and BASE (whose exact meaning depends on the
     * used VCS system)
     *
     */

    /**
     * Is the given location under version control?
     */
    virtual bool isVersionControlled( const KUrl& localLocation ) = 0;

    /**
     * adds a local file/dir to the list of versioned files recursive is
     * only active for directories
     */
    virtual int add( const KUrl::List& localLocations,
                      RecursiveMode recursion ) = 0;

    /**
     * removes a local file/dir from the list of versioned files recursive
     * is only active for directories
     */
    virtual int remove( const KUrl::List& localLocations,
                         RecursiveMode recursion ) = 0;

    /**
     * this is for files only, it makes a file editable, this may be a no-op
     */
    virtual int edit( const KUrl& localLocation ) = 0;

    /**
     * this is for files only, it makes a file un-editable, this may be a no-op
     * This is different from revert because it doesn't change the content of the
     * file (it may fail if the file has changed).
     */
    virtual int unedit( const KUrl& localLocation ) = 0;

    /**
     * retrieves status information for a file or dir recursive is only
     * active for directories
     */
    virtual int status( const KUrl& localLocation,
                        RecursiveMode recursion ) = 0;
    /**
     * gives the revision of file/dir, that is the revision to which this files
     * was updated when update() was run the last time
     */
    virtual int localRevision( const KUrl& localLocation,
                                    Revision::Type ) = 0;

    /**
     * executes a copy of a file/dir, preserving history if the VCS system
     * allows that, may be implemented by filesystem copy+add
     */
    virtual int copy( const KUrl& localLocationSrc,
                       const KUrl& localLocationDst,
                       RecursiveMode recursion ) = 0;

    /**
     * moves src to dst, this may be implemented as copy+remove,
     * depending on the VCS, may preserve history if VCS allows that
     */
    virtual int move( const KUrl& localLocationSrc,
                      const KUrl& localLocationDst,
                      RecursiveMode recursion ) = 0;

    /**
     * revert all local changes on the given file, making its content equal
     * to the version in the repository
     * un-edit() (if not a no-op) is implied
     */
    virtual int revert( const KUrl::List& localLocations,
                        RecursiveMode recursion ) = 0;

    /**
     * fetches the latest changes from the repository, if there are
     * conflicts a merge needs to be executed separately
     */
    virtual int update( const KUrl::List& localLocations,
                        RecursiveMode recursion ) = 0;

    /**
     * Checks in the changes of the given file/dir into the repository
     */
    virtual int commit( const KUrl::List& localLocations,
                        RecursiveMode recursion ) = 0;

    /**
     * Shows a diff between the two locations at the given revisions
     *
     * The QVariant should either be a KUrl, which is assumed to be a local file
     * or a QString which is assumed to define a repository path
     */
    virtual int diff( const QVariant& localOrRepoLocationSrc,
                      const QVariant& localOrRepoLocationDst,
                      const Revision& srcRevision,
                      const Revision& dstRevision ) = 0;

    /**
     * Show the history of changes of a given local url at a given revision
     */
    virtual int log( const KUrl& localLocation ) = 0;

    /**
     * merge/integrate the changes between src and dest into the given local file
     *
     * The QVariant should either be a KUrl, which is assumed to be a local file
     * or a QString which is assumed to define a repository path
     */
    virtual int merge( const QVariant& localOrRepoLocationSrc,
                       const QVariant& localOrRepoLocationDst,
                       const Revision& srcRevision,
                       const Revision& dstRevision,
                       const KUrl& localLocation ) = 0;

    /**
     * check for conflicts in the given file and eventually present a
     * conflict solving dialog to the user
     */
    virtual int resolve( const KUrl::List& localLocations,
                         RecursiveMode recursion ) = 0;

    /**
     * The following two methods are part of the basic interface so other plugins
     * can depend on them, for example the appwizard. These two don't need a
     * valid VCS-dir, in fact they should be invoked with a local directory that
     * is either empty or contains a non-VCed project
     */

    /**
     * take the local directory and add it into the repository at the given
     * target
     */
    virtual int import( const KUrl& localLocation,
                         const QString& repositoryLocation,
                         RecursiveMode recursion ) = 0;

    /**
     * Checks out files or dirs from a repository into a local directory
     * hierarchy. The mapping exactly tells which file in the repository
     * should go to which local file
     */
    virtual int checkout( const VCSMapping& mapping ) = 0;

    /**
     * Get the repository location of a local file
     */
    virtual int repositoryLocation( const KUrl& localLocation ) = 0;


    /**
     * This waits until the given action is finished and returns
     * a QVariant containing either the value (for status for example),
     * an empty string if the action doesn't return anything or an
     * error message
     */
    virtual QVariant wait( int id ) = 0;

Q_SIGNALS:
    virtual void commandFinished( int id, const QVariant& data ) = 0;
    virtual void commandFailed( int id, const QString& errormsg ) = 0;
};

}

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
