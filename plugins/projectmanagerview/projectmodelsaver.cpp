/* This file is part of KDevelop
    Copyright 2012 Andrew Fuller <mactalla.obair@gmail.com>

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

#include "projectmodelsaver.h"
#include "projecttreeview.h"

#include "project/projectmodel.h"
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <QtCore/QStringList>
#include <QAbstractProxyModel>

namespace KDevelop
{

ProjectModelSaver::ProjectModelSaver()
: m_project(0)
{
}

void ProjectModelSaver::setProject(IProject* project)
{
    m_project = project;
}

QModelIndex ProjectModelSaver::indexFromConfigString(const QAbstractItemModel *viewModel, const QString &key) const
{
    const KDevelop::ProjectModel *projectModel = KDevelop::ICore::self()->projectController()->projectModel();

    const QModelIndex sourceIndex = projectModel->pathToIndex(key.split('/'));

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

    ProjectBaseItem* item = index.data(ProjectModel::ProjectItemRole).value<ProjectBaseItem*>();
    if ( !item || item->project() != m_project ) {
        return QString();
    }

    return ICore::self()->projectController()->projectModel()->pathFromIndex( item->index() ).join("/");
}

}

