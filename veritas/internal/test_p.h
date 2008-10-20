/*
* KDevelop xUnit integration
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

#ifndef VERITAS_INTERNAL_TEST_P_H
#define VERITAS_INTERNAL_TEST_P_H

#include "../test.h"
#include "../veritasexport.h"
#include <QModelIndex>

namespace Veritas
{

/*! Veritas::Test's internal interface. Contains operations that should 
 *  only be accessible by classes inside the library. If this were Java, 
 *  they'd be declared package private in Test. */
class VERITAS_EXPORT Test::Internal // the only reason this is exported is to
                                    // be able to access this in tests,
                                    // without double compilation
{
public:
    Internal(Veritas::Test* self);

    /*! Reset this tests result */
    void clear();

    void setIndex(const QModelIndex& index);
    QModelIndex index() const;

    QVariant data(int column) const;
    void setData(int column, const QVariant& value);

    /*! Is this item checked by the user in the tree-runnerview? */
    bool isChecked() const;
    /*! Check this test and all its children */
    void check();
    /*! Recursively lift check state */
    void unCheck();

    bool isRunning() const;
    void setIsRunning(bool);

private:
    friend class Test;

    Veritas::Test* self;
    QString name;
    QModelIndex index_;
    TestResult* result;
    bool isChecked_;
    QMap<QString, Test*> childMap;
    QList<Test*> children;
    QList<QVariant> itemData;
    bool needVerboseToggle;
    bool needSelectionToggle;
    bool m_isRunning;

    static const int columnCount;
};

}

#endif // VERITAS_INTERNAL_TEST_P_H
