/* KDevelop xUnit plugin
 *
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

#ifndef CPPUNIT_CPPUNITVIEWDATA_H
#define CPPUNIT_CPPUNITVIEWDATA_H

#include <veritas/testrunnertoolview.h>

namespace Veritas { class Test; }

/*! Assorted test runner operations and data */
class CppUnitViewData : public Veritas::TestViewData
{
Q_OBJECT
public:
    CppUnitViewData(QObject* parent);
    virtual ~CppUnitViewData();
    void registerTests();
    QString fetchExe();
    virtual QString resultsViewId();

    int m_id;
    static int id; // used to get unique names.
};


#endif
