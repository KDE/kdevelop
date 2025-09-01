/*
    SPDX-FileCopyrightText: 2020 Jonathan L. Verner <jonathan.verner@matfyz.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "committoolview.h"

#include "diffviewsctrl.h"
#include "gitplugin.h"
#include "repostatusmodel.h"
#include "simplecommitform.h"

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <vcs/vcsjob.h>

#include <KLocalizedString>

#include <QAbstractItemView>
#include <QAction>
#include <QBoxLayout>
#include <QDockWidget>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QUrl>

using namespace KDevelop;

CommitToolViewFactory::CommitToolViewFactory()
    : m_diffViewsCtrl(new DiffViewsCtrl)
{
}

QWidget* CommitToolViewFactory::create(QWidget* parent)
{
    auto statusModel = m_statusModel.lock();
    if (!statusModel) {
        statusModel = std::make_shared<RepoStatusModel>();
        m_statusModel = statusModel;
    }

    auto* const tool = new CommitToolView(std::move(statusModel), parent);
    tool->connect(tool, &CommitToolView::updateDiff, m_diffViewsCtrl, [=](const QUrl& url, const RepoStatusModel::Areas area){
        m_diffViewsCtrl->updateDiff(url, area, DiffViewsCtrl::NoActivate);
    });
    tool->connect(tool, &CommitToolView::updateUrlDiffs, m_diffViewsCtrl, &DiffViewsCtrl::updateUrlDiffs);
    tool->connect(tool, &CommitToolView::updateProjectDiffs, m_diffViewsCtrl, &DiffViewsCtrl::updateProjectDiffs);
    tool->connect(tool, &CommitToolView::showDiff, m_diffViewsCtrl, [=](const QUrl& url, const RepoStatusModel::Areas area){
        m_diffViewsCtrl->updateDiff(url, area, DiffViewsCtrl::Activate);
    });
    tool->connect(tool, &CommitToolView::showSource, m_diffViewsCtrl, [=](const QUrl& url) {
        if (url.fileName().isEmpty()) return;
        auto* docCtrl = ICore::self()->documentController();
        if (auto* srcDoc = docCtrl->openDocument(url)) {
            docCtrl->activateDocument(srcDoc);
        }
    });
    return tool;
}

CommitToolViewFactory::~CommitToolViewFactory()
{
    delete m_diffViewsCtrl;
}


Qt::DockWidgetArea CommitToolViewFactory::defaultPosition() const
{
    return Qt::RightDockWidgetArea;
}

QString CommitToolViewFactory::id() const
{
    return QStringLiteral("org.kdevelop.CommitToolView");
}

/**
 * A filter to be used on RepoStatusModel to hide the areas
 * (index, worktree, conflicts, untracked) which are empty
 */
class FilterEmptyItemsProxyModel : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        const QModelIndex rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);

        if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return false;

        if (sourceModel()->hasChildren(rowIndex))
            return true;

        const auto area = rowIndex.data(RepoStatusModel::AreaRole);
        return area == RepoStatusModel::Index || area == RepoStatusModel::WorkTree || area == RepoStatusModel::Conflicts
            || area == RepoStatusModel::Untracked;
    }
};

/**
 * A style delegate to show active project in bold
 */
class ActiveStyledDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& idx) const override
    {
        QStyledItemDelegate::initStyleOption(option, idx);
        if (idx == m_activeProject)
            option->font.setBold(true);
    }
    /**
     * Sets the active project which will be styled in bold.
     */
    void setActive(const QModelIndex& idx) { m_activeProject = idx; }

private:
    QPersistentModelIndex m_activeProject;
};

void CommitToolView::doLayOut(const Qt::DockWidgetArea area)
{
    if (layout()) {
        delete layout();
    }

    QSplitter* _splitter;
    QBoxLayout* _layout;
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea || area == Qt::NoDockWidgetArea) {
        _layout = new QHBoxLayout(this);
        _splitter = new QSplitter(Qt::Vertical, this);
        _splitter->addWidget(m_commitForm);
        _splitter->addWidget(m_filter);
        _splitter->addWidget(m_view);
        _splitter->setStretchFactor(0, 1);
        _splitter->setStretchFactor(2, 5);
    } else {
        _layout = new QVBoxLayout(this);
        _splitter = new QSplitter(Qt::Horizontal, this);
        auto _filter_plus_view = new QSplitter(Qt::Vertical, this);
        _filter_plus_view->addWidget(m_filter);
        _filter_plus_view->addWidget(m_view);
        _splitter->addWidget(m_commitForm);
        _splitter->addWidget(_filter_plus_view);
    }
    _layout->addWidget(_splitter);
    m_filter->setMaximumHeight(35);
    setLayout(_layout);
}

