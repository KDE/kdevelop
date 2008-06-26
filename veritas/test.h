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
 * \file  test.h
 *
 * \brief Declares class Test.
 */

#ifndef VERITAS_TEST_H
#define VERITAS_TEST_H

//#include "qxrunner_global.h"
#include "veritasexport.h"
#include <itest.h>
#include <testresult.h>

#include <QMap>
#include <QVariant>

class QAbstractItemModel;

namespace Veritas
{

/*!
 * \brief The Test class represents an executable item in a tree
 *        view and contains several columns of data.
 *
 * This class holds information about its position in the RunnerModel
 * tree structure, column data and the code that gets executed in a
 * Veritas run.
 *
 * The class is a basic C++ class. It is not intended to be used directly,
 * but must be subclassed. It does not inherit from QObject or provide
 * signals and slots. Subclasses nevertheless can be a QObject, for
 * example to support localization with \c tr().
 *
 * Subclasses must reimplement the abstract run() method to do useful
 * application specific work.
 *
 */
class VERITAS_EXPORT Test : public ITest
{
    Q_OBJECT
public: // Operations

    /*!
     * Constructs a runner item with the given \a parent and the
     * \a data associated with each column. Ensures that number of
     * columns equals number of columns in \a parent. Initial result
     * is set to Veritas::NoResult.
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
    QString name() const;

    Test* parent() const;
    void addChild(ITest* child);
    Test* child(int row) const;
    Test* childNamed(const QString& name) const;
    int childCount() const;
    int row() const;

    bool selected() const;
    void setSelected(bool select);

    int columnCount() const;
    QVariant data(int column) const;
    void setData(int column, const QVariant& value);

    TestState state() const;
    void setState(TestState result);
    void setResult(const TestResult& res);
    TestResult result() const;
    void clear();

    void setIndex(const QModelIndex& index);
    QModelIndex index() const;

private: // Operations

    // Copy and assignment not supported.
    Test(const Test&);
    Test& operator=(const Test&);

private: // Attributes
    Test* m_parentItem;
    QList<QVariant> m_itemData;
    QList<Test*> m_childItems;
    QMap<QString, Test*> m_childMap;
    QModelIndex m_index;
    QString m_name;
    TestState m_state;

    bool m_selected;
};

} // namespace

#endif // VERITAS_TEST_H
