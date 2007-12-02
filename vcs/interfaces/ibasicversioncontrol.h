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

#ifndef IBASICVERSIONCONTROL_H
#define IBASICVERSIONCONTROL_H

#include <iextension.h>
#include <kurl.h>
#include "../vcsrevision.h"
#include "../vcsdiff.h"

class QString;

namespace KDevelop
{

class VcsJob;
class VcsMapping;
class VcsLocation;
class VcsImportMetadataWidget;
/**
 * This is the basic interface that all Version Control or Source Code Management
 * plugins need to implement. None of the methods in this interface are optional.
 *
 * This only works on a local checkout from the repository, if your plugin should
 * offer functionality that works solely on the server see the
 * IRepositoryVersionControl interface
 *
 */

class IBasicVersionControl
{
public:

    enum RecursionMode
    {
        Recursive,
        NonRecursive
    };
    virtual ~IBasicVersionControl(){}

    /**
     * return a user-visible string for the version control plugin
     * Can be used for example when importing a project into a
     * version control system to choose the apropriate system
     *
     * @returns a translated user-visible name for this version control plugin
     */
    virtual QString name() const = 0;
    /**
     * provides a widget that fetches the needed input data from the user
     * to import a project into a version control system.
     *
     * If this returns 0 the plugin will not be available as an option for import
     * when creating a new project
     *
     * @param parent the parent widget for the newly created widget
     * @returns a widget to fetch metadata needed to import a project
     */
    virtual VcsImportMetadataWidget* importMetadataWidget( QWidget* parent ) = 0;

    /**
     * These methods rely on a valid vcs-directory with vcs-metadata in it.
     *
     * revisions can contain a date in format parseable by QDate, a number,
     * or the special words HEAD and BASE (whose exact meaning depends on the
     * used VCS system)
     */

    /**
     * Is the given @p localLocation under version control? This checks wether
     * the @p localLocation is under control of the versioning system or not.
     * It does not only check wether the @p localLocation lies in a version
     * controlled directory
     *
     * @returns true if the version control system knows the given @p localLocation
     */
    virtual bool isVersionControlled( const KUrl& localLocation ) = 0;

    /**
     * Get the repository location of a local file
     */
    virtual VcsJob* repositoryLocation( const KUrl& localLocation ) = 0;

    /**
     * adds a local file/dir to the list of versioned files recursive is
     * only active for directories
     */
    virtual VcsJob* add( const KUrl::List& localLocations,
                         RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive ) = 0;

    /**
     * removes a local file/dir from the list of versioned files
     */
    virtual VcsJob* remove( const KUrl::List& localLocations ) = 0;

    /**
     * this is for files only, it makes a file editable, this may be a no-op
     */
    virtual VcsJob* edit( const KUrl& localLocation ) = 0;

    /**
     * this is for files only, it makes a file un-editable, this may be a no-op
     * This is different from revert because it doesn't change the content of the
     * file (it may fail if the file has changed).
     */
    virtual VcsJob* unedit( const KUrl& localLocation ) = 0;

    /**
     * retrieves status information for a file or dir recursive is only
     * active for directories
     */
    virtual VcsJob* status( const KUrl::List& localLocations,
                            RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive ) = 0;
    /**
     * gives the revision of file/dir, that is the revision to which this files
     * was updated when update() was run the last time
     */
    virtual VcsJob* localRevision( const KUrl& localLocation,
                                   VcsRevision::RevisionType ) = 0;

    /**
     * executes a copy of a file/dir, preserving history if the VCS system
     * allows that, may be implemented by filesystem copy+add
     */
    virtual VcsJob* copy( const KUrl& localLocationSrc,
                          const KUrl& localLocationDstn ) = 0;

    /**
     * moves src to dst, preserving history if the VCS system allows that, may
     * be implemented by copy+remove
     */
    virtual VcsJob* move( const KUrl& localLocationSrc,
                          const KUrl& localLocationDst ) = 0;

    /**
     * revert all local changes on the given file, making its content equal
     * to the version in the repository
     * unedit() (if not a no-op) is implied.
     */
    virtual VcsJob* revert( const KUrl::List& localLocations,
                            RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive ) = 0;

