/*
    SPDX-FileCopyrightText: 2020 Jonathan L. Verner <jonathan.verner@matfyz.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "repostatusmodel.h"

#include "debug.h"
#include "gitplugin.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/interfaces/ibranchingversioncontrol.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsstatusinfo.h>

#include <KLocalizedString>

#include <QDir>
#include <QIcon>
#include <QMimeDatabase>

#include <array>

using namespace KDevelop;

Q_DECLARE_METATYPE(IProject*)

RepoStatusModel::RepoStatusModel(QObject* parent)
    : QStandardItemModel(parent)
{
    const auto projects = ICore::self()->projectController()->projects();
    for (IProject* p : projects) {
        addProject(p);
    }

    connect(ICore::self()->projectController(), &IProjectController::projectOpened, this, &RepoStatusModel::addProject);
    connect(ICore::self()->projectController(), &IProjectController::projectClosing, this,
            &RepoStatusModel::removeProject);
    connect(ICore::self()->projectController()->projectModel(), &ProjectModel::rowsInserted, this,
            &RepoStatusModel::itemsAdded);
    connect(ICore::self()->documentController(), &IDocumentController::documentSaved, this,
            &RepoStatusModel::documentSaved);
    connect(ICore::self()->runController(), &IRunController::jobUnregistered, this, &RepoStatusModel::jobUnregistered);
}

RepoStatusModel::~RepoStatusModel()
{
    const auto projects = ICore::self()->projectController()->projects();
    for (auto* const project : projects) {
        removeProject(project);
    }
}

void RepoStatusModel::addProject(const IProject* p)
{
    auto* const versionControlPlugin = p->versionControlPlugin();
    if (!versionControlPlugin) {
        return;
    }
    auto* const plugin = versionControlPlugin->extension<GitPlugin>();
    if (!plugin) {
        return;
    }

    const auto projectIt = new QStandardItem(p->name());
    const auto indexIt =
        new QStandardItem(QIcon::fromTheme(QStringLiteral("flag-green")),
                          i18nc("Files in a vcs which have changes staged for commit", "Staged changes"));
    const auto worktreeIt =
        new QStandardItem(QIcon::fromTheme(QStringLiteral("flag-yellow")),
                          i18nc("Files in a vcs which have changes not yet staged for commit", "Unstaged changes"));
    const auto conflictIt =
        new QStandardItem(QIcon::fromTheme(QStringLiteral("flag-red")),
                          i18nc("Files in a vcs which have unresolved (merge) conflits", "Conflicts"));
    const auto untrackedIt = new QStandardItem(QIcon::fromTheme(QStringLiteral("flag-black")),
                                               i18nc("Files which are not tracked by a vcs", "Untracked files"));
    const auto pluginInfo = ICore::self()->pluginController()->pluginInfo(plugin);
    const auto pathUrl = p->path().toUrl();

    projectIt->setData(p->name(), RepoStatusModel::NameRole);
    projectIt->setData(pathUrl, RepoStatusModel::ProjectUrlRole);
    projectIt->setData(ProjectRoot, AreaRole);
    projectIt->setSelectable(false);
    projectIt->setIcon(QIcon::fromTheme(pluginInfo.iconName()));

    indexIt->setData(i18nc("Files in a vcs which have changes staged for commit", "Staged"), RepoStatusModel::NameRole);
    indexIt->setToolTip(i18n("Files with changes staged for commit"));
    indexIt->setData(IndexRoot, AreaRole);
    indexIt->setData(pathUrl, RepoStatusModel::ProjectUrlRole);
    indexIt->setSelectable(false);

    worktreeIt->setData(i18nc("Files in a vcs which have changes not checked in to repo", "Modified"),
                        RepoStatusModel::NameRole);
    worktreeIt->setToolTip(i18n("Files with changes"));
    worktreeIt->setData(WorkTreeRoot, AreaRole);
    worktreeIt->setData(pathUrl, RepoStatusModel::ProjectUrlRole);
    worktreeIt->setSelectable(false);

    conflictIt->setData(i18nc("Files in git which have unresolved (merge) conflits", "Conflicts"),
                        RepoStatusModel::NameRole);
    conflictIt->setToolTip(i18n("Files with unresolved (merge) conflicts"));
    conflictIt->setData(ConflictRoot, AreaRole);
    conflictIt->setData(pathUrl, RepoStatusModel::ProjectUrlRole);
    conflictIt->setSelectable(false);

    untrackedIt->setData(i18nc("Files which are not tracked by a vcs", "Untracked"), RepoStatusModel::NameRole);
    untrackedIt->setToolTip(i18n("Files not tracked in VCS"));
    untrackedIt->setData(UntrackedRoot, AreaRole);
    untrackedIt->setData(pathUrl, RepoStatusModel::ProjectUrlRole);
    untrackedIt->setSelectable(false);

    appendRow(projectIt);
    projectIt->appendRows({indexIt, worktreeIt, conflictIt, untrackedIt});

    /* The project has the current branch appended to its name in the display,
     * we therefore need to update it whenever the branch changes */
    plugin->registerRepositoryForCurrentBranchChanges(pathUrl, this);
    connect(plugin, &GitPlugin::repositoryBranchChanged, this, &RepoStatusModel::repositoryBranchChanged,
            Qt::UniqueConnection);
    repositoryBranchChanged(pathUrl);
}

