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

#include "checkoutputparser.h"
#include "testresult.h"
#include "testcase.h"
#include "testsuite.h"
#include "testbase.h"

#include <qxrunner/runneritem.h>
#include <QStringRef>
#include <kdebug.h>

using QxCheck::TestResult;
using QxCheck::TestCase;
using QxCheck::TestBase;
using QxCheck::TestSuite;
using QxCheck::CheckOutputParser;

/*example xml:
<?xml version="1.0"?>
<testsuites xmlns="http://check.sourceforge.net/ns">
  <datetime>2008-06-20 13:59:25</datetime>
  <suite>
    <title>Money</title>
    <test result="success">
      <path>.</path>
      <fn>check_money.c:28</fn>
      <id>test_money_create</id>
      <iteration>0</iteration>
      <description>Core</description>
      <message>Passed</message>
    </test>
    <test result="error">
      <path>.</path>
      <fn>check_money.c:37</fn>
      <id>test_money_create_neg</id>
      <iteration>0</iteration>
      <description>Limits</description>
      <message>Test timeout expired</message>
    </test>
    <test result="success">
      <path>.</path>
      <fn>check_money.c:46</fn>
      <id>test_money_create_zero</id>
      <iteration>0</iteration>
      <description>Limits</description>
      <message>Passed</message>
    </test>
  </suite>
  <duration>0.000000</duration>
</testsuites>
*/

const QString CheckOutputParser::c_suite("suite");
const QString CheckOutputParser::c_test("test");
const QString CheckOutputParser::c_title("title");
const QString CheckOutputParser::c_path("path");
const QString CheckOutputParser::c_file("fn");
const QString CheckOutputParser::c_id("id");
const QString CheckOutputParser::c_description("description");
const QString CheckOutputParser::c_message("message");
const QString CheckOutputParser::c_result("result");
const QString CheckOutputParser::c_error("error");
const QString CheckOutputParser::c_failure("failure");
const QString CheckOutputParser::c_success("success");

CheckOutputParser::CheckOutputParser(QIODevice* device)
        : QXmlStreamReader(device)
{}

CheckOutputParser::~CheckOutputParser()
{}

bool CheckOutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool CheckOutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

void CheckOutputParser::go(TestSuite* suite)
{
    m_suite = suite;
    if (!device()->isOpen())
        device()->open(QIODevice::ReadOnly);
    if (!device()->isReadable()) {
        // do something
    }
    while (!atEnd() && m_result.isGood()) {
        readNext();
        if (isStartElement_(c_suite)) {
            processSuite();
        }
    }
    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void CheckOutputParser::processSuite()
{
    m_currentSuite = fetchName();
    while (!atEnd() && !isEndElement_(c_suite)) {
        readNext();
        if (isStartElement_(c_test))
            processTest();
    }
}

void CheckOutputParser::processTest()
{
    QString caze, cmd, result, file, msg;
    bool ok = readTestElement(caze, cmd, result, file, msg);
    if (ok) postResult(caze, cmd, result, file, msg);
}

void CheckOutputParser::postResult(const QString& caze, const QString& cmd, const QString& result,
                                   const QString& file, const QString& msg)
{
    if (result == c_success) {
        setSuccess();
    } else if (result == c_error || result == c_failure) {
        setFailure(file, msg);
    } else {
        kDebug() << "unkwown result: " << result << " for " << caze << ":" << cmd;
    }
    m_case = m_suite->findTestNamed(caze);
    Q_ASSERT(m_case);
    m_cmd = m_case->findTestNamed(cmd);
    Q_ASSERT(m_cmd);
    m_cmd->setResult_(m_result);
    m_suite->signalStarted(m_cmd->index());
    m_suite->signalCompleted(m_cmd->index());
    m_result = TestResult();
}

/*
<test result="success">
    <path>.</path>
    <fn>check_money.c:28</fn>
    <id>test_money_create</id>
    <iteration>0</iteration>
    <description>Core</description>
    <message>Passed</message>
</test>
*/
bool CheckOutputParser::readTestElement(QString& caze, QString& cmd, QString& result, 
                                        QString& file, QString& msg)
{
    int count = 0;
    QString path;
    result = attributes().value("result").toString();
    while (!atEnd() && !isEndElement_(c_test)) {
        readNext();
        if (!isStartElement()) {
            continue;
        } else if (name() == c_path) {
            path = readElementText();
        } else if(name() == c_file) {
            file = readElementText();
        } else if (name() == c_description) {
            caze = readElementText();
        } else if (name() == c_message) {
            msg = readElementText();
        } else if (name() == c_id) {
            cmd = readElementText();
        }
        count++;
    }
    if (isEndElement_(c_test)) {
        Q_ASSERT_X(count == 6, "readTestElement", QString("Execpted 5 elements, got " + QString::number(count)).toAscii());
        return true;
    } else {
        return false;
    }
}

QString CheckOutputParser::fetchName()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement_(c_title)) {
            return readElementText();
        }
    }
    Q_ASSERT(0);
    return "??????";
}

void CheckOutputParser::setSuccess()
{
    m_result.setState(QxRunner::RunSuccess);
}

void CheckOutputParser::setFailure(const QString& location, const QString& msg)
{
    kDebug() << location;
    m_result.setState(QxRunner::RunError);
    QStringList spl = location.split(':');
    m_result.setFile(QFileInfo(spl.value(0)));
    m_result.setLine(spl.value(1).toInt());
    m_result.setMessage(msg);
}
