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

#ifndef KDEVPLATFORM_IBRANCHINGVERSIONCONTROL_H
#define KDEVPLATFORM_IBRANCHINGVERSIONCONTROL_H

#include <QObject>

class QUrl;
class QString;

namespace KDevelop
{
class VcsRevision;
class VcsJob;

class IBranchingVersionControl
{
public:
    virtual ~IBranchingVersionControl() {}
    
    /**
     * Creates a branch from the given mapping information
     *
     * @param repository repository directory on which we're going to branch
     * @param rev What revision of the requested items should be branched.
     * @param branchName Short, descriptive name for the branch used for VCS's
     * that take branch names instead of destination paths, or store metadata
     * about the branch. Not all VCS's will use this parameter.
     */
    virtual VcsJob* branch( const QUrl& repository,
                            const VcsRevision& rev,
                            const QString& branchName ) = 0;


    /**
     * Creates a new tag from the given mapping information
     *
     * @param rev What revision of the requested items should be tagged.
     * @param tagName Short, descriptive name for the tag used for VCS's that
     * take tag names instead of destination paths, or store metadata about the
     * tag. Not all VCS's will use this parameter.
     */
    virtual VcsJob* tag( const QUrl& repository,
                         const QString& commitMessage,
                         const VcsRevision& rev,
                         const QString& tagName ) = 0;
                         
   /**
    * Switches to the desired branch inside the specified repository
    *
    * @param repository The repository path where the branch will be switched
    * @param branchName The branch name that will be switched to inside the specified repostory
    */
   virtual VcsJob* switchBranch(const QUrl &repository, const QString &branchName) = 0;

   /**
    * Returns the branches inside the specified repository in a QStringList
    *
    * @param repository The repository path where the branches will be returned from
    */
   virtual VcsJob* branches(const QUrl &repository) = 0;

   /**
    * Returns the current branch name inside the specified repository in a QString
    *
    * @note If we are not on a branch currently, the string will be empty
    *
    * @param repository The repository path where the current branch will be returned from
    */
   virtual VcsJob* currentBranch(const QUrl &repository) = 0;

   /**
    * Deletes the desired branch inside the specified repository
    *
    * @param repository The repository path where the current branch will be deleted from
    * @param branchName The branch name that will be deleted inside the specified repository
    */
   virtual VcsJob* deleteBranch(const QUrl &repository, const QString &branchName) = 0;
   
   /**
    * Deletes the desired branch inside the specified repository
    *
    * @param repository The repository path where the current branch will be deleted from
    * @param oldBranchName The branch name that will be renamed
    * @param newBranchName The new branch name
    */
   virtual VcsJob* renameBranch(const QUrl &repository, const QString &oldBranchName, const QString &newBranchName) = 0;

   /**
    * Tells the implementation to report about changes in the current branch of a
    * given @p repository.
    */
   virtual void registerRepositoryForCurrentBranchChanges(const QUrl& repository) = 0;

Q_SIGNALS:
    void repositoryBranchChanged(const QUrl& repository);
};

}

Q_DECLARE_INTERFACE( KDevelop::IBranchingVersionControl, "org.kdevelop.IBranchingVersionControl" )

#endif

