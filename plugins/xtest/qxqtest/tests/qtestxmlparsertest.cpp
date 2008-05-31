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

#include <qtestxmlparser.h>
#include <qtestresult.h>

#include "qtestxmlparsertest.h"

#include <qtest_kde.h>
#include <kasserts.h>

#include <QBuffer>
#include <QMetaType>
#include <QFileInfo>

using QxQTest::QTestXmlParser;
using QxQTest::QTestResult;

Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(QxRunner::RunnerResult)

namespace
{

/*example xml output :
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

QByteArray testFunctionXml(QByteArray name, QByteArray incident)
{
    return QByteArray(
        "<TestFunction name=\"" + name + "\">\n"
        + incident +
        "\n</TestFunction>\n");
}

QByteArray incidentXml(QByteArray type, QByteArray file, QByteArray line, QByteArray msg)
{
    return QByteArray(
        "<Incident type=\"" + type + "\" file=\"" + file + "\" line=\"" + line + "\">\n"
            "<Description><![CDATA[" + msg + "]]></Description>\n"
        "</Incident>\n");
}

QByteArray successIncidentXml()
{
    return QByteArray("<Incident type=\"pass\" file=\"\" line=\"0\" />\n");
}

QByteArray initTestCaseXml = testFunctionXml("initTestCase", successIncidentXml());
QByteArray cleanupTestCaseXml = testFunctionXml("cleanupTestCase", successIncidentXml());
QByteArray functionXml = testFunctionXml("someCommand", successIncidentXml());

const QByteArray headerXml(
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<TestCase name=\"RunnerItemTest\">\n"
        "<Environment>\n"
            "<QtVersion>4.4.0-rc1</QtVersion>\n"
            "<QTestVersion>4.4.0-rc1</QTestVersion>\n"
        "</Environment>\n");
const QByteArray footerXml("</TestCase>\n");

} // end anonymous namespace

void QTestXmlParserTest::parse_data()
{
    setupColumns();
    addSunnyDayData();
    addBasicFailureData();
    addInitFailureData();
    addCleanupFailureData();
}

void QTestXmlParserTest::setupColumns()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QxRunner::RunnerResult>("state");
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<int>("line");
    QTest::addColumn<QString>("message");
}

QByteArray failureIncidentXml()
{
    return incidentXml("fail", "/path/to/file.cpp", "100", "some message");
}

void QTestXmlParserTest::constructFailureRow(QString name, QByteArray xml)
{
    QTest::newRow(name.toAscii())
            << xml << QxRunner::RunWarning 
            << QFileInfo("/path/to/file.cpp") << 100 
            << "some message";
}

// test data
void QTestXmlParserTest::addSunnyDayData()
{
    // first row - sunny day test succes
    QByteArray input = headerXml + initTestCaseXml
                                 + functionXml 
                                 + cleanupTestCaseXml
                                 + footerXml;
    QTest::newRow("sunny day test succes")
            << input << QxRunner::RunSuccess 
            << QFileInfo("") << -1 << "";
}

// test data
void QTestXmlParserTest::addBasicFailureData()
{
    // second row - test failure
    QByteArray input = headerXml 
                     + initTestCaseXml
                     + testFunctionXml("somename", failureIncidentXml())
                     + cleanupTestCaseXml
                     + footerXml;
    constructFailureRow("basic failure", input);
}

// test data
void QTestXmlParserTest::addInitFailureData()
{
    // third row - failure in initTestCase
    QByteArray input = headerXml
                    + testFunctionXml("initTestCase", failureIncidentXml())
                    + footerXml;
    constructFailureRow("initTestCase failure", input);
}

// test data
void QTestXmlParserTest::addCleanupFailureData()
{
    // fourth row - failure in initTestCase
    QByteArray input = headerXml 
                     + initTestCaseXml + functionXml 
                     + testFunctionXml("cleanupTestCase", failureIncidentXml())
                     + footerXml;
    constructFailureRow("cleanupTestCase failure", input);
}

// test command
void QTestXmlParserTest::parse()
{
    // exercise
    QFETCH(QByteArray, xml);
    QBuffer xmlOut(&xml, 0);
    QTestXmlParser parser(&xmlOut);
    QTestResult result = parser.go();

    // verify
    QFETCH(QxRunner::RunnerResult, state);
    KOMPARE_MSG(state, result.state(), 
                "Expected " + QString::number(state) + 
                " got " + QString::number(result.state()));

    QFETCH(QFileInfo, file);
    KOMPARE(file, result.file());

    QFETCH(int, line);
    KOMPARE(line, result.line());

    QFETCH(QString, message);
    KOMPARE_MSG(message, result.message(), 
                "Expected " + message + " got " + result.message());
}

QTEST_KDEMAIN( QTestXmlParserTest, NoGUI )
