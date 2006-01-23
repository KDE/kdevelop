/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2005 Marius Bugge Monsen <mariusbu@pvv.org>

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

#ifndef KDEVPROJECTFILTER_H
#define KDEVPROJECTFILTER_H

#include <kfiltermodel.h>

class KDevProjectModel;

class KDevProjectOverviewFilter: public KFilterModel
{
  Q_OBJECT
public:
  explicit KDevProjectOverviewFilter(KDevProjectModel *model, QObject *parent = 0);
  virtual ~KDevProjectOverviewFilter();

protected:
  virtual bool matches(const QModelIndex &index) const;
};

class KDevProjectDetailsFilter: public KFilterModel
{
  Q_OBJECT
public:
  explicit KDevProjectDetailsFilter(KDevProjectModel *model, QObject *parent = 0);
  virtual ~KDevProjectDetailsFilter();

protected:
  virtual bool matches(const QModelIndex &index) const;
};

#endif // KDEVPROJECTFILTER_H

// kate: space-indent on; indent-width 2; replace-tabs on;

