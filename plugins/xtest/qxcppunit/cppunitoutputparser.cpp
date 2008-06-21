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

#include "cppunitoutputparser.h"
#include "testresult.h"
#include "testcase.h"
#include "testsuite.h"
#include "testbase.h"

#include <qxrunner/runneritem.h>
#include <QStringRef>
#include <kdebug.h>

using QxCppUnit::TestResult;
using QxCppUnit::TestCase;
using QxCppUnit::TestBase;
using QxCppUnit::TestSuite;
using QxCppUnit::CppUnitOutputParser;

/*example xml:
<?xml version="1.0" encoding="ISO-8859-1"?>
<root>
<suite name="RootSuite">
    <case name="FooTest">
        <command name="testCmd1"></command>
        <command name="testCmd2"></command>
    </case>
    <case name="BarTest">
        <command name="testCmd1"></command>
        <command name="testCmd2">
            <status type="assertion" file="/home/nix/KdeDev/kdevelop/plugins/xtest/qxcppunit/tests/fake_sunnysuite.cpp" line="63"/>
        </command>
    </case>
    <case name="BazTest">
        <command name="testCmd1"></command>
    </case>
</suite>
</root>
*/

const QString CppUnitOutputParser::c_suite("suite");
const QString CppUnitOutputParser::c_case("case");
const QString CppUnitOutputParser::c_command("command");
const QString CppUnitOutputParser::c_name("name");
const QString CppUnitOutputParser::c_status("status");
const QString CppUnitOutputParser::c_type("type");
const QString CppUnitOutputParser::c_assertion("assertion");
const QString CppUnitOutputParser::c_file("file");
const QString CppUnitOutputParser::c_line("line");

CppUnitOutputParser::CppUnitOutputParser(QIODevice* device)
    : QXmlStreamReader(device),
      m_processingSuite(false),
      m_processingCase(false),
      m_processingCmd(false)
{}

CppUnitOutputParser::~CppUnitOutputParser()
{}

bool CppUnitOutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool CppUnitOutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

void CppUnitOutputParser::go(TestSuite* suite)
{
    m_suite = suite;
    if (!device()->isOpen())
        device()->open(QIODevice::ReadOnly);
    if (!device()->isReadable()) {
        // do something
    }

    if (m_processingCmd)
        processCmd();
    if (m_processingCase)
        processCase();
    if (m_processingSuite)
        processSuite();

    while (!atEnd() && m_result.isGood()) {
        readNext();
        if (isStartElement_(c_suite)) {
            processSuite();
        }
    }

    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void CppUnitOutputParser::processSuite()
{
    m_processingSuite = true;
    m_currentSuite = fetchName();
    while (!atEnd() && !isEndElement_(c_suite)) {
        readNext();
        if (isStartElement_(c_case))
            processCase();
    }
    if (isEndElement_(c_suite))
        m_processingSuite = false;
}

void CppUnitOutputParser::processCase()
{
    if (!m_processingCase) {
        m_currentCase = fetchName();
        m_case = m_suite->findTestNamed(m_currentCase);
    }
    Q_ASSERT(m_case);
    m_processingCase = true;
    while (!atEnd() && !isEndElement_(c_case)) {
        readNext();
        if (isStartElement_(c_command))
            processCmd();
    }
    if (isEndElement_(c_case))
        m_processingCase = false;
}

QString CppUnitOutputParser::fetchName()
{
    return attributes().value(c_name).toString();
}

void CppUnitOutputParser::processCmd()
{
    if (!m_processingCmd) {
        m_currentCmd = fetchName();
        m_cmd = m_case->findTestNamed(m_currentCmd);
        Q_ASSERT(m_cmd);
        m_suite->signalStarted(m_cmd->index());
    }
    Q_ASSERT(m_cmd);
    m_processingCmd = true;
    bool gotFailureMsg = false;
    while (!atEnd() && !isEndElement_(c_command)) {
        readNext();
        if (isStartElement_(c_status)) {
            gotFailureMsg = true;
            setFailure();
        }
    }
    if (isEndElement_(c_command)) {
        m_processingCmd = false;
        if (!gotFailureMsg)
            setSuccess();
        m_cmd->setResult_(m_result);
        m_suite->signalCompleted(m_cmd->index());
        m_result = TestResult();
    }
}

void CppUnitOutputParser::setSuccess()
{
    m_result.setState(QxRunner::RunSuccess);
}

void CppUnitOutputParser::setFailure()
{
    m_result.setState(QxRunner::RunError);
    m_result.setFile(QFileInfo(attributes().value(c_file).toString()));
    m_result.setLine(attributes().value(c_line).toString().toInt());
}
