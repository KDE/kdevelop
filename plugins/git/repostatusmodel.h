/*
 * This file is part of KDevelop
 * Copyright 2020 Jonathan L. Verner <jonathan.verner@matfyz.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_REPO_STATUS_MODEL_H
#define KDEVPLATFORM_PLUGIN_REPO_STATUS_MODEL_H

#include "gitplugin.h"

#include <project/projectchangesmodel.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

class KJob;

namespace KDevelop {
    class IProject;
    class IDocument;
}

/**
 * This implements the model for vcs status of files in a project.
 *
 * The tree structure of the model is:
 *
 *   - project item
 *     - index
 *       - file with staged changes
 *       - ...
 *     - worktree
 *       - file with unstaged changes
 *       - ...
 *     - conflicts
 *       - file with unresolved conflicts
 *       - ...
 *     - untrackded
 *       - untracked file
 *       - ...
 *
 * It is modeled on (and part of the code is copied from) the @class ProjectChangesModel
 * (see kdevplatform/vcs/interfaces/ibasicversioncontrol.h)
 *
 * @author Jonathan L. Verner <jonathan.verner@matfyz.cz>
 */
class RepoStatusModel : public QStandardItemModel
{
    Q_OBJECT

public:
    enum ItemRoles {
        UrlRole = Qt::UserRole+1, /**< an url to the item */
        AreaRole,                 /**< the area where the item belongs (index, worktree, untracked) **/
        NameRole,                 /**< A human readable name of the item */
        BranchNameRole,           /**< the git branch on which the item is located */
        StatusRole,               /**< the git status of the item (GitPlugin::ExtendedState) */
        ReadableStatusRole,       /**< a human-readable git status of the item */
        ProjectUrlRole,          /**< the url of the project */
    };

    /**
     * The areas into which files are sorted. A file may be in
     * more than one area (e.g. if it has both staged and unstaged
     * changes, it will be in Index and WorkTree). The special
     * areas ProjectRoot, IndexRoot, WorkTreeRoot, UntrackedRoot and ConflictsRoot
     * correspond to the root items.
     */
    enum Areas {
        ProjectRoot,    /**< The root item of the project */
        IndexRoot,      /**< The root item of the staged changes */
        WorkTreeRoot,   /**< The root item of the unstaged changes */
        UntrackedRoot,  /**< The root item of the untracked files */
        ConflictRoot,   /**< The root item of the files with unresolved conflicts */
        Index,          /**< An item with staged changes */
        WorkTree,       /**< An item with unstaged changes */
        Untracked,      /**< An untracked item */
        Conflicts,      /**< An item with unresolved conflicts */
        None,           /**< None of the above */
    };


    explicit RepoStatusModel(QObject* parent);
    ~RepoStatusModel() override;

    /**
     * Updates the items described by `status`, i.e. moves the
     * item(s) corresponding to status.url to the respective areas and
     * sets their area role data.
     *
     * @param p the project the item belongs to
     * @param status the status data
     */
    void updateState(KDevelop::IProject* p, const KDevelop::VcsStatusInfo& status);

    /**
     * Runs a job to fetches the statuses of elements (given by `urls`)
     * of project `project`.
     *
     * @param project the project the elements belong to
     * @param urls the urls to fetch the status for
     * @param mode whether to recurse into subdirectories, if an url is a directory
     */
    void fetchStatusesForUrls(KDevelop::IProject* project, const QList<QUrl>& urls,
                              KDevelop::IBasicVersionControl::RecursionMode mode);

    /**
     * Returns a list of items corresponding to project roots.
     */
    const QList<QStandardItem*> projectRoots() const;

    /**
     * Retrieves the items in a given area of a project.
     *
     * @param project a project root
     * @param area    the area to get the items from
     *
     * @returns a list of items in the given area of the project
     */
    const QList<QStandardItem*> items(const QStandardItem* project, Areas area) const;

