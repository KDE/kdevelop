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

#include "qtestoutputparsertest.h"
#include <qtestoutputparser.h>
#include <qtestresult.h>
#include <qtestcase.h>
#include <qtestcommand.h>
#include <qtest_kde.h>
#include <kasserts.h>
#include <QBuffer>
#include <QMetaType>
#include <QFileInfo>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractListModel>

using QxQTest::QTestOutputParser;
using QxQTest::QTestResult;
using QxQTest::QTestCase;
using QxQTest::QTestBase;
using QxQTest::QTestCommand;

using QxQTest::ut::QTestOutputParserTest;

Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(QxRunner::RunnerResult)
Q_DECLARE_METATYPE(QxQTest::QTestCase*)
Q_DECLARE_METATYPE(QModelIndex)

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

void QTestOutputParserTest::parse_data()
{
    setupColumns();
    addSunnyDayData();
    addBasicFailureData();
}

void QTestOutputParserTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

void QTestOutputParserTest::init()
{
    m_caze = new QTestCase("RunnerItemTest", QFileInfo(), 0);
    m_caze->addTest(new QTestCommand("someCommand", m_caze));
}

void QTestOutputParserTest::cleanup()
{
    delete m_caze;
}

void QTestOutputParserTest::setupColumns()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QxRunner::RunnerResult>("state");
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<int>("line");
    QTest::addColumn<QString>("message");
    QTest::addColumn<QTestCase*>("case");
}

QByteArray failureIncidentXml()
{
    return incidentXml("fail", "/path/to/file.cpp", "100", "some message");
}

// test data
void QTestOutputParserTest::addSunnyDayData()
{
    // first row - sunny day test succes
    QByteArray input = headerXml + initTestCaseXml
                       + functionXml
                       + cleanupTestCaseXml
                       + footerXml;
    QTest::newRow("sunny day test succes")
    << input << QxRunner::RunSuccess
    << QFileInfo("") << 0 << "" << m_caze;
}

// test data
void QTestOutputParserTest::addBasicFailureData()
{
    // second row - test failure
    QByteArray input = headerXml
                       + initTestCaseXml
                       + testFunctionXml("someCommand", failureIncidentXml())
                       + cleanupTestCaseXml
                       + footerXml;
    QTest::newRow("basic failure")
    << input << QxRunner::RunError
    << QFileInfo("/path/to/file.cpp") << 100
    << "some message" << m_caze;
}

#include <QSignalSpy>

namespace
{
// sole purpose of this is to get a decent QModelIndex ...
class FakeModel : public QAbstractListModel
{
public:
    int rowCount(const QModelIndex&) const {
        return 1;
    }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
        return QVariant("owk");
    }
    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const {
        QModelIndex i = createIndex(row, column, test);
        test->setIndex(i);
        return i;
    }
    QTestBase* test;
};
}

// test command
void QTestOutputParserTest::parse()
{
    // exercise
    QFETCH(QByteArray, xml);
    QBuffer xmlOut(&xml, 0);
    QTestOutputParser parser(&xmlOut);
    QSignalSpy compSpy(m_caze, SIGNAL(completed(QModelIndex)));
    QSignalSpy starSpy(m_caze, SIGNAL(started(QModelIndex)));
    FakeModel fm;
    fm.test = m_caze->testAt(0);
    QModelIndex index = fm.index(0);
    parser.go(m_caze);

    // verify
    KOMPARE(1, starSpy.count());
    KOMPARE(1, compSpy.count());
    QModelIndex i1 = compSpy.takeFirst().value(0).value<QModelIndex>();
    QModelIndex i2 = starSpy.takeFirst().value(0).value<QModelIndex>();
    KOMPARE(index, i1);
    KOMPARE(index, i2);

    QFETCH(QxRunner::RunnerResult, state);
    QTestResult result = m_caze->testAt(0)->result_();
    KOMPARE_MSG(state, result.state(),
                "Expected " + QString::number(state) +
                " got " + QString::number(result.state()));

    QFETCH(QFileInfo, file);
    KOMPARE(file.filePath(), result.file().filePath());

    QFETCH(int, line);
    KOMPARE(line, result.line());

    QFETCH(QString, message);
    KOMPARE_MSG(message, result.message(),
                "Expected " + message + " got " + result.message());
}

// test command
void QTestOutputParserTest::initFailure()
{
    QByteArray xml = headerXml
            + testFunctionXml("initTestCase", failureIncidentXml())
            + footerXml;
    QBuffer xmlOut(&xml, 0);
    QTestOutputParser parser(&xmlOut);
    QSignalSpy compSpy(m_caze, SIGNAL(completed(QModelIndex)));
    QSignalSpy starSpy(m_caze, SIGNAL(started(QModelIndex)));
    FakeModel fm;
    fm.test = m_caze;
    QModelIndex index = fm.index(0);
    parser.go(m_caze);

    // verify
    KOMPARE(1, starSpy.count());
    KOMPARE(1, compSpy.count());
    QModelIndex i1 = compSpy.takeFirst().value(0).value<QModelIndex>();
    QModelIndex i2 = starSpy.takeFirst().value(0).value<QModelIndex>();
    KOMPARE(index, i1);
    KOMPARE(index, i2);

    QTestResult expected(QxRunner::RunError, "some message", 100, QFileInfo("/path/to/file.cpp"));
    QTestResult result = m_caze->result_();
    KOMPARE(expected, result);
}

// test command
void QTestOutputParserTest::cleanupFailure()
{
    QByteArray xml = headerXml
            + initTestCaseXml + functionXml
            + testFunctionXml("cleanupTestCase", failureIncidentXml())
            + footerXml;
    QBuffer xmlOut(&xml, 0);
    QTestOutputParser parser(&xmlOut);
    QSignalSpy compSpy(m_caze, SIGNAL(completed(QModelIndex)));
    QSignalSpy starSpy(m_caze, SIGNAL(started(QModelIndex)));
    FakeModel fm;
    fm.test = m_caze;
    QModelIndex caseIndex = fm.index(0);
    FakeModel fm2;
    fm2.test = m_caze->testAt(0);
    QModelIndex cmdIndex = fm2.index(0);

    parser.go(m_caze);

    // verify
    KOMPARE(2, starSpy.count());
    KOMPARE(2, compSpy.count());

    // the testcommand should have been completed succesfully
    QModelIndex i1 = compSpy.takeFirst().value(0).value<QModelIndex>();
    QModelIndex i2 = starSpy.takeFirst().value(0).value<QModelIndex>();
    KOMPARE(cmdIndex, i1);
    KOMPARE(cmdIndex, i2);

    QTestResult expected(QxRunner::RunSuccess, "", 0, QFileInfo(""));
    QTestResult result = m_caze->testAt(0)->result_();
    KOMPARE(expected, result);

    // the testcase should have failed
    i1 = compSpy.takeFirst().value(0).value<QModelIndex>();
    i2 = starSpy.takeFirst().value(0).value<QModelIndex>();
    KOMPARE(caseIndex, i1);
    KOMPARE(caseIndex, i2);

    expected = QTestResult(QxRunner::RunError, "some message", 100, QFileInfo("/path/to/file.cpp"));
    result = m_caze->result_();
    KOMPARE(expected, result);
}

QTEST_KDEMAIN(QTestOutputParserTest, NoGUI)