void RepoStatusModel::removeProject(const IProject* p)
{
    // when the project is closed before it was fully populated, we won't ever see a
    // projectOpened signal - handle this gracefully by just ignoring the remove request
    // Also, we need to ignore this for projects which don't have a git VCS and thus were
    // never added
    if (auto it = findProject(p)) {
        removeRow(it->row());
    }

    if (auto* const plugin = p->versionControlPlugin()) {
        if (auto* const gitPlugin = plugin->extension<GitPlugin>()) {
            const auto pathUrl = p->path().toUrl();
            gitPlugin->unregisterRepositoryForCurrentBranchChanges(pathUrl, this);
        }
    }
}

/* Finds the immediate child of a `parent` item whose data with role `role` has value `value` */
QStandardItem* findItemChild(const QStandardItem* parent, const QVariant& value, int role = Qt::DisplayRole)
{
    for (int i = 0; i < parent->rowCount(); i++) {
        QStandardItem* curr = parent->child(i);

        if (curr->data(role) == value)
            return curr;
    }

    return nullptr;
}

QStandardItem* RepoStatusModel::findProject(const IProject* project) const
{
    if (!project)
        return nullptr;
    return findItemChild(invisibleRootItem(), project->name(), RepoStatusModel::NameRole);
}

RepoStatusModel::ProjectItem RepoStatusModel::projectItem(const IProject* p) const
{
    if (auto proj = findProject(p)) {
        return projectItem(proj);
    }
    return {}; // This occurs when the project has another VCS or no VCS configured.
}

RepoStatusModel::ProjectItem RepoStatusModel::projectItem(QStandardItem* proj) const
{
    Q_ASSERT(proj);
    ProjectItem pi;
    pi.project = proj;
    pi.index = findItemChild(proj, RepoStatusModel::IndexRoot, RepoStatusModel::AreaRole);
    pi.worktree = findItemChild(proj, RepoStatusModel::WorkTreeRoot, RepoStatusModel::AreaRole);
    pi.conflicts = findItemChild(proj, RepoStatusModel::ConflictRoot, RepoStatusModel::AreaRole);
    pi.untracked = findItemChild(proj, RepoStatusModel::UntrackedRoot, RepoStatusModel::AreaRole);
    return pi;
}

bool RepoStatusModel::showInIndex(const GitPlugin::ExtendedState state)
{
    switch (state) {
    case GitPlugin::GitMX:
    case GitPlugin::GitMM:
    case GitPlugin::GitMD:
    case GitPlugin::GitAX:
    case GitPlugin::GitAM:
    case GitPlugin::GitAD:
    case GitPlugin::GitDX:
    case GitPlugin::GitDR:
    case GitPlugin::GitDC:
    case GitPlugin::GitRX:
    case GitPlugin::GitRM:
    case GitPlugin::GitRD:
    case GitPlugin::GitCX:
    case GitPlugin::GitCM:
    case GitPlugin::GitCD:
        return true;
    default:
        return false;
    }
}

bool RepoStatusModel::showInWorkTree(const GitPlugin::ExtendedState state)
{
    switch (state) {
    case GitPlugin::GitXM:
    case GitPlugin::GitXD:
    case GitPlugin::GitXR:
    case GitPlugin::GitXC:
    case GitPlugin::GitMM:
    case GitPlugin::GitMD:
    case GitPlugin::GitAM:
    case GitPlugin::GitAD:
    case GitPlugin::GitDR:
    case GitPlugin::GitDC:
    case GitPlugin::GitRM:
    case GitPlugin::GitRD:
    case GitPlugin::GitCM:
    case GitPlugin::GitCD:
        return true;
    default:
        return false;
    }
}

