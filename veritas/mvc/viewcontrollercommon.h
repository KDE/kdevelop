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
 * \file  viewcontrollercommon.h
 *
 * \brief Declares class ViewControllerCommon.
 */

#ifndef VERITAS_VIEWCONTROLLERCOMMON_H
#define VERITAS_VIEWCONTROLLERCOMMON_H

#include <QTreeView>
#include <QHeaderView>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace Veritas
{

/*!
 * \brief The ViewControllerCommon class defines a set of common
 *        methods for view controllers.
 *
 * This class should be seen as a 'static decorator' for the view
 * controller classes. It is not intended to be used directly, but
 * must be subclassed.
 *
 * \sa \ref views
 */

class ViewControllerCommon : public QObject
{
Q_OBJECT
public: // Operations

    /*!
     * Constructs a view controller common for the given \a view. Sets
     * view default properties.
     */
    ViewControllerCommon(QObject*, QTreeView* view);

    /*!
     * Destroys this view controller common.
     */
    virtual ~ViewControllerCommon();

    /*!
     * Returns the index of column 0 of the highlighted row. The index
     * isn't necessarily the current view index. If no row is highlighted
     * the returned index is invalid.
     */
    QModelIndex highlightedRow() const;

    /*!
     * Highlights the row of the \a index independent of the selection
     * mode and ensures that it is visible. The given index becomes the
     * new current index.
     */
    void setHighlightedRow(const QModelIndex& index) const;

public slots:
    void expandOrCollapse(const QModelIndex& index) const;

protected: // Operations

    /*!
     * Returns the view widget.
     */
    QTreeView* view() const;

    /*!
     * Returns the header of the view widget.
     */
    QHeaderView* header() const;

    /*!
     * Returns the model that contains the data that is available
     * through the proxy model.
     */
    QAbstractItemModel* model() const;

    /*!
     * Returns the proxy model that the view is presenting.
     */
    QSortFilterProxyModel* proxyModel() const;

private: // Operations

    // Copy and assignment not supported.
    ViewControllerCommon(const ViewControllerCommon&);
    ViewControllerCommon& operator=(const ViewControllerCommon&);

private: // Attributes

    QTreeView* m_view;
};

} // namespace

#endif // VERITAS_VIEWCONTROLLERCOMMON_H
