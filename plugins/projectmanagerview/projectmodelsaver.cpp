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

#include "project/projectmodel.h"

#include <QtCore/QStringList>
#include <QtGui/QAbstractProxyModel>

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

QModelIndex ProjectModelSaver::indexFromConfigString(const QAbstractItemModel *model, const QString &key) const
{
    const QAbstractProxyModel *proxy = qobject_cast<const QAbstractProxyModel*>(model);
    if( !proxy ) {
        return QModelIndex();
    }

    const KDevelop::ProjectModel *projectModel = qobject_cast<const KDevelop::ProjectModel*>(proxy->sourceModel());

    const QModelIndex sourceIndex = projectModel->pathToIndex(key.split("/"));

    if ( m_project && sourceIndex.isValid() ) {
        ProjectBaseItem* item = projectModel->itemFromIndex(sourceIndex);
        if ( !item || item->project() != m_project ) {
            return QModelIndex();
        }
    }

    return proxy->mapFromSource(sourceIndex);
}

QString ProjectModelSaver::indexToConfigString(const QModelIndex& index) const
{
    if( !index.isValid() ) {
        return QString();
    }

    const QAbstractProxyModel *proxy = qobject_cast<const QAbstractProxyModel*>(index.model());
    if( !proxy ) {
        return QString();
    }

    const QModelIndex sourceIndex = proxy->mapToSource(index);

    const KDevelop::ProjectModel *projectModel = qobject_cast<const KDevelop::ProjectModel*>(proxy->sourceModel());

    if ( m_project ) {
        ProjectBaseItem* item = projectModel->itemFromIndex( sourceIndex );
        if ( !item || item->project() != m_project ) {
            return QString();
        }
    }

    return projectModel->pathFromIndex( sourceIndex ).join("/");
}

}

