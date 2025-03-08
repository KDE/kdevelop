/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectchangesmodel.h"

#include "debug.h"

#include <KLocalizedString>

#include <vcs/interfaces/ibasicversioncontrol.h>
#include <interfaces/ibranchingversioncontrol.h>
#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <vcs/vcsstatusinfo.h>
#include <vcs/vcsjob.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <QDir>
#include <QHash>
#include <QIcon>

#include <array>

using namespace KDevelop;

ProjectChangesModel::ProjectChangesModel(QObject* parent)
    : VcsFileChangesModel(parent)
{
    const auto projects = ICore::self()->projectController()->projects();
    for (IProject* p : projects) {
        addProject(p);
    }

    connect(ICore::self()->projectController(), &IProjectController::projectOpened,
                                              this, &ProjectChangesModel::addProject);
    connect(ICore::self()->projectController(), &IProjectController::projectClosing,
                                              this, &ProjectChangesModel::removeProject);
    
    connect(ICore::self()->documentController(), &IDocumentController::documentSaved,
                                                this, &ProjectChangesModel::documentSaved);
    connect(ICore::self()->projectController()->projectModel(), &ProjectModel::rowsInserted,
                                                this, &ProjectChangesModel::itemsAdded);
    
    connect(ICore::self()->runController(), &IRunController::jobUnregistered, this, &ProjectChangesModel::jobUnregistered);
}

ProjectChangesModel::~ProjectChangesModel()
{
    const auto projects = ICore::self()->projectController()->projects();
    for (auto* const project : projects) {
        removeProject(project);
    }
}

void ProjectChangesModel::addProject(IProject* p)
{
    auto* it = new QStandardItem(p->name());
    it->setData(p->name(), ProjectChangesModel::ProjectNameRole);
    IPlugin* plugin = p->versionControlPlugin();
    if(plugin) {
        auto* vcs = plugin->extension<IBasicVersionControl>();

        auto info = ICore::self()->pluginController()->pluginInfo(plugin);

        it->setIcon(QIcon::fromTheme(info.iconName()));
        it->setToolTip(vcs->name());

        auto* branchingExtension = plugin->extension<KDevelop::IBranchingVersionControl>();
        if(branchingExtension) {
            const auto pathUrl = p->path().toUrl();
            branchingExtension->registerRepositoryForCurrentBranchChanges(pathUrl, this);
            // can't use new signal slot syntax here, IBranchingVersionControl is not a QObject
            connect(plugin, SIGNAL(repositoryBranchChanged(QUrl)), this, SLOT(repositoryBranchChanged(QUrl)),
                    Qt::UniqueConnection);
            repositoryBranchChanged(pathUrl);
        } else
            reload(QList<IProject*>() << p);
    } else {
        it->setEnabled(false);
    }
    
    appendRow(it);
}

void ProjectChangesModel::removeProject(IProject* p)
{
    QStandardItem* it=projectItem(p);
    if (!it) {
        // when the project is closed before it was fully populated, we won't ever see a
        // projectOpened signal - handle this gracefully by just ignoring the remove request
        return;
    }
    removeRow(it->row());

    if (auto* const plugin = p->versionControlPlugin()) {
        if (auto* const branchingExtension = plugin->extension<KDevelop::IBranchingVersionControl>()) {
            const auto pathUrl = p->path().toUrl();
            branchingExtension->unregisterRepositoryForCurrentBranchChanges(pathUrl, this);
        }
    }
}

QStandardItem* findItemChild(QStandardItem* parent, const QVariant& value, int role = Qt::DisplayRole)
{
    for(int i=0; i<parent->rowCount(); i++) {
        QStandardItem* curr=parent->child(i);
        
        if(curr->data(role) == value)
            return curr;
    }
    return nullptr;
}

QStandardItem* ProjectChangesModel::projectItem(const IProject* project) const
{
    return findItemChild(invisibleRootItem(), project->name(), ProjectChangesModel::ProjectNameRole);
}

void ProjectChangesModel::updateState(IProject* p, const KDevelop::VcsStatusInfo& status)
{
    QStandardItem* pItem = projectItem(p);
    Q_ASSERT(pItem);
    
    VcsFileChangesModel::updateState(pItem, status);
}

