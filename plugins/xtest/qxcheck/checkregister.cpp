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

#include "checkregister.h"
#include "testsuite.h"
#include "testcase.h"
#include "testcommand.h"

#include <QIODevice>
#include <klocalizedstring.h>
#include <kdebug.h>

using QxCheck::CheckRegister;
using QxCheck::TestSuite;
using QxCheck::TestCase;
using QxCheck::TestBase;
using QxCheck::TestCommand;
using QxRunner::RunnerItem;

/* example xml
<?xml version="1.0"?>
        <testsuites>
        <suite name="Money">
        <case name="Core">
        <command name="test_money_create"/>
        </case>
        <case name="Limits">
        <command name="test_money_create_neg"/>
        <command name="test_money_create_zero"/>
        </case>
        </suite>
        </testsuites> */


const QString CheckRegister::c_suite("suite");
const QString CheckRegister::c_case("case");
const QString CheckRegister::c_cmd("command");
const QString CheckRegister::c_name("name");

CheckRegister::CheckRegister()
        : m_root("")
{
        // Data for column headers is stored in the root item.
    QList<QVariant> rootData;
    rootData << i18n("Test Name") << i18n("Result") << i18n("Message")
             << i18n("File Name") << i18n("Line Number");
    m_rootItem = new TestBase(rootData);
}

CheckRegister::~CheckRegister()
{
}

TestBase* CheckRegister::rootItem()
{
    return m_rootItem;
}

bool CheckRegister::isStartElement_(const QString& elem)
{
    return isStartElement() && (name() == elem);
}

bool CheckRegister::isEndElement_(const QString& elem)
{
    return isEndElement() && (name() == elem);
}

void CheckRegister::addFromXml(QIODevice* dev)
{
    Q_ASSERT(dev != 0);
    setDevice(dev);
    if (!device()->isOpen()) 
        device()->open(QIODevice::ReadOnly);

    while (!atEnd())
    {
        readNext();
        if (isStartElement_(c_suite))
            processSuite();
    }

    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void CheckRegister::processSuite()
{
    TestSuite* suite = new TestSuite(fetchName(), m_exe, m_rootItem);
    m_suites.push_back(suite);
    m_rootItem->appendChild(suite);
    kDebug() << suite->name();

    while (!atEnd() && !isEndElement_(c_suite))
    {
        readNext();
        if (isStartElement_(c_case))
            processCase(suite);
    }
}

void CheckRegister::processCase(TestSuite* suite)
{
    TestCase* caze = new TestCase(fetchName(), suite);
    suite->addTest(caze);
    kDebug() << caze->name();
    while (!atEnd() && !isEndElement_(c_case))
    {
        readNext();
        if (isStartElement_(c_cmd))
            processCmd(caze);
    }
}

void CheckRegister::processCmd(TestCase* caze)
{
    TestCommand* cmd = new TestCommand(fetchName(), caze);
    caze->addTest(cmd);
    kDebug() << cmd->name();
}

QString CheckRegister::fetchName()
{
    return attributes().value(c_name).toString();
}

unsigned CheckRegister::testSuiteCount()
{
    return m_suites.size();
}

TestSuite* CheckRegister::takeSuite(unsigned i)
{
    return m_suites.value(i);
}
