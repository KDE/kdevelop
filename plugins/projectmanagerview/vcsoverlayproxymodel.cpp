/* This file is part of KDevelop
    Copyright 2013 Aleix Pol <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "vcsoverlayproxymodel.h"
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iplugin.h>
#include <vcs/interfaces/ibranchingversioncontrol.h>
#include <vcs/vcsjob.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <KLocale>

#include <QDebug>

using namespace KDevelop;

using SafeProjectPointer = QPointer<KDevelop::IProject>;
Q_DECLARE_METATYPE(SafeProjectPointer)

VcsOverlayProxyModel::VcsOverlayProxyModel(QObject* parent): QIdentityProxyModel(parent)
{
    connect(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
                                              SLOT(addProject(KDevelop::IProject*)));
    connect(ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
                                              SLOT(removeProject(KDevelop::IProject*)));

    foreach (auto project, ICore::self()->projectController()->projects()) {
        addProject(project);
    }
}

QVariant VcsOverlayProxyModel::data(const QModelIndex& proxyIndex, int role) const
{
    if(role == BranchNameRole && !proxyIndex.parent().isValid()) {
        IProject* p = qobject_cast<IProject*>(proxyIndex.data(ProjectModel::ProjectRole).value<QObject*>());
        return m_branchName.value(p);
    } else
        return QAbstractProxyModel::data(proxyIndex, role);;
}

void VcsOverlayProxyModel::addProject(IProject* p)
{
    IPlugin* plugin = p->versionControlPlugin();
    if(!plugin)
        return;
    
    IBranchingVersionControl* branchingExtension = plugin->extension<KDevelop::IBranchingVersionControl>();
    if(branchingExtension) {
        const KUrl url = p->path().toUrl();
        branchingExtension->registerRepositoryForCurrentBranchChanges(url);
        connect(plugin, SIGNAL(repositoryBranchChanged(KUrl)), SLOT(repositoryBranchChanged(KUrl)));
        repositoryBranchChanged(url);
    }
}

void VcsOverlayProxyModel::repositoryBranchChanged(const KUrl& url)
{
    QList<IProject*> allProjects = ICore::self()->projectController()->projects();
    foreach(IProject* project, allProjects) {
        if( url.isParentOf(project->folder())) {
            IPlugin* v = project->versionControlPlugin();
            Q_ASSERT(v);
            IBranchingVersionControl* branching = v->extension<IBranchingVersionControl>();
            Q_ASSERT(branching);
            VcsJob* job = branching->currentBranch(url);
            connect(job, SIGNAL(resultsReady(KDevelop::VcsJob*)), SLOT(branchNameReady(KDevelop::VcsJob*)));
            job->setProperty("project", QVariant::fromValue<SafeProjectPointer>(project));
            ICore::self()->runController()->registerJob(job);
        }
    }
}

void VcsOverlayProxyModel::branchNameReady(KDevelop::VcsJob* job)
{
    if(job->status()==VcsJob::JobSucceeded) {
        SafeProjectPointer p = job->property("project").value<SafeProjectPointer>();
        QModelIndex index = indexFromProject(p);
        if(index.isValid()) {
            IProject* project = p.data();
            m_branchName[project] = job->fetchResults().toString();
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