CommitToolView::CommitToolView(std::shared_ptr<RepoStatusModel> statusModel, QWidget* parent)
    : QWidget(parent)
    , m_statusmodel(std::move(statusModel))
    , m_proxymodel(new FilterEmptyItemsProxyModel(this))
    , m_commitForm(new SimpleCommitForm(this))
    , m_view(new QTreeView(this))
    , m_filter(new QLineEdit(this))
    , m_refreshMenu(new QMenu(this))
    , m_toolviewMenu(new QMenu(this))
    , m_styleDelegate(new ActiveStyledDelegate)
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("git")));


    // FIXME: We should get the current area dock area from somewhere (the sublime area?)
    //        Right now we initially assume it is in the Qt::RightWidgetArea and layout
    //        accordingly (this may be wrong if the user previously moved it to, e.g. the
    //        bottom so its restored in the bottom area); when the user moves the dock
    //        widget we re-layout it and then the layout is always correct, since the signal
    doLayOut(Qt::RightDockWidgetArea);
    connect(dynamic_cast<QDockWidget*>(parent), &QDockWidget::dockLocationChanged, this, &CommitToolView::doLayOut);

    // Creates a proxy model so that we can filter the
    // items by the text entered into the filter lineedit
    m_proxymodel->setSourceModel(m_statusmodel.get());
    m_proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxymodel->setSortRole(Qt::DisplayRole);
    m_proxymodel->setRecursiveFilteringEnabled(true);
    connect(m_filter, &QLineEdit::textEdited, m_proxymodel, &QSortFilterProxyModel::setFilterWildcard);
    m_filter->setToolTip(i18n("Filter by filename/project name"));
    m_filter->setPlaceholderText(i18n("Filter by filename/project name"));

    // Sets up the view
    m_view->setModel(m_proxymodel);
    m_view->setHeaderHidden(true);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setSelectionMode(QAbstractItemView::SelectionMode::ContiguousSelection);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setAnimated(true);
    m_view->setItemDelegate(m_styleDelegate);

    connect(m_view, &QTreeView::customContextMenuRequested, this, &CommitToolView::popupContextMenu);
    connect(m_view, &QTreeView::doubleClicked, this, &CommitToolView::dblClicked);
    connect(m_view, &QTreeView::clicked, this, &CommitToolView::clicked);
    connect(m_view, &QTreeView::expanded, this, &CommitToolView::activateProject);

    // Construct the tool view context menus & actions
    m_refreshModelAct = m_refreshMenu->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("Refresh"));
    m_stageFilesAct = m_toolviewMenu->addAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Stage selected"));
    m_unstageFilesAct
        = m_toolviewMenu->addAction(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Unstage selected"));
    m_revertFilesAct
        = m_toolviewMenu->addAction(QIcon::fromTheme(QStringLiteral("edit-undo")), i18n("Revert unstaged edits"));

    m_toolviewMenu->addAction(m_refreshModelAct);

    // Refresh diffs when documents are saved
    connect(ICore::self()->documentController(), &IDocumentController::documentSaved, this,
            [=](KDevelop::IDocument* doc) {
                emit updateUrlDiffs(doc->url());
    });



    // Connect the commit form
    connect(m_commitForm, &SimpleCommitForm::committed, this, &CommitToolView::commitActiveProject);

    // Disable the commit button if the active project has no staged changes
    connect(m_statusmodel.get(), &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex& parent) {
        if (parent.data(RepoStatusModel::AreaRole) == RepoStatusModel::IndexRoot
            && m_statusmodel->itemFromIndex(parent)->rowCount() == 0 && isActiveProject(parent.parent()))
            m_commitForm->disableCommitButton();
    });
    connect(m_statusmodel.get(), &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent) {
        if (parent.data(RepoStatusModel::AreaRole) == RepoStatusModel::IndexRoot
            && m_statusmodel->itemFromIndex(parent)->rowCount() > 0 && isActiveProject(parent.parent()))
            m_commitForm->enableCommitButton();
    });
}

CommitToolView::~CommitToolView()
{
    delete m_styleDelegate;
}

