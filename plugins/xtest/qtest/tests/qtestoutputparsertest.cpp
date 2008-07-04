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
#include <qtestcase.h>
#include <qtestcommand.h>
#include <qtest_kde.h>
#include <kasserts.h>
#include <QBuffer>
#include <QMetaType>
#include <QFileInfo>
#include <QVariant>
#include <QModelIndex>
#include <KDebug>
#include <QAbstractListModel>

using QTest::QTestOutputParser;
using QTest::QTestCase;
using QTest::QTestCommand;
using Veritas::Test;
using Veritas::TestResult;

using QTest::ut::QTestOutputParserTest;

Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(Veritas::TestState)
Q_DECLARE_METATYPE(QTest::QTestCase*)
Q_DECLARE_METATYPE(QModelIndex)

namespace
{

/*example xml output :
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
    "<TestCase name=\"TestTest\">\n"
    "<Environment>\n"
    "<QtVersion>4.4.0-rc1</QtVersion>\n"
    "<QTestVersion>4.4.0-rc1</QTestVersion>\n"
    "</Environment>\n");
const QByteArray footerXml("</TestCase>\n");

} // end anonymous namespace

#define XML_HEADER \
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"\
    "<TestCase name=\"TestTest\">\n"\
    "<Environment>\n"\
        "<QtVersion>4.4.0-rc1</QtVersion>\n"\
        "<QTestVersion>4.4.0-rc1</QTestVersion>\n"\
    "</Environment>\n"

#define XML_INIT_TESTCASE\
    "<TestFunction name=\"initTestCase\">\n"\
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"\
    "</TestFunction>\n"

#define XML_CLEANUP_TESTCASE\
    "<TestFunction name=\"cleanupTestCase\">\n"\
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"\
    "</TestFunction>\n"

#define FAILURE_MSG "some failure msg"
#define SPAM_MSG1 "some spam msg"
#define SPAM_MSG2 "some other spam msg"
#define SPAM_MSG3 "some more spam"

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
    m_caze = new QTestCase("TestTest", QFileInfo(), 0);
    m_caze->addChild(new QTestCommand("someCommand", m_caze));
}

void QTestOutputParserTest::cleanup()
{
    delete m_caze;
}

void QTestOutputParserTest::setupColumns()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<Veritas::TestState>("state");
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
    << input << Veritas::RunSuccess
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
    << input << Veritas::RunError
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
    int rowCount(const QModelIndex&) const
    {
        return 1;
    }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        Q_UNUSED(index);
        Q_UNUSED(role);
        return QVariant("owk");
    }
    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        QModelIndex i = createIndex(row, column, test);
        test->setIndex(i);
        return i;
    }
    Test* test;
};
}

// test command
void QTestOutputParserTest::parse()
{
    // exercise
    QFETCH(QByteArray, xml);
    QBuffer xmlOut(&xml, 0);
    QTestOutputParser parser(&xmlOut);
    QTestCommand* cmd = m_caze->child(0);
    QSignalSpy compSpy(cmd, SIGNAL(finished(QModelIndex)));
    QSignalSpy starSpy(cmd, SIGNAL(started(QModelIndex)));
    FakeModel fm;
    fm.test = m_caze->child(0);
    QModelIndex index = fm.index(0);
    parser.go(m_caze);

    // verify
    KOMPARE(1, starSpy.count());
    KOMPARE(1, compSpy.count());
    QModelIndex i1 = compSpy.takeFirst().value(0).value<QModelIndex>();
    QModelIndex i2 = starSpy.takeFirst().value(0).value<QModelIndex>();
    KOMPARE(index, i1);
    KOMPARE(index, i2);

    QFETCH(Veritas::TestState, state);
    TestResult* result = m_caze->child(0)->result();
    KOMPARE_MSG(state, result->state(),
                "Expected " + QString::number(state) +
                " got " + QString::number(result->state()));

    QFETCH(QFileInfo, file);
    KOMPARE(file.filePath(), result->file().filePath());

    QFETCH(int, line);
    KOMPARE(line, result->line());

    QFETCH(QString, message);
    KOMPARE_MSG(message, result->message(),
                "Expected " + message + " got " + result->message());
}

namespace QTest
{
template<> inline char* toString(const TestResult& res)
{
    return qstrdup((QString::number(res.state()) + ' ' +
                   res.message()).toLatin1().constData());
}
}

// test command
void QTestOutputParserTest::initFailure()
{
    QByteArray xml = headerXml
                     + testFunctionXml("initTestCase", failureIncidentXml())
                     + footerXml;
    QBuffer xmlOut(&xml, 0);
    QTestOutputParser parser(&xmlOut);
    QSignalSpy compSpy(m_caze, SIGNAL(finished(QModelIndex)));
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

    TestResult expected(Veritas::RunError, "some message", 100, QFileInfo("/path/to/file.cpp"));
    TestResult* result = m_caze->result();
    KOMPARE(expected, *result);
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
    QSignalSpy compSpy(m_caze, SIGNAL(finished(QModelIndex)));
    QSignalSpy starSpy(m_caze, SIGNAL(started(QModelIndex)));
    QTestCommand* cmd = m_caze->child(0);
    QSignalSpy compSpyCmd(cmd, SIGNAL(finished(QModelIndex)));
    QSignalSpy starSpyCmd(cmd, SIGNAL(started(QModelIndex)));

    FakeModel fm;
    fm.test = m_caze;
    QModelIndex caseIndex = fm.index(0);
    FakeModel fm2;
    fm2.test = m_caze->child(0);
    QModelIndex cmdIndex = fm2.index(0);

    parser.go(m_caze);

    // the testcommand should have been completed successfully
    assertCompleted(cmd, starSpyCmd, compSpyCmd);
    TestResult expected(Veritas::RunSuccess, "", 0, QFileInfo(""));
    assertResult(expected, *cmd->result());

    // the testcase should have failed
    assertCompleted(m_caze, starSpy, compSpy);
    TestResult expected2(Veritas::RunError, "some message", 100, QFileInfo("/path/to/file.cpp"));
    assertResult(expected2, *m_caze->result());
}

// test command
void QTestOutputParserTest::spammer()
{
    QByteArray xml =
        XML_HEADER
        XML_INIT_TESTCASE
        "<TestFunction name=\"someCommand\">\n"
            "<Message type=\"qdebug\" file=\"\" line=\"0\">\n"
                "<Description><![CDATA[" SPAM_MSG1 "]]></Description>\n"
            "</Message>\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
                "<Description><![CDATA[" FAILURE_MSG "]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        XML_CLEANUP_TESTCASE
        "</TestCase>\n";

    QTestCommand* cmd = initTestCmd(0);
    QBuffer xmlOut(&xml, 0);
    QTestOutputParser parser(&xmlOut);
    QSignalSpy completedSpy(cmd, SIGNAL(finished(QModelIndex)));
    QSignalSpy startSpy(cmd, SIGNAL(started(QModelIndex)));

    parser.go(m_caze);

    // verify
    assertCompleted(cmd, startSpy, completedSpy);
    TestResult expected(Veritas::RunError, FAILURE_MSG , 100, QFileInfo("/path/to/file.cpp"));
    assertResult(expected, *cmd->result());

    KOMPARE(1, cmd->result()->outputLineCount());
    KOMPARE(SPAM_MSG1, cmd->result()->outputLine(0));
}

void QTestOutputParserTest::spamMulti()
{
    QByteArray xml =
            XML_HEADER
            XML_INIT_TESTCASE
            "<TestFunction name=\"someCommand\">\n"
            "<Message type=\"qdebug\" file=\"\" line=\"0\">\n"
                "<Description><![CDATA[" SPAM_MSG1 "]]></Description>\n"
            "</Message>\n"
            "<Message type=\"qdebug\" file=\"\" line=\"0\">\n"
                "<Description><![CDATA[" SPAM_MSG2 "]]></Description>\n"
            "</Message>\n"
            "<Message type=\"qdebug\" file=\"\" line=\"0\">\n"
                "<Description><![CDATA[" SPAM_MSG3 "]]></Description>\n"
            "</Message>\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
                "<Description><![CDATA[" FAILURE_MSG "]]></Description>\n"
            "</Incident>\n"
            "</TestFunction>\n"
            XML_CLEANUP_TESTCASE
            "</TestCase>\n";

    QTestCommand* cmd = initTestCmd(0);
    QBuffer xmlOut(&xml, 0);
    QTestOutputParser parser(&xmlOut);
    QSignalSpy completedSpy(cmd, SIGNAL(finished(QModelIndex)));
    QSignalSpy startSpy(cmd, SIGNAL(started(QModelIndex)));

    parser.go(m_caze);

    // verify
    assertCompleted(cmd, startSpy, completedSpy);
    TestResult expected(Veritas::RunError, FAILURE_MSG , 100, QFileInfo("/path/to/file.cpp"));
    assertResult(expected, *cmd->result());

    KOMPARE(3, cmd->result()->outputLineCount());
    KOMPARE(SPAM_MSG1, cmd->result()->outputLine(0));
    KOMPARE(SPAM_MSG2, cmd->result()->outputLine(1));
    KOMPARE(SPAM_MSG3, cmd->result()->outputLine(2));
}


// helper
QTestCommand* QTestOutputParserTest::initTestCmd(int i)
{
    QTestCommand *cmd = m_caze->child(i);
    FakeModel fm;
    fm.test = cmd;
    cmd->setIndex(fm.index(0));
    return cmd;
}

// helper
void QTestOutputParserTest::assertCompleted(Test* test, QSignalSpy& started, QSignalSpy& completed)
{
    KOMPARE(1, started.count());
    KOMPARE(1, completed.count());
    QModelIndex i1 = completed.takeFirst().value(0).value<QModelIndex>();
    QModelIndex i2 = started.takeFirst().value(0).value<QModelIndex>();
    KOMPARE(test, static_cast<Test*>(i1.internalPointer()));
    KOMPARE(test, static_cast<Test*>(i2.internalPointer()));
}

// helper
void QTestOutputParserTest::assertResult(const TestResult& expected, const TestResult& actual)
{
    KOMPARE_MSG(expected.state(), actual.state(),
                "Expected " + QString::number(expected.state()) +
                " got " + QString::number(actual.state()));
    KOMPARE(expected.file().filePath(), actual.file().filePath());
    KOMPARE(expected.line(), actual.line());
    KOMPARE_MSG(expected.message(), actual.message(),
                "Expected " + expected.message() + " got " + actual.message());
}

QTEST_KDEMAIN(QTestOutputParserTest, NoGUI)
