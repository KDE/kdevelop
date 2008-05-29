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
 * \file  cppunititem.cpp
 *
 * \brief Implements class CppUnitItem.
 */

#include "cppunititem.h"

#include <cppunit/TestResult.h>
#include <cppunit/TestFailure.h>
#include <cppunit/Exception.h>

namespace QxCppUnit
{

CppUnitItem::CppUnitItem(const QList<QVariant>& data, RunnerItem* parent,
                         CPPUNIT_NS::Test* test)
        : RunnerItem(data, parent), m_test(test)
{

}

CppUnitItem::~CppUnitItem()
{

}

int CppUnitItem::run()
{
    if (child(0))
        return QxRunner::NoResult; // Have nothing to do as a parent

    // Expect the best.
    setResult(QxRunner::RunSuccess);

    // Prepare for receiving failure notifications.
    CPPUNIT_NS::TestResult testResult;
    testResult.addListener(this);

    // Run the test
    m_test->run(&testResult);

    return result();
}

void CppUnitItem::addFailure(const CPPUNIT_NS::TestFailure& failure)
{
    CPPUNIT_NS::Exception* e = failure.thrownException();

    QVariant msg = QString(e->what()).trimmed();

    QVariant fileName;
    QVariant lineNumber;

    CPPUNIT_NS::SourceLine line = failure.sourceLine();

    if (line.isValid()) {
        fileName = QString(line.fileName().c_str()).trimmed();
        lineNumber = QString().setNum(line.lineNumber());
    }

    setData(2, msg);
    setData(3, fileName);
    setData(4, lineNumber);

    if (failure.isError()) {
        setResult(QxRunner::RunError);
    } else {
        setResult(QxRunner::RunWarning);
        setData(1, "Failure");    // Use CppUnit terminology
    }
}

} // namespace
