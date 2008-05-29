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
 * \file  testrunner.cpp
 *
 * \brief Implements class TestRunner.
 */

#include "testrunner.h"
#include "cppunitmodel.h"

#include <qxrunner/runner.h>
#include <qxrunner/runnerwindow.h>
#include <cppunit/TestSuite.h>
#include <QIcon>

// Helper function needed to expand Q_INIT_RESOURCE outside the namespace.
static void initQxCppUnitResource()
{
    Q_INIT_RESOURCE(qxcppunit);
}

using namespace QxRunner;

namespace QxCppUnit
{

TestRunner::TestRunner()
{
    m_runner = 0;
    m_model = new CppUnitModel;
}

TestRunner::~TestRunner()
{
    // Delete the runner first.
    delete m_runner;
    delete m_model;
}

void TestRunner::addTest(CPPUNIT_NS::Test* test) const
{
    m_model->addTest(test);
}

void TestRunner::addTests(const CppUnitVector<CPPUNIT_NS::Test*>& tests) const
{
    CppUnitVector<CPPUNIT_NS::Test*>::const_iterator it = tests.begin();

    for (; it != tests.end(); ++it) {
        addTest(*it);
    }
}

void TestRunner::run()
{
    m_runner = new QxRunner::Runner(m_model);

    // Application icon.
    initQxCppUnitResource();
    m_runner->setWindowIcon(QIcon(":/icons/qxcppunit_16x16.png"));

    m_runner->run();
}

QWidget* TestRunner::spawn()
{
    QxRunner::RunnerWindow* window = new QxRunner::RunnerWindow;
    window->setModel(m_model);
    return window;
}

} // namespace
