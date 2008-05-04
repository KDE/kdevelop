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
 * \file  resultsviewcontroller.cpp
 *
 * \brief Implements class ResultsViewController.
 */

#include "resultsviewcontroller.h"
#include "resultsmodel.h"
#include "resultsproxymodel.h"

namespace QxRunner
{

ResultsViewController::ResultsViewController(QObject* parent, QTreeView* view)
        : QObject(parent), ViewControllerCommon(view)
{
    // Allow clicking in the header, sorting gets enabled only when there are results.
    header()->setClickable(true);
    connect(header(), SIGNAL(sectionClicked(int)), SLOT(setupSorting()));
}

ResultsViewController::~ResultsViewController()
{

}

void ResultsViewController::enableSorting(bool enable) const
{
    if (!enable) {
        header()->setSortIndicatorShown(false);

        // Sorting non-existent column removes any sorting.
        proxyModel()->sort(-1);
        return;
    }

    if (header()->isSortIndicatorShown()) {
        // Sorting columns already enabled.
        return;
    }

    // Initial sorting of current column.
    int section = header()->sortIndicatorSection();
    proxyModel()->sort(section, Qt::DescendingOrder);
    header()->setSortIndicator(section, Qt::DescendingOrder);
    header()->setSortIndicatorShown(true);

    ///
    /// \todo Verify that sort(-1) is the correct way to remove sorting
    ///       from the model.
    ///
}

ResultsModel* ResultsViewController::resultsModel() const
{
    return static_cast<ResultsModel*>(model());
}

ResultsProxyModel* ResultsViewController::resultsProxyModel() const
{
    return static_cast<ResultsProxyModel*>(proxyModel());
}

void ResultsViewController::setupSorting() const
{
    bool enable;

    if (proxyModel()->rowCount() < 1) {
        // No sorting when no results are displayed.
        enable = false;
    } else {
        // Sorting enabled when results are displayed.
        enable = true;
    }

    enableSorting(enable);
}

} // namespace
