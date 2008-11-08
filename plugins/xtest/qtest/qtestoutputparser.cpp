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

#include "qtestoutputparser.h"
#include "qtestcase.h"

#include <veritas/test.h>

#include <KUrl>
#include <KDebug>

using QTest::Case;
using QTest::OutputParser;
using Veritas::Test;
using Veritas::TestResult;
using Veritas::TestState;

/*example xml:
     "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
     "<TestCase name=\"TestTest\">"
     "<Environment>"
     "<QtVersion>4.4.0-rc1</QtVersion>"
     "<QTestVersion>4.4.0-rc1</QTestVersion>"
     "</Environment>"
     "<TestFunction name=\"initTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"simpleRoot\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"cleanupTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "</TestCase>)"*/


const QString OutputParser::c_testfunction("TestFunction");
const QString OutputParser::c_description("Description");
const QString OutputParser::c_incident("Incident");
const QString OutputParser::c_type("type");
const QString OutputParser::c_file("file");
const QString OutputParser::c_line("line");
const QString OutputParser::c_pass("pass");
const QString OutputParser::c_skip("skip");
const QString OutputParser::c_qfatal("qfatal");
const QString OutputParser::c_message("Message");
const QString OutputParser::c_fail("fail");
const QString OutputParser::c_initTestCase("initTestCase");
const QString OutputParser::c_cleanupTestCase("cleanupTestCase");

namespace
{
int g_result_constructed = 0;
int g_result_assigned = 0;
int g_result_destructed = 0;
}

bool OutputParser::fto_hasResultMemoryLeaks(int& amountLost)
{
    Q_ASSERT(g_result_constructed >= (g_result_assigned + g_result_destructed));
    amountLost = g_result_constructed - (g_result_assigned + g_result_destructed);
    return amountLost != 0;
}

void OutputParser::fto_resetResultMemoryLeakStats()
{
    g_result_constructed = g_result_assigned = g_result_destructed = 0;
}

void OutputParser::deleteResult()
{
    if (m_result) {
        delete m_result;
        m_result = 0;
        g_result_destructed++;
    }
}

void OutputParser::newResult()
{
    if (!m_result) {
        m_result = new TestResult;
        g_result_constructed++;
    }
}

void OutputParser::setResult(Test* test)
{
    Q_ASSERT(m_result);
    if (m_subResults.isEmpty()) {
        test->setResult(m_result);
    } else {
        TestResult* result = new TestResult;
        test->setResult(result);
        TestState state = m_result->state();
        foreach(TestResult* sub, m_subResults) {
            if (sub->state() == Veritas::RunError) {
                state = Veritas::RunError;
            } else if (sub->state() == Veritas::RunFatal) {
                state = Veritas::RunFatal;
                break;
            }
        }
        result->setState(state);
        foreach(TestResult* sub, m_subResults) {
            result->appendChild(sub);
        }
        result->appendChild(m_result);
    }
    m_result = 0;
    m_subResults.clear();
    g_result_assigned++;
}

OutputParser::OutputParser()
    : m_state(Main),
      m_buzzy(false),
      m_result(0),
      m_block(false)
{}

OutputParser::~OutputParser()
{
    deleteResult();
}

void OutputParser::reset()
{
    m_case = 0;
    deleteResult();
    m_result = 0;
    m_buzzy = false;
    m_state = Main;
    m_block = false;
}

bool OutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool OutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

void OutputParser::assertDeviceSet()
{
    Q_ASSERT_X(device(), "OutputParser::go()",
               "Illegal usage. Client classes should set a QIODevice*, with setDevice().");
}

void OutputParser::assertCaseSet()
{
    Q_ASSERT_X(m_case, "OutputParser::go()",
               "Illegal usage. TestCase should have been set, with setCase().");
}

void OutputParser::setCase(Case* caze)
{
    Q_ASSERT(caze);
    m_case = caze;
}

void OutputParser::block()
{
    m_block = true;
}

void OutputParser::go()
{
    if (m_buzzy || m_block) return; // do not disturb.
    m_buzzy = true;

    assertCaseSet();
    assertDeviceSet();

    if (!device()->isOpen()) {
        device()->open(QIODevice::ReadOnly);
    }
    if (!device()->isReadable()) {
        //kWarning() << "Device not readable. Failed to parse test output.";
        m_buzzy = false;
        return;
    }

    // recover from previous state by falling through.
    // this method is triggered repeatedly, since the parser works
    // incrementally by recovering from previous errors. It remembers
    // the state it was in and starts where it left off.
    switch (m_state) {
        case QSkip: {
            processQSkip();
            goto TestFunctionLabel;
        } case QAssert: {
            processQAssert();
            goto TestFunctionLabel;
        } case Failure: {
            setFailure();
        } case TestFunction: TestFunctionLabel: {
            processTestFunction();
        } case Main: {
            iterateTestFunctions();
            break;
        } default: {
            kError() << "Serious corruption, impossible switch value.";
        }
    }
    m_buzzy = false;
}