QString extendedStateToStr(const GitPlugin::ExtendedState state)
{
    switch (state) {
    case GitPlugin::GitXM:
        return i18nc("@item file has unstaged changes", "Modified (unstaged)");
    case GitPlugin::GitXD:
        return i18nc("@item file was deleted from worktree", "Deleted (unstaged)");
    case GitPlugin::GitXR:
        return i18nc("@item file was renamed in worktree", "Renamed (unstaged)");
    case GitPlugin::GitXC:
        return i18nc("@item file was copied in worktree", "Copied (unstaged)");
    case GitPlugin::GitMX:
        return i18nc("@item file has staged changes", "Modified (staged)");
    case GitPlugin::GitMM:
        return i18nc("@item file has both staged and unstaged changes", "Modified (unstaged changes)");
    case GitPlugin::GitMD:
        return i18nc("@item file has staged changes and was deleted in worktree", "Modified (unstaged deletion)");
    case GitPlugin::GitAM:
        return i18nc("@item file was added to versioncontrolsystem and has unstaged changes",
                     "Added (unstaged changes)");
    case GitPlugin::GitAD:
        return i18nc("@item file was added to versioncontrolsystem and deleted in worktree",
                     "Added (unstaged deletion)");
    case GitPlugin::GitDR:
        return i18nc("@item file was deleted from versioncontrolsystem and renamed in worktree",
                     "Deleted (unstaged rename)");
    case GitPlugin::GitDC:
        return i18nc("@item file was deleted from versioncontrolsystem and copied in worktree",
                     "Deleted (unstaged copy)");
    case GitPlugin::GitRX:
        return i18nc("@item file was renamed in versioncontrolsystem", "Renamed (staged)");
    case GitPlugin::GitRM:
        return i18nc("@item file was renamed in versioncontrolsystem and has unstaged changes",
                     "Renamed (unstaged changes)");
    case GitPlugin::GitRD:
        return i18nc("@item file was renamed in versioncontrolsystem and was deleted in worktree",
                     "Renamed (unstaged deletion)");
    case GitPlugin::GitCX:
        return i18nc("@item file was copied in versioncontrolsystem", "Copied");
    case GitPlugin::GitCM:
        return i18nc("@item file was copied in versioncontrolsystem and has unstaged changes",
                     "Copied (unstaged changes)");
    case GitPlugin::GitCD:
        return i18nc("@item file was copied in versioncontrolsystem and was deleted in worktree",
                     "Copied (unstaged deletion)");
    case GitPlugin::GitConflicts:
        return i18nc("@item file has unresolved merge conflicts", "Unresolved conflicts");
    case GitPlugin::GitUntracked:
        return i18nc("@item file is not under vcs", "Untracked");
    case GitPlugin::GitInvalid:
    default:
        return i18nc("file in unknown (invalid) state", "Unknown");
    }
}

bool RepoStatusModel::showInConflicts(const GitPlugin::ExtendedState state)
{
    switch (state) {
    case GitPlugin::GitConflicts:
        return true;
    default:
        return false;
    }
}

bool RepoStatusModel::showInUntracked(const GitPlugin::ExtendedState state)
{
    switch (state) {
    case GitPlugin::GitUntracked:
        return true;
    default:
        return false;
    }
}

const QList<QStandardItem*> RepoStatusModel::allItems(const QStandardItem* parent) const
{
    QList<QStandardItem*> ret;
    if (!parent)
        parent = invisibleRootItem();
    const int rc = parent->rowCount();
    ret.reserve(rc);
    for (int i = 0; i < rc; i++) {
        QStandardItem* child = parent->child(i);
        ret << parent->child(i);
        ret += allItems(child);
    }

    return ret;
}

void RepoStatusModel::removeUrl(const QUrl& url, const QStandardItem* parent)
{
    for (const auto item : allItems(parent)) {
        if (item->data(UrlRole).toUrl() == url)
            removeRow(item->index().row(), item->parent()->index());
    }
}

QList<QUrl> RepoStatusModel::childUrls(const ProjectItem& pItem) const
{
    Q_ASSERT(pItem.project);
    if (!pItem.project) {
        qCWarning(PLUGIN_GIT) << "A null QStandardItem passed to" << Q_FUNC_INFO;
        return {};
    }

    const auto children
        = allItems(pItem.index) + allItems(pItem.worktree) + allItems(pItem.conflicts) + allItems(pItem.untracked);

    QList<QUrl> ret;
    ret.reserve(children.size());
    for (const auto ch : children) {
        ret << indexFromItem(ch).data(UrlRole).toUrl();
    }

    return ret;
}

