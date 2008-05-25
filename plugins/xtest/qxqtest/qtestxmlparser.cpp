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

#include "qtestxmlparser.h"
#include "qtestresult.h"

#include <QStringRef>
#include <QDebug>

using QxQTest::QTestResult;
using QxQTest::QTestXmlParser;

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

QTestXmlParser::QTestXmlParser(QIODevice* device)
    : QXmlStreamReader(device)
{
}

QTestXmlParser::~QTestXmlParser()
{
}

bool QTestXmlParser::isStartElement_(const char* elementName)
{
    return isStartElement() && (name() == elementName);
}

bool QTestXmlParser::isEndElement_(const char* elementName)
{
    return isEndElement() && (name() == elementName);
}

QTestResult QTestXmlParser::go()
{
    if (!device()->isOpen()) device()->open(QIODevice::ReadOnly);
    if (!device()->isReadable())
    {
        // do something
    }

    while (!atEnd() && m_result.isGood())
    {
        readNext();
        if (isStartElement_("TestFunction")) processTestFunction();
    }

    if (hasError())
    {
        qDebug() << "ERR: " << errorString() << " @ "
                 << lineNumber() << ":" << columnNumber();
    }
    return m_result;
}

void QTestXmlParser::processTestFunction()
{
    while (!atEnd())
    {
        readNext();
        if (isStartElement_("Incident")) fillResult();
        if (isEndElement_("TestFunction")) return;
    }
}

void QTestXmlParser::fillResult()
{
    QString type = attributes().value("type").toString();
    if (type == "pass") m_result.setState(QxRunner::RunSuccess);
    else if (type == "fail") setFailure();
}

void QTestXmlParser::setFailure()
{
    m_result.setState(QxRunner::RunWarning);
    m_result.setFile(QFileInfo(attributes().value("file").toString()));
    m_result.setLine(attributes().value("line").toString().toInt());

    while (!atEnd())
    {
        readNext();
        if (isCDATA()) m_result.setMessage(text().toString());
        if (isEndElement_("Description")) break;
    }
}