void ProjectChangesModel::changes(IProject* project, const QList<QUrl>& urls, IBasicVersionControl::RecursionMode mode)
{
    IPlugin* vcsplugin=project->versionControlPlugin();
    IBasicVersionControl* vcs = vcsplugin ? vcsplugin->extension<IBasicVersionControl>() : nullptr;
    
    if(vcs && vcs->isVersionControlled(urls.first())) { //TODO: filter?
        VcsJob* job=vcs->status(urls, mode);
        job->setProperty("urls", QVariant::fromValue<QList<QUrl>>(urls));
        job->setProperty("mode", QVariant::fromValue<int>(mode));
        job->setProperty("project", QVariant::fromValue(project));
        connect(job, &VcsJob::finished, this, &ProjectChangesModel::statusReady);
        
        ICore::self()->runController()->registerJob(job);
    }
}

void ProjectChangesModel::statusReady(KJob* job)
{
    auto* status=static_cast<VcsJob*>(job);

    const QList<QVariant> states = status->fetchResults().toList();
    auto* project = job->property("project").value<KDevelop::IProject*>();
    if(!project)
        return;

    QSet<QUrl> foundUrls;
    foundUrls.reserve(states.size());
    for (const QVariant& state : states) {
        const VcsStatusInfo st = state.value<VcsStatusInfo>();
        foundUrls += st.url();

        updateState(project, st);
    }

    QStandardItem* itProject = projectItem(project);
    if (!itProject) {
        qCDebug(PROJECT) << "Project no longer listed in model:" << project->name() << "- skipping update";
        return;
    }

    IBasicVersionControl::RecursionMode mode = IBasicVersionControl::RecursionMode(job->property("mode").toInt());
    const QList<QUrl> projectUrls = urls(itProject);
    const QSet<QUrl> uncertainUrls = QSet<QUrl>(projectUrls.begin(), projectUrls.end()).subtract(foundUrls);
    const QList<QUrl> sourceUrls = job->property("urls").value<QList<QUrl>>();
    for (const QUrl& url : sourceUrls) {
        if(url.isLocalFile() && QDir(url.toLocalFile()).exists()) {
            for (const QUrl& currentUrl : uncertainUrls) {
                if((mode == IBasicVersionControl::NonRecursive && currentUrl.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) == url.adjusted(QUrl::StripTrailingSlash))
                    || (mode == IBasicVersionControl::Recursive && url.isParentOf(currentUrl))
                ) {
                    removeUrl(itProject->index(), currentUrl);
                }
            }
        }
    }
}

void ProjectChangesModel::documentSaved(KDevelop::IDocument* document)
{
    reload({document->url()});
}

void ProjectChangesModel::itemsAdded(const QModelIndex& parent, int start, int end)
{
    ProjectModel* model=ICore::self()->projectController()->projectModel();
    ProjectBaseItem* item=model->itemFromIndex(parent);

    if(!item)
        return;

    IProject* project=item->project();
    
    if(!project)
        return;

    // FIXME: this code is utterly broken. When a new file is created, the signal ProjectModel::rowsInserted() is
    //        emitted and the item has a parent. But the loop below does not process the added file, because the
    //        `start` and `end` parameters of the signal QAbstractItemModel::rowsInserted() are inclusive indices.
    //        If this bug is fixed by changing the loop's condition to `i <= end`, the urls list would remain empty
    //        because ProjectBaseItem() appends the item under construction to the model while its type is still
    //        ProjectBaseItem, so the item->type() checks below fail. Even if the checks are removed, the path
    //        of a bare ProjectBaseItem is always empty, so the changes() call below would not work anyway.
    //
    //        Suppose one fixes all these bugs, what then? KDevelop would freeze when many files are added,
    //        e.g. while switching between distant git revisions! That's because this slot, and therefore
    //        IBasicVersionControl::status(), would be invoked separately for each added file. Each call to
    //        GitPlugin::status() would schedule an invocation of GitPlugin::parseGitStatusOutput().
    //        parseGitStatusOutput() calls GitPlugin::getLsFiles(), which runs a
    //        `git ls-files` process synchronously. This implementation must be optimized before fixing
    //        the bugs. The UI freezes have been reported at https://bugs.kde.org/show_bug.cgi?id=486949
    //
    //        Another related bug is that the signal QAbstractItemModel::rowsRemoved() is not connected to for cleanup.
    //
    //        This slot and related code have been copied almost verbatim to the class RepoStatusModel. Therefore,
    //        RepoStatusModel suffers from all these bugs as well. The extensive code duplication causes another
    //        performance bug: when both ProjectChangesModel and RepoStatusModel instances exist at the same time, each
    //        of them runs the same git processes and invokes the same slow handling of process results in GitPlugin.
    //        This work should not be duplicated and ought to be performed at most once in order to improve performance.

    QList<QUrl> urls;
    
    for(int i=start; i<end; i++) {
        QModelIndex idx=parent.model()->index(i, 0, parent);
        item=model->itemFromIndex(idx);
        
        if(item->type()==ProjectBaseItem::File || item->type()==ProjectBaseItem::Folder || item->type()==ProjectBaseItem::BuildFolder)
            urls += item->path().toUrl();
    }
        
    if(!urls.isEmpty())
        changes(project, urls, KDevelop::IBasicVersionControl::NonRecursive);
}

