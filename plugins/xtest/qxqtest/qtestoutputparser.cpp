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
#include "qtestresult.h"

#include <qxrunner/runneritem.h>
#include <QStringRef>
#include <kdebug.h>

using QxQTest::QTestResult;
using QxQTest::QTestItem;
using QxQTest::QTestOutputParser;
using QxRunner::RunnerItem;

/*example xml:
     "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
     "<TestCase name=\"RunnerItemTest\">"
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
const QString QTestOutputParser::c_fail("fail");

QTestOutputParser::QTestOutputParser(QIODevice* device)
        : QXmlStreamReader(device), m_processingTestFunction(false),
        m_fillingResult(false), m_settingFailure(false)
{
}

QTestOutputParser::~QTestOutputParser()
{
}

bool QTestOutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool QTestOutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

QTestResult QTestOutputParser::go()
{
    if (!device()->isOpen())
        device()->open(QIODevice::ReadOnly);
    if (!device()->isReadable()) {
        // do something
    }

    while (!atEnd() && m_result.isGood()) {
        readNext();
        if (isStartElement_(c_testfunction))
            processTestFunction();
    }

    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
    return m_result;
}

void QTestOutputParser::goAsync(QTestItem* caze)
{
    if (!device()->isOpen())
        device()->open(QIODevice::ReadOnly);
    if (!device()->isReadable()) {
        // do something
    }

    if (m_settingFailure)
        setFailure();
    if (m_processingTestFunction)
        processTestFunction();

    while (!atEnd() && m_result.isGood()) {
        readNext();
        QString funcName;
        // TODO clean this mess
        if (isStartElement_(c_testfunction)) {
            funcName = attributes().value("name").toString();
            kDebug() << "funcname: " << funcName;
            for (int i = 0; i < caze->childCount(); i++) {
                RunnerItem* qcmd = caze->child(i);
                if (funcName == qcmd->data(0)) {
                    caze->signalStarted(qcmd->index());
                }
            }
            processTestFunction();
        }
        if (isEndElement_(c_testfunction)) {
            for (int i = 0; i < caze->childCount(); i++) {
                RunnerItem* qcmd = caze->child(i);
                kDebug() << qcmd->data(0) << " " << qcmd->data(1);
                if (funcName == qcmd->data(0)) {
                    // found the correct testcase
                    qcmd->setData(2, m_result.message());
                    qcmd->setData(3, m_result.file().filePath());
                    qcmd->setData(4, m_result.line());
                    qcmd->setResult(m_result.state());
                    caze->signalCompleted(qcmd->index());
                }
            }
        }
    }

    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void QTestOutputParser::processTestFunction()
{
    m_processingTestFunction = true;
    while (!atEnd() && !isEndElement_(c_testfunction)) {
        readNext();
        if (isStartElement_(c_incident))
            fillResult();
    }
    if (isEndElement_(c_testfunction))
        m_processingTestFunction = false;
}

void QTestOutputParser::fillResult()
{
    QString type = attributes().value(c_type).toString();
    if (type == c_pass)
        setSuccess();
    else if (type == c_fail)
        setFailure();
}

void QTestOutputParser::setSuccess()
{
    m_result.setState(QxRunner::RunSuccess);
}

void QTestOutputParser::setFailure()
{
    if (!m_settingFailure) {
        m_result.setState(QxRunner::RunError);
        m_result.setFile(QFileInfo(attributes().value(c_file).toString()));
        m_result.setLine(attributes().value(c_line).toString().toInt());
    }
    m_settingFailure = true;

    while (!atEnd() && !isEndElement_(c_description)) {
        readNext();
        if (isCDATA())
            m_result.setMessage(text().toString());
    }

    if (isEndElement_(c_description))
        m_settingFailure = false;
}
