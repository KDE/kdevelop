/*
    SPDX-FileCopyrightText: 2020 Jonathan Verner <jonathan@temno.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DIFF_VIEWS_CTRL_H
#define DIFF_VIEWS_CTRL_H

#include "repostatusmodel.h"

#include <KTextEditor/Attribute>

#include <QObject>

class GitPlugin;
class QAction;

namespace KDevelop
{
class IDocument;
class IProject;
class VcsJob;
class VcsDiff;
}

namespace KTextEditor
{
class Document;
class View;
}

/**
 * A class which handles displaying & updating tabs showing
 * staged/unstaged changes.
 */
class DiffViewsCtrl : public QObject
{
    Q_OBJECT
public:
    DiffViewsCtrl(QObject* parent = nullptr);
    ~DiffViewsCtrl();

    /* Enum values to be passed to the updateDiff method */
    enum UpdateDiffParams { Activate, NoActivate };

public Q_SLOTS:

    /**
     * Updates the diff view showing the changes to url and,
     * optionally, activates the view.
     *
     * @param url the url of the file to update the diff for
     * @param area the area (determines what type of changes are shown,
     * e.g. if area == Index, the diff shows the staged changes)
     * @param p if equal to Activate (default), activates the tab,
     * if equal to NoActivate, the tab is not activated (e.g. when it is
     * refreshed because of changes in an active document).
     *
     * @note: The tab is not opened immediately, rather a job to compute the
     * diff is scheduled and the tab is opened by the @method diffReady
     * when the job producing the diff finishes.
     *
     * @note: If p is equal to NoActivate and there is no tab already
     * showing the diff, the function returns without doing anything.
     */
    void updateDiff(const QUrl& url, const RepoStatusModel::Areas area, const UpdateDiffParams p = Activate);

    /**
     * Updates all diff views which are shown for a project.
     * The diff views which become empty are closed.
     *
     * @param proj the project to update diff views for
     */
    void updateProjectDiffs(KDevelop::IProject* proj);

    /**
     * Updates all diff views which are shown for a given url.
     * The diff views which become empty are closed.
     *
     * @param url the url to update diff views for
     */
    void updateUrlDiffs(const QUrl& url);


private Q_SLOTS:
    /**
     * A handler called to open a document tab with a diff when
     * the job producing the diff finishes.
     *
     * @param diffJob the job producing the diff
     */
    void diffReady(KDevelop::VcsJob* diffJob);

private:
    /* Describes an action on selected lines/hunk in a diff */
    enum ApplyAction {
        Stage,
        Unstage,
        Revert,
    };

    /**
     * A helper function which applies an action to the currently
     * selected lines/hunk in the active diff view.
     *
     * @param act the action to apply (stage, unstage, revert)
     *
     * @note: If the view has a non-empty selection, the action will
     *        be applied to the selected lines; if the selection is
     *        empty, it will be applied to the hunk which contains
     *        the current cursor position.
     * @note: No confirmation dialog is shown before doing the
     *        potentially dangerous revert action.
     */
    void applySelected(ApplyAction act);

    /**
     * Reverts the selected lines/diff in the currently active
     * document tab showing a diff.
     *
     * @note: This is a helper method for showing a confirmation
     * dialog before reverting, since reverting is an irreversible
     * and dangerous action. The actual work is done using the
     * @ref:applySelected method.
     */
    void revertSelected();

    /**
     * Opens the source document at the line corresponding
     * to the current line in the currently shown diff
     */
    void gotoSrcLine();

    /**
     * A helper function which sets up the actions appropriate for a diff view.
     *
     * The function also creates a context menu for the view,
     * adds the appropriate actions to it and sets up a
     * connection to update the action's texts based on whether
     * a hunk or lines are selected in the active
     * view when the menu is shown.
     *
     * @param view the view to add the actions to
     * @param diffType the type of diff view (staged/unstaged). Allowed
     *                 values are @ref RepoStatusModel::Index and
     *                 @ref RepoStatusModel::WorkTree
     */
    void setupDiffActions(KTextEditor::View* view, const RepoStatusModel::Areas diffType) const;

    QAction *m_stageSelectedAct,    /**< Action to stage selected hunk/lines */
            *m_unstageSelectedAct,  /**< Action to unstage selected hunk/lines */
            *m_revertSelectedAct,   /**< Action to revert selected hunk/lines */
            *m_gotoSrcLineAct;      /**< Action to goto a line in the source file */

    /**
     * A structure for holding information about a tab
     * showing a diff.
     *
     * @note: A valid instance **must** have non null doc, ktDoc, vcs & project
     * project members.
     */
    class ViewData
    {
    public:
        RepoStatusModel::Areas area
            = RepoStatusModel::None; /**< The type of diff shown (staged/unstaged changes to a file; summary of
                                        staged/unstaged changes for a projects) */
        KDevelop::IDocument* doc = nullptr; /**< The associated IDocument */
        KTextEditor::Document* ktDoc = nullptr; /**< The associated KTextEditor::Document */
        KTextEditor::View* actView
            = nullptr; /**< The associated KTextEditor::View, if the tab is currently active; nullptr otherwise */
        GitPlugin* vcs = nullptr; /**< A reference to the git plugin */
        KDevelop::IProject* project = nullptr; /**< A pointer to the project plugin */
        QUrl url; /**< The url of the source file (for which changes are shown), or of the project (if showing a summary
                     of changes) */

        /**
         * Returns true if the instance is valid.
         */
        bool isValid() const;
    };

    /**
     * A helper function to get a ViewData structure for an url.
     *
     * It first tries to find a suitable structure in m_views and, if not
     * found, it creates a new one, caches it in m_views and sets up a connection
     * to remove it from the cache when the document is closed (e.g. by the user)
     * and to update the shown diff when the document is saved.
     *
     * @param url the file with the changes
     * @param area the type of the diff (staged/unstaged)
     */
    const ViewData createView(const QUrl& url, RepoStatusModel::Areas area);

    /**
     * Returns a ViewData structure for the currently active view.
     *
     * @note: If there is no active view or it has no associated ViewData structure,
     * the returned structure has the actView member set to nullptr.
     */
    const ViewData activeView();

    /**
     * A helper function to construct the key into the `m_views` map.
     *
     * @param url the url of the file being diffed
     * @param area the type of diff (i.e. staged (Index) / unstaged (WorkTree) changes)
     */
    static const QString viewKey(const QUrl& url, RepoStatusModel::Areas area);

    /**
     * A map holding ViewData structures for opened diff windows.
     * A diff window is opened only once per file / diff type (staged, unstaged) and
     * is later reused.
     *
     * The keys are formed by concatenating the url of the document
     * being diffed with ':' followed by the RepoStatusModel::Areas
     * enum value identifing what type of diff it is (i.e. showing
     * staged/unstaged changes).
     */
    std::map<QString, ViewData> m_views;
};

#endif // DIFF_VIEWS_CTRL_H
