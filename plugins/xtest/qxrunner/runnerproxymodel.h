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

/*!
 * \file  runnerproxymodel.h
 *
 * \brief Declares class RunnerProxyModel.
 */

#ifndef RUNNERROXYMODEL_H
#define RUNNERROXYMODEL_H

#include "proxymodelcommon.h"

#include <QSortFilterProxyModel>

namespace QxRunner
{

/*!
 * \brief The RunnerProxyModel class provides support for sorting and
 *        filtering data passed between RunnerModel and a view.
 *
 * When a column of the model is disabled, no data is returned for
 * that column.
 *
 * \sa \ref views
 */

class RunnerProxyModel : public QSortFilterProxyModel,
                         public ProxyModelCommon
{
    Q_OBJECT

public: // Operations

    /*!
     * Constructs a runner proxy model with the given \a parent.
     */
    RunnerProxyModel(QObject* parent);

    /*!
     * Destroys this runner proxy model.
     */
    ~RunnerProxyModel();

    /*!
     * Returns the data stored under the given \a role for the item
     * referred to by \a index.
     */
    QVariant data(const QModelIndex& index, int role) const;

private: // Operations

    // Copy and assignment not supported.
    RunnerProxyModel(const RunnerProxyModel&);
    RunnerProxyModel& operator=(const RunnerProxyModel&);
};

} // namespace

#endif // RUNNERROXYMODEL_H
