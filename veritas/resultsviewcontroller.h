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
 * \file  resultsviewcontroller.h
 *
 * \brief Declares class ResultsViewController.
 */

#ifndef VERITAS_RESULTSVIEWCONTROLLER_H
#define VERITAS_RESULTSVIEWCONTROLLER_H

#include "viewcontrollercommon.h"

#include <QObject>

namespace Veritas
{

class ResultsModel;
class ResultsProxyModel;

/*!
 * \brief The ResultsViewController class provides functionality for
 *        the results view.
 *
 * This helper class is introduced to avoid bloating the main window
 * class with code specific for views representing data from ResultsModel.
 *
 * Sorting of data isn't available as long as there is no data to
 * represent. When the model contains data then clicking on a column
 * header sorts the data according to that column.
 *
 * \sa \ref views
 */
class ResultsViewController : public QObject,
                              public ViewControllerCommon
{
    Q_OBJECT

public: // Operations

    /*!
     * Constructs a results view controller with the given \a parent
     * and \a view.
     */
    ResultsViewController(QObject* parent, QTreeView* view);

    /*!
     * Destroys this results view controller.
     */
    ~ResultsViewController();

    /*!
     * Enables sorting if \a enable is true, otherwise disables
     * sorting.
     */
    void enableSorting(bool enable) const;

    /*!
     * Returns the model that contains the data that is available
     * through the proxy model.
     */
    ResultsModel* resultsModel() const;

    /*!
     * Returns the proxy model that the view is presenting.
     */
    ResultsProxyModel* resultsProxyModel() const;

private slots:

    /*!
     * Enables sortign when the model has data, otherwise disables
     * sorting.
     */
    void setupSorting() const;

private: // Operations

    // Copy and assignment not supported.
    ResultsViewController(const ResultsViewController&);
    ResultsViewController& operator=(const ResultsViewController&);
};

} // namespace

#endif // VERITAS_RESULTSVIEWCONTROLLER_H