    /**
     * The collection of top-level items (i.e. the ones not containing file items)
     * corresponding to a project.
     */
    struct ProjectItem {
        QStandardItem *project, *index, *worktree, *conflicts, *untracked;
    };

    /**
     * Returns the ProjectItem structure (containing the project, index, worktree,
     * conflicts & untracked items) for project `p`.
     *
     * @note: The project must be present in the model.
     *
     * @param p the project
     */
    ProjectItem projectItem(KDevelop::IProject* p) const;

    /**
     * Returns the ProjectItem structure (containing the project, index, worktree,
     * conflicts & untracked items) for the project whose ProjectRoot
     * item is `p_item`
     *
     * @param p_item the ProjectRoot item
     */
    ProjectItem projectItem(QStandardItem* p_item) const;

public Q_SLOTS:

    /**
     * Updates all open projects and removes projects which are not open anymore.
     */
    void reloadAll();

    /**
     * Updates all items belonging to the projects `p` (and removes the ones
     * no longer in the projects).
     *
     * @param projects the projects to update
     */
    void reload(const QList<KDevelop::IProject*>& projects);

    /**
     * Updates the projects given byl urls `p`.
     *
     * @param projects the urls of projects to update
     */
    void reload(const QList<QUrl>& projects);

    /**
     * Adds a new project to the model (e.g. when a new project is opened).
     *
     * @param p the project to add.
     */
    void addProject(KDevelop::IProject* p);

    /**
     * Removes a project from the model (e.g. when a project is closed).
     *
     * @param p the project to remove
     */
    void removeProject(KDevelop::IProject* p);

    /**
     * Processes the result of a job started by `fetchStatusesForUrls`
     *
     * @param job the KJob to whose results are processed.
     */
    void statusReady(KJob* job);

    /**
     * A handler called to update the status of the item(s) in the model
     * when a document is saved
     *
     * @param doc the document (the item(s) corresponding to this will be
     * updated)
     */
    void documentSaved(KDevelop::IDocument* doc);

    /**
     * A handler called when items are added to a project
     */
    void itemsAdded(const QModelIndex& idx, int start, int end);

    /**
     * A handler called when a job is unregistered.
     *
     * This is used to monitor when jobs run by the VCS
     * (e.g. git add, git commit, etc.) finish so that
     * we can schedule an update of the corresponding projects.
     */
    void jobUnregistered(KJob*);

    /**
     * A handler called when the branch is switched in a repo
     * to schedule the update of the name of the project.
     */
    void repositoryBranchChanged(const QUrl& url);

    /**
     * A handler for processing the result of a job getting
     * the current branch name of a repo. (this job is scheduled
     * by the `repositoryBranchChanged` method)
     */
    void branchNameReady(KDevelop::VcsJob* job);

private:
    /* Functions determining which area(s) a file should be listed in
     * depending on its status */
    static bool showInIndex(const GitPlugin::ExtendedState state);
    static bool showInWorkTree(const GitPlugin::ExtendedState state);
    static bool showInConflicts(const GitPlugin::ExtendedState state);
    static bool showInUntracked(const GitPlugin::ExtendedState state);

    /**
     * Removes all items with url `url` from the model.
     *
     * @param url the url to remove
     * @param parent the parent whose children will be processed (if it is
     * null all elements in the model will be processed)
     */
    void removeUrl(const QUrl& url, const QStandardItem* parent = nullptr);

    /**
     * Return the list of all urls for items in project `project`
     *
     * @param project the project
     */
    const QList<QUrl> childUrls(const ProjectItem project) const;

    /**
     * A helper function to recursively compute all the descendants of
     * `parent` and return them as a list. If `parent` is null, all items
     * (except the invisible root) are returned
     *
     * @param parent the node whose children will be returned (if it is
     * null all non-root nodes will be returned)
     */
    const QList<QStandardItem*> allItems(const QStandardItem* parent = nullptr) const;
};

#endif
