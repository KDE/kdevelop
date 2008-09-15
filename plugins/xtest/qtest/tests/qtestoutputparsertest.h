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

#ifndef QXQTEST_QTESTOUTPUTPARSERTEST
#define QXQTEST_QTESTOUTPUTPARSERTEST

#include <QtTest/QtTest>
#include <veritas/testresult.h>
#include <veritas/test.h>

class QSignalSpy;

namespace QTest {
class QTestCase;
class QTestCommand;
class QTestOutputParser;
namespace Test {

// TODO this test is far from sufficient.
//      need to add some slow-updating XML chunks
//      to test the recover stuff. maybe with a random test.
/*! @unitundertest QTest::QTestOutputParser */
class QTestOutputParserTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void parse_data();
    void parse();
    void initFailure();
    void cleanupFailure();
    void doubleFailure();
    void tdd_skipSingle();
    void tdd_skipAll();

private:
    struct TestInfo;

    // test data helpers
    void setupColumns();
    void addSunnyDayData();
    void addBasicFailureData();

    // custom assertions
    void assertParsed(TestInfo& testInfo);
    void assertResult(const Veritas::TestResult& expected, const Veritas::TestResult& actual);
    void assertResult(Veritas::TestResult* expected, Veritas::TestResult* actual);
    void checkResult(TestInfo& testInfo);

    // creation methods
    QTestCase* createTestCase(TestInfo&);
    void createTestCommand(TestInfo&, QTestCase* parent, QString name);

    // setup helpers
    void initParser(QByteArray& xml, QTestCase* caze);
    void setExpectedSuccess(TestInfo& tInfo);
    void setExpectedFailure(TestInfo& tInfo);
    void setExpectedResult(TestInfo& tInfo, Veritas::TestState state,
                           QString filepath, int lineNumber, QString msg);

private:
    QTestOutputParser* m_parser;
    QTestCase* m_caze;

    struct TestInfo
    {
        Veritas::Test* test;
        QSignalSpy* started;
        QSignalSpy* finished;
        Veritas::TestResult* result;
    };

    TestInfo m_cazeInfo;
    TestInfo m_command1Info;
    TestInfo m_command2Info;
};

}}

#endif // QXQTEST_QTESTOUTPUTPARSERTEST