KDevelop::IProject* CommitToolView::activeProject() const
{
    auto* proj_item = activeProjectItem();
    if (proj_item && isActiveProject(proj_item->index())) {
        return ICore::self()->projectController()->findProjectByName(
            proj_item->data(RepoStatusModel::NameRole).toString());
    }
    return nullptr;
}

QStandardItem* CommitToolView::activeProjectItem() const
{
    for (auto* pr : m_statusmodel->projectRoots()) {
        if (isActiveProject(pr->index()))
            return pr;
    }
    return nullptr;
}

bool CommitToolView::isActiveProject(const QModelIndex& idx) const
{
    return (m_view->isExpanded(m_proxymodel->mapFromSource(idx)));
}

void CommitToolView::activateProject(const QModelIndex& idx)
{
    if (idx.data(RepoStatusModel::AreaRole).toInt() == RepoStatusModel::ProjectRoot) {
        m_styleDelegate->setActive(idx);
        auto repoIdx = m_proxymodel->mapToSource(idx);
        for (const auto* pr : m_statusmodel->projectRoots()) {
            if (pr->index() != repoIdx)
                m_view->collapse(m_proxymodel->mapFromSource(pr->index()));
        }
        m_commitForm->setProjectName(idx.data(RepoStatusModel::NameRole).toString());
        m_commitForm->setBranchName(idx.data(RepoStatusModel::BranchNameRole).toString());
        m_commitForm->clearError();
        m_commitForm->enable();
        if (m_statusmodel->projectItem(m_statusmodel->itemFromIndex(repoIdx)).index->rowCount() == 0)
            m_commitForm->disableCommitButton();
        else
            m_commitForm->enableCommitButton();
    }
}

void CommitToolView::popupContextMenu(const QPoint& pos)
{
    QList<QUrl> urls;
    const QModelIndexList selectionIdxs = m_view->selectionModel()->selectedIndexes();

    // If there are no selected files just show an action to refresh the model
    if (selectionIdxs.isEmpty()) {
        QModelIndex idx = m_view->indexAt(pos);
        IProject* project
            = ICore::self()->projectController()->findProjectByName(idx.data(RepoStatusModel::NameRole).toString());

        // Show the context menu & evaluate the results
        QAction* res = m_refreshMenu->exec(m_view->viewport()->mapToGlobal(pos));
        if (res == m_refreshModelAct) {
            if (project)
                m_statusmodel->reload({ project });
            else
                m_statusmodel->reloadAll();
        }
        return;
    }

    // Convert the selection into a list of urls;
    for (const QModelIndex& idx : selectionIdxs) {
        if (idx.column() == 0) {
            if (idx.parent().isValid())
                urls += idx.data(RepoStatusModel::UrlRole).value<QUrl>();
        }
    }

    // Show the context menu & evaluate the results
    QAction* res = m_toolviewMenu->exec(m_view->viewport()->mapToGlobal(pos));
    if (res == m_refreshModelAct) {
        if (!urls.isEmpty())
            m_statusmodel->reload(urls);
        else
            m_statusmodel->reloadAll();
    } else if (res == m_stageFilesAct) {
        if (!urls.isEmpty())
            stageSelectedFiles(urls);
    } else if (res == m_unstageFilesAct) {
        if (!urls.isEmpty())
            unstageSelectedFiles(urls);
    } else if (res == m_revertFilesAct) {
        if (!urls.isEmpty())
            revertSelectedFiles(urls);
    }
}

void CommitToolView::dblClicked ( const QModelIndex& idx )
{
    // A different action is performed based on where the
    // file that was double-clicked on is.
    switch (idx.data(RepoStatusModel::AreaRole).toInt()) {

    // Files in the staging area are unstaged
    case RepoStatusModel::Index:
        unstageSelectedFiles({ idx.data(RepoStatusModel::UrlRole).toUrl() });
        break;
    // Files in the other areas are staged for commit
    // (including marking conflicts as resolved and adding the
    //  untracked files into the repo)
    case RepoStatusModel::WorkTree:
    case RepoStatusModel::Conflicts:
    case RepoStatusModel::Untracked:
        idx.data(RepoStatusModel::UrlRole).toUrl();
        stageSelectedFiles({ idx.data(RepoStatusModel::UrlRole).toUrl() });
        break;
    default:
        break;
    }
}

