/*
    SPDX-FileCopyrightText: 2020 Jonathan L. Verner <jonathan.verner@matfyz.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_COMMIT_TOOLVIEW_H
#define KDEVPLATFORM_PLUGIN_COMMIT_TOOLVIEW_H

#include "repostatusmodel.h"

#include <interfaces/iuicontroller.h>

#include <QWidget>

class ActiveStyledDelegate;
class DiffViewsCtrl;
class FilterEmptyItemsProxyModel;
class SimpleCommitForm;

class QAction;
class QMenu;
class QModelIndex;
class QLineEdit;
class QTreeView;
class QUrl;

namespace KDevelop {
    class IBasicVersionControl;
    class IProject;
}

/**
 * This implements the git-cola like toolview for preparing commits.
 *
 * The view contains a list of projects. Each project contains four
 * lists:
 *
 *   - the staged changes list lists all files in the project which
 *     have changes staged for commit;
 *   - the unstaged changes list lists all files in the project which
 *     have changes which are not currently staged for commit;
 *   - the conflicts list lists all files which have unresolved (merge)
 *     conflicts; and
 *   - the untracked list which lists all files not tracked in the VCS
 *
 * Clicking on a file in one of the staged/unstaged lists opens a document
 * tab with the diff showing the changes. The user can then select lines/hunks
 * from the diff and remove/add them from the staged changes using the context menu.
 *
 * Double clicking on a file will, instead, stage/unstage all changes in the file
 * or mark the conflicts as resolved or add the file to be tracked in VCS.
 *
 * Above these lists a lineedit and a textedit may be used to prepare a
 * commit message. The commit button will commit the staged changes to the
 * repo. If several projects are listed, the one which is expanded will be
 * used (only one project is allowed to be expaned to show the lists at a time,
 * an expaned project is automatically collapsed when a different one is expanded).
 *
 * @author Jonathan L. Verner <jonathan.verner@matfyz.cz>
 */

class CommitToolView : public QWidget
{
    Q_OBJECT

public:
    enum ShowDiffParams { Activate, NoActivate };

    /**
     * @note: m_statusmodel remains the property of the caller whose
     * responsibility is to delete it (and care must be taken not
     * to delete it before the CommitToolView is deleted)
     */
    CommitToolView(QWidget* parent, RepoStatusModel* m_statusmodel);
    ~CommitToolView() override;

    /**
     * @returns the currently active project (i.e. the one that
     *          is expanded in the treeview)
     */
    KDevelop::IProject* activeProject() const;

    /**
     * @returns the index of the currently active project (i.e. the one that
     *          is expanded in the treeview)
     */
    QStandardItem* activeProjectItem() const;

    /**
     * @returns true if the item pointed to by the repostatusmodel index
     *          idx is the root item of the currently active project.
     */
    bool isActiveProject(const QModelIndex& idx) const;

Q_SIGNALS:

    /**
     * This signal is emitted when the view wants to show a diff
     *
     * @param url the url to display the changes for
     * @param area the type of changes to display
     */
    void showDiff(const QUrl& url, const RepoStatusModel::Areas area);

    /**
     * This signal is emitted when the view wants to show a file
     *
     * @param url the url of the file to show
     */
    void showSource(const QUrl& url);

    /**
     * This signal is emitted when the diff showing changes of type @param area
     * to the file @param url needs to be updated.
     */
    void updateDiff(const QUrl& url, const RepoStatusModel::Areas area);

    /**
     * This signal is emitted when all diffs showing changes to files in
     * project @param project need to be updated.
     */
    void updateProjectDiffs(KDevelop::IProject* project);

    /**
     * This signal is emitted when all diffs showing changes to the file
     * @param url need to be updated.
     *
     * @note: In contrast to the updateDiff signal, this also includes diffs
     * showing all changes to the owning project (staged/unstaged)
     */
    void updateUrlDiffs(const QUrl& url);

public Q_SLOTS:
    /**
     * Shows the toolview context menu
     */
    void popupContextMenu(const QPoint& pos);

    /**
     * A handler called when the user double clicks
     * an item in the treeview.
     */
    void dblClicked(const QModelIndex& idx);

    /**
     * A handler called when the user clicks an item
     * in the treeview.
     */
    void clicked(const QModelIndex& idx);

    /**
     * A handler called when a user expands an item
     * in the treeview.
     */
    void activateProject(const QModelIndex& idx);

    /**
     * Stages the staged changes in the given files.
     *
     * @param urls the list of files whose changes to stage
     */
    void stageSelectedFiles(const QList<QUrl>& urls);

    /**
     * Unstages the staged changes in the given files.
     *
     * @param urls the list of files whose changes to unstage
     */
    void unstageSelectedFiles(const QList<QUrl>& urls);

    /**
     * Reverts the uncommited changes in the given files.
     *
     * @param urls the list of files whose changes to revert
     *
     * @note: This is an irreversible and dangerous action,
     * a confirmation dialog is shown before it is applied
     */
    void revertSelectedFiles(const QList<QUrl>& urls);

    /**
     * Runs git commit on the staged changes. The commit message
     * is constructed from the data in the commit form.
     *
     * @note This function assumes that there are some staged changes.
     * @note The extended description of the commit is wrapped at 70 columns
     */
    void commitActiveProject();

private:

    /* Describes an action on selected lines/hunk in a diff */
    enum ApplyAction {
        Stage,
        Unstage,
        Revert,
    };

    /**
     * Updates the toolview layout based on the dock area position:
     *
     * When the toolview is placed on the left/right, all the widgets
     * sit on top of each other; when it is placed on the top/bottom,
     * the commit area (commit header, button, description textedit)
     * will sit to the left of the changes view with the search filter.
     */

    void doLayOut(const Qt::DockWidgetArea area);

    /**
     * A helper function which return the VCS plugin which
     * handles `url`.
     *
     * @param url the url for which the plugin is returned
     *
     * @note: Returns nullptr if no project/VCS plugin for the
     * given url exists.
     */
    KDevelop::IBasicVersionControl* vcsPluginForUrl(const QUrl& url) const;

    /**
     * The model which lists the projects and staged/modified/... files
     * which are shown in the treeview.
     */
    RepoStatusModel* m_statusmodel;

    /**
     * The filtered repostatus model
     */
    FilterEmptyItemsProxyModel* m_proxymodel;

    /** The form for composing the commit message and doing the commit. */
    SimpleCommitForm* m_commitForm = nullptr;

    /** The treeview listing the projects & their staged/modified/... files */
    QTreeView* m_view = nullptr;

    /** The lineedit for filtering the treeview */
    QLineEdit* m_filter = nullptr;

    /****************************************
     * Various contextmenus & their actions *
     ****************************************/

    QMenu
        /** Menu with a single "Refresh" action (shown for projects in the toolview) */
          *m_refreshMenu
        /** Menu with stage/unstage/revert actions (shown for files in the toolview) */
        , *m_toolviewMenu
        ;
    QAction *m_refreshModelAct
          , *m_stageFilesAct
          , *m_unstageFilesAct
          , *m_revertFilesAct
          ;

    /** A style delegate for showing the currently selected project in bold */
    ActiveStyledDelegate* m_styleDelegate;
};

/**
 * A factory for creating CommitToolViews.
 */
class CommitToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    explicit CommitToolViewFactory(RepoStatusModel* statusModel);
    ~CommitToolViewFactory();
    QWidget* create(QWidget* parent = nullptr) override;
    Qt::DockWidgetArea defaultPosition() const override;
    QString id() const override;

private:
    RepoStatusModel* m_statusmodel;
    DiffViewsCtrl* m_diffViewsCtrl;
};

#endif