    /**
     * fetches the latest changes from the repository, if there are
     * conflicts a merge needs to be executed separately
     *
     * @param localLocation the local files/dirs that should be updated
     * @param rev Update to this revision. The operation will fail if @p rev is
     * a range.
     * @param recursion defines wether the directories should be updated
     * recursively
     */
    virtual VcsJob* update( const KUrl::List& localLocations,
                            const VcsRevision& rev,
                            RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive ) = 0;

    /**
     * Checks in the changes of the given file(s)/dir(s) into the repository
     */
    virtual VcsJob* commit( const QString& message,
                            const KUrl::List& localLocations,
                            RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive ) = 0;

    /**
     * Retrieves a diff between the two locations at the given revisions
     *
     * The diff is in unified diff format for text files by default
     */
    virtual VcsJob* diff( const VcsLocation& localOrRepoLocationSrc,
                          const VcsLocation& localOrRepoLocationDst,
                          const VcsRevision& srcRevision,
                          const VcsRevision& dstRevision,
                          VcsDiff::Type = KDevelop::VcsDiff::DiffUnified,
                          IBasicVersionControl::RecursionMode recursion
                                       = KDevelop::IBasicVersionControl::Recursive ) = 0;

    /**
     * Retrieve the history of a given local url
     *
     * The resulting VcsJob will emit the resultsReady signal every time new
     * log events are available. The fetchResults method will return a QList<QVariant>
     * where the QVariant is a KDevelop::VcsEvent.
     *
     * @param rev List @p rev and earlier. The default is HEAD.
     * @param limit Restrict to the most recent @p limit entries. Note that the
     * limit is @e advisory and may be ignored.
     */
    virtual VcsJob* log( const KUrl& localLocation,
                         const VcsRevision& rev,
                         unsigned long limit ) = 0;

    /**
     * Retrieve the history of a given local url
     *
     * The resulting VcsJob will emit the resultsReady signal every time new
     * log events are available. The fetchResults method will return a QList<QVariant>
     * where the QVariant is a KDevelop::VcsEvent.
     *
     * @param rev List @p rev and earlier. The default is HEAD.
     * @param limit Do not show entries earlier than @p limit. Note that the
     * limit is @e advisory and may be ignored.
     */
    virtual VcsJob* log( const KUrl& localLocation,
                         const VcsRevision& rev,
                         const VcsRevision& limit ) = 0;

    /**
     * Annotate each line of the given local url at the given revision
     * with information about who changed it and when.
     *
     * The job should return a QList<QVariant> where the QVariant wraps
     * a KDevelop::VcsAnnotationLine. The instance contains all information
     * needed for the caller to construct a KDevelop::VcsAnnotation
     *
     * @see KDevelop::VcsAnnotation
     * @see KDevelop::VcsAnnotationLine
     *
     * @param localLocation local file that should be annotated.
     * @param rev Revision that should be annotated.
     */
    virtual VcsJob* annotate( const KUrl& localLocation,
                              const VcsRevision& rev ) = 0;

    /**
     * merge/integrate the changes between src and dest into the given local file
     *
     * Note: This might create conflicts in the file(s) that are changed
     */
    virtual VcsJob* merge( const VcsLocation& localOrRepoLocationSrc,
                           const VcsLocation& localOrRepoLocationDst,
                           const VcsRevision& srcRevision,
                           const VcsRevision& dstRevision,
                           const KUrl& localLocation ) = 0;

    /**
     * check for conflicts in the given file and eventually present a
     * conflict solving dialog to the user
     */
    virtual VcsJob* resolve( const KUrl::List& localLocations,
                             RecursionMode recursion ) = 0;

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
    virtual VcsJob* import( const KUrl& localLocation,
                            const QString& repositoryLocation,
                            RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive ) = 0;

    /**
     * Checks out files or dirs from a repository into a local directory
     * hierarchy. The mapping exactly tells which file in the repository
     * should go to which local file
     */
    virtual VcsJob* checkout( const VcsMapping& mapping ) = 0;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IBasicVersionControl, "org.kdevelop.IBasicVersionControl" )
Q_DECLARE_INTERFACE( KDevelop::IBasicVersionControl, "org.kdevelop.IBasicVersionControl" )

#endif