bool OutputParser::fixtureFailed(const QString& cmd)
{
    if (cmd != c_initTestCase && cmd != c_cleanupTestCase) {
        return false;
    } else {
        return m_result->state() != Veritas::NoResult &&
               m_result->state() != Veritas::RunSuccess;
    }
}

void OutputParser::iterateTestFunctions()
{
    while (!atEnd()) {                 // main loop
        readNext();
        if (isStartElement_(c_testfunction)) {
            if (m_result) m_result->setState(Veritas::NoResult);
            m_cmdName = attributes().value("name").toString();
            kDebug() << m_cmdName;
            m_cmd = m_case->childNamed(m_cmdName);
            newResult();
            if (m_cmd) m_cmd->signalStarted();
            m_state = TestFunction;
            processTestFunction();
            if (m_state != Main) return;
        }
        if (isEndElement_("TestCase")) {
            emit done();
        }
    }
    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void OutputParser::processQSkip()
{
    while (!atEnd() && !isEndElement_(c_description)) {
        readNext();
        if (isCDATA()) {
            m_result->setMessage(text().toString() + " (skipped)");
        }
    }

    if (isEndElement_(c_description)) {
        m_state = TestFunction;
    }
}

void OutputParser::processQAssert()
{
    // Q_ASSERT   "<Description><![CDATA[ASSERT: \"condition\" in file /path/to/file.cpp, line 66]]></Description>\n"
    // Q_ASSERT_X "<Description><![CDATA[ASSERT failure in command: \"message\", file /path/to/file.cpp, line 66]]></Description>\n"

    while (!atEnd() && !isEndElement_(c_description)) {
        readNext();
        if (isCDATA()) {
            QString cdata = text().toString();
            int lineStart = cdata.lastIndexOf(", line ");
            int lineEnd = lineStart+7;
            int fileStart = cdata.lastIndexOf(" in file ");
            int fileEnd = fileStart + 9;
            int file2Start = cdata.lastIndexOf(", file ");
            if (file2Start > fileStart) {
                fileStart = file2Start;
                fileEnd = file2Start + 7;
            }
            m_result->setFile(KUrl(cdata.mid(fileEnd, lineStart - fileEnd)));
            m_result->setLine(cdata.mid(lineEnd).toInt());
            m_result->setMessage(cdata.mid(0, fileStart));
        }
    }

    if (isEndElement_(c_description)) {
        m_state = TestFunction;
    }
}

void OutputParser::processMessage()
{
    QString type = attributes().value(c_type).toString();
    if (type == c_skip) {
        clearResult();
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        m_result->setState(Veritas::RunInfo);
        m_state = QSkip;
        processQSkip();
    } else if (type == c_qfatal) {
        clearResult();
        m_result->setState(Veritas::RunFatal);
        m_state = QAssert;
        processQAssert();
    }
}

void OutputParser::processTestFunction()
{
    while (!atEnd() && !isEndElement_(c_testfunction)) {
        readNext();
        if (isStartElement_(c_message)) {
            processMessage();
        }
        if (isStartElement_(c_incident)) {
            fillResult();
            if (m_state != TestFunction) return;
        }
    }
    if (isEndElement_(c_testfunction)) {
        if (m_cmd) {
            setResult(m_cmd);
            m_cmd->signalFinished();
        } else if (fixtureFailed(m_cmdName)) {
            kDebug() << "init/cleanup TestCase failed";
            m_case->signalStarted();
            setResult(m_case);
            m_case->signalFinished();
        }
        m_state = Main;
    }
}

void OutputParser::clearResult()
{
    if (m_result->state() != Veritas::NoResult) { // parsed a previous result
        m_subResults << m_result;
        m_result = new TestResult;
    }
}

void OutputParser::fillResult()
{
    QString type = attributes().value(c_type).toString();
    if (type == c_pass) {
        setSuccess();
    } else if (type == c_fail) {
        if (m_result->state() == Veritas::RunFatal) return;
        clearResult();
        m_result->setState(Veritas::RunError);
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        m_state = Failure;
        setFailure();
    }
}

void OutputParser::setSuccess()
{
    if (m_result->state() != Veritas::RunInfo) {
        m_result->setState(Veritas::RunSuccess);
    }
}

void OutputParser::setFailure()
{
    while (!atEnd() && !isEndElement_(c_description)) {
        readNext();
        if (isCDATA()) {
            m_result->setMessage(text().toString());
        }
    }

    if (isEndElement_(c_description)) {
        m_state = TestFunction;
    }
}

#include "qtestoutputparser.moc"
