/*
 * This file is part of KDevelop
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

#ifndef VERITAS_ITEST_H
#define VERITAS_ITEST_H

#include <QString>
#include <QModelIndex>
#include <testresult.h>
#include <veritasexport.h>

namespace Veritas
{

class VERITAS_EXPORT ITest : public QObject
{
    Q_OBJECT
public:
    ITest();
    virtual ~ITest();

    virtual QString name() const = 0; // must be unique across siblings
    static const int s_columnCount;

    /*!
     * Custom code to be executed when this test is run.
     */
    virtual int run() = 0;

    /*!
     * Notify the toolview that execution commenced
     */
    void started();
    /*!
     * Notify the toolview that execution completed
     */
    void finished();

    virtual QModelIndex index() const = 0;
    virtual void setIndex(const QModelIndex& index) = 0;

    /*!
     * Returns false if this test run method
     * should not be executed. eg because it is
     * a test container (suite).
     */
    virtual bool shouldRun() const = 0;
    //virtual void setShouldRun(bool) const = 0;

    /*!
     * Returns true if this test is checked/selected
     * by the user, and thus should be run.
     */
    virtual bool selected() const = 0;
    virtual void setSelected(bool select) = 0;

    /*!
     * Reset the TestResult
     */
    virtual void clear() = 0;
    virtual void setResult(const TestResult&) = 0;
    virtual Veritas::TestState state() const = 0;
    virtual void setState(TestState) = 0;
    virtual TestResult result() const = 0;

    virtual ITest* parent() const = 0;
    virtual void addChild(ITest*) = 0;
    virtual ITest* child(int) const = 0;
    virtual int childCount() const = 0;
    virtual ITest* childNamed(const QString& name) const = 0;

    virtual QVariant data(int column) const = 0;

    /*!
     * Reports the item's location within its parent's list
     */
    virtual int row() const = 0;

signals:
    void started(QModelIndex);
    void finished(QModelIndex);
};

} // namespace

#endif // VERITAS_ITEST_H
