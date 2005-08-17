/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kdevprojectmanager_model.h"
#include <kdevprojectmodel.h>

KDevProjectManagerModel::KDevProjectManagerModel(ProjectModel *project, QObject *parent)
  : QAbstractItemModel(parent),
    m_project(project)
{
}

KDevProjectManagerModel::~KDevProjectManagerModel()
{
}

ProjectModel *KDevProjectManagerModel::project() const
{
  return m_project;
}

QModelIndex KDevProjectManagerModel::indexOf(ProjectModelItem *item, int column) const
{
}

ProjectModelItem *KDevProjectManagerModel::privateData(const QModelIndex &index) const
{
  return reinterpret_cast<ProjectModelItem*>(index.internalPointer());
}

QModelIndex KDevProjectManagerModel::index(int row, int column, const QModelIndex &parent) const
{
  return index;
}

QModelIndex KDevProjectManagerModel::parent(const QModelIndex &index) const
{
}

int KDevProjectManagerModel::rowCount(const QModelIndex &parent) const
{
}

int KDevProjectManagerModel::columnCount(const QModelIndex &parent) const
{
}

QVariant KDevProjectManagerModel::data(const QModelIndex &index, int role) const
{
}

bool KDevProjectManagerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
}

QVariant KDevProjectManagerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
}

#include "kdevprojectmanager_model.moc"
