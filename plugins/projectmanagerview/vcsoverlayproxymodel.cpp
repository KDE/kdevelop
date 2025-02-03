/*
    SPDX-FileCopyrightText: 2013 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsoverlayproxymodel.h"
#include <projectchangesmodel.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iplugin.h>
#include <vcs/interfaces/ibranchingversioncontrol.h>
#include <vcs/vcsjob.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <KLocalizedString>

#include <QPointer>


using namespace KDevelop;

using SafeProjectPointer = QPointer<KDevelop::IProject>;

VcsOverlayProxyModel::VcsOverlayProxyModel(QObject* parent): QIdentityProxyModel(parent)
{
    connect(ICore::self()->projectController(), &IProjectController::projectOpened,
                                              this, &VcsOverlayProxyModel::addProject);
    connect(ICore::self()->projectController(), &IProjectController::projectClosing,
                                              this, &VcsOverlayProxyModel::removeProject);

    const auto projects = ICore::self()->projectController()->projects();
    for (const auto project : projects) {
        addProject(project);
    }
}

QVariant VcsOverlayProxyModel::data(const QModelIndex& proxyIndex, int role) const
{
    if(role == VcsStatusRole) {
        if (!proxyIndex.parent().isValid()) {
            auto* p = qobject_cast<IProject*>(proxyIndex.data(ProjectModel::ProjectRole).value<QObject*>());
            return m_branchName.value(p);
        } else {
            ProjectChangesModel* model = ICore::self()->projectController()->changesModel();
            const QUrl url = proxyIndex.data(ProjectModel::UrlRole).toUrl();
            const auto idx = model->match(model->index(0, 0), ProjectChangesModel::UrlRole, url, 1, Qt::MatchExactly).value(0);
            return idx.sibling(idx.row(), 1).data(Qt::DisplayRole);
        }
    } else
        return QIdentityProxyModel::data(proxyIndex, role);
}

void VcsOverlayProxyModel::addProject(IProject* p)
{
    IPlugin* plugin = p->versionControlPlugin();
    if(!plugin)
        return;

    // TODO: Show revision in case we're in "detached" state
    auto* branchingExtension = plugin->extension<KDevelop::IBranchingVersionControl>();
    if(branchingExtension) {
        const QUrl url = p->path().toUrl();
        branchingExtension->registerRepositoryForCurrentBranchChanges(url);
        //can't use new signal/slot syntax here, IBranchingVersionControl is not a QObject
        connect(plugin, SIGNAL(repositoryBranchChanged(QUrl)), SLOT(repositoryBranchChanged(QUrl)),
                Qt::UniqueConnection);
        repositoryBranchChanged(url);
    }
}

void VcsOverlayProxyModel::repositoryBranchChanged(const QUrl& url)
{
    const QList<IProject*> allProjects = ICore::self()->projectController()->projects();
    for (IProject* project : allProjects) {
        const bool isExactMatch = url.matches(project->path().toUrl(), QUrl::StripTrailingSlash);
        const bool isParentOf = url.isParentOf(project->path().toUrl());
        if (isParentOf || isExactMatch) {
            // example projects in KDevelop:
            // - /path/to/mygitrepo/:          isParentOf=0 isExactMatch=1,
            // - /path/to/mygitrepo/myproject: isParentOf=1 isExactMatch=0
            // - /path/to/norepo:              isParentOf=0 isExactMatch=0
            // isParentOf=1 isExactMatch=1 is not a valid combination

            IPlugin* v = project->versionControlPlugin();
            Q_ASSERT(!isExactMatch || v); // project url == 'change' url => project should be associated with a VCS plugin
            if (!v) {
                continue;
            }

            auto* branching = v->extension<IBranchingVersionControl>();
            Q_ASSERT(branching);
            VcsJob* job = branching->currentBranch(url);
            connect(job, &VcsJob::resultsReady, this, &VcsOverlayProxyModel::branchNameReady);
            job->setProperty("project", QVariant::fromValue<SafeProjectPointer>(project));
            ICore::self()->runController()->registerJob(job);
        }
    }
}

void VcsOverlayProxyModel::branchNameReady(KDevelop::VcsJob* job)
{
    const QString noBranchStr = i18nc("Version control: Currently not on a branch", "(no branch)");

    if(job->status()==VcsJob::JobSucceeded) {
        SafeProjectPointer p = job->property("project").value<SafeProjectPointer>();
        QModelIndex index = indexFromProject(p);
        if(index.isValid()) {
            IProject* project = p.data();
            const auto branchName =  job->fetchResults().toString();
            m_branchName[project] = branchName.isEmpty() ? noBranchStr : branchName;
            emit dataChanged(index, index);
        }
    }
}

void VcsOverlayProxyModel::removeProject(IProject* p)
{
    m_branchName.remove(p);
}

QModelIndex VcsOverlayProxyModel::indexFromProject(QObject* project)
{
    for(int i=0; i<rowCount(); i++) {
        QModelIndex idx = index(i,0);
        if(idx.data(ProjectModel::ProjectRole).value<QObject*>()==project) {
            return idx;
        }
    }
    return QModelIndex();
}

#include "moc_vcsoverlayproxymodel.cpp"
