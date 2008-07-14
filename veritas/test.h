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

#ifndef VERITAS_TEST_H
#define VERITAS_TEST_H

#include "veritas/testresult.h"
#include "veritas/veritasexport.h"

#include "veritas/interfaces/itest.h"

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

class QAbstractItemModel;

namespace Veritas
{

/*!
 * \brief The Test class represents an executable item
 *        it is also the primary model's underlying data 
 *        item.
 *
 * This class holds information about its position in the RunnerModel
 * tree structure, column data and the code that gets executed in a
 * Veritas run.
 *
 * It is not intended to be used directly, but must be subclassed.
 *
 * Subclasses must reimplement the abstract run() method to do useful
 * application specific work.
 *
 * Implements the ITest interface. For guidance on implementing  subclasses
 * refer to veritas/interfaces/itest.h
 *
 */
// TODO d-pointer
class VERITAS_EXPORT Test : public ITest
{
Q_OBJECT
public: // Operations

    /*!
     * Constructs a test item with the given \a parent and the
     * \a data associated with each column. Ensures that number of
     * columns equals number of columns in \a parent. Initial result
     * is set to Veritas::NoResult.
     *
     * TODO this constructor is deprecated and should be removed
     */
    explicit Test(const QList<QVariant>& data, Test* parent = 0);

    explicit Test(const QString& name, Test* parent = 0);
    virtual ~Test();

    /*!
     * Default to doing nadda, implement this in a subclass.
     */
    int run();
    /*!
     * Do not run by default. Implement this in a subclass.
     */
    bool shouldRun() const;
    /*!
     * Identifies this test. Also shown in the runnerview-tree
     */
    QString name() const;

    Test* parent() const;
    void addChild(ITest* child);
    Test* child(int row) const;
    Test* childNamed(const QString& name) const;
    int childCount() const;
    int row() const;

    /*!
     * Is this item checked by the user in the tree-runnerview?
     */
    bool selected() const;
    void setSelected(bool select);

    int columnCount() const;

    QVariant data(int column) const; // TODO might want to get rid of this
    void setData(int column, const QVariant& value);

    TestState state() const;
    void setState(TestState result);
    void setResult(TestResult* res);
    TestResult* result() const;
    void clear();

    void setIndex(const QModelIndex& index); // TODO this should be removed
    QModelIndex index() const;
    QList<ITest*> leafs() const;

private: // Operations

    // Copy and assignment not supported.
    Test(const Test&);
    Test& operator=(const Test&);

private: // Attributes
    Test* m_parentItem;
    QList<QVariant> m_itemData;
    QList<Test*> m_children;
    QMap<QString, Test*> m_childMap;
    QModelIndex m_index;
    QString m_name;
    TestState m_state;
    TestResult* m_result;
    bool m_selected;
};

} // namespace

#endif // VERITAS_TEST_H
