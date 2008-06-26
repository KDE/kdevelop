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

#include "outputparser.h"
#include "testsuite.h"
#include <test.h>
#include <testcase.h>
#include <QStringRef>
#include <KDebug>

using Veritas::Test;
using Veritas::TestResult;
using Veritas::TestCase;
using CppUnit::TestSuite;
using CppUnit::OutputParser;

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

const QString OutputParser::c_suite("suite");
const QString OutputParser::c_case("case");
const QString OutputParser::c_command("command");
const QString OutputParser::c_name("name");
const QString OutputParser::c_status("status");
const QString OutputParser::c_type("type");
const QString OutputParser::c_assertion("assertion");
const QString OutputParser::c_file("file");
const QString OutputParser::c_line("line");

OutputParser::OutputParser(QIODevice* device)
    : QXmlStreamReader(device),
      m_processingSuite(false),
      m_processingCase(false),
      m_processingCmd(false)
{}

OutputParser::~OutputParser()
{}

bool OutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool OutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

void OutputParser::go(TestSuite* suite)
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

    while (!atEnd()) {
        readNext();
        if (isStartElement_(c_suite)) {
            processSuite();
        }
    }

    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void OutputParser::processSuite()
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

void OutputParser::processCase()
{
    if (!m_processingCase) {
        m_currentCase = fetchName();
        m_case = m_suite->childNamed(m_currentCase);
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

QString OutputParser::fetchName()
{
    return attributes().value(c_name).toString();
}

void OutputParser::processCmd()
{
    if (!m_processingCmd) {
        m_currentCmd = fetchName();
        m_cmd = m_case->childNamed(m_currentCmd);
        Q_ASSERT(m_cmd);
        m_cmd->started();
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
        m_cmd->setResult(m_result);
        m_cmd->finished();
        m_result.clear();
    }
}

void OutputParser::setSuccess()
{
    m_result.setState(Veritas::RunSuccess);
}

void OutputParser::setFailure()
{
    m_result.setState(Veritas::RunError);
    m_result.setFile(QFileInfo(attributes().value(c_file).toString()));
    m_result.setLine(attributes().value(c_line).toString().toInt());
}
