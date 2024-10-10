/*
    SPDX-FileCopyrightText: 2020 Jonathan Verner <jonathan@temno.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "diffviewsctrl.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <util/path.h>
#include <vcs/vcsdiff.h>
#include <vcs/vcsjob.h>

#include <KActionCollection>
#include <KColorScheme>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QAction>
#include <QIcon>
#include <QMenu>

using namespace KDevelop;

/**
 * A helper function which returns the gitplugin responsible
 * for a given url
 */
GitPlugin* gitForUrl(const QUrl& url)
{
    auto* project = ICore::self()->projectController()->findProjectForUrl(url);
    auto* vcsplugin = (project ? project->versionControlPlugin() : nullptr);
    return (vcsplugin ? vcsplugin->extension<GitPlugin>() : nullptr);
}

bool DiffViewsCtrl::ViewData::isValid() const
{
    return (project != nullptr && vcs != nullptr && doc != nullptr && ktDoc != nullptr );
}

DiffViewsCtrl::DiffViewsCtrl(QObject* parent)
    : QObject(parent)
    , m_stageSelectedAct(
          new QAction(QIcon::fromTheme(QStringLiteral("view-add")), i18n("Stage selected lines or hunk"), this))
    , m_unstageSelectedAct(
          new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("Unstage selected lines or hunk"), this))
    , m_revertSelectedAct(
          new QAction(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Revert selected lines or hunk"), this))
    , m_gotoSrcLineAct(
          new QAction(QIcon::fromTheme(QStringLiteral("go-parent-folder")), i18n("Go to line in source"), this))

{
    // Setup style attributes for highlighting diffs
    // (green background for +, red background+black fg for -, gray color for @@)
    auto colors = KColorScheme();

    // Prevent triggering the actions via shortcuts when a tool view, such as Terminal tool view, has focus.
    // KTextEditor::ViewPrivate::setupActions() sets Qt::WidgetWithChildrenShortcut context
    // for all actions it adds to the view's actionCollection(). However, only the narrower
    // Qt::WidgetShortcut context prevents our actions from stealing key presses from a focused tool view.
    for (auto* action : {m_stageSelectedAct, m_unstageSelectedAct, m_revertSelectedAct, m_gotoSrcLineAct}) {
        action->setShortcutContext(Qt::WidgetShortcut);
    }

    // Connect the diff windows actions
    connect(m_stageSelectedAct, &QAction::triggered, this, [=] { applySelected(Stage); });
    connect(m_unstageSelectedAct, &QAction::triggered, this, [=] { applySelected(Unstage); });
    connect(m_revertSelectedAct, &QAction::triggered, this, &DiffViewsCtrl::revertSelected);
    connect(m_gotoSrcLineAct, &QAction::triggered, this, &DiffViewsCtrl::gotoSrcLine);
}

DiffViewsCtrl::~DiffViewsCtrl()
{
    // Close the diff views so that kdevelop does
    // not show a lot of Untitled empty tabs on
    // when starting again
    for (const auto& d : std::as_const(m_views)) {
        if (d.second.doc)
            d.second.doc->close();
    }
    m_views.clear();
}

void DiffViewsCtrl::setupDiffActions(KTextEditor::View* view, const RepoStatusModel::Areas diffType) const
{
    // Context Menu Setup
    QMenu* ret = new QMenu;
    if (diffType == RepoStatusModel::Index || diffType == RepoStatusModel::IndexRoot) {
        ret->addAction(m_unstageSelectedAct);
    } else if (diffType == RepoStatusModel::WorkTree || diffType == RepoStatusModel::WorkTreeRoot) {
        ret->addAction(m_stageSelectedAct);
        ret->addAction(m_revertSelectedAct);
    }
    ret->addAction(m_gotoSrcLineAct);
    view->setContextMenu(ret);

    // Set the text of the actions based on whether some lines
    // are selected or not
    connect(view, &KTextEditor::View::contextMenuAboutToShow, this, [=] {
        auto haveSelection = !view->selectionRange().isEmpty();
        if (haveSelection) {
            m_unstageSelectedAct->setText(i18n("Unstage selected lines"));
            m_stageSelectedAct->setText(i18n("Stage selected lines"));
            m_revertSelectedAct->setText(i18n("Revert selected lines"));
        } else {
            m_unstageSelectedAct->setText(i18n("Unstage selected hunk"));
            m_stageSelectedAct->setText(i18n("Stage selected hunk"));
            m_revertSelectedAct->setText(i18n("Revert selected hunk"));
        }
    });

    // Add the actions to the view action collection, so that
    // shortcuts work and can be edited
    auto actCollection = view->actionCollection();
    if (diffType == RepoStatusModel::Index || diffType == RepoStatusModel::IndexRoot) {
        actCollection->addAction(QStringLiteral("git_unstage_selected"), m_unstageSelectedAct);
        actCollection->addAction(QStringLiteral("git_goto_source"), m_gotoSrcLineAct);
        actCollection->setDefaultShortcut(m_unstageSelectedAct, i18n("S"));
        actCollection->setDefaultShortcut(m_gotoSrcLineAct, i18n("G"));
    } else if (diffType == RepoStatusModel::WorkTree || diffType == RepoStatusModel::WorkTreeRoot) {
        actCollection->addAction(QStringLiteral("git_stage_selected"), m_stageSelectedAct);
        actCollection->addAction(QStringLiteral("git_revert_selected"), m_revertSelectedAct);
        actCollection->addAction(QStringLiteral("git_goto_source"), m_gotoSrcLineAct);
        actCollection->setDefaultShortcut(m_stageSelectedAct, i18n("S"));
        actCollection->setDefaultShortcut(m_gotoSrcLineAct, i18n("G"));
    }
}

const QString DiffViewsCtrl::viewKey(const QUrl& url, RepoStatusModel::Areas area)
{
    if (area == RepoStatusModel::WorkTreeRoot || area == RepoStatusModel::IndexRoot) {
        if ( auto* project = ICore::self()->projectController()->findProjectForUrl(url) ) {
            return project->path().toUrl().toString() + QStringLiteral(":") + QString::number(area);
        }
        return QStringLiteral(":") + QString::number(area);
    }
    return url.toString() + QStringLiteral(":") + QString::number(area);
}

const DiffViewsCtrl::ViewData DiffViewsCtrl::createView(const QUrl& url, RepoStatusModel::Areas area)
{
    auto* docCtrl = ICore::self()->documentController();

    // If an appropriate view is already cached
    // return it
    QString key = viewKey(url, area);
    auto viewDataIt = m_views.find(key);
    if (viewDataIt != m_views.end()) {
        return viewDataIt->second;
    }

    // Create a new view and populate the
    // ViewData structure
    ViewData data;
    data.project = ICore::self()->projectController()->findProjectForUrl(url);

    if (data.project == nullptr)
        return data;

    data.area = area;
    data.doc = docCtrl->openDocumentFromText(QString());
    data.ktDoc = data.doc->textDocument();
    data.url = url;
    data.vcs = gitForUrl(url);

    // Cache the new view data
    m_views[key] = data;

    // Set the view title
    if (area == RepoStatusModel::Index)
        data.doc->setPrettyName(i18n("%1 (staged)", url.fileName()));
    else if (area == RepoStatusModel::IndexRoot)
        data.doc->setPrettyName(i18n("Staged (%1)", url.fileName()));
    else if (area == RepoStatusModel::WorkTreeRoot)
        data.doc->setPrettyName(i18n("Unstaged (%1)", url.fileName()));
    else if (area == RepoStatusModel::WorkTree)
        data.doc->setPrettyName(i18n("%1 (unstaged)", url.fileName()));

    // Connect cleanup handlers on document/project closure and kdevelop shutdown
    connect(ICore::self()->projectController(), &IProjectController::projectClosed, this, [=] (KDevelop::IProject* proj) {
        if (proj == data.project) {
            auto dataIt = m_views.find(key);
            if (dataIt != m_views.end())
                dataIt->second.doc->close();
        }
    });
    connect(ICore::self(), &ICore::aboutToShutdown, this, [=] {
        auto dataIt = m_views.find(key);
        if (dataIt != m_views.end())
            dataIt->second.doc->close();
    });
    connect(data.ktDoc, &KTextEditor::Document::aboutToClose, this, [=]() { m_views.erase(key); });

    // Set the context menu for the document & add the appropriate actions to it
    const auto& views = data.ktDoc->views();
    for (auto view : views)
        setupDiffActions(view, area);

    return data;
}

const DiffViewsCtrl::ViewData DiffViewsCtrl::activeView()
{
    auto view = ICore::self()->documentController()->activeTextDocumentView();
    auto doc = view->document();
    if (view) {
        for (auto data : m_views) {
            if (data.second.ktDoc == doc) {
                data.second.actView = view;
                return data.second;
            }
        }
    }
    ViewData ret;
    return ret;
}

void DiffViewsCtrl::updateDiff(const QUrl& url, const RepoStatusModel::Areas area, const UpdateDiffParams p)
{
    // If p == NoActivate and the url+area has no associated view
    // return early
    auto key = viewKey(url, area);
    if (p == NoActivate && m_views.find(key) == m_views.end())
        return;

    if (auto* vcs = gitForUrl(url)) {
        VcsRevision src, dst;
        if (area == RepoStatusModel::Index || area == RepoStatusModel::IndexRoot) {
            dst = VcsRevision::createSpecialRevision(VcsRevision::Working);
            src = VcsRevision::createSpecialRevision(VcsRevision::Head);
            src.setRevisionValue(QStringLiteral("HEAD"), VcsRevision::Special);
        } else if (area == RepoStatusModel::WorkTree || area == RepoStatusModel::WorkTreeRoot) {
            dst = VcsRevision::createSpecialRevision(VcsRevision::Base);
            src = VcsRevision::createSpecialRevision(VcsRevision::Working);
        } else
            return;
        VcsJob* job = nullptr;
        if (area == RepoStatusModel::Index || area == RepoStatusModel::WorkTree)
            job = vcs->diff(url, src, dst, IBasicVersionControl::NonRecursive);
        else if (area == RepoStatusModel::IndexRoot || area == RepoStatusModel::WorkTreeRoot)
            job = vcs->diff(url, src, dst);
        if (job) {
            job->setProperty("key", QVariant::fromValue<QString>(key));
            job->setProperty("url", QVariant::fromValue<QUrl>(url));
            job->setProperty("area", area);
            job->setProperty("activate", p);
            connect(job, &VcsJob::resultsReady, this, &DiffViewsCtrl::diffReady);
            ICore::self()->runController()->registerJob(job);
        }
    }
}

void DiffViewsCtrl::updateProjectDiffs(KDevelop::IProject* proj)
{
    for(auto [_, vData] : m_views) {
        Q_UNUSED(_);
        if (vData.project == proj)
            updateDiff(vData.url, vData.area, UpdateDiffParams::NoActivate);
    }
}

void DiffViewsCtrl::updateUrlDiffs(const QUrl& url)
{
    if (auto* project = ICore::self()->projectController()->findProjectForUrl(url)) {
        for(auto [_, vData] : m_views) {
            Q_UNUSED(_);
            if (vData.project != project)
                continue;
            if (
                vData.url == url ||
                vData.area == RepoStatusModel::WorkTreeRoot ||
                vData.area == RepoStatusModel::IndexRoot ||
                vData.area == RepoStatusModel::ConflictRoot ||
                vData.area == RepoStatusModel::UntrackedRoot
            )
                updateDiff(vData.url, vData.area, UpdateDiffParams::NoActivate);
        }
    }
}



void DiffViewsCtrl::diffReady(KDevelop::VcsJob* diffJob)
{
    if (diffJob->status() == VcsJob::JobSucceeded) {
        // Fetch the job results
        auto diff = diffJob->fetchResults().value<VcsDiff>();
        auto key = diffJob->property("key").toString();
        auto p = (UpdateDiffParams)diffJob->property("activate").toInt();

        ViewData vData;
        auto vDataIt = m_views.find(key);

        // If the diff is empty, close the view if present
        // and return
        if (diff.isEmpty()) {
            if (vDataIt != m_views.end() && vDataIt->second.doc)
                vDataIt->second.doc->close();
            return;
        }

        if (vDataIt != m_views.end()) {
            vData = vDataIt->second;
        } else {
            vData = createView(diffJob->property("url").toUrl(),
                               (RepoStatusModel::Areas)diffJob->property("area").toInt());
            if (! vData.isValid())
                return;
        }

        auto position = vData.ktDoc->views().constFirst()->cursorPosition(); // assume there is only one view
        vData.ktDoc->setReadWrite(true);
        vData.ktDoc->setText(diff.diff());
        vData.ktDoc->setReadWrite(false);
        vData.ktDoc->setModified(false);
        vData.ktDoc->views().constFirst()->setCursorPosition(position); // assume there is only one view
        vData.ktDoc->setMode(QStringLiteral("diff"));
        vData.ktDoc->setHighlightingMode(QStringLiteral("diff"));

        // Activate the diff document, if required
        if (p == Activate) {
            auto* docCtrl = ICore::self()->documentController();
            docCtrl->activateDocument(vData.doc);
        }
    }
}

void DiffViewsCtrl::revertSelected()
{
    auto res = KMessageBox::questionTwoActions(
        nullptr,
        i18n("The selected lines will be reverted and the changes lost. This "
             "operation cannot be undone. Continue?"),
        {}, KGuiItem(i18nc("@action:button", "Revert"), QStringLiteral("list-remove")), KStandardGuiItem::cancel());
    if (res != KMessageBox::PrimaryAction)
        return;

    applySelected(Revert);
}

void DiffViewsCtrl::applySelected(DiffViewsCtrl::ApplyAction act)
{
    auto vData = activeView();
    if (! vData.isValid() )
        return;

    if (vData.area != RepoStatusModel::None) {
        // Setup arguments to subDiff & apply based on the required action
        auto [direction, params] = [act]() -> std::pair<VcsDiff::DiffDirection, GitPlugin::ApplyParams> {
            switch (act) {
            case Stage:
                return { VcsDiff::Normal, GitPlugin::Index };
            case Unstage:
                return { VcsDiff::Reverse, GitPlugin::Index };
            case Revert:
                return { VcsDiff::Reverse, GitPlugin::WorkTree };
            }
            Q_UNREACHABLE();
        }();

        // Construct the selected diff (either from the selected lines
        // or the hunk containing the current cursor position)
        VcsDiff fullDiff, selectedDiff;
        fullDiff.setDiff(vData.ktDoc->text());
        fullDiff.setBaseDiff(vData.project->path().toUrl());
        auto range = vData.actView->selectionRange();
        if (range.isEmpty()) {
            selectedDiff = fullDiff.subDiffHunk(vData.actView->cursorPosition().line(), direction);
        } else {
            int startLine = range.start().line();
            int endLine = range.end().line();
            selectedDiff = fullDiff.subDiff(startLine, endLine, direction);
        }

        // Run the apply job
        VcsJob* indexJob = vData.vcs->apply(selectedDiff, params);
        connect(indexJob, &VcsJob::resultsReady, this, [=] {
            if (indexJob->status() == VcsJob::JobSucceeded) {
                updateUrlDiffs(vData.url);
            }
        });
        ICore::self()->runController()->registerJob(indexJob);
    }
}

void DiffViewsCtrl::gotoSrcLine()
{
    auto vData = activeView();
    if (!vData.isValid() || !vData.actView)
        return;

    auto* docCtrl = ICore::self()->documentController();
    auto diffLn = vData.actView->cursorPosition().line();
    auto diffCol = vData.actView->cursorPosition().column();
    VcsDiff diff;
    diff.setDiff(vData.ktDoc->text());

    // Find the closest line in the diff which has a corresponding
    // source line
    auto last_line = vData.ktDoc->documentEnd().line();
    int delta = 0;
    while(diffLn - delta >= 1 || diffLn + delta < last_line) {
        auto src = diff.diffLineToTarget(diffLn-delta);
        if ( src.line < 0 ) src = diff.diffLineToTarget(diffLn+delta);
        if ( src.line >= 0 ) {
            auto path = KDevelop::Path(vData.project->path(), src.path);
            if (auto* srcDoc = docCtrl->openDocument(path.toUrl())) {
                srcDoc->setCursorPosition(KTextEditor::Cursor(src.line, diffCol-1));
                docCtrl->activateDocument(srcDoc);
            }
            return;
        }
        delta += 1;
    }
}

#include "moc_diffviewsctrl.cpp"
