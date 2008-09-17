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

using QTest::QTestCase;
using QTest::QTestOutputParser;
using Veritas::Test;
using Veritas::TestResult;

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


const QString QTestOutputParser::c_testfunction("TestFunction");
const QString QTestOutputParser::c_description("Description");
const QString QTestOutputParser::c_incident("Incident");
const QString QTestOutputParser::c_type("type");
const QString QTestOutputParser::c_file("file");
const QString QTestOutputParser::c_line("line");
const QString QTestOutputParser::c_pass("pass");
const QString QTestOutputParser::c_skip("skip");
const QString QTestOutputParser::c_message("Message");
const QString QTestOutputParser::c_fail("fail");
const QString QTestOutputParser::c_initTestCase("initTestCase");
const QString QTestOutputParser::c_cleanupTestCase("cleanupTestCase");

namespace
{
int g_result_constructed = 0;
int g_result_assigned = 0;
int g_result_destructed = 0;
}

bool QTestOutputParser::fto_hasResultMemoryLeaks(int& amountLost)
{
    Q_ASSERT(g_result_constructed >= (g_result_assigned + g_result_destructed));
    amountLost = g_result_constructed - (g_result_assigned + g_result_destructed);
    return amountLost != 0;
}

void QTestOutputParser::fto_resetResultMemoryLeakStats()
{
    g_result_constructed = g_result_assigned = g_result_destructed = 0;
}

void QTestOutputParser::deleteResult()
{
    if (m_result) {
        delete m_result;
        m_result = 0;
        g_result_destructed++;
    }
}

void QTestOutputParser::newResult()
{
    if (!m_result) {
        m_result = new TestResult;
        g_result_constructed++;
    }
}

void QTestOutputParser::setResult(Test* test)
{
    Q_ASSERT(m_result);
    test->setResult(m_result);
    m_result = 0;
    g_result_assigned++;
}

QTestOutputParser::QTestOutputParser()
    : m_state(Main),
      m_buzzy(false),
      m_result(0),
      m_block(false)
{}

QTestOutputParser::~QTestOutputParser()
{
    deleteResult();
}

void QTestOutputParser::reset()
{
    m_case = 0;
    deleteResult();
    m_result = 0;
    m_buzzy = false;
    m_state = Main;
    m_block = false;
}

bool QTestOutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool QTestOutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

void QTestOutputParser::assertDeviceSet()
{
    Q_ASSERT_X(device(), "QTestOutputParser::go()",
               "Illegal usage. Client classes should set a QIODevice*, with setDevice().");
}

void QTestOutputParser::assertCaseSet()
{
    Q_ASSERT_X(m_case, "QTestOutputParser::go()",
               "Illegal usage. TestCase should have been set, with setCase().");
}

void QTestOutputParser::setCase(QTestCase* caze)
{
    Q_ASSERT(caze);
    m_case = caze;
}

void QTestOutputParser::block()
{
    m_block = true;
}

void QTestOutputParser::go()
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
        case Message: {
            processMessage();
            goto TestFunctionLabel;
        } case Failure: {
            setFailure();
        } case TestFunction: {
            TestFunctionLabel:
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

bool QTestOutputParser::doingOK() const
{
    return (!m_result || m_result->state() == Veritas::NoResult 
                      || m_result->state() == Veritas::RunSuccess);
}

bool QTestOutputParser::fixtureFailed(const QString& cmd)
{
    if (cmd != c_initTestCase && cmd != c_cleanupTestCase) {
        return false;
    } else {
        return !doingOK();
    }
}

void QTestOutputParser::iterateTestFunctions()
{
    while (!atEnd()) {                 // main loop
        readNext();
        if (isStartElement_(c_testfunction)) {
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
    //kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void QTestOutputParser::processMessage()
{
    QString type = attributes().value(c_type).toString();
    if (type == c_skip) {
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        while (!atEnd() && !isEndElement_(c_description)) {
            readNext();
            if (isCDATA()) {
                m_result->setMessage(text().toString() + " (skipped)");
            }
        }
    } else {
        m_state = TestFunction;
    }

    if (isEndElement_(c_description)) {
        m_state = TestFunction;
    }

}

void QTestOutputParser::processTestFunction()
{
    while (!atEnd() && !isEndElement_(c_testfunction)) {
        readNext();
        if (isStartElement_(c_message)) {
            m_state = Message;
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

void QTestOutputParser::fillResult()
{
    QString type = attributes().value(c_type).toString();
    if (type == c_pass) {
        setSuccess();
    } else if (type == c_fail) {
        m_result->setState(Veritas::RunError);
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        m_state = Failure;
        setFailure();
    }
}

void QTestOutputParser::setSuccess()
{
    m_result->setState(Veritas::RunSuccess);
}

void QTestOutputParser::setFailure()
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
