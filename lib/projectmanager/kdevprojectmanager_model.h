/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef KDEVPROJECTMANAGER_MODEL_H
#define KDEVPROJECTMANAGER_MODEL_H

#include <QtCore/QAbstractItemModel>

class ProjectModel;
class ProjectModelItem;

class KDevProjectManagerModel: public QAbstractItemModel
{
  Q_OBJECT
public:
  KDevProjectManagerModel(ProjectModel *project, QObject *parent = 0);
  virtual ~KDevProjectManagerModel();

  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  virtual QModelIndex parent(const QModelIndex &index) const;

  virtual int rowCount(const QModelIndex &parent) const;
  virtual int columnCount(const QModelIndex &parent) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  ProjectModel *project() const;

  QModelIndex indexOf(ProjectModelItem *item, int column = 0) const;
  ProjectModelItem *privateData(const QModelIndex &index) const;

private:
  ProjectModel *m_project;
};

#endif // KDEVPROJECTMANAGER_MODEL_H