void RepoStatusModel::updateState(const ProjectItem& pItem, const VcsStatusInfo& status)
{
    Q_ASSERT(pItem.project);

    /* To make the code cleaner, we remove the item from the
     * model first before adding it again to the correct areas */
    removeUrl(status.url(), pItem.project);

    QString path = ICore::self()->projectController()->prettyFileName(status.url(), IProjectController::FormatPlain);
    int pos = path.indexOf(QLatin1Char(':'));
    if (pos > -1)
        path = path.mid(pos + 1);
    QMimeType mime = status.url().isLocalFile()
        ? QMimeDatabase().mimeTypeForFile(status.url().toLocalFile(), QMimeDatabase::MatchExtension)
        : QMimeDatabase().mimeTypeForUrl(status.url());
    QIcon icon = QIcon::fromTheme(mime.iconName());

    QStandardItem* item = nullptr;
    GitPlugin::ExtendedState state = (GitPlugin::ExtendedState)status.extendedState();
    QString stateStr = extendedStateToStr(state);

    if (showInIndex(state)) {
        item = new QStandardItem(icon, path);
        item->setData(Index, AreaRole);
        item->setData(status.url(), UrlRole);
        item->setToolTip(status.url().path() + i18n(" (staged)"));
        pItem.index->appendRow(item);
    }

    if (showInWorkTree(state)) {
        item = new QStandardItem(icon, path);
        item->setData(WorkTree, AreaRole);
        item->setData(status.url(), UrlRole);
        item->setToolTip(status.url().path() + i18n(" (unstaged)"));
        pItem.worktree->appendRow(item);
    }

    if (showInConflicts(state)) {
        item = new QStandardItem(icon, path);
        item->setData(Conflicts, AreaRole);
        item->setData(status.url(), UrlRole);
        item->setToolTip(status.url().path() + i18n(" (conflicts)"));
        pItem.conflicts->appendRow(item);
    }

    if (showInUntracked(state)) {
        item = new QStandardItem(icon, path);
        item->setData(Untracked, AreaRole);
        item->setData(status.url(), UrlRole);
        item->setToolTip(status.url().path() + i18n(" (untracked)"));
        pItem.untracked->appendRow(item);
    }

    if (item) {
        item->setData(state, StatusRole);
        item->setData(stateStr, ReadableStatusRole);
        item->setData(pItem.project->data(ProjectUrlRole), ProjectUrlRole);
    }
}

void RepoStatusModel::fetchStatusesForUrls(IProject* project, const QList<QUrl>& urls,
                                           IBasicVersionControl::RecursionMode mode)
{
    IPlugin* vcsplugin = project->versionControlPlugin();

    if (auto* vcs = vcsplugin ? vcsplugin->extension<IBasicVersionControl>() : nullptr) {
        VcsJob* job = vcs->status(urls, mode);
        job->setProperty("urls", QVariant::fromValue<QList<QUrl>>(urls));
        job->setProperty("mode", QVariant::fromValue<int>(mode));
        job->setProperty("project", QVariant::fromValue(project));
        connect(job, &VcsJob::finished, this, &RepoStatusModel::statusReady);
        ICore::self()->runController()->registerJob(job);
    }
}

const QList<QStandardItem*> RepoStatusModel::projectRoots() const
{
    QList<QStandardItem*> ret;
    auto* root = invisibleRootItem();
    for (int i = 0; i < root->rowCount(); i++) {
        QStandardItem* child = root->child(i);

        if (child->data(AreaRole) == ProjectRoot)
            ret << child;
    }

    return ret;
}

const QList<QStandardItem*> RepoStatusModel::items(const QStandardItem* project, Areas area) const
{
    QList<QStandardItem*> ret;
    for (auto* it : allItems(project)) {
        if ((RepoStatusModel::Areas)it->data(RepoStatusModel::AreaRole).toInt() == area)
            ret << it;
    }

    return ret;
}

void RepoStatusModel::statusReady(KJob* job)
{
    auto* status = qobject_cast<VcsJob*>(job);
    if (!status)
        return;

    const QList<QVariant> states = status->fetchResults().toList();
    auto* project = job->property("project").value<IProject*>();

    const auto itProject = projectItem(project);
    if (!itProject.isValid())
        return;

    QSet<QUrl> foundUrls;
    foundUrls.reserve(states.size());
    for (const auto& state : states) {
        const VcsStatusInfo st = state.value<VcsStatusInfo>();
        foundUrls += st.url();
        updateState(itProject, st);
    }

    IBasicVersionControl::RecursionMode mode = IBasicVersionControl::RecursionMode(job->property("mode").toInt());
    const QList<QUrl> projectUrls = childUrls(itProject);
    const QSet<QUrl> uncertainUrls = QSet<QUrl>(projectUrls.begin(), projectUrls.end()).subtract(foundUrls);
    const QList<QUrl> sourceUrls = job->property("urls").value<QList<QUrl>>();
    for (const QUrl& url : sourceUrls) {
        if (url.isLocalFile() && QDir(url.toLocalFile()).exists()) {
            for (const QUrl& currentUrl : uncertainUrls) {
                if ((mode == IBasicVersionControl::NonRecursive
                     && currentUrl.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash)
                         == url.adjusted(QUrl::StripTrailingSlash))
                    || (mode == IBasicVersionControl::Recursive && url.isParentOf(currentUrl))) {
                    removeUrl(currentUrl, itProject.project);
                }
            }
        }
    }
}

