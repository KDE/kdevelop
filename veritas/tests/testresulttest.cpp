/* This file is part of KDevelop
 *
 * Copyright 2009 Manuel Breugelmans <mbr.nxi@gmail.com>
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


#include "testresulttest.h"
#include "../testresult.h"
#include "../test.h"

#include <QtTest/QTest>
#include "testutils.h"

using Veritas::Test;
using Veritas::TestResultTest;
using Veritas::TestResult;

///////////// FIXTURE METHODS ////////////////////////////////////////////////

void TestResultTest::init()
{
    m_testResult = new TestResult();
}

void TestResultTest::cleanup()
{
    delete m_testResult;
}

////////////// TEST COMMANDS /////////////////////////////////////////////////

void TestResultTest::defaultResultShouldBeEmpty()
{
    // verify
    KOMPARE(Veritas::NoResult, m_testResult->state());
    KOMPARE(QString(""), m_testResult->message());
    KOMPARE(0, m_testResult->line());
    KOMPARE(KUrl(), m_testResult->file());
    KOMPARE(0, m_testResult->owner());
}

void TestResultTest::resettedResultShouldBeEmpty()
{
    // setup
    Test* t = new Test("foo", 0);
    m_testResult->setLine( 10 );
    m_testResult->setMessage( "foo" );
    m_testResult->setFile(KUrl("/foo/bar"));
    m_testResult->setOwner(t);
    m_testResult->setState( Veritas::RunInfo );

    // exercise
    m_testResult->clear();

    // verify
    KOMPARE(Veritas::NoResult, m_testResult->state());
    KOMPARE(QString(""), m_testResult->message());
    KOMPARE(0, m_testResult->line());
    KOMPARE(KUrl(), m_testResult->file());
    KOMPARE(0, m_testResult->owner());

    // teardown
    delete t;
}

void TestResultTest::setOwnerShouldBeRecursive()
{
    // setup
    TestResult* childResult = new TestResult;
    Test* owner = new Test("foo", 0);
 
    // exercise
    m_testResult->setOwner( owner );
    m_testResult->appendChild( childResult );

    // verify
    KOMPARE(owner, m_testResult->owner());
    KOMPARE(owner, childResult->owner());

    // teardown
    delete owner;
}

QTEST_MAIN( TestResultTest )
#include "testresulttest.moc"
