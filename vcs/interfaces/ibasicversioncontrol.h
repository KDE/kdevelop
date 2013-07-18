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

#ifndef KDEVPLATFORM_IBASICVERSIONCONTROL_H
#define KDEVPLATFORM_IBASICVERSIONCONTROL_H

#include <KDE/KUrl>
#include "../vcsrevision.h"
#include "../vcsdiff.h"

class QString;
class KTextEdit;

namespace KDevelop
{

class VcsJob;
class VcsLocation;
class VcsImportMetadataWidget;
class VcsLocationWidget;

/**
 * This is the basic interface that all Version Control or Source Code Management
 * plugins need to implement. None of the methods in this interface are optional.
 *
 * This only works on a local checkout from the repository, if your plugin should
 * offer functionality that works solely on the server see the
 * IRepositoryVersionControl interface
 *
 */

class KDEVPLATFORMVCS_EXPORT IBasicVersionControl
{
public:

    enum RecursionMode
    {
        Recursive    /**< run recursively through subdirectories */,
        NonRecursive /**< don't run recursively through subdirectories */
    };
    virtual ~IBasicVersionControl(){}

    /**
     * return a user-visible string for the version control plugin
     * Can be used for example when importing a project into a
     * version control system to choose the appropriate system
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
    virtual VcsImportMetadataWidget* createImportMetadataWidget( QWidget* parent ) = 0;

    /**
     * These methods rely on a valid vcs-directory with vcs-metadata in it.
     *
     * revisions can contain a date in format parseable by QDate, a number,
     * or the special words HEAD and BASE (whose exact meaning depends on the
     * used VCS system)
     */

    /**
     * Is the given @p localLocation under version control? This checks whether
     * the @p localLocation is under control of the versioning system or not.
     * It does not only check whether the @p localLocation lies in a version
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
     * adds a local unversioned file or directory to the list of versioned files.
     * @param localLocations a list of files or directories that should be put under version control
     * @param recursion whether to add directories and their children or only directories themselves
     * @returns a job that executes the addition
     */
    virtual VcsJob* add( const KUrl::List& localLocations,
                         RecursionMode recursion = IBasicVersionControl::Recursive ) = 0;

    /**
     * removes a local versioned file or directory from the version control system
     * @param localLocations the list of files/directories to remove from the VCS
     * @returns a job that executes the removal
     */
    virtual VcsJob* remove( const KUrl::List& localLocations ) = 0;

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
     * Fetches the status of the given local vcs locations.
     *
     * The returned VcsJob populates a QList<QVariant> that can be fetched
     * using fetchResults(). The QVariant inside the list wraps a
     * KDevelo::VcsStatusInfo object which contains all the relevant
     * information about the status of a specific file or folder
     */
    virtual VcsJob* status( const KUrl::List& localLocations,
                            RecursionMode recursion = IBasicVersionControl::Recursive ) = 0;

    /**
     * revert all local changes on the given file, making its content equal
     * to the version in the repository
     * unedit() (if not a no-op) is implied.
     */
    virtual VcsJob* revert( const KUrl::List& localLocations,
                            RecursionMode recursion = IBasicVersionControl::Recursive ) = 0;

    /**
     * fetches the latest changes from the repository, if there are
     * conflicts a merge needs to be executed separately
     *
     * @param localLocations the local files/dirs that should be updated
     * @param rev Update to this revision. The operation will fail if @p rev is
     * a range.
     * @param recursion defines whether the directories should be updated
     * recursively
     */
    virtual VcsJob* update( const KUrl::List& localLocations,
                            const VcsRevision& rev = VcsRevision::createSpecialRevision( VcsRevision::Head ),
                            RecursionMode recursion = IBasicVersionControl::Recursive ) = 0;

    /**
     * Checks in the changes of the given file(s)/dir(s) into the repository
     */
    virtual VcsJob* commit( const QString& message,
                            const KUrl::List& localLocations,
                            RecursionMode recursion = IBasicVersionControl::Recursive ) = 0;

    /**
     * Retrieves a diff between two revisions of a file
     *
     * The diff is in unified diff format for text files by default
     */
    virtual VcsJob* diff( const KUrl& fileOrDirectory,
                          const VcsRevision& srcRevision,
                          const VcsRevision& dstRevision,
                          VcsDiff::Type = VcsDiff::DiffUnified,
                          IBasicVersionControl::RecursionMode recursion
                                       = IBasicVersionControl::Recursive ) = 0;

    /**
     * Retrieve the history of a given local url
     *
     * The resulting VcsJob will emit the resultsReady signal every time new
     * log events are available. The fetchResults method will return a QList<QVariant>
     * where the QVariant is a KDevelop::VcsEvent.
     *
     * @param rev List @p rev and earlier.
     * @param limit Restrict to the most recent @p limit entries. Note that the
     * limit is @e advisory and may be ignored.
     */
    virtual VcsJob* log( const KUrl& localLocation,
                         const VcsRevision& rev,
                         unsigned long limit = 0 ) = 0;

    /**
     * Retrieve the history of a given local url
     *
     * The resulting VcsJob will emit the resultsReady signal every time new
     * log events are available. The fetchResults method will return a QList<QVariant>
     * where the QVariant is a KDevelop::VcsEvent.
     *
     * @param rev List @p rev and earlier. The default is BASE.
     * @param limit Do not show entries earlier than @p limit. Note that the
     * limit is @e advisory and may be ignored. The default is the first revision.
     */
    virtual VcsJob* log( const KUrl& localLocation,
                         const VcsRevision& rev = VcsRevision::createSpecialRevision( VcsRevision::Base ),
                         const VcsRevision& limit= VcsRevision::createSpecialRevision( VcsRevision::Start ) ) = 0;

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
                              const VcsRevision& rev = VcsRevision::createSpecialRevision( VcsRevision::Head ) ) = 0;

    /**
     * check for conflicts in the given file and eventually present a
     * conflict solving dialog to the user
     */
    virtual VcsJob* resolve( const KUrl::List& localLocations,
                             RecursionMode recursion ) = 0;

    /**
     * Creates a working copy of the latest revision of the repository sourceRepository in
     * the newly created directory destinationDirectory.
     * In the case of a decentralized VCSs, there is no working copy without repository,
     * therefor checkout() includes the creation of a local copy of the repository.
     * 
     * @param sourceRepository source of the checkout
     * @param destinationDirectory location of the created working copy (local repository)
     */
    virtual VcsJob* createWorkingCopy(const VcsLocation & sourceRepository, const KUrl & destinationDirectory, RecursionMode recursion = IBasicVersionControl::Recursive) = 0;
    
    virtual VcsLocationWidget* vcsLocation(QWidget* parent) const=0;

    /**
     * Optionally apply VCS specific settings to the commit message editor.
     * Overwrite this to set e.g. a default commit message or to setup highlighting or validation.
     * The default implementation does nothing.
     */
    virtual void setupCommitMessageEditor(const KUrl&, KTextEdit* edit) const;
};

}

Q_DECLARE_INTERFACE( KDevelop::IBasicVersionControl, "org.kdevelop.IBasicVersionControl" )

#endif

