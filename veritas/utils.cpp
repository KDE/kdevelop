/* KDevelop xUnit plugin
 *
 * Copyright 2006 Ernst Huber <qxrunner@systest.ch>
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

#include "veritas/utils.h"
#include "veritas/test.h"

#include <QTreeView>
#include <QHeaderView>
#include <QIcon>
#include <KIcon>
#include <KIconLoader>
#include <QSortFilterProxyModel>

namespace Veritas
{

QList<int> Utils::columnSizes(QTreeView* view)
{
    QList<int> sizes;

    for (int i = 0; i < view->header()->count(); i++) {
        sizes.append(view->columnWidth(i));
    }

    return sizes;
}

namespace {
bool g_init = false;
QVariant g_failIcon;
QVariant g_notRunIcon;
QVariant g_successIcon;
}


QVariant Utils::resultIcon(int result)
{
    if (!g_init) {
        g_failIcon = QIcon(":/icons/arrow-right-bordeaux-16.png");
        g_notRunIcon = QIcon(":/icons/arrow-right-grey-16.png");
        g_successIcon = KIcon("arrow-right");
        g_init = true;
    }
    switch (result) {
    case Veritas::RunSuccess:
        return g_successIcon;
    case Veritas::NoResult:
        return g_notRunIcon;
    case Veritas::RunError:
    case Veritas::RunFatal:
    case Veritas::RunException:
        return g_failIcon;
    default:
        Q_ASSERT(0);
    }
    return QVariant();
}

QAbstractItemModel* Utils::modelFromProxy(QAbstractItemModel* model)
{
    QSortFilterProxyModel* proxyModel;
    proxyModel = static_cast<QSortFilterProxyModel*>(model);
    if (proxyModel) {
        return proxyModel->sourceModel();
    } else {
        return 0;
    }
}

QModelIndex Utils::modelIndexFromProxy(QAbstractItemModel* model,
                                       const QModelIndex& index)
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    QSortFilterProxyModel* proxyModel;
    proxyModel = static_cast<QSortFilterProxyModel*>(model);
    return proxyModel->mapToSource(index);
}

QModelIndex Utils::proxyIndexFromModel(QAbstractItemModel* model,
                                       const QModelIndex& index)
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    QSortFilterProxyModel* proxyModel;
    proxyModel = static_cast<QSortFilterProxyModel*>(model);
    return proxyModel->mapFromSource(index);
}

} // namespace