void RepoStatusModel::documentSaved(const IDocument* document)
{
    reload({ document->url() });
}

void RepoStatusModel::itemsAdded(const QModelIndex& parent, int start, int end)
{
    ProjectModel* model = ICore::self()->projectController()->projectModel();
    ProjectBaseItem* item = model->itemFromIndex(parent);

    if (!item)
        return;

    IProject* project = item->project();

    if (!findProject(project))
        return;

    // FIXME: this code is utterly broken. See the long comment in the slot
    //        ProjectChangesModel::itemsAdded(), from which the code has been copied.

    QList<QUrl> urls;

    for (int i = start; i < end; i++) {
        QModelIndex idx = parent.model()->index(i, 0, parent);
        item = model->itemFromIndex(idx);

        if (item->type() == ProjectBaseItem::File || item->type() == ProjectBaseItem::Folder
            || item->type() == ProjectBaseItem::BuildFolder)
            urls += item->path().toUrl();
    }

    if (!urls.isEmpty())
        fetchStatusesForUrls(project, urls, IBasicVersionControl::NonRecursive);
}

void RepoStatusModel::reload(const QList<IProject*>& projects)
{
    for (IProject* project : projects) {
        if (findProject(project)) {
            fetchStatusesForUrls(project, { project->path().toUrl() }, IBasicVersionControl::Recursive);
        }
    }
}

void RepoStatusModel::reload(const QList<QUrl>& urls)
{
    for (const QUrl& url : urls) {
        IProject* project = ICore::self()->projectController()->findProjectForUrl(url);

        if (findProject(project)) {
            // FIXME: merge multiple urls of the same project
            fetchStatusesForUrls(project, { url }, IBasicVersionControl::NonRecursive);
        }
    }
}

void RepoStatusModel::reloadAll()
{
    QList<IProject*> projects = ICore::self()->projectController()->projects();
    reload(projects);
}

void RepoStatusModel::jobUnregistered(KJob* job)
{
    static const std::array<VcsJob::JobType, 9> readOnly = {
        VcsJob::Add,
        VcsJob::Remove,
        VcsJob::Pull,
        VcsJob::Commit,
        VcsJob::Move,
        VcsJob::Copy,
        VcsJob::Revert,
        VcsJob::Reset,
        VcsJob::Apply
    };

    auto* vcsjob = qobject_cast<VcsJob*>(job);
    if (vcsjob && std::find(readOnly.begin(), readOnly.end(), vcsjob->type()) != readOnly.end()) {
        reloadAll();
    }
}

void RepoStatusModel::repositoryBranchChanged(const QUrl& url)
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    if (findProject(project)) {
        IPlugin* v = project->versionControlPlugin();
        Q_ASSERT(v);
        auto* branching = v->extension<IBranchingVersionControl>();
        Q_ASSERT(branching);
        VcsJob* job = branching->currentBranch(url);
        connect(job, &VcsJob::resultsReady, this, &RepoStatusModel::branchNameReady);
        job->setProperty("project", QVariant::fromValue<QObject*>(project));
        ICore::self()->runController()->registerJob(job);
    }
}

void RepoStatusModel::branchNameReady(VcsJob* job)
{
    auto* const project = qobject_cast<IProject*>(job->property("project").value<QObject*>());
    const auto item = findProject(project);
    if (!item)
        return;

    if (job->status() == VcsJob::JobSucceeded) {
        QString name = job->fetchResults().toString();
        QString branchName = name.isEmpty() ? i18n("no branch") : name;
        item->setText(i18nc("project name (branch name)", "%1 (%2)", project->name(), branchName));
        item->setData(branchName, RepoStatusModel::BranchNameRole);
    } else {
        item->setData(QStringLiteral("unknown"), RepoStatusModel::BranchNameRole);
        item->setText(project->name());
    }

    reload(QList<IProject*>() << project);
}

#include "moc_repostatusmodel.cpp"
