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

#include "kdevprojectfilter.h"
#include <kdevprojectmodel.h>

KDevProjectOverviewFilter::KDevProjectOverviewFilter(KDevProjectModel *model, QObject *parent)
  : KFilterModel(model, parent)
{
}

KDevProjectOverviewFilter::~KDevProjectOverviewFilter()
{
}

bool KDevProjectOverviewFilter::matches(const QModelIndex &index) const
{
  return true;
}

KDevProjectDetailsFilter::KDevProjectDetailsFilter(KDevProjectModel *model, QObject *parent)
  : KFilterModel(model, parent)
{
}

KDevProjectDetailsFilter::~KDevProjectDetailsFilter()
{
}

bool KDevProjectDetailsFilter::matches(const QModelIndex &index) const
{
  return true;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
