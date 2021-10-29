/*
    SPDX-FileCopyrightText: 2012 Andrew Fuller <mactalla.obair@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectmodelsaver.h"
#include "projecttreeview.h"

#include "project/projectmodel.h"
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <QStringList>
#include <QAbstractProxyModel>

namespace KDevelop
{

ProjectModelSaver::ProjectModelSaver()
{
}

void ProjectModelSaver::setProject(IProject* project)
{
    m_project = project;
}

QModelIndex ProjectModelSaver::indexFromConfigString(const QAbstractItemModel *viewModel, const QString &key) const
{
    const KDevelop::ProjectModel *projectModel = KDevelop::ICore::self()->projectController()->projectModel();

    const QModelIndex sourceIndex = projectModel->pathToIndex(key.split(QLatin1Char('/')));

    if ( m_project && sourceIndex.isValid() ) {
        ProjectBaseItem* item = projectModel->itemFromIndex(sourceIndex);
        if ( item && item->project() == m_project ) {
            return ProjectTreeView::mapFromSource(qobject_cast<const QAbstractProxyModel*>(viewModel), sourceIndex);
        }
    }
    return QModelIndex();
}

QString ProjectModelSaver::indexToConfigString(const QModelIndex& index) const
{
    if( !index.isValid() || !m_project ) {
        return QString();
    }

    auto* item = index.data(ProjectModel::ProjectItemRole).value<ProjectBaseItem*>();
    if ( !item || item->project() != m_project ) {
        return QString();
    }

    return ICore::self()->projectController()->projectModel()->pathFromIndex( item->index() ).join(QLatin1Char('/'));
}

}

