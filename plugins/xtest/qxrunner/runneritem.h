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
 * \file  runneritem.h
 *
 * \brief Declares class RunnerItem.
 */

#ifndef RUNNERITEM_H
#define RUNNERITEM_H

#include "qxrunner_global.h"
#include "qxrunnerexport.h"

#include <QVariant>

namespace QxRunner
{

/*!
 * \brief The RunnerItem class represents an executable item in a tree
 *        view and contains several columns of data.
 *
 * This class holds information about its position in the RunnerModel
 * tree structure, column data and the code that gets executed in a
 * QxRunner run.
 *
 * The class is a basic C++ class. It is not intended to be used directly,
 * but must be subclassed. It does not inherit from QObject or provide
 * signals and slots. Subclasses nevertheless can be a QObject, for
 * example to support localization with \c tr().
 *
 * Subclasses must reimplement the abstract run() method to do useful
 * application specific work.
 *
 * \sa \ref runner_model_item and \ref runner_item_index
 */
class QXRUNNER_EXPORT RunnerItem
{
public: // Operations

    /*!
     * Constructs a runner item with the given \a parent and the
     * \a data associated with each column. Ensures that number of
     * columns equals number of columns in \a parent. Initial result
     * is set to QxRunner::NoResult.
     */
    explicit RunnerItem(const QList<QVariant>& data, RunnerItem* parent = 0);

    /*!
     * Destroys this runner item and all its children.
     */
    virtual ~RunnerItem();

    /*!
     * Returns the item's parent.
     */
    RunnerItem* parent() const;

    /*!
     * Returns the child that corresponds to the specified \a row
     * number in the item's list of child items.
     */
    RunnerItem* child(int row) const;

    /*!
     * Adds \a child to the item's list of child items.
     */
    void appendChild(RunnerItem* child);

    /*!
     * Returns the number of child items held.
     */
    int childCount() const;

    /*!
     * Reports the item's location within its parent's list of items.
     */
    int row() const;

    /*!
     * Returns the number of columns of data in the item.
     */
    int columnCount() const;

    /*!
     * Returns the data of \a column.
     */
    QVariant data(int column) const;

    /*!
     * Sets the data for the \a column to \a value.
     */
    void setData(int column, const QVariant& value);

    /*!
     * Returns true if the item is selected for execution, otherwise
     * false.
     */
    bool isSelected() const;

    /*!
     * If \a select is true the item can be executed by QxRunner,
     * otherwise not.
     */
    void setSelected(bool select);

    /*!
     * Returns the result of the last execution. The returned value
     * is of type QxRunner::RunnerResult.
     */
    int result() const;

    /*!
     * Sets the \a result for this item. The given value must be of
     * type QxRunner::RunnerResult.
     */
    void setResult(int result);

    /*!
     * Except for column 0 the data is set to an empty string. The
     * item's result is set to QxRunner::NoResult.
     */
    void clear();

    /*!
     * The custom code to be executed when an item is run by QxRunner
     * must be placed in the run method of subclasses.
     */
    virtual int run() = 0;

private: // Operations

    // Copy and assignment not supported.
    RunnerItem(const RunnerItem&);
    RunnerItem& operator=(const RunnerItem&);

private: // Attributes

    RunnerItem*        m_parentItem;
    QList<QVariant>    m_itemData;
    QList<RunnerItem*> m_childItems;

    bool m_selected;
    int  m_result;
};

} // namespace

#endif // RUNNERITEM_H