void CommitToolView::clicked ( const QModelIndex& idx )
{
    auto url = idx.data(RepoStatusModel::UrlRole).toUrl();
    auto projectUrl = idx.data(RepoStatusModel::ProjectUrlRole).toUrl();

    switch (idx.data(RepoStatusModel::AreaRole).toInt()) {
    case RepoStatusModel::IndexRoot:
        emit showDiff(projectUrl, RepoStatusModel::IndexRoot);
        break;
    case RepoStatusModel::Index:
        emit showDiff(url, RepoStatusModel::Index);
        break;
    case RepoStatusModel::WorkTreeRoot:
        emit showDiff(projectUrl, RepoStatusModel::WorkTreeRoot);
        break;
    case RepoStatusModel::WorkTree:
        emit showDiff(url, RepoStatusModel::WorkTree);
        break;
    case RepoStatusModel::Untracked:
        emit showSource(url);
        break;
    }
}

IBasicVersionControl* CommitToolView::vcsPluginForUrl ( const QUrl& url ) const
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    IPlugin* vcsplugin = project ? project->versionControlPlugin() : nullptr;
    return vcsplugin ? vcsplugin->extension<IBasicVersionControl>() : nullptr;
}

// Note that the this is a dangerous operation;
// we rely on the vcs job to show a confirmation dialog
void CommitToolView::revertSelectedFiles ( const QList<QUrl>& urls )
{

    IProject* project = ICore::self()->projectController()->findProjectForUrl(urls.front());
    IBasicVersionControl* vcs = vcsPluginForUrl(urls.front());

    if (vcs) {
        VcsJob* job = vcs->revert(urls, IBasicVersionControl::NonRecursive);
        job->setProperty("urls", QVariant::fromValue<QList<QUrl>>(urls));
        job->setProperty("project", QVariant::fromValue(project));
        ICore::self()->runController()->registerJob(job);
        connect(job, &VcsJob::resultsReady, this, [=]() {
            // Close the document tabs showing diffs for the urls
            for (const auto& url : urls) {
                emit updateUrlDiffs(url);
            }
        });
    }
}

void CommitToolView::stageSelectedFiles ( const QList<QUrl>& urls )
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(urls.front());
    IBasicVersionControl* vcs = vcsPluginForUrl(urls.front());
    if (vcs) {
        VcsJob* job = vcs->add(urls, IBasicVersionControl::NonRecursive);
        job->setProperty("urls", QVariant::fromValue<QList<QUrl>>(urls));
        job->setProperty("project", QVariant::fromValue(project));
        connect(job, &VcsJob::resultsReady, this, [=]() {
            // Close the document tabs showing diffs for the urls
            for (const auto& url : urls) {
                emit updateUrlDiffs(url);
            }
        });
        ICore::self()->runController()->registerJob(job);
    }
}

void CommitToolView::unstageSelectedFiles(const QList<QUrl>& urls)
{
    if (GitPlugin* git = dynamic_cast<GitPlugin*>(vcsPluginForUrl(urls.front()))) {
        IProject* project = ICore::self()->projectController()->findProjectForUrl(urls.front());
        VcsJob* job = git->reset(urls, IBasicVersionControl::NonRecursive);
        job->setProperty("urls", QVariant::fromValue<QList<QUrl>>(urls));
        job->setProperty("project", QVariant::fromValue(project));
        connect(job, &VcsJob::resultsReady, this, [=]() {
            for (const auto& url : urls) {
                emit updateUrlDiffs(url);
            }
        });
        ICore::self()->runController()->registerJob(job);
    }
}

void CommitToolView::commitActiveProject()
{
    if (auto* proj = activeProject()) {
        if (auto* vcs = proj->versionControlPlugin()->extension<GitPlugin>()) {
            QString msg = m_commitForm->summary();
            QString extended = m_commitForm->extendedDescription(70);
            if (extended.length() > 0)
                msg += QStringLiteral("\n\n") + extended;
            VcsJob* job = vcs->commitStaged(msg, proj->projectItem()->path().toUrl());
            m_commitForm->clearError();
            m_commitForm->disable();
            connect(job, &VcsJob::finished, m_commitForm, [=]{
                if (job->status() == VcsJob::JobSucceeded){
                    m_commitForm->clear();
                    emit updateProjectDiffs(proj);
                } else {
                    m_commitForm->showError(i18n("Committing failed. See Version Control tool view."));
                }
                m_commitForm->enable();
            });
            ICore::self()->runController()->registerJob(job);
        }
    }
}

#include "committoolview.moc"
#include "moc_committoolview.cpp"
