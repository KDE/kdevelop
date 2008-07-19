/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_RUNNERPROXYMODEL_H
#define VERITAS_RUNNERPROXYMODEL_H

#include "veritas/mvc/proxymodelcommon.h"

#include <QSortFilterProxyModel>

namespace Veritas
{

/*!
 * \brief The RunnerProxyModel class provides support for sorting and
 *        filtering data passed between RunnerModel and a view.
 *
 * It is currently not used, but will probably be in the near
 * future. Most likely for a regex-test filter box.
 */
class RunnerProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
    RunnerProxyModel(QObject* parent);
    ~RunnerProxyModel();

    /*! Returns the data stored under the given \a role for the item
     * referred to by \a index. */
    QVariant data(const QModelIndex& index, int role) const;

private:
    // Copy and assignment not supported.
    RunnerProxyModel(const RunnerProxyModel&);
    RunnerProxyModel& operator=(const RunnerProxyModel&);
};

} // namespace

#endif // VERITAS_RUNNERPROXYMODEL_H
