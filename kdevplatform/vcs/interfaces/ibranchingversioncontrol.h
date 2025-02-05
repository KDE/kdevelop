/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    * @param branchName The branch name that will be switched to inside the specified repository
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
    * @param repository The repository path where the specified branch will be deleted
    * @param branchName The branch name that will be deleted inside the specified repository
    */
   virtual VcsJob* deleteBranch(const QUrl &repository, const QString &branchName) = 0;

   /**
    * Merges the selected branch into the current one.
    * @param repository The repository path where the current branch will be merged into.
    * @param branchName The name of the selected branch to be merged into the current one.
    */
   virtual VcsJob* mergeBranch(const QUrl &repository, const QString &branchName) = 0;

   /**
    * Renames the desired branch inside the specified repository
    *
    * @param repository The repository path where the specified branch will be renamed
    * @param oldBranchName The branch name that will be renamed
    * @param newBranchName The new branch name
    */
   virtual VcsJob* renameBranch(const QUrl &repository, const QString &oldBranchName, const QString &newBranchName) = 0;

    /**
     * Tells the implementation to report about changes in the current branch of a
     * given repository to a given non-null listener by emitting the signal repositoryBranchChanged().
     */
    virtual void registerRepositoryForCurrentBranchChanges(const QUrl& repository, const QObject* listener) = 0;
    /**
     * Tells the implementation that a given non-null listener is no longer
     * interested in current branch changes to a given repository.
     */
    virtual void unregisterRepositoryForCurrentBranchChanges(const QUrl& repository, const QObject* listener) = 0;

Q_SIGNALS:
    void repositoryBranchChanged(const QUrl& repository);
};

}

Q_DECLARE_INTERFACE( KDevelop::IBranchingVersionControl, "org.kdevelop.IBranchingVersionControl" )

#endif