void ProjectChangesModel::reload(const QList<IProject*>& projects)
{
    for (IProject* project : projects) {
        changes(project, {project->path().toUrl()}, KDevelop::IBasicVersionControl::Recursive);
    }
}

void ProjectChangesModel::reload(const QList<QUrl>& urls)
{
    QHash<IProject*, QList<QUrl>> groupedUrls;

    for (const QUrl& url : urls) {
        IProject* project=ICore::self()->projectController()->findProjectForUrl(url);
        
        if (project) {
            groupedUrls[project].push_back(url);
        }
    }

    for (const auto& [project, urls] : std::as_const(groupedUrls).asKeyValueRange()) {
        changes(project, urls, IBasicVersionControl::NonRecursive);
    }
}

void ProjectChangesModel::reloadAll()
{
    QList< IProject* > projects = ICore::self()->projectController()->projects();
    reload(projects);
}

void ProjectChangesModel::jobUnregistered(KJob* job)
{
    static const std::array<VcsJob::JobType, 7> readOnly = {
        KDevelop::VcsJob::Add,
        KDevelop::VcsJob::Remove,
        KDevelop::VcsJob::Pull,
        KDevelop::VcsJob::Commit,
        KDevelop::VcsJob::Move,
        KDevelop::VcsJob::Copy,
        KDevelop::VcsJob::Revert,
    };

    auto* vcsjob = qobject_cast<VcsJob*>(job);
    if (vcsjob && std::find(readOnly.begin(), readOnly.end(), vcsjob->type()) != readOnly.end()) {
        reloadAll();
    }
}

void ProjectChangesModel::repositoryBranchChanged(const QUrl& url)
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    if(project) {
        IPlugin* v = project->versionControlPlugin();
        Q_ASSERT(v);
        auto* branching = v->extension<IBranchingVersionControl>();
        Q_ASSERT(branching);
        VcsJob* job = branching->currentBranch(url);
        connect(job, &VcsJob::resultsReady, this, &ProjectChangesModel::branchNameReady);
        job->setProperty("project", QVariant::fromValue<QObject*>(project));
        ICore::self()->runController()->registerJob(job);
    }
}

void ProjectChangesModel::branchNameReady(VcsJob* job)
{
    auto* project = qobject_cast<IProject*>(job->property("project").value<QObject*>());
    if(job->status()==VcsJob::JobSucceeded) {
        QString name = job->fetchResults().toString();
        const QString branchName = name.isEmpty() ? i18nc("@item:intext", "no branch") : name;
        projectItem(project)->setText(i18nc("project name (branch name)", "%1 (%2)", project->name(), branchName));
    } else {
        projectItem(project)->setText(project->name());
    }

    reload(QList<IProject*>() << project);
}

#include "moc_projectchangesmodel.cpp"
